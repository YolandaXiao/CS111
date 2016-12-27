#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

/////////////////////////////
//initiailization
/////////////////////////////

struct super_block{
  unsigned int s_magic;                 //1
  unsigned int s_inodes_count;          //2
  unsigned int s_blocks_count;          //3
  unsigned int s_log_block_size;        //4calculation
  unsigned int s_log_frag_size;         //5calculation
  unsigned int s_blocks_per_group;      //6
  unsigned int s_inodes_per_group;      //7
  unsigned int s_frags_per_group;       //8
  unsigned int s_first_data_block;      //9
} s_block;

struct group_descriptor{
  unsigned int block_count;
  unsigned int bg_free_blocks_count;
  unsigned int bg_free_inodes_count;
  unsigned int bg_used_dirs_count; //directory count
  unsigned int bg_inode_bitmap; //bg_inode_bitmap ? free_inode_bitmap_block
  unsigned int bg_block_bitmap; //bg_block_bitmap ? free_block_bitmap_block
  unsigned int bg_inode_table;
};

struct indirect_dir_entry{
  unsigned int block_num_of_containing_block;
  unsigned int entry_num;
  unsigned int block_ptr_val;
} i_dir_entry;

/////////////////////////////
//data calculation
/////////////////////////////

//put data into superblock-----------------------
void superblock(int fs){
  unsigned int magic_num=0;
  pread(fs,(void *)&magic_num,2,1024+56);
  s_block.s_magic = magic_num;

  unsigned int inodes_count=0;
  pread(fs,(void *)&inodes_count,4,1024+0);
  s_block.s_inodes_count = inodes_count;

  unsigned int blocks_count=0;
  pread(fs,(void *)&blocks_count,4,1024+4);
  s_block.s_blocks_count = blocks_count;

  unsigned int log_block_size=0;
  pread(fs,(void *)&log_block_size,4,1024+24);
  s_block.s_log_block_size = 1024 << log_block_size;

  unsigned int log_frag_size=0;
  pread(fs,(void *)&log_frag_size,4,1024+28);
  s_block.s_log_frag_size = 1024 << log_frag_size;

  unsigned int blocks_per_group=0;
  pread(fs,(void *)&blocks_per_group,4,1024+32);
  s_block.s_blocks_per_group = blocks_per_group;

  unsigned int frags_per_group=0;
  pread(fs,(void *)&frags_per_group,4,1024+36);
  s_block.s_frags_per_group = frags_per_group;

  unsigned int inodes_per_group=0;
  pread(fs,(void *)&inodes_per_group,4,1024+40);
  s_block.s_inodes_per_group = inodes_per_group;

  unsigned int first_data_block=0;
  pread(fs,(void *)&first_data_block,4,1024+20);
  s_block.s_first_data_block = first_data_block;
}

//group descriptor--------------------------------
struct group_descriptor groupdescriptor(int fs, int x){

  struct group_descriptor gd;

  unsigned int blocks_count=0;
  pread(fs,(void *)&blocks_count,4,1024+32);
  gd.block_count = blocks_count;

  unsigned int free_blocks_count=0;
  pread(fs,(void *)&free_blocks_count,2,2048+12+x);
  gd.bg_free_blocks_count = free_blocks_count;

  unsigned int free_inodes_count=0;
  pread(fs,(void *)&free_inodes_count,2,2048+14+x);
  gd.bg_free_inodes_count = free_inodes_count;

  unsigned int used_dirs_count=0;
  pread(fs,(void *)&used_dirs_count,2,2048+16+x);
  gd.bg_used_dirs_count = used_dirs_count;

  unsigned int inode_bitmap=0;
  pread(fs,(void *)&inode_bitmap,4,2048+4+x);
  gd.bg_inode_bitmap = inode_bitmap;

  unsigned int block_bitmap=0;
  pread(fs,(void *)&block_bitmap,4,2048+0+x);
  gd.bg_block_bitmap = block_bitmap;

  unsigned int inode_table=0;
  pread(fs,(void *)&inode_table,4,2048+8+x);
  gd.bg_inode_table = inode_table;

  return gd;
}

