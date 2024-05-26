# IPK 2023/2024 - Chat server

This project's task is to design and implement a remote server which allows users to communicate between each other using the IPK24-CHAT protocol. The protocol has got two variants - TCP and UDP, each built on top of a different transport protocol.

## Introduction
The server for IPK24-CHAT protocol was written in C, the program is fully compatible with the Linux operating system. The project was completed on a virtual machine using the Linux distribution NixOs and using development environment [dev-envs](#bibliography). Also was used the [Wireshark](https://www.wireshark.org) traffic analyzer to track packets when communicating with a remote server. 

### CLI
Program can be ran by the following command:

```
    ./ipk24chat-server -l <ip address> -p <port> -d <delay> -r <number of retransmissions>
```

There is another possible way. Values for delay and retransmissions for UDP variant will be 250 and 3 respectively:

```
    ./ipk24chat-server -l <ip address> -p <port>
```

If the command contains `-h` parameter, the program will only print help information to `STDOUT` and terminate the program.

`Ip address` - Server IP. Only IPv4 IP addresses supported.

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


1. Create welcome socket using `socket()` for TCP and UDP.
2. Bind servers welcome socket using `bind()` for TCP and UDP.
3. Listening servers socket, only in case of TCP.
3. In case of any incoming messages fro client accept using `accept()` communication, only for TCP.

#### Transfer packets

* Once a connection is established, data between the client and server is transferred in both directions through the established TCP connection.
* The server accepts data packets arriving on the specified port. It does not track the connection state, but simply processes incoming packets independently of each other.

1. Send packets/datagrams using `send()` for TCP and `sendto()` for UDP
2. Receive them using `recv()` for TCP and `recvfrom()` for UDP

#### Finishing a connection
* After the communication is completed, one of the parties sends a packet with a request to close the connection `(FIN)`. The other side (the server) sends an acknowledgment `(ACK)` and closes its part of the connection, and then sends its own `FIN` packet. The client acknowledges receipt of the `FIN` and closes the connection.
* Since UDP is a stateless protocol, there is no formal connection termination as such. The client and server simply stop sending and receiving packets from each other.

1. Close all created sockets using `close()`. 

## Testing
To test the functionality of the server i used my previous project - client part for [IPK24-CHAT](#bibliography). The following snippets show 3 different example of communication: UDP + UDP, TCP + TCP and TCP + UDP. In the end, program on clients side was terminated by `C-c`. The most inetresting part is when client on the left changes channel. Client on the right wrote `hello from default!` and the left client did not get it, because their channels are not the same. The same way for `hello from general!`.

I also added output from wireshark. You can find it in [here](https://git.fit.vutbr.cz/xmazhi00/IPK24_proj2/src/branch/main/recources/wireshark_output.txt)

```
                                                    /auth username secret displayname
                                                    Success: Auth success. 
                                                    Server: displayname joined default
                                                      
    /auth xmazhi00 secret abobus                    
    Success: Auth success.
    Server: abobus joined default
                                                    Server: abobus joined default
    hello
                                                    abobus: hello
                                                    hi!
    displayname: hi!
    /join general
                                                    Server: abobus left default
                                                    hello from default!
    Success: Join success.
    Server: abobus joined general
    hello from general!
                                                    /join general
                                                    Success: Join success.
                                                    Server: displayname joined general
                                                    hello again
    Server: displayname joined general
    displayname: hello again
    hi 
                                                    abobus: hi
                                                    Server: abobus left general
```

1. UDP + UDP
```
RECV 127.0.0.1:52380 | AUTH
SENT 127.0.0.1:52380 | CONFIRM
SENT 127.0.0.1:52380 | REPLY
SENT 127.0.0.1:52380 | MSG
RECV 127.0.0.1:52380 | CONFIRM
RECV 127.0.0.1:52380 | CONFIRM
RECV 127.0.0.1:43883 | AUTH
SENT 127.0.0.1:43883 | CONFIRM
SENT 127.0.0.1:43883 | REPLY
SENT 127.0.0.1:43883 | MSG
SENT 127.0.0.1:52380 | MSG
RECV 127.0.0.1:52380 | CONFIRM
RECV 127.0.0.1:43883 | CONFIRM
RECV 127.0.0.1:43883 | CONFIRM
RECV 127.0.0.1:43883 | MSG
SENT 127.0.0.1:43883 | CONFIRM
SENT 127.0.0.1:52380 | MSG
RECV 127.0.0.1:52380 | CONFIRM
RECV 127.0.0.1:52380 | MSG
SENT 127.0.0.1:52380 | CONFIRM
SENT 127.0.0.1:43883 | MSG
RECV 127.0.0.1:43883 | CONFIRM
RECV 127.0.0.1:43883 | JOIN
SENT 127.0.0.1:43883 | CONFIRM
SENT 127.0.0.1:52380 | MSG
SENT 127.0.0.1:43883 | REPLY
SENT 127.0.0.1:43883 | MSG
RECV 127.0.0.1:52380 | CONFIRM
RECV 127.0.0.1:43883 | CONFIRM
RECV 127.0.0.1:43883 | CONFIRM
RECV 127.0.0.1:43883 | MSG
SENT 127.0.0.1:43883 | CONFIRM
RECV 127.0.0.1:52380 | MSG
SENT 127.0.0.1:52380 | CONFIRM
RECV 127.0.0.1:52380 | JOIN
SENT 127.0.0.1:52380 | CONFIRM
SENT 127.0.0.1:52380 | REPLY
SENT 127.0.0.1:52380 | MSG
SENT 127.0.0.1:43883 | MSG
RECV 127.0.0.1:52380 | CONFIRM
RECV 127.0.0.1:43883 | CONFIRM
RECV 127.0.0.1:52380 | CONFIRM
RECV 127.0.0.1:52380 | MSG
SENT 127.0.0.1:52380 | CONFIRM
SENT 127.0.0.1:43883 | MSG
RECV 127.0.0.1:43883 | CONFIRM
RECV 127.0.0.1:43883 | MSG
SENT 127.0.0.1:43883 | CONFIRM
SENT 127.0.0.1:52380 | MSG
RECV 127.0.0.1:52380 | CONFIRM
RECV 127.0.0.1:43883 | BYE
SENT 127.0.0.1:43883 | CONFIRM
SENT 127.0.0.1:52380 | MSG
RECV 127.0.0.1:52380 | CONFIRM
RECV 127.0.0.1:52380 | BYE
SENT 127.0.0.1:52380 | CONFIRM
SENT 127.0.0.1:43883 | MSG
```

2. TCP + TCP
```
RECV 127.0.0.1:45484 | AUTH
SENT 127.0.0.1:45484 | REPLY
SENT 127.0.0.1:45484 | MSG
RECV 127.0.0.1:44446 | AUTH
SENT 127.0.0.1:44446 | REPLY
SENT 127.0.0.1:44446 | MSG
SENT 127.0.0.1:45484 | MSG
RECV 127.0.0.1:45484 | MSG
SENT 127.0.0.1:44446 | MSG
RECV 127.0.0.1:44446 | MSG
SENT 127.0.0.1:45484 | MSG
RECV 127.0.0.1:45484 | JOIN
SENT 127.0.0.1:44446 | MSG
SENT 127.0.0.1:45484 | REPLY
SENT 127.0.0.1:45484 | MSG
RECV 127.0.0.1:45484 | MSG
RECV 127.0.0.1:44446 | MSG
RECV 127.0.0.1:44446 | JOIN
SENT 127.0.0.1:44446 | REPLY
SENT 127.0.0.1:44446 | MSG
SENT 127.0.0.1:45484 | MSG
RECV 127.0.0.1:44446 | MSG
SENT 127.0.0.1:45484 | MSG
RECV 127.0.0.1:45484 | MSG
SENT 127.0.0.1:44446 | MSG
RECV 127.0.0.1:45484 | BYE
SENT 127.0.0.1:44446 | MSG
RECV 127.0.0.1:44446 | BYE
```

3. TCP + UDP
```
RECV 127.0.0.1:37696 | AUTH
SENT 127.0.0.1:37696 | REPLY
SENT 127.0.0.1:37696 | MSG
RECV 127.0.0.1:58751 | AUTH
SENT 127.0.0.1:58751 | CONFIRM
SENT 127.0.0.1:58751 | REPLY
SENT 127.0.0.1:58751 | MSG
SENT 127.0.0.1:37696 | MSG
RECV 127.0.0.1:58751 | CONFIRM
RECV 127.0.0.1:58751 | CONFIRM
RECV 127.0.0.1:37696 | MSG
SENT 127.0.0.1:58751 | MSG
RECV 127.0.0.1:58751 | CONFIRM
RECV 127.0.0.1:58751 | MSG
SENT 127.0.0.1:58751 | CONFIRM
SENT 127.0.0.1:37696 | MSG
RECV 127.0.0.1:37696 | JOIN
SENT 127.0.0.1:58751 | MSG
SENT 127.0.0.1:37696 | REPLY
SENT 127.0.0.1:37696 | MSG
RECV 127.0.0.1:58751 | CONFIRM
RECV 127.0.0.1:37696 | MSG
RECV 127.0.0.1:58751 | MSG
SENT 127.0.0.1:58751 | CONFIRM
RECV 127.0.0.1:58751 | JOIN
SENT 127.0.0.1:58751 | CONFIRM
SENT 127.0.0.1:58751 | REPLY
SENT 127.0.0.1:58751 | MSG
SENT 127.0.0.1:37696 | MSG
RECV 127.0.0.1:58751 | CONFIRM
RECV 127.0.0.1:58751 | CONFIRM
RECV 127.0.0.1:58751 | MSG
SENT 127.0.0.1:58751 | CONFIRM
SENT 127.0.0.1:37696 | MSG
RECV 127.0.0.1:37696 | MSG
SENT 127.0.0.1:58751 | MSG
RECV 127.0.0.1:58751 | CONFIRM
RECV 127.0.0.1:37696 | BYE
SENT 127.0.0.1:58751 | MSG
RECV 127.0.0.1:58751 | CONFIRM
RECV 127.0.0.1:58751 | BYE
SENT 127.0.0.1:58751 | CONFIRM
```

## Evaluation
    9.73/20

## Bibliography

[RFC768] Postel, J. _User Datagram Protocol_ [online]. March 1997. [cited 2024-02-11]. DOI: 10.17487/RFC0768. Available at: https://datatracker.ietf.org/doc/html/rfc768

[RFC9293] Eddy, W. _Transmission Control Protocol (TCP)_ [online]. August 2022. [cited 2024-02-11]. DOI: 10.17487/RFC9293. Available at: https://datatracker.ietf.org/doc/html/rfc9293

[RFC5234] Crocker, D. _Augmented BNF for Syntax Specifications: (ABNF)_ [online]. January 2008 [cited 2024-02-26]. DOI: 10.17487/RFC5234. Available at: https://datatracker.ietf.org/doc/html/rfc5234

[IPK24-CHAT] Dolejska, D., Zavrel, J., Vesely, V. _Client for a chat server using IPK24-CHAT protocol_ [online]. February 2024 [citied 2024-02-26]. Available at: https://git.fit.vutbr.cz/NESFIT/IPK-Projects-2024/src/branch/master/Project%201/README.md

[DEV-ENVS] Dolejska, D., Zavrel, J., Vesely, V. _Shared Development Environments_ [online]. 2023 [citied 2024-02-26]. Available at: https://git.fit.vutbr.cz/NESFIT/dev-envs

[LUA-SCRIPT] Dolejska, D., Zavrel, J., Vesely, V. _Wireshark Protocol Dissector_ [online]. 2024 [citied 2024-02-26]. Available at: https://git.fit.vutbr.cz/NESFIT/IPK-Projects-2024/src/branch/master/Project%201/resources

[DEMO-TCP] Dolejska, D., Zavrel, J. _Demo C_ [online]. 2024 [citied 2024-02-26]. Available at: https://git.fit.vutbr.cz/NESFIT/IPK-Projects-2024/src/branch/master/Stubs/cpp/DemoTcp

[DEMO-UDP] Dolejska, D., Zavrel, J. _Demo C_ [online]. 2024 [citied 2024-02-26]. Available at: https://git.fit.vutbr.cz/NESFIT/IPK-Projects-2024/src/branch/master/Stubs/cpp/DemoUdp

[IPK24-CHAT-CLEINT] Mazhirinov, A. _Client for a chat server using IPK24-CHAT protocol_ [online]. April 2024 [citied 2024-02-26]. Available at: https://git.fit.vutbr.cz/xmazhi00/IPK24_proj1