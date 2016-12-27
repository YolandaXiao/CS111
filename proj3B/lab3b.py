import csv
import sys
import os

class Inode(object):
	def __init__(self, inode_num, links_count):
		self._inode_num = inode_num
		self._inode_ref_list = []
		self._links_count = links_count;
		self._ptrs = []
		return

class Block(object):
	def __init__(self, block_num):
		self._block_num = block_num
		self._block_ref_list = []
		return

class OutputError(object):
	def __init__(self):
		self.inode_bitmaps = []
		self.block_bitmaps = []
		self.free_inode_list = []
		self.free_block_list = []

		self.allocated_inodes = dict()
		self.allocated_blocks = dict()
		self.indirect_map = dict()
		self.directory_map = dict()

		#output
		self.total_num_of_blocks = 0
		self.s_inodes_count = 0
		self.s_inodes_per_group = 0
		self.s_blocks_per_group = 0
		self.invalid_block_pointer_7 = []
		self.unallocated_inodes_3 = dict()
		self.incorrect_entry_6 =[]
		
		return	

	def read_files(self):
		#read super.csv
		with open('super.csv', 'r') as file:
			for line in csv.reader(file):
				self.total_num_of_blocks = int(line[2])
				self.s_inodes_per_group = int(line[6])
				self.s_blocks_per_group = int(line[5])
				self.s_inodes_count = int(line[1])

		#read group.csv
		with open('group.csv', 'r') as file:
			for line in csv.reader(file):
				block_value = int(line[5],16)
				block_inode_value = int(line[4],16)
				self.block_bitmaps.append(block_value)
				self.inode_bitmaps.append(block_inode_value)
				self.allocated_blocks[block_value] = Block(block_value)
				self.allocated_blocks[block_inode_value] = Block(block_inode_value)

		#read bitmap.csv
		with open('bitmap.csv', 'r') as file:
			for line in csv.reader(file):
				block_num = int(line[0],16)
				entry_num = int(line[1])
				if block_num in self.block_bitmaps :
					self.free_block_list.append(entry_num)
				elif block_num in self.inode_bitmaps :
					self.free_inode_list.append(entry_num)

		#read indirect.csv
		with open('indirect.csv', 'r') as file:
			for line in csv.reader(file):
				block_num_of_containing_block = int(line[0], 16)
				entry_num = int(line[1])
				block_ptr_val = int(line[2], 16)
				if block_num_of_containing_block not in self.indirect_map :
					self.indirect_map[block_num_of_containing_block] = [(entry_num,block_ptr_val)]
				else :
					self.indirect_map[block_num_of_containing_block].append((entry_num,block_ptr_val))
		return

	def read_directory(self):
		with open('directory.csv', 'r') as file:
			for line in csv.reader(file):
				parent_inode_num = int(line[0])
				child_inode_num = int(line[4])
				entry_num = int(line[1])
				entry_name = line[5]
				if parent_inode_num != child_inode_num or parent_inode_num == 2:
					self.directory_map[child_inode_num] = parent_inode_num
				if child_inode_num in self.allocated_inodes :
					self.allocated_inodes[child_inode_num]._inode_ref_list.append((parent_inode_num,entry_num))
				else :
					if child_inode_num in self.unallocated_inodes_3 :
						self.unallocated_inodes_3[child_inode_num].append((parent_inode_num,entry_num))
					else :
						self.unallocated_inodes_3[child_inode_num] = [(parent_inode_num,entry_num)]
				if entry_num == 0 and child_inode_num!= parent_inode_num :
					self.incorrect_entry_6.append((parent_inode_num,entry_name,child_inode_num,parent_inode_num))
				elif entry_num == 1 and child_inode_num != self.directory_map[parent_inode_num] :
					self.incorrect_entry_6.append((parent_inode_num,entry_name,child_inode_num,self.directory_map[parent_inode_num]))
		return

	def read_inode(self):
		with open('inode.csv', 'r') as file:
			for line in csv.reader(file):
				inode_num = int(line[0]);
				links_count = int(line[5]);
				self.allocated_inodes[inode_num] = Inode(inode_num,links_count)
				number_of_blocks = int(line[10]);

				#direct blocks 
				if number_of_blocks <= 12 :
					for i in range(0,number_of_blocks) :
						block_num = int(line[i+11],16)
						self.update_block(block_num,inode_num,0,i)
				#indirect blocks
				else :
					x = int(line[i+12],16) 
					if x==0 or x >= self.total_num_of_blocks :
						self.invalid_block_pointer_7.append((x,inode_num,x,12))
					else:
						if x in self.indirect_map :
							for value in self.indirect_map[x] :
								self.update_block(value[1],inode_num,x,value[0])
		return 

	def update_block(self, block_num, inode_num, indirect_block_num, entry_num):
		if block_num==0 or block_num > self.total_num_of_blocks :
			self.invalid_block_pointer_7.append((block_num,inode_num,indirect_block_num,entry_num))
		elif block_num in self.allocated_blocks :
			self.allocated_blocks[block_num]._block_ref_list.append((inode_num, indirect_block_num,entry_num)) #???
		else :
			self.allocated_blocks[block_num] = Block(block_num)
			self.allocated_blocks[block_num]._block_ref_list.append((inode_num,indirect_block_num,entry_num)) #???
		return

	def read_values(self):
	    self.read_files()
	    self.read_inode()
	    self.read_directory()
	    return

	def write_values(self):
		with open('lab3b_check.txt', 'w') as file:
			#3 unallocated inode
			for key in self.unallocated_inodes_3 :
				file.write("UNALLOCATED INODE < "+ str(key) +" > REFERENCED BY ")
				for value in self.unallocated_inodes_3[key] :
					file.write("DIRECTORY < "+ str(value[0]) +" > ENTRY < " + str(value[1]) + " > ")
				file.write("\n")

			#7 invalid block pointers
			for value in self.invalid_block_pointer_7:
				file.write("INVALID BLOCK < "+ str(value[0]) +" > IN INODE < "+ str(value[1]) +" > ENTRY < "+ str(value[3]) +" >\n")

			#6 incorrect entry
			for value in self.incorrect_entry_6:
				file.write("INCORRECT ENTRY IN < "+ str(value[0]) +" > NAME < "+ str(value[1]) +" > LINK TO < "+ str(value[2]) +" > SHOULD BE < "+ str(value[3]) +" >\n")

			for a in self.allocated_inodes :
				# 4 missing inode
				if a > 10 and not self.allocated_inodes[a]._inode_ref_list :
					file.write("MISSING INODE < " + str(a) + " > SHOULD BE IN FREE LIST < "+ str(4+a/self.s_inodes_per_group*self.s_inodes_per_group) +" >\n")#????
				# 5 incorrect link count
				ref_length = len(self.allocated_inodes[a]._inode_ref_list)
				link_count = self.allocated_inodes[a]._links_count
				if ref_length !=  link_count :
					file.write("LINKCOUNT < "+ str(a) +" > IS < " + str(link_count) + " > SHOULD BE < " + str(ref_length) + " >\n")
				# #3 unallocated inode
				if a in self.free_inode_list and a in self.allocated_inodes :
				 	file.write("UNALLOCATED INODE < "+ str(a) +" > REFERENCED BY ")
				 	for value in self.allocated_inodes[a] :
				 		file.write("DIRECTORY < "+ str(value[0]) +" > ENTRY < " + str(value[1]) + " > ")
				 	file.write("\n")

			# 4 missing inode	
			for x in range(11,self.s_inodes_count) :
				if x not in self.free_inode_list and x not in self.allocated_inodes :
					file.write("MISSING INODE < " + str(x) + " > SHOULD BE IN FREE LIST < "+ str(4+a/self.s_inodes_per_group*self.s_inodes_per_group) +" >\n")#????

			for x in self.allocated_blocks :
				# 2 duplicately allocated blocks
				if len(self.allocated_blocks[x]._block_ref_list) > 1:
					file.write("MULTIPLY REFERENCED BLOCK < " + str(x) + " > BY ")
					for value in self.allocated_blocks[x]._block_ref_list:
						if value[1]!=0:
							file.write("INODE < " +str(value[0])+ " > INDIRECT BLOCK < "+str(value[1])+" > ENTRY < "+str(value[2])+" >")
						else:
							file.write("INODE < " +str(value[0])+ " > ENTRY < "+str(value[2])+" > ")
					file.write("\n")
				# 1 unallocated block
				if x in self.free_block_list and x in self.allocated_blocks:
					file.write("UNALLOCATED BLOCK < "+ str(x) +" > REFERENCED BY ")
					for value in self.allocated_blocks[x]._block_ref_list :
						if value[1]!=0:
							file.write("INODE < " +str(value[0])+ " > INDIRECT BLOCK < "+str(value[1])+" > ENTRY < "+str(value[2])+" >")
						else:
							file.write("INODE < " +str(value[0])+ " > ENTRY < "+str(value[2])+" >")
					file.write("\n")
		return 

if __name__ == '__main__':
	OutputError = OutputError()
	OutputError.read_values()
	OutputError.write_values()




