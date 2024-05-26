# IPK 2023/2024 - Client for a chat server

This project's task is to design and implement a client application, which is able to communicate with a remote server using the IPK24-CHAT protocol. The protocol has got two variants - TCP and UDP, each built on top of a different transport protocol.


## Introduction
The client for IPK24-CHAT protocol was written in C, the program is fully compatible with the Linux operating system. The project was completed on a virtual machine using the Linux distribution NixOs and using development environment [dev-envs](#bibliography). Also was used the [Wireshark](https://www.wireshark.org) traffic analyzer to track packets when communicating with a remote server. 

### CLI
Program can be ran by the following command:

```
    ./ipk24chat-client -t <protocol> -s <hostname> -p <port> -d <delay> -r <number of retransmissions>
```

There is another possible way. Values for delay and retransmissions for UDP variant will be 250 and 3 respectively:

```
    ./ipk24chat-client -t <protocol> -s <hostname> -p <port>
```

If the command contains `-h` parameter, the program will only print help information to `STDOUT` and terminate the program.

`Protocol` - Transport protocol used for connection. Possible values are TCP and UDP.

`Hostname` - Server IP or hostname. Only IPv4 IP addresses supported.

`Port` - Server port. The value must be in interval <1024, 65535>.

`Delay` - UDP confirmation timeout.

`Retransmissions` - Maximum number of UDP retransmissions.


## Specification

The IPK24-CHAT protocol defines a high-level behaviour, which can then be implemented on top of one of the well known transport protocols such as TCP [RFC9293](#bibliography) or UDP [RFC768](#bibliography). 

Each message is generated depending on the protocol. In TCP messages are full text-based, in UDP messages are a sequence of bytes. But for user this information is nonmandatory, because user sends his messages by using commands, which are the same for all variants.

| Command | Parameters |
| ------  | ---------- |
| /auth   | {Username} {Secret} {DisplayName} |    
| /join   | {ChannelID} |
| /rename | {DisplayName} |
| /help   |            |

All commands and parameters are case-sensitive. So make sure, if you typed the commands and parameters correctly.

### Commands

#### `/auth`
This commands allows user to pass the authentication. The order of the parameters is important. Example: 
```
/auth xmazhi00 secret Alisher
```

#### `/join`
This commands allows user to join the existing channel. Example: 
```
/join general
```

#### `/rename`
This commands allows user to change the `DisplayName`.Example: 
```
/rename Abobus
```

#### `/help`
This commands prints some useful information on how to communicate with a remote server. Example: 
```
/help
```


### Communicating with server
#### TCP

This protocol is built on top of TCP [RFC9293](#bibliography) and uses text-based commands to communicate with a remote server. TCP provides a reliable, in-order, byte-stream service to applications. TCP is a connection-oriented protocol that provides a three-way handshake to allow a client and server to set up a connection and negotiate features and provides mechanisms for orderly completion and immediate teardown of a connection

Each of text-based commands are based on ABNF [RFC5234](#bibliography), which means that user input data are case-sensitive. After the user enters the required command, the program will first need to check the correctness of the entered command. Next step is the correct message generation for each protocol. 

Snippets below represents some examples of message, that is sent to the server:

```
    $ /auth xmazhi00 SecretCode Alisher
    AUTH xmazhi00 AS Alisher USING SecretCode
```

```
    $ /join general
    JOIN general AS Alisher
```


#### UDP

Another protocol is built on top of UDP [RFC768](#bibliography). This protocol  provides  a procedure  for application  programs  to send messages  to other programs  with a minimum  of protocol mechanism.  The protocol  is transaction oriented, and delivery and duplicate protection are not guaranteed. As the UDP is connection-less and the delivery is unreliable, using it as a transport layer protocol poses certain challenges which need to be addressed at the application layer. Every UDP datagrams has the header, which allows server to understand what type of messages user sends.

| Field name  | Value     | Notes
| :---------- | --------: | -----
| `Type`      | `uint8`   | *N/A*
| `MessageID` | `uint16`  | Sent in network byte order

Snippets below represents some examples of message, that is sent to the server:
```
    $ /auth xmazhi00 SecretCode Alisher
    \x02\x00xmazhi00\x00Alisher\x00SecretCode\x00
```

```
    $ /join general
    \x03\x01general\x00Alisher\x00
```

At the end of this section i would like to tell general information about my implementation. Communication with the server using the TCP and UDP protocols has some similarities, but also differences in its implementation.

#### Establishing a connection
* The first step of every TCP communication is establishing a connection `(Handshake)`. When a client wants to establish a connection with a server, it sends a connection request `(SYN)` packet to a specific port on the server. The server, in turn, sends a confirmation packet `(SYN-ACK)`, informing the client that it is ready to accept the connection. The client then sends a final acknowledgment packet `(ACK)` and the connection is established.
* UDP does not require a connection to be established before transmitting data, unlike TCP, which uses an established connection to transfer data between the client and server. In the UDP protocol, data is simply sent from the sender to the recipient in the form of `datagrams`, each of which contains information about the recipient's port and address.

1. Resolve hostname using special functions such as `getaddrinfo()` or `gethostbyname()`.
2. Create socket using `socket()` 
3. Connect to a server `connect`, in case of TCP.

#### Transfer packets

* Once a connection is established, data between the client and server is transferred in both directions through the established TCP connection.
* The server accepts data packets arriving on the specified port. It does not track the connection state, but simply processes incoming packets independently of each other.

1. Send packets/datagrams using `send()` for TCP and `sendto()` for UDP
2. Receive them using `recv()` for TCP and `recvfrom()` for UDP

#### Finishing a connection
* After the communication is completed, one of the parties sends a packet with a request to close the connection `(FIN)`. The other side (the server) sends an acknowledgment `(ACK)` and closes its part of the connection, and then sends its own `FIN` packet. The client acknowledges receipt of the `FIN` and closes the connection.
* Since UDP is a stateless protocol, there is no formal connection termination as such. The client and server simply stop sending and receiving packets from each other.

1. Close socket `close()` and `freeaddrinfo()` freeing memory allocated when using a function `getaddrinfo()`. Only for TCP

## Testing
To test the functionality of the client part, i made a simple server that could receive and send messages. This server can communicate with only one user and is a prototype for another [project](https://git.fit.vutbr.cz/NESFIT/IPK-Projects-2024/src/branch/master/Project%202/iota). The Wireshark traffic analyzer was used to track sent and received packets. Also i used some open source [code implementations](#bibliography) for creating this server. My testing server shows, how must the communication look like, the order of sending messages, how user or server must work with it. In this section i wanted to show general way of communicating with a server, how it probably is in other chat applications.

> <span style="color:orange">The implementation of this server may differ from the above-mentioned server for the IOTA project. This server is used only for testing the client part of IPK24-CHAT protocol and contains less functionality!
</span>



#### TCP
The following example shows the implementation of TCP connection. 
Server side:
```
    ./server -t tcp -s 0.0.0.0 -p 4567
```
After creating server, server will print to stdin information about what port will be listeting, length of received buffer and information about connected user.

Client side:
```
    ./ipk24chat-client -t tcp -s 0.0.0.0 -p 4567 -d 250 -r 3
```

Following screenshot shows how communication looks in terminal:
![terminal](/recources/tcp_test_terminal.png)

Following screenshot shows how communication looks in Wireshark. Thanks to the provided [Lua script](#bibliography), you can more comfortably monitor communication with the server:
![terminal](/recources/test_tcp_wireshark.png)


#### UDP
The following example shows the implementation of UDP connection. 
Server side:
```
    ./server -t udp -s 0.0.0.0 -p 4567
```
After creating server, server will print to stdin information about what port will be listeting, length of received buffer and information about connected user.

Client side:
```
    ./ipk24chat-client -t udp -s 0.0.0.0 -p 4567 -d 250 -r 3
```

Following screenshot shows how communication looks in terminal:
![terminal](/recources/udp_test_terminal.png)

Following screenshot shows how communication looks in Wireshark:
![terminal](/recources/test_udp_wireshark.png)


## Evaluation
    14.58/20

## Bibliography

[RFC768] Postel, J. _User Datagram Protocol_ [online]. March 1997. [cited 2024-02-11]. DOI: 10.17487/RFC0768. Available at: https://datatracker.ietf.org/doc/html/rfc768

[RFC9293] Eddy, W. _Transmission Control Protocol (TCP)_ [online]. August 2022. [cited 2024-02-11]. DOI: 10.17487/RFC9293. Available at: https://datatracker.ietf.org/doc/html/rfc9293

[RFC5234] Crocker, D. _Augmented BNF for Syntax Specifications: (ABNF)_ [online]. January 2008 [cited 2024-02-26]. DOI: 10.17487/RFC5234. Available at: https://datatracker.ietf.org/doc/html/rfc5234

[IPK24-CHAT] Dolejska, D., Zavrel, J., Vesely, V. _Client for a chat server using IPK24-CHAT protocol_ [online]. February 2024 [citied 2024-02-26]. Available at: https://git.fit.vutbr.cz/NESFIT/IPK-Projects-2024/src/branch/master/Project%201/README.md

[DEV-ENVS] Dolejska, D., Zavrel, J., Vesely, V. _Shared Development Environments_ [online]. 2023 [citied 2024-02-26]. Available at: https://git.fit.vutbr.cz/NESFIT/dev-envs

[LUA-SCRIPT] Dolejska, D., Zavrel, J., Vesely, V. _Wireshark Protocol Dissector_ [online]. 2024 [citied 2024-02-26]. Available at: https://git.fit.vutbr.cz/NESFIT/IPK-Projects-2024/src/branch/master/Project%201/resources

[DEMO-TCP] Dolejska, D., Zavrel, J. _Demo C_ [online]. 2024 [citied 2024-02-26]. Available at: https://git.fit.vutbr.cz/NESFIT/IPK-Projects-2024/src/branch/master/Stubs/cpp/DemoTcp

[DEMO-UDP] Dolejska, D., Zavrel, J. _Demo C_ [online]. 2024 [citied 2024-02-26]. Available at: https://git.fit.vutbr.cz/NESFIT/IPK-Projects-2024/src/branch/master/Stubs/cpp/DemoUdp