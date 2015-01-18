/*
 * httpclient.h
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

int getlist_forward(char *argv, int arglen, char *Iam, int hopcount, char *buf);
int redirect(char *argv, int arglen, char * route, int clifd, int hopcount,
		char * Iam);
extern int tcp_connect(const char *host, const char *serv);
extern int httpget(int sockfd, const char *host, const char *route,
		const char *fname, int clifd, int hopcount, char * Iam);
int put_forward(char *argv, int arglen, char *Iam, char *route, int hopcount,
		int fd);
extern int httpput(int sockfd, const char *host, const char *route,
		int hopcount, char * Iam, int fd);

#endif /* HTTPCLIENT_H_ */
