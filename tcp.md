# TCP  

## Three-Way Handshake  

The following scenario occurs when a TCP connection is established:  

1.The server must be prepared to accept an incoming connection. This is normally done by calling **socket**, **bind**, and **listen** and is called a passive open.  

2.The client issues an active open by calling **connect**. This causes the **client TCP** to send a 'synchronize' (**SYN**) segment, which tells the server the client’s initial sequence number for the data that the client will send on the connection. Normally, there is no data sent with the SYN; it just contains an IP header, a TCP header, and possible TCP options (which we will talk about shortly.  

3.The **server** must acknowledge (**ACK**) the client’s SYN and the server must also send its own SYN containing the initial sequence number for the data that the server will send on the connection. The server **sends its SYN and the ACK of the client’s SYN** in a single segment.  

4.The **client** must **acknowledge** the server’s SYN.  

## TCP Connection Termination  

While it takes three segments to establish a connection, it takes four to terminate a connection.

1.One application calls close first, and we say that this end performs the active close. This end’s TCP sends a FIN segment, which means it is finished sending data.  

2.The other end that receives the FIN performs the passive close. The received FIN is acknowledged by TCP. The receipt of the FIN is also passed to the application as an end-of-file (after any data that may have already been queued for the application to receive), since the receipt of the FIN means the application will not receive any additional data on the connection.  

3.Sometime later, the application that received the end-of-file will close its socket. This causes its TCP to send a FIN.  

4.The TCP on the system that receives this final FIN (the end that did the active close) acknowledges the FIN.  
  
**copied from UNP, chapter 2**.