//main---------------------------------
int main(int argc, char *argv[]){

    //openfile
    int fs = open(argv[1], O_RDONLY);
    if (fs < 0){
      fprintf(stderr,"There's an input error.");
      exit(1);
    }
  
    //superblock-----------------------------------
    superblock(fs);
    int fd_sb = creat("super.csv", 0666);
    dprintf(fd_sb, "%x,%d,%d,%d,%d,%d,%d,%d,%d\n", s_block.s_magic, s_block.s_inodes_count, 
                                           s_block.s_blocks_count, s_block.s_log_block_size, 
                                           s_block.s_log_frag_size, s_block.s_blocks_per_group, 
                                           s_block.s_inodes_per_group, s_block.s_frags_per_group, 
                                           s_block.s_first_data_block);

    //group descriptor-----------------------------------
    int fd_gd = creat("group.csv", 0666);
    int group_num = s_block.s_blocks_count/s_block.s_blocks_per_group;
    struct group_descriptor g_descriptor[group_num];
    for(int i = 0; i < group_num; i++)
    {
      g_descriptor[i] =  groupdescriptor(fs, i * 32);
      dprintf(fd_gd, "%d,%d,%d,%d,%x,%x,%x\n", g_descriptor[i].block_count, g_descriptor[i].bg_free_blocks_count,
                                               g_descriptor[i].bg_free_inodes_count, g_descriptor[i].bg_used_dirs_count,
                                               g_descriptor[i].bg_inode_bitmap, g_descriptor[i].bg_block_bitmap,
                                               g_descriptor[i].bg_inode_table);
    }

    //find free bitmap entry -----------------------------------
    FILE *fd_fbe;                                                                          
    fd_fbe = fopen("bitmap.csv", "w");
    unsigned int mask = 1;
    unsigned int block_bitmap_entry=0;
    unsigned int inode_bitmap_entry=0;
    for(int i = 0; i < group_num; i++)
    {
      //for blocks
      int block_num = g_descriptor[i].bg_block_bitmap;
      int block_entry = block_num*s_block.s_log_block_size;
      for(int j = i * s_block.s_blocks_per_group; j < (i+1) * s_block.s_blocks_per_group; j+=8)
      {                                
        pread(fs,(void *)&block_bitmap_entry,1,block_entry++);
        for(int k = 0; k < 8; k++)
        {
          if((block_bitmap_entry & mask)==0)
          {
            int entry_num = j + k + 1;
            fprintf(fd_fbe, "%x,%d\n", block_num , entry_num);
          }
          block_bitmap_entry = block_bitmap_entry >> 1;
        }
      }

      //for inodes
      int inode_num = g_descriptor[i].bg_inode_bitmap;
      int inode_entry = inode_num*s_block.s_log_block_size;
      for(int j = i * s_block.s_inodes_per_group; j < (i+1) * s_block.s_inodes_per_group; j+=8)
      {                                  
        pread(fs,(void *)&inode_bitmap_entry,1,inode_entry++);
        for(int k = 0; k < 8; k++)
        {
          if((inode_bitmap_entry & mask)==0)
          {
            int entry_num = j + k + 1;
            fprintf(fd_fbe, "%x,%d\n", inode_num , entry_num);
          }
          inode_bitmap_entry = inode_bitmap_entry >> 1;
        }
      }
    }
    fclose(fd_fbe);


    //find inode -------------------------------------------------------
    FILE *fd_i;                                                                          
    fd_i = fopen("inode.csv", "w");
    unsigned int inode_bitmap_entry_1=0;
    unsigned int first_xbits_in_inode_map = 1;
    for(int i = 0; i < group_num; i++)
    {
      //for inodes
      int inode_num = g_descriptor[i].bg_inode_bitmap;
      int inode_entry = inode_num*s_block.s_log_block_size;
      int inode_table_entry_num = g_descriptor[i].bg_inode_table * s_block.s_log_block_size;
      for(int j = i * s_block.s_inodes_per_group; j < (i+1) * s_block.s_inodes_per_group; j+=8)
      {                                 
        pread(fs,(void *)&inode_bitmap_entry_1,1,inode_entry++);
        for(int k = 0; k < 8; k++)
        {
          if((inode_bitmap_entry_1 & mask)!=0) //if the bit is allocated, we read from the inode table
          {
            int inode_table_entry = inode_table_entry_num + (j-i*s_block.s_inodes_per_group + k)*128 ;

            //file type
            unsigned int imode = 0;
            pread(fs,(void *)&imode,2,inode_table_entry);
            char file_type = '?';
            if(imode & 0x8000) 
              file_type = 'f';
            else if (imode & 0xA000) 
              file_type = 's';
            else if (imode & 0x4000) 
              file_type = 'd';

            //i_uid
            unsigned int iuid = 0;
            pread(fs,(void *)&iuid,2,inode_table_entry+2);
            unsigned int iuid_2 = 0;
            pread(fs,(void *)&iuid_2,2,inode_table_entry+116+4);
            iuid += (iuid_2 << 16);

            //i_gid
            unsigned int igid = 0;
            pread(fs,(void *)&igid,2,inode_table_entry+24);
            unsigned int igid_2 = 0;
            pread(fs,(void *)&igid_2,2,inode_table_entry+116+6);
            igid += (igid_2 << 16);

            //i_links_count
            unsigned int linkscount = 0;
            pread(fs,(void *)&linkscount,2,inode_table_entry+26);

            //time
            unsigned int ictime = 0;
            pread(fs,(void *)&ictime,4,inode_table_entry+12);
            unsigned int imtime = 0;
            pread(fs,(void *)&imtime,4,inode_table_entry+16);
            unsigned int iatime = 0;
            pread(fs,(void *)&iatime,4,inode_table_entry+8);

            //last two
            unsigned int size = 0;
            pread(fs,(void *)&size,4,inode_table_entry+4);
            unsigned int blocks = 0;
            pread(fs,(void *)&blocks,4,inode_table_entry+28);
            blocks = blocks/(2<<s_block.s_log_block_size);


            //print
            fprintf(fd_i, "%d,%c,%o,%d,%d,%d,%x,%x,%x,%d,%d", first_xbits_in_inode_map , file_type,
                                                                imode, iuid,
                                                                igid, linkscount,
                                                                ictime, imtime, iatime,
                                                                size, blocks);

            unsigned int blockpointer = 0;
            for(int x = 0; x < 14; x++)
            {
              pread(fs,(void *)&blockpointer,4,inode_table_entry+40+4*x);
              fprintf(fd_i, ",%x", blockpointer);
            }
            pread(fs,(void *)&blockpointer,4,inode_table_entry+40+4*14);
            fprintf(fd_i, ",%x\n", blockpointer);
          }
          first_xbits_in_inode_map++;
          inode_bitmap_entry_1 = inode_bitmap_entry_1 >> 1;
        }
      }
    }
    fclose(fd_i);

    //print directories-----------------------------------
    FILE *fd_d;                                                                          
    fd_d = fopen("directory.csv", "w");
    unsigned int inode_bitmap_entry_2=0;
    unsigned int first_xbits_in_inode_map_1 = 1;
    //unsigned int entry_num = 0;
    for(int i = 0; i < group_num; i++)
    {
      //for inodes
      int inode_num = g_descriptor[i].bg_inode_bitmap;
      int inode_entry = inode_num*s_block.s_log_block_size;
      int inode_table_entry_num = g_descriptor[i].bg_inode_table * s_block.s_log_block_size;
      for(int j = i * s_block.s_inodes_per_group; j < (i+1) * s_block.s_inodes_per_group; j+=8)
      {                                 
        pread(fs,(void *)&inode_bitmap_entry_2,1,inode_entry++);
        for(int k = 0; k < 8; k++)
        {
          if((inode_bitmap_entry_2 & mask)!=0) //if the bit is allocated, we read from the inode table
          {
            int inode_table_entry = inode_table_entry_num + (j-i*s_block.s_inodes_per_group + k)*128 ;

            //if the file is directory
            unsigned int imode = 0;
            pread(fs,(void *)&imode,2,inode_table_entry);
            if (imode & 0x4000) 
            {
              unsigned int blockpointer = 0;
              unsigned int entry_num = 0;
              for(int x = 0; x < 12; x++)
              {
                pread(fs,(void *)&blockpointer,4,inode_table_entry+40+4*x);

                if(blockpointer!=0)
                {
                  unsigned int y = s_block.s_log_block_size*blockpointer;
                  unsigned int ymax = s_block.s_log_block_size + y;
                  do{
                    unsigned int rec_len = 0;
                    pread(fs,(void *)&rec_len,2,y+4);
                    unsigned int name_len = 0;
                    pread(fs,(void *)&name_len,1,y+6);
                    unsigned int inode_num_of_file = 0;
                    pread(fs,(void *)&inode_num_of_file,4,y+0);
                    char name[257];
                    memset(name,0,257);
                    pread(fs,(void *)&name,name_len,y+8);
                    
                    //print
                    if(rec_len != 0 && name_len != 0 && inode_num_of_file!=0)
                    {
                      fprintf(fd_d, "%d,%d,%d,%d,%d,\"%s\"\n", first_xbits_in_inode_map_1 , entry_num,
                                                           rec_len, name_len,
                                                           inode_num_of_file, name);
                    }
                    entry_num++;
                    y+=rec_len;
                  }while(y < ymax);
                }
              }

              //if there are indirect blocks
              unsigned int block_entry_num = 0;
              pread(fs,(void *)&blockpointer,4,inode_table_entry+40+4*12);
              if(blockpointer!=0)
              {
                for(; block_entry_num < s_block.s_log_block_size/4; block_entry_num++)
                {
                  unsigned int block_ptr_val = 0;
                  pread(fs,(void *)&block_ptr_val,4,blockpointer*s_block.s_log_block_size+block_entry_num*4);
                  if(block_ptr_val!=0)
                  {
                    unsigned int y = s_block.s_log_block_size*block_ptr_val;
                    unsigned int ymax = s_block.s_log_block_size + y;
                    do{
                      unsigned int rec_len = 0;
                      pread(fs,(void *)&rec_len,2,y+4);
                      unsigned int name_len = 0;
                      pread(fs,(void *)&name_len,1,y+6);
                      unsigned int inode_num_of_file = 0;
                      pread(fs,(void *)&inode_num_of_file,4,y+0);
                      char name[257];
                      memset(name,0,257);
                      pread(fs,(void *)&name,name_len,y+8);
                      
                      //print
                      if(rec_len != 0 && name_len != 0 && inode_num_of_file!=0)
                      {
                        fprintf(fd_d, "%d,%d,%d,%d,%d,\"%s\"\n", first_xbits_in_inode_map_1 , entry_num,
                                                             rec_len, name_len,
                                                             inode_num_of_file, name);
                      }
                      entry_num++;
                      y+=rec_len;
                    }while(y < ymax);
                  }
                }
              }

            }
          }
          first_xbits_in_inode_map_1++;
          inode_bitmap_entry_2 = inode_bitmap_entry_2 >> 1;
        }
      }
    }
    fclose(fd_d);



    //indirect directories-----------------------------------
    FILE *fd_id;                                                                          
    fd_id = fopen("indirect.csv", "w");
    unsigned int inode_bitmap_entry_3=0;
    unsigned int first_xbits_in_inode_map_2 = 1;
    //unsigned int entry_num = 0;
    for(int i = 0; i < group_num; i++)
    {
      //for inodes
      int inode_num = g_descriptor[i].bg_inode_bitmap;
      int inode_entry = inode_num*s_block.s_log_block_size;
      int inode_table_entry_num = g_descriptor[i].bg_inode_table * s_block.s_log_block_size;
      for(int j = i * s_block.s_inodes_per_group; j < (i+1) * s_block.s_inodes_per_group; j+=8)
      {                                 
        pread(fs,(void *)&inode_bitmap_entry_3,1,inode_entry++);
        for(int k = 0; k < 8; k++)
        {
          if((inode_bitmap_entry_3 & mask)!=0) //if the bit is allocated, we read from the inode table
          {
            //if the file is directory
            int inode_table_entry = inode_table_entry_num + (j-i*s_block.s_inodes_per_group + k)*128 ;
            unsigned int imode = 0;
            pread(fs,(void *)&imode,2,inode_table_entry);
            if (imode & 0x4000) //make sure its a directory
            {
              unsigned int blocks = 0;
              pread(fs,(void *)&blocks,4,inode_table_entry+28);
              blocks = blocks/(2<<s_block.s_log_block_size);

              //if there are indirect blocks
              if(blocks>11)
              {
                unsigned int block_entry_num = 0;

                //12-indirect directory
                unsigned int blockpointer = 0;
                pread(fs,(void *)&blockpointer,4,inode_table_entry+40+4*12);
                if(blockpointer!=0)
                {
                  for(; block_entry_num < s_block.s_log_block_size/4; block_entry_num++)
                  {
                    unsigned int block_ptr_val = 0;
                    pread(fs,(void *)&block_ptr_val,4,blockpointer*s_block.s_log_block_size+block_entry_num*4);
                    if(block_ptr_val!=0)
                    {
                      fprintf(fd_id, "%x,%d,%x\n", blockpointer, block_entry_num, block_ptr_val);
                    }
                  }
                }

                //13-doubly indirect directory
                pread(fs,(void *)&blockpointer,4,inode_table_entry+40+4*13);
                if(blockpointer!=0)
                {
                  //same as above
                  for(; block_entry_num < s_block.s_log_block_size/4; block_entry_num++)
                  {
                    unsigned int block_ptr_val = 0;
                    pread(fs,(void *)&block_ptr_val,4,blockpointer*s_block.s_log_block_size+block_entry_num*4);
                    if(block_ptr_val!=0)
                    {
                      fprintf(fd_id, "%x,%d,%x\n", blockpointer, block_entry_num, block_ptr_val);
                    }
                  }
                  //deal with the double part
                  for(; block_entry_num < s_block.s_log_block_size/4; block_entry_num++)
                  {
                    unsigned int block_entry_num_1 = 0;

                    unsigned int block_ptr_val = 0;
                    pread(fs,(void *)&block_ptr_val,4,blockpointer*s_block.s_log_block_size+block_entry_num_1*4);
                    if(block_ptr_val!=0)
                    {
                      for(; block_entry_num_1 < s_block.s_log_block_size/4; block_entry_num_1++)
                      {
                        unsigned int block_ptr_val_1 = 0;
                        pread(fs,(void *)&block_ptr_val_1,4,block_ptr_val*s_block.s_log_block_size+block_entry_num_1*4);
                        if(block_ptr_val_1!=0)
                        {
                          fprintf(fd_id, "%x,%d,%x\n", block_ptr_val, block_entry_num_1, block_ptr_val_1);
                        }
                      }
                    }
                  }
                  /*end of double part*/
                }

                //14-triply indirect directory
                pread(fs,(void *)&blockpointer,4,inode_table_entry+40+4*14);
                if(blockpointer!=0)
                {
                  //outer most -1
                  for(; block_entry_num < s_block.s_log_block_size/4; block_entry_num++)
                  {
                    unsigned int block_ptr_val = 0;
                    pread(fs,(void *)&block_ptr_val,4,blockpointer*s_block.s_log_block_size+block_entry_num*4);
                    if(block_ptr_val!=0)
                    {
                      fprintf(fd_id, "%x,%d,%x\n", blockpointer, block_entry_num, block_ptr_val);
                    }
                  }
                  //outer most -2
                  for(; block_entry_num < s_block.s_log_block_size/4; block_entry_num++)
                  {
                    unsigned int block_entry_num_1 = 0;

                    unsigned int block_ptr_val = 0;
                    pread(fs,(void *)&block_ptr_val,4,blockpointer*s_block.s_log_block_size+block_entry_num_1*4);
                    if(block_ptr_val!=0)
                    {
                      //middle -1
                      for(; block_entry_num_1 < s_block.s_log_block_size/4; block_entry_num_1++)
                      {
                        unsigned int block_ptr_val_1 = 0;
                        pread(fs,(void *)&block_ptr_val_1,4,blockpointer*s_block.s_log_block_size+block_entry_num_1*4);
                        if(block_ptr_val_1!=0)
                        {
                          fprintf(fd_id, "%x,%d,%x\n", blockpointer, block_entry_num_1, block_ptr_val_1);
                        }
                      }
                      //middle -2
                      for(; block_entry_num < s_block.s_log_block_size/4; block_entry_num++)
                      {
                        unsigned int block_entry_num_2 = 0;

                        unsigned int block_ptr_val_2 = 0;
                        pread(fs,(void *)&block_ptr_val_2,4,blockpointer*s_block.s_log_block_size+block_entry_num_2*4);
                        if(block_ptr_val_2!=0)
                        {
                          //inner
                          for(; block_entry_num_2 < s_block.s_log_block_size/4; block_entry_num_2++)
                          {
                            unsigned int block_ptr_val_3 = 0;
                            pread(fs,(void *)&block_ptr_val_3,4,block_ptr_val_2*s_block.s_log_block_size+block_entry_num_2*4);
                            if(block_ptr_val_3!=0)
                            {
                              fprintf(fd_id, "%x,%d,%x\n", block_ptr_val_2, block_entry_num_2, block_ptr_val_3);
                            }
                          }
                        }
                      }
                    }
                  }
                }
                /*end of triple part*/
              }
            }
          }
          first_xbits_in_inode_map_2++;
          inode_bitmap_entry_3 = inode_bitmap_entry_3 >> 1;
        }
      }
    }
    fclose(fd_id);

    //closefile
  	close(fs);
    exit(0);

}

