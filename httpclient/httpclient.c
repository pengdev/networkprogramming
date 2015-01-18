/*
 * libhttpclient.c
 *
 *  Created on: 9.2.2012
 *      Author: liup1
 */

#include "libhttpclient.h"

/*main function starts */
int main(int argc, char *argv[]) {
	int sockfd;
	int len;
	int i, j;
	char host[128] = { 0 };
	char port[100] = { 0 };
	char route[100] = { 0 };
	char fname[100] = { 0 };
	/* the argument number should be 2 or 3,otherwise print usage infornmation*/
	if (argc != 2 && argc != 3) {
		fprintf(
				stderr,
				"Usage:\n\t%s hostname:portnumber/<route> \n\t\t to get a file OR\n\t%s hostname:portnumber/<route> <localfile route>\n\t\t to put a file\n",
				argv[0], argv[0]);
		return -1;
	}
	/* put the second argument into the host buffer*/
	strcpy(host, argv[1]);
	len = strlen(host);
	/* scan to ':' and clear the chars besides host string in the host buffer
	 * and put the string starts from port number to the port buffer */
	for (i = 0; i < len; i++) {
		if (host[i] == ':') {
			host[i] = '\0';
			i++;
			for (j = 0; i < len; i++, j++) {
				port[j] = host[i];
				host[i] = 0;
			}
			port[j + 1] = '\0';
			break;
		}
	}
	len = strlen(port);
	/* scan to first '/' and clear the chars beside port number in the port buffer
	 * and put the string starts from first '/' to the route buffer */
	for (i = 0; i < len; i++) {
		if (port[i] == '/') {
			port[i] = '\0';
			i++;
			route[0] = '/';
			for (j = 1; i < len; i++, j++) {
				route[j] = port[i];
				port[i] = 0;
			}
			route[j + 1] = '\0';
			break;
		}
	}
	len = strlen(route);
	/* scan back to the last '/' in the route buffer
	 * and put the string starts from first '/' to the fname buffer*/
	for (i = len - 1; i >= 0; i--) {
		if (route[i] == '/') {
			i++;
			for (j = 0; i < len; i++, j++) {
				fname[j] = route[i];
			}
			fname[j + 1] = '\0';
			break;
		}
	}
	/* resolve the host name and establish the TCP connection to server*/
	sockfd = tcp_connect(host, port);
	if (sockfd >= 0) {
		printf("Connection established successfully!\n");
	} else {
		return -1;
	}
	/* if the argument format is for get function, call the function httpget*/
	if (argc == 2) {
		if (httpget(sockfd, host, route, fname) != 0) {
			fprintf(stderr, "HTTP GET ERROR！");
			return -1;
		}
	}
	/* if the argument format is for put function, call the function httpput*/
	if (argc == 3) {
		if (httpput(sockfd, host, route, argv[2]) != 0) {
			fprintf(stderr, "HTTP PUT ERROR！");
			return -1;
		}
	}
	return 0;
}
