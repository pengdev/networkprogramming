/*
 * httpserver.c
 *
 *  Created on: 8.3.2012
 *      Author: liup1
 */


#include "libhttpserver.h"

int main(int argc, char *argv[]) {
	int sockfd, clifd, total;
	struct sockaddr_in servaddr;
	pid_t pid;
	int port=8888;
	total = 0;
	// create daemon process
	if(argc==2){
		port=atoi(argv[1]);
	}
	daemonize();
	log_message(LOG_FILE, "Server initinalization done");

	// create socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		log_message(LOG_FILE, "socket error");
		return 1;
	}

	// specify server address: port 8888, don't care about local IP address
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
		log_message(LOG_FILE, "bind error");
		return -1;
	}

	// Tell socket that it is passive, and listens incoming connections
	if (listen(sockfd, 5) < 0) {
		log_message(LOG_FILE, "listen error");
		return -1;
	}

	while (1) {
		// Wait until a TCP connection request comes in
		if ((clifd = accept(sockfd, NULL, NULL)) < 0) {
			log_message(LOG_FILE, "accept error");
			return -1;
		}
		//when request received, fork child process for the service
		if ((pid = fork()) < 0) {
			log_message(LOG_FILE,  "fork error");
		} else if (pid == 0) {
			//child process, service is called
			if (service(clifd) != 0) {
				log_message(LOG_FILE, "service error ");
				return -1;
			}

			return 0;
		} else {
			sleep(1); /* run */
		}
	}
	return 0;
}
