all : 
	$(MAKE) compile

compile : 
	g++ -g tcpserver.cpp -o tcpserver -lpthread
	g++ -g tcpclient.cpp -o tcpclient -lpthread
	g++ -g udpserver.cpp -o udpserver -lpthread
	g++ -g udpclient.cpp -o udpclient -lpthread	

tcpserver : 	
	$(MAKE) compile
	./tcpserver

tcpclient : 	
	$(MAKE) compile
	./tcpclient

udpserver : 	
	$(MAKE) compile
	./udpserver

udpclient : 	
	$(MAKE) compile
	./udpclient
