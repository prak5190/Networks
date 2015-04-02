all: 
	g++ -std=c++11 src/app.cpp -o app -lpthread -lrt
receive: 
	./app -r 9000
send:
	./app -s file localhost 9000
clean: 
	rm ./app
scl:
	scl enable devtoolset-2 bash
