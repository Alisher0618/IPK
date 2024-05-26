## 19.02.2024
* LICENSE, README.md and CHANGELOG.md added

## 29.02.2024
* Parsing of CLI arguments added

## 03.03.2024
* Added connection to server using TCP protocol

## 09.03.2024
* Big update - sending messages by commands
* Separated the ways of sending messages - from stdin and from a file
* Refactored code, parsing and sending tcp messages in another file(tcp.c)
* Updated Makefile - took it from my IFJ project

## 11.03.2024
* Refactored code - separate file for macros, sending and receiving messages to/from server
* Updated makefile

## 18.03.2024
* UDP communication started
* creating send message added
* can only send /auth messages
* some modifications added

## 19.03.2024
* sending AUTH, CONFIRM and receiving messages from server added
* some code refactoring added

## 20.03.2024
* sending MSG added

## 21.03.2024
* fixed problems with TCP
* TODO: rewrite UDP receiving messages function

## 25.03.2024
* fixed parsing of MessageId

## 26.03.2024
* added receiving various types of packeges from Server(including unexpected packages for different state of FSM)
* added reading a file from stdin for UDP
* fixed some errors catched by valgrind

## 27.03.2024
* small fixes added

## 28.03.2024
* Success on stderr

## 29-30.03.2024
* Ctrl+D support added
* Updated receiving algorithm of messages from the remote server for TCP and UDP
* Started writing documentation in README.md

## 30.03.2024
* Split udp.c file into two different - udp_send.c/.h and udp_receive.c/.h
* updated README

## 31.03.2024
* Added testing server for TCP and UDP implementations