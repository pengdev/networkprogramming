/*
 * libhttpclient.c
 *
 *  Created on: 9.2.2012
 *      Author: liup1
 */

#include "libhttpclient.h"
#include "libhttpproxy.h"

int getlist_forward(char *argv, int arglen, char *Iam, int hopcount, char *buf) {
	int sockfd;
	int len;
	int i, j;
	int stop = 0;
	int fg = 0;

	int stopcounter = 0;
	char host[128] = { 0 };
	char port[100] = { 0 };

	char buf1[65535] = { 0 };
	char getreq[200] = { 0 };
	char *slen = NULL;
	char length[100] = { 0 };

	strncpy(host, argv, arglen);
	len = strlen(host);
	for (i = 0; i < len; i++) {
		if (host[i] == ':') {
			host[i] = '\0';
			i++;
			for (j = 0; i < len; i++, j++) {
				if (host[i] == '\n')
					port[i] = '\0';
				else
					port[j] = host[i];
				host[i] = 0;

			}
			port[j + 1] = '\0';
			break;
		}
	}

	//			memset(temp, 0, sizeof(temp));
	//			sprintf(temp, "%s %d", host, strlen(host));
	//			log_message(LOG_FILE, temp);
	//			memset(temp, 0, sizeof(temp));
	//			sprintf(temp, "%s %d", port, strlen(port));
	//			log_message(LOG_FILE, temp);
	//	printf("%s %d\n", route, strlen(route));
	//	printf("%s %d\n", fname, strlen(fname));

	sockfd = tcp_connect(host, port);
	if (sockfd >= 0) {
		log_message(LOG_FILE, "Connection established successfully!");
	} else {
		return -1;
	}
	sprintf(
			getreq,
			"GET /index HTTP/1.1\r\nHOST:%s \r\nIam: leosplan \r\nHop-Count: %d\r\n\r\n",
			host, hopcount);
	//	printf("%s", getreq);
	if (write(sockfd, getreq, strlen(getreq)) != strlen(getreq)) {
		log_message(LOG_FILE, "error forward get index request");
		return -1;
	}

	while (read(sockfd, buf1, 65535) != 0) {
		if (fg == 0) {

			slen = strstr(buf1, "Content-") + strlen("Content-Length: ");

			for (i = 0; i < strlen(slen); i++) {
				if (*(slen + i) == '\r' && *(slen + i + 1) == '\n')
					break;
				length[i] = *(slen + i);
			}
			len = atoi(length);

			slen = strstr(buf1, "HTTP/") + strlen("HTTP/1.1 ");
			if (*slen == '2')
				stop = 0;
			else if (*slen == '4') {
				stop = 1;
				return 0;
			}
			if (strstr(buf1, "\r\n\r\n") == NULL) {
				bzero(buf1, sizeof(buf1));
				fg = 1;
				continue;
			}
			slen = strstr(buf1, "\r\n\r\n") + strlen("\r\n\r\n");
			strncpy(buf, slen, strlen(slen));
			buf += strlen(slen);
			stopcounter += strlen(slen);
			bzero(buf1, sizeof(buf1));
			fg = 1;
			continue;
		}
		strncpy(buf, buf1, strlen(buf1));
		buf += strlen(buf1);
		stopcounter += strlen(buf1);
		if (stopcounter > len - 2)
			return 0;

	}
	return 0;

}

