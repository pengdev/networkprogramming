/*
 * libhttpclient.c
 *
 *  Created on: 9.2.2012
 *      Author: liup1
 */

#include "libhttpclient.h"

/* resolve host name to IP address, establish TCP connection with server*/
int tcp_connect(const char *host, const char *serv) {
	int sockfd, n;
	struct addrinfo hints, *res, *ressave;
	char outbuf[80];
	/* clear the structure in case of memory check error*/
	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC; /* support both IPv4 and IPv6*/
	hints.ai_socktype = SOCK_STREAM;
	/* call the function getaddrinfo to resolve host name to IP address*/
	if ((n = getaddrinfo(host, serv, &hints, &res)) != 0) {
		fprintf(stderr, "tcp_connect error for %s, %s: %s\n", host, serv,
				gai_strerror(n));
		return -1;
	}
	ressave = res;
	do {
		/* open a socket to the certain port on the host*/
		sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (sockfd < 0)
			continue;
		/* establish TCP connection with the server through the socket*/
		if (connect(sockfd, res->ai_addr, res->ai_addrlen) == 0)
			break;
		close(sockfd);
	} while ((res = res->ai_next) != NULL);
	if (res == NULL) {
		fprintf(stderr, "tcp_connect error for %s, %s\n", host, serv);
		sockfd = -1;
	} else {
		/* get the IP address in the addrinfo structure and print the Server's IP address*/
		struct sockaddr_in *sin = (struct sockaddr_in *) res->ai_addr;
		const char *ret = inet_ntop(res->ai_family, &(sin->sin_addr), outbuf,
				sizeof(outbuf));
		printf("Server's IP address is %s\n", ret);
	}
	freeaddrinfo(ressave); /* free the addrinfo structure*/
	return (sockfd);
}
int httpget(int sockfd, const char *host, const char *route, const char *fname) {
	char buf1[65535] = { 0 }; /* set send receive buffer size 65535*/
	char getreq[200] = { 0 };
	char *slen = NULL;
	char length[100] = { 0 };
	struct stat s; /* stat structure to get the file length*/
	int file_len = 0;
	int len = 0;
	int fd;
	int i;
	int fg = 0;
	/* create a local file with the same file name on the server*/
	fd = open(fname, O_RDWR | O_CREAT | O_NONBLOCK, S_IRWXU);
	if (fd < 0) {
		fprintf(stderr, "open error\n");
		return -1;
	}
	/* print formated request message in a single string */
	sprintf(getreq, "GET %s HTTP/1.1\r\nHOST:%s \r\nIam: leosplan \r\n\r\n",
			route, host);
	/* write the request message to the socket*/
	if (write(sockfd, getreq, strlen(getreq)) != strlen(getreq)) {
		fprintf(stderr, "partial/failed write\n");
		return -1;
	}
	/* read the response message from server*/
	while (read(sockfd, buf1, 65536) != 0) {
		/* for the first time reading, search for the content length and where the body starts*/
		if (fg == 0) {
			/* get the content length in the header and store in value len*/
			slen = strstr(buf1, "Content-Length: ")
					+ strlen("Content-Length: ");
			for (i = 0; i < strlen(slen); i++) {
				if (*(slen + i) == '\r' && *(slen + i + 1) == '\n')
					break;
				length[i] = *(slen + i);
			}
			len = atoi(length);
			/* move pointer slen to where the body starts, and save the rest content in the buffer to local file*/
			slen = strstr(buf1, "\r\n\r\n") + strlen("\r\n\r\n");
			if (write(fd, slen, strlen(slen)) != strlen(slen)) {
				fprintf(stderr, "partial/failed write\n");
				return -1;
			}
			/* clear the buffer when write done*/
			bzero(buf1, sizeof(buf1));
			/* set flag, not check for header again */
			fg = 1;
		}
		/* read from the socket and save to local file, buffer size is 65535 at a time*/
		if (write(fd, buf1, strlen(buf1)) != strlen(buf1)) {
			fprintf(stderr, "partial/failed write\n");
			return -1;
		}
		/* clear buffer when write done*/
		bzero(buf1, sizeof(buf1));
		/* get the local file length*/
		if (stat(fname, &s) == 0) {
			file_len = (int) s.st_size;
		} else {
			perror("handle error");

		}
		/* compare if local file length equals the length indicated in the header, break ,quit the loop*/
		if (len == file_len)
			break;
	}
	close(fd);
	/* print byte number get from the server */
	printf("Get %d Bytes successfully!\n", len);
	return 0;
}

/* put local file to the server*/
int httpput(int sockfd, const char *host, const char *route, const char *file) {
	char *buf = NULL;/* buffer size is due to the local file size*/
	char putreq[200] = { 0 };
	int fd;
	struct stat s;
	int file_len = 0;
	/* open local file as read only*/
	fd = open(file, O_RDONLY, S_IRUSR);
	if (fd < 0) {
		fprintf(stderr, "open error\n");
		return -1;
	}
	/* calculate the file length of local file*/
	if (stat(file, &s) == 0) {
		file_len = (int) s.st_size;
	} else {
		perror("handle error");

	}
	/* memory allocation for the sending buffer, buffer size equals the file length*/
	buf = (char *) malloc(file_len * sizeof(char));
	/* clear buffers*/
	memset(buf, 0, sizeof(buf));
	memset(putreq, 0, sizeof(putreq));
	/* print formated request message header into a sigle string*/
	sprintf(
			putreq,
			"PUT %s HTTP/1.1\r\nHOST:%s \r\nContent-Type: text/plain \r\nIam: leosplan \r\nContent-Length: %d \r\n\r\n",
			route, host, file_len);
	/* send the put request header*/
	if (write(sockfd, putreq, strlen(putreq)) != strlen(putreq)) {
		fprintf(stderr, "partial/failed write\n");
		return -1;
	}
	/* read from the local file and write it into the socket as message body*/
	while (read(fd, buf, file_len) != 0) {
		if (write(sockfd, buf, file_len) != file_len) {
			fprintf(stderr, "partial/failed write\n");
			return -1;
		}
		bzero(buf, sizeof(buf));
	}
	/* print the number of bytes put to server*/
	printf("Put %d Bytes successfully!\n", file_len);
	close(fd);
	free(buf);/* free malloced memory*/
	return 0;
}
