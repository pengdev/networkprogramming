/*
 * libhttpserver.h
 *
 *  Created on: 11.3.2012
 *      Author: liup1
 */

#ifndef LIBHTTPSERVER_H_
#define LIBHTTPSERVER_H_
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>

#define BUFSIZE 20000
#define RUNNING_DIR	"LEOsROOT"
#define LOCK_FILE	"daemon.lock"
#define LOG_FILE	"record.log"

void daemonize(void);
int getfile(char *route, int clifd, char *Iam, int hopcount);
int getlist(int clifd, char *Iam, int hopcount);
int service(int clifd);
void log_message(char *filename, char *message);
void signal_handler(int sig);
int show_nf(int sockfd); // show the not found message

#endif /* LIBHTTPSERVER_H_ */
