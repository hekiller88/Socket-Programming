# Intro 
An twitter imitation Message Feeds System using _Datagram_ Sockets using c.

# How to Run
## Command to Compile
`gcc -o UDPEchoServer UDPEchoServer.c DieWithError.c ServerTable.c
`gcc -o UDPEchoClient UDPEchoClient.c DieWithError.c
Notice: There's a ServerTable.c for Server.

## Command to Run
`./UDPEchoClient 127.0.0.1 "HAHAH" 29999
`./UDPEchoServer 29999