int redirect(char *argv, int arglen, char *route, int clifd, int hopcount,
		char *Iam) {
	int sockfd;
	int len;
	int i, j;
	char host[128] = { 0 };
	char port[100] = { 0 };

	strncpy(host, argv, arglen);
	len = strlen(host);
	for (i = 0; i < len; i++) {
		if (host[i] == ':') {
			host[i] = '\0';
			i++;
			for (j = 0; i < len; i++, j++) {
				if (host[i] == '\n')
					port[i] = '\0';
				else
					port[j] = host[i];
				host[i] = 0;

			}
			port[j + 1] = '\0';
			break;
		}
	}
	//
	//		memset(temp, 0, sizeof(temp));
	//		sprintf(temp, "%s %d", host, strlen(host));
	//		log_message(LOG_FILE, temp);
	//		memset(temp, 0, sizeof(temp));
	//		sprintf(temp, "%s %d", port, strlen(port));
	//		log_message(LOG_FILE, temp);
	//	printf("%s %d\n", route, strlen(route));
	//	printf("%s %d\n", fname, strlen(fname));

	sockfd = tcp_connect(host, port);
	if (sockfd >= 0) {
		log_message(LOG_FILE, "Connection established successfully!");
	} else {
		return -1;
	}
	switch (httpget(sockfd, host, route, route, clifd, hopcount, Iam)) {
	case 0:
		return 1; // get content successfully
		break;
	case 1:
		return -1; // have not found
		break;
	default:
		log_message(LOG_FILE, "HTTP GET ERROR！");
		break;
	}
	return -1; // default return error
}

int tcp_connect(const char *host, const char *serv) {

	int sockfd, n;
	struct addrinfo hints, *res, *ressave;
	char temp[100] = { 0 };

	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((n = getaddrinfo(host, serv, &hints, &res)) != 0) {
		memset(temp, 0, sizeof(temp));
		sprintf(temp, "tcp_connect error for %s, %s: %s", host, serv,
				gai_strerror(n));
		log_message(LOG_FILE, temp);
		return -1;
	}
	ressave = res;

	do {
		sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (sockfd < 0)
			continue; /* ignore this one */

		if (connect(sockfd, res->ai_addr, res->ai_addrlen) == 0)
			break; /* success */

		close(sockfd); /* ignore this one */
	} while ((res = res->ai_next) != NULL);

	if (res == NULL) { /* errno set from final connect() */
		memset(temp, 0, sizeof(temp));
		sprintf(temp, "tcp_connect error for %s, %s", host, serv);
		log_message(LOG_FILE, temp);
		sockfd = -1;
	} else {
		//	struct sockaddr_in *sin = (struct sockaddr_in *) res->ai_addr;

	}

	freeaddrinfo(ressave);
	return (sockfd);
}

