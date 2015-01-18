# Network Programming
Homework for network programming course at Aalto University

## HTTP Client
### INFORMATION:
This is a program to perform HTTP GET/PUT function, it can get a text file from the HTTP server and put a text file to the server.

### BULID & RUN & CLEAN:  

In the home directory, type "make" to build.
$make

Usage of the program:
"Usage:
	./httpclient hostname:portnumber/<route> 
	to get a file OR
	./httpclient hostname:portnumber/<route> <localfile route>
	to put a file on the server

The get function can get the text file from the server and save it to the local disk with the same filename.
The put function can put the local text file indicated by <localfile route> to the <route> on the server.



The cleaning work is also simply type "make clean" in the terminal.
$make clean
Cleaning work will remove all generated files, including the excutable file and all the got .txt file.

## HTTP Server
### INFORMATION:          

This is a program to perform HTTP Server function, it can support standard GET and PUT request from several clients at the same time.

### BULID & RUN & CLEAN:  

In the home directory, type "make" to build.
$make

Usage of the program:
Usage:
	./httpserver <port number>

if the port number is given in the command line, it will be used as the listening port, otherwise, the default port number is used, default port number is predefined as 8888.



The cleaning work is also simply type "make clean" in the terminal.
$make clean
Cleaning work will remove all generated files, including the excutable file.

## HTTP Proxy
### INFORMATION:          

This is a program to perform HTTP proxy function, it can accept http get request and get a text file which is not existed in local folder from the other HTTP server in the routelist,it can also get and combine the index from the nearby servers in routelist.
On the other hand, it can forward put a text file to the nearby server in the routelist as well as store it in local folder.
The new header Hop-Count is added to scale the range of proxy.

### BULID & RUN & CLEAN:  

In the home directory, type "make" to build.
$make

Usage of the program:
Usage:
	./httpproxy <port number>

if the port number is given in the command line, it will be used as the listening port, otherwise, the default port number is used, default port number is predefined as 8888.



The cleaning work is also simply type "make clean" in the terminal.
$make clean
Cleaning work will remove all generated files, including the excutable file.

