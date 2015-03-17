## Reliable UDP

#### init :
* Start off with simple UDP app which can send and recieve files

#### Header Data structure :
* 1500 bytes
* IP headers - Address and other stuff - 2 * 32bits , -- Not to worry just need size to subtracct from 1500
  * Source IP - alreadypresent
  * Dest IP - alreadypresent
  * TTL 
  * Checksum - Already present
* Length : 1500 - sizeof(header) - 20 bytes of IP
* Sequence number 32bit , - Similar to TCP under IPv4
* ACK flag - Duplicate acks, repeated acks for 
* Advertised window is not part of header - What is this ?? (Command line option or properties file)


#### Sliding window algo

#### Adaptive retransmission - Jacobson/Karels alog
* Need estimated RTT and Deviation - How are we going to achieve this ??

#### Congestion control
* Additive increase , multiplicative decrease - Once you have a packet drop
* Slow start initially



#### Simulate loss and delay
* Variable packet loss rate - Using rand()
* Latency using sleep


## Structure

* Initial Header design 
* Parse headers - regex.cpp [ not really - no strings present , just count and do bit manipulation ]
* Sending and recieving over UDP
* *Get RTT* 
* Jacob karsels algo - Meant for timeout of messages
* *Sliding window algo*
* Slow start
* AIMD
* Loss and delay


