/*
 * libhttpclient.c
 *
 *  Created on: 9.2.2012
 *      Author: liup1
 */

#ifndef LIBHTTPCLIENT_H_
#define LIBHTTPCLIENT_H_
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <stddef.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

/*this function is used to resolve host name to IP address
 * and establish a TCP connection with the server
 *
 * char * host is the host name char *serv is the port number*/
extern int tcp_connect(const char *host, const char *serv);

/*this function is used to get the file on the server
 * and save the file to local disk with the same filename it was on the server
 *
 * sockfd is the socket descriptor, char *host is the host name
 * char *route is the route on the server, char *fname is the file name of local file*/
extern int httpget(int sockfd, const char *host, const char *route,
		const char *fname);

/*this function is used to put the file on the local disk to the server
 *
 * sockfd is the socket descriptor, char *host is the host name
 * char *route is the route on the server*/
extern int httpput(int sockfd, const char *host, const char *route,
		const char *file);
#endif                    
