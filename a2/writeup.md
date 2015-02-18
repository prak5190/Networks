### Write up ###

Implemented persistent connection in the following way :

- Set a length 
- Kept the connection open with timeout of 10 seconds - So connection gets reused if request is made in the timespan.

### Persistent vs Non-Persistent TCP connections : ###

##### Test conditions : #####
Recorded the time using time.h. The file size is **2014434 bytes**
Ran the tcpclient using the following command 

    for i in `seq 1 10` :
    do 
    ./tcpclient -p 2000 -h localhost -f /file -t 
    done > pers.txt

### The statistics is as follows: ###
**Sending 1 to 10 files using non persistent server**
Each file is of 2014434 bytes.

    Real     User     Sys
    
    0.217s   0.018s   0.178s
    0.581s   0.044s   0.467s
    0.873s   0.069s   0.714s
    1.030s   0.085s   0.832s
    1.435s   0.115s   1.134s
    1.879s   0.147s   1.490s
    2.001s   0.163s   1.622s
    2.501s   0.199s   2.040s
    2.864s   0.226s   2.231s
    3.157s   0.248s   2.488s

The time doesn't seem to increase linearly, possibly due to increased load and traffic. Since the non-persistent Http server closes the socket immedialtely, so no reuse takes place as well.

**Sending 1 to 10 files using persistent server**

    Real       User      Sys

    0.198s   0.016s  0.145s
    0.327s   0.029s  0.267s
    0.840s   0.069s  0.696s
    1.000s   0.084s  0.818s
    1.187s   0.101s  0.967s
    1.650s   0.135s  1.355s
    1.931s   0.158s  1.572s
    2.370s   0.190s  1.869s
    2.511s   0.206s  2.005s
    2.838s   0.229s  2.245s

The time doesn't seem to increase linearly, but the throughput is slightly better due to socket reuse.


### For just sending one 2 Mb file and averaging out after 10 time ###
**Non persistent Http**

    Time taken 0.188812
    Time taken 0.162422
    Time taken 0.252917
    Time taken 0.18025
    Time taken 0.197345
    Time taken 0.169124
    Time taken 0.167161
    Time taken 0.176085
    Time taken 0.169622
    Time taken 0.229706
    Time taken 0.22821

Average : 0.212166199

**Persistent Http**

    Time taken 0.133096
    Time taken 0.215962
    Time taken 0.131388
    Time taken 0.161088
    Time taken 0.193296
    Time taken 0.141694
    Time taken 0.178694
    Time taken 0.214977
    Time taken 0.24357
    Time taken 0.15778
    Time taken 0.161689
    
Average : 0.1933234

#### Packet loss in UDP : ####

No packet loss is observed when transferring data of small size , i.e < 20 kilobytes .
But as packet size increases, greater packet loss is observed. When testing with a 2 Mb file a discernible loss occurs. The time taken to transfer and the size recieved in 10 tries is as follows : 

    Total Size is : 2014434 Time taken 0.305561
    Total Size is : 1987619 Time taken 0.259942
    Total Size is : 2014434 Time taken 0.303734
    Total Size is : 2014434 Time taken 0.298796
    Total Size is : 2014434 Time taken 0.403613
    Total Size is : 2014434 Time taken 0.339462
    Total Size is : 2014434 Time taken 0.360929
    Total Size is : 2014434 Time taken 0.351598
    Total Size is : 2014434 Time taken 0.372711
    Total Size is : 2014434 Time taken 0.371817
    Total Size is : 1998319 Time taken 0.370614
    Total Size is : 2012293 Time taken 0.310553
    Total Size is : 2014434 Time taken 0.28811

    $ ls -l | grep file
    -rwxrwxrwx 1 root root 2014434 Feb 16 20:49 file

The actual size is 2014443 bytes, so it is pretty clear that packet loss occurs.

