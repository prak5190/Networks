## TCP Server
Usage : 

    ./tcpserver -p 1124
    ./tcpclient -p 1124 -h localhost -f /file


- Implement a 404 mechanism  - Done
- Works with client and browser
- Currently looks for files in the www directory
- Persistent connections ?? 


## UDP Server
Usage :

    ./udpserver -p 1112
    ./udpclient -p 1112 -h localhost -f /tcpserver
    
- Client and server talk to each other using http headers 
- Client displays size of request 
- Server indicates end of transmission by sending a packet of size 0

## Threads

- TCP server is multi threaded - can handle simultaneous requests
- Thread handling is not that great - Memory leaks may be present. Currently threads are created and assumption is that they exit when socket finishes.

## Writeup

- Need to deploy it on CS machine and test RTT
