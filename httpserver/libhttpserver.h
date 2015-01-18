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
// define the root directory
#define RUNNING_DIR	"LEOsROOT"
// define file to save the daemon pid
#define LOCK_FILE	"daemon.lock"
// define file to save the log information
#define LOG_FILE	"record.log"

// initialize the daemon process and block signals except for terminate and hangup
void daemonize(void);

// realize the response to GET certain file
int getfile(char *route,int clifd,char *Iam);

// realize the function to response to get index
int getlist(int clifd,char *Iam);

// the entrance of serving the HTTP request
int service(int clifd);

// write information to the log file
void log_message(char *filename, char *message);

// deal with the terminate and hangup signal
void signal_handler(int sig);


#endif /* LIBHTTPSERVER_H_ */