int httpget(int sockfd, const char *host, const char *route, const char *fname,
		int clifd, int hopcount, char *Iam) {
	char buf1[65535] = { 0 };
	char getreq[200] = { 0 };
	char *slen = NULL;
	char length[100] = { 0 };
	int stop = 0;
	struct stat s;
	int file_len;
	int len;
	int fd;
	int i;
	int fg = 0;
	char temp[100] = { 0 };
	sprintf(
			getreq,
			"GET /%s HTTP/1.1\r\nHOST:%s \r\nIam: leosplan \r\nHop-Count: %d\r\n\r\n",
			route, host, hopcount);
	//	printf("%s", getreq);
	if (write(sockfd, getreq, strlen(getreq)) != strlen(getreq)) {
		log_message(LOG_FILE, "error forward http get request");
		return -1;
	}

	while (read(sockfd, buf1, 65536) != 0) {

		if (fg == 0) {
			slen = strstr(buf1, "Content-") + strlen("Content-Length: ");
			for (i = 0; i < strlen(slen); i++) {
				if (*(slen + i) == '\r' && *(slen + i + 1) == '\n')
					break;
				length[i] = *(slen + i);
			}
			len = atoi(length);
			slen = strstr(buf1, "HTTP/") + strlen("HTTP/1.1 ");
			if (*slen == '2')
				stop = 0;
			else if (*slen == '4') {
				stop = 1;
				break;
			}
			if (strstr(buf1, "\r\n\r\n") == NULL) {
				fg = 1;
				bzero(buf1, sizeof(buf1));
				continue;
			}
			slen = strstr(buf1, "\r\n\r\n") + strlen("\r\n\r\n");
			fd = open(fname, O_RDWR | O_CREAT | O_NONBLOCK, S_IRWXU); // | O_APPEND
			if (fd < 0) {
				log_message(LOG_FILE, "create file error");
				return -1;
			}
			write(fd, slen, strlen(slen));
			write(clifd, slen, strlen(slen));
			bzero(buf1, sizeof(buf1));
			fg = 1;
			continue;
		}

		write(fd, buf1, strlen(buf1));
		write(clifd, buf1, strlen(buf1));
		bzero(buf1, sizeof(buf1));
		if (stat(fname, &s) == 0) {
			file_len = (int) s.st_size;
		} else {
			log_message(LOG_FILE, "handle error");
			/* handle error */
		}
		if (len == file_len)
			break;

	}
	close(fd);
	if (stop == 1) {
		memset(temp, 0, sizeof(temp));
		sprintf(temp, "Do not find %s for %s from %s ", route, Iam, host);
		log_message(LOG_FILE, temp);
	} else if (stop == 0) {
		memset(temp, 0, sizeof(temp));
		sprintf(temp, "Get %s for %s from %s successfully", route, Iam, host);
		log_message(LOG_FILE, temp);
	}
	return stop;
}
int put_forward(char *argv, int arglen, char *Iam, char *route, int hopcount,
		int fd) {
	int sockfd;
	int len;
	int i, j;
	char host[128] = { 0 };
	char port[100] = { 0 };

	strncpy(host, argv, arglen);
	len = strlen(host);
	for (i = 0; i < len; i++) {
		if (host[i] == ':') {
			host[i] = '\0';
			i++;
			for (j = 0; i < len; i++, j++) {
				if (host[i] == '\n')
					port[i] = '\0';
				else
					port[j] = host[i];
				host[i] = 0;

			}
			port[j + 1] = '\0';
			break;
		}
	}
	//
	//		memset(temp, 0, sizeof(temp));
	//		sprintf(temp, "%s %d", host, strlen(host));
	//		log_message(LOG_FILE, temp);
	//		memset(temp, 0, sizeof(temp));
	//		sprintf(temp, "%s %d", port, strlen(port));
	//		log_message(LOG_FILE, temp);
	//	printf("%s %d\n", route, strlen(route));
	//	printf("%s %d\n", fname, strlen(fname));

	sockfd = tcp_connect(host, port);
	if (sockfd >= 0) {
		log_message(LOG_FILE, "Connection established successfully!");
	} else {
		return -1;
	}
	if (httpput(sockfd, host, route, hopcount, Iam, fd) != 0) {
		log_message(LOG_FILE, "forward put error");
		return -1;
	}
	return 0; // default return error

}
int httpput(int sockfd, const char *host, const char *route, int hopcount,
		char * Iam, int fd) {
	char *buf = NULL;
	char putreq[200] = { 0 };
	char temp[80] = { 0 };
	struct stat s;
	int file_len;

	if (stat(route, &s) == 0) {
		file_len = (int) s.st_size;
	} else {
		log_message(LOG_FILE, "handle error");
		/* handle error */
	}
	buf = (char *) malloc(file_len * sizeof(char));
	memset(buf, 0, sizeof(buf));
	memset(putreq, 0, sizeof(putreq));
	sprintf(
			putreq,
			"PUT %s HTTP/1.1\r\nHOST:%s \r\nContent-Type: text/plain \r\nIam: leosplan \r\nContent-Length: %d \r\nHop-Count: %d \r\n\r\n",
			route, host, file_len, hopcount);
	if (write(sockfd, putreq, strlen(putreq)) != strlen(putreq)) {
		log_message(LOG_FILE, "partial/failed write\n");
		return -1;
	}
	while (read(fd, buf, file_len) != 0) {
		write(sockfd, buf, file_len);
		bzero(buf, sizeof(buf));
	}
	memset(temp, 0, sizeof(temp));
	sprintf(temp, "Put %s to %s successfully!\n", route, host);
	log_message(LOG_FILE, temp);
	close(fd);
	free(buf);
	return 0;
}
