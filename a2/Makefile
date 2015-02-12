all : 
	$(MAKE) run

compile : 
	g++ -g main.cpp -o a.out -lpthread
run : 	
	$(MAKE) compile
	./a.out

debug : 
	$(MAKE) compile
	gdb a.out
