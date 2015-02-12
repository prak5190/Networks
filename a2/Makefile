all : 
	$(MAKE) compile
	$(MAKE) run

compile : 
	g++ -g main.cpp -o a.out	
run : 	
	$(MAKE) compile
	./a.out
debug : 
	$(MAKE) compile
	gdb a.out
