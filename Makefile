lab0: lab0.c
	gcc lab0.c -o lab0

check:
	@echo "Test input and output files" > input_check
	@rm -f output_check
	@./lab0 --input=input_check --output=output_check
	@diff input_check output_check > /dev/null && echo "1)Input and output test passed" || echo "Error: Input and output test failed."

	@echo "Segmentation fault" > seg_input
	@(./lab0 --segfault;true) >& seg_check | grep -o "Segmentation fault" seg_check > seg_c
	@diff seg_input seg_c > /dev/null && echo "2)Segmentation test passed" || echo "Error: Segmentation test failed."
	@echo "Program quitting, signal number: 11." > catch_input
	@(./lab0 --segfault --catch; true)  >& catch_check
	@diff catch_input catch_check > /dev/null && echo "3)Catch test passed" || echo "Error: Catch test failed."
	@./lab0 --input=nonexist.txt > output.txt > /dev/null 2>&1 || [ $$? -eq 1 ] && (echo "4)Nonexist file test passed") || (echo "Error:Nonexist file test failed.")
	@echo "Input file" > input.txt
	@chmod -w output.txt
	@./lab0 --output=output.txt < input.txt > /dev/null 2>&1 || [ $$? -eq 2 ] && (echo "5)Nonwritable test passed")|| (echo "Error:Nonwritable test failed.")

	@rm -rf *_input *_check *_c *.txt

clean:
	$(RM) $(TARGET)
	$(RM) -rf *.o

dist:
	rm -rf lab0-904581627.tar.gz
	tar -cf lab0-904581627.tar.gz lab0.c Makefile Backtrace.png Breakpoint.png README
