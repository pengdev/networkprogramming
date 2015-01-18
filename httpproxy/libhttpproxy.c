/*
 * libhttpserver.c
 *
 *  Created on: 11.3.2012
 *      Author: liup1
 */
#include "libhttpproxy.h"
#include "libhttpclient.h"

void daemonize() {
	time_t now;
	char *time_string;
	int i, lfp;
	char str[100];
	time(&now);
	time_string = asctime(localtime(&now));
	*(time_string + 24) = '\0';
	if (getppid() == 1)
		return; /* already a daemon */
	i = fork();
	if (i < 0)
		exit(1); /* fork error */
	if (i > 0)
		exit(0); /* parent exits */
	/* child (daemon) continues */
	setsid(); /* obtain a new process group */
	for (i = getdtablesize(); i >= 0; --i)
		close(i); /* close all descriptors */
	//	i = open("/dev/null", O_RDWR);
	//	dup(i);
	//	dup(i); /* handle standart I/O */
	umask(027); /* set newly created file permissions */
	if (access(RUNNING_DIR, 0) != 0)
		mkdir(RUNNING_DIR, S_IRWXU);
	chdir(RUNNING_DIR);/* change running directory */
	lfp = open(LOCK_FILE, O_RDWR | O_CREAT, 0640);
	if (lfp < 0) {
		printf("open error\n");
		exit(1); /* can not open */
	}
	if (lockf(lfp, F_TLOCK, 0) < 0) {
		printf("lock error\n");
		exit(0); /* can not lock */
	}
	/* first instance continues */

	sprintf(str, "[%s]: Server daemon process created, pid= %d\n", time_string,
			getpid());
	write(lfp, str, strlen(str)); /* record pid to lockfile */
	signal(SIGCHLD, SIG_IGN); /* ignore child */
	signal(SIGTSTP, SIG_IGN); /* ignore tty signals */
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGHUP, signal_handler); /* catch hangup signal */
	signal(SIGTERM, signal_handler); /* catch kill signal */
}

int service(int clifd) {
	char *slen = NULL;
	char route[100] = { 0 };
	char Iam[100] = { 0 };
	char hopc[3] = { 0 };
	char length[100] = { 0 };
	char buf1[65536] = { 0 };
	char temp[80] = { 0 };
	struct stat s;
	int file_len = 0;
	int hopcount = 0;
	int mode = 88;
	int fg = 0;
	int i;
	int fd;
	FILE *fp = NULL;
	ssize_t rd;
	char *line = NULL;
	size_t len;

	while (read(clifd, buf1, 65536) != 0) {

		if (fg == 0) {
			//			log_message(LOG_FILE, buf1);
			if (strstr(buf1, "GET ")) {
				slen = strstr(buf1, "GET /") + strlen("GET /");
				for (i = 0; i < strlen(slen); i++) {
					if (*(slen + i) == ' ' && *(slen + i + 1) == 'H')
						break;
					route[i] = *(slen + i);
				}
				mode = 0;
				if (strstr(buf1, "GET /index")) {
					mode = 2;
				}
			}

			if (strstr(buf1, "PUT ")) {
				slen = strstr(buf1, "PUT /") + strlen("PUT /");
				for (i = 0; i < strlen(slen); i++) {
					if (*(slen + i) == ' ' && *(slen + i + 1) == 'H')
						break;
					route[i] = *(slen + i);
				}
				slen = strstr(buf1, "Content-Length: ") + strlen(
						"Content-Length: ");
				for (i = 0; i < strlen(slen); i++) {
					if (*(slen + i) == '\r' && *(slen + i + 1) == '\n')
						break;
					length[i] = *(slen + i);
				}
				len = atoi(length);
				//			log_message(LOG_FILE, "work here!!");
				mode = 1;
			}
			//			log_message(LOG_FILE, "work here!!");
			if (strstr(buf1, "Iam: ")) {
				slen = strstr(buf1, "Iam: ") + strlen("Iam: ");
				for (i = 0; i < strlen(slen); i++) {
					if (*(slen + i) == '\r' && *(slen + i + 1) == '\n')
						break;
					Iam[i] = *(slen + i);
				}
			}
			if (strstr(buf1, "Hop-Count: ")) {
				slen = strstr(buf1, "Hop-Count: ") + strlen("Hop-Count: ");
				for (i = 0; i < strlen(slen); i++) {
					if (*(slen + i) == '\r' && *(slen + i + 1) == '\n')
						break;
					hopc[i] = *(slen + i);
				}
				hopcount = atoi(hopc);
			} else {
				hopcount = 2;
			}
			//			log_message(LOG_FILE, "work here!!");
			if (strstr(buf1, "\r\n\r\n") == NULL) {
				bzero(buf1, sizeof(buf1));
				fg = 1;
				continue;
			}
			slen = strstr(buf1, "\r\n\r\n") + strlen("\r\n\r\n");
			//			log_message(LOG_FILE, "work here!!");
			switch (mode) {
			case 2:
				return getlist(clifd, Iam, 1);
			case 1:
				fd = open(route, O_RDWR | O_CREAT | O_NONBLOCK, S_IRWXU);
				if (fd < 0) {
					log_message(LOG_FILE, "open local file error");
					return -1;
				}
				if (write(fd, slen, strlen(slen)) != strlen(slen)) {
					log_message(LOG_FILE, "partial/failed write");
					return -1;
				}
				bzero(buf1, sizeof(buf1));
				fg = 1;
				break;
			case 0:
				return getfile(route, clifd, Iam, hopcount);
			default:
				sprintf(temp, "illegal request from %s!", Iam);
				log_message(LOG_FILE, temp);
				return -1;

				//			sprintf(temp, "Iam %s", Iam);
				//			log_message(LOG_FILE, temp);
				//			sprintf(temp, "route %s", route);
				//			log_message(LOG_FILE, temp);
				//			sprintf(temp, "length %d", len);
				//			log_message(LOG_FILE, temp);

			}
		}

		if (write(fd, buf1, strlen(buf1)) != strlen(buf1)) {
			log_message(LOG_FILE, "partial/failed write");
			return -1;
		}
		bzero(buf1, sizeof(buf1));
		if (stat(route, &s) == 0) {
			file_len = (int) s.st_size;
		} else {
			log_message(LOG_FILE, "handle error");

		}
		if (len == file_len)
			break;
	}
	//////////////////////////////////
	hopcount = 1; //for forward http put ,hopcount=1
	fp = fopen("../routelist", "r");
	hopcount -= 1;
	while ((hopcount >= 0 && (rd = getline(&line, &len, fp)) != -1)) {
		memset(temp, 0, sizeof(temp));
		sprintf(temp, "put %s to %s for %s", route, line, Iam);
		log_message(LOG_FILE, temp);
		if (put_forward(line, strlen(line), Iam, route, hopcount, fd) != 0)
			log_message(LOG_FILE, "forward put error");

		//		if (redirect(line, strlen(line), route, clifd, hopcount, Iam) == 1) {
		//			close(clifd);
		//			fclose(fp);
		//			return 1;
		//		}
	}
	fclose(fp);

	/////////////////////////////////
	close(fd);
	close(clifd);
	sprintf(temp, "Get %d Bytes from %s successfully!", len, Iam);
	log_message(LOG_FILE, temp);
	return 0;
}

int getlist(int clifd, char *Iam, int hopcount) {
	DIR *d;
	FILE *fp = NULL;
	struct dirent *dirp;
	char *p = NULL;
	char *buf = NULL;
	char temp[80] = { 0 };
	char response[200] = { 0 };
	char rescon[1000] = { 0 };
	char buffer[10000] = { 0 };
	ssize_t rd;
	char *line = NULL;

	size_t len;
	d = opendir("./");
	p = rescon;
	while ((dirp = readdir(d)) != NULL) {
		strcpy(p, dirp -> d_name);
		p = p + strlen(dirp -> d_name);
		*p = '\n';
		p++;
	}
	p = '\0';

	fp = fopen("../routelist", "r");
	hopcount -= 1;
	buf = buffer;
	while ((hopcount >= 0 && (rd = getline(&line, &len, fp)) != -1)) {
		memset(temp, 0, sizeof(temp));
		sprintf(temp, "get index from %s for %s", line, Iam);
		log_message(LOG_FILE, temp);
		if (getlist_forward(line, strlen(line), Iam, hopcount, buf) != 0)
			log_message(LOG_FILE, "forward index error");
		buf = buffer + strlen(buffer);

		//		if (redirect(line, strlen(line), route, clifd, hopcount, Iam) == 1) {
		//			close(clifd);
		//			fclose(fp);
		//			return 1;
		//		}
	}
	fclose(fp);

	len = strlen(rescon) + strlen(buffer);
	sprintf(
			response,
			"HTTP/1.1 200 OK \r\nServer:LeosServer 1.0 \r\nContent-Type: text/plain \r\nIam: leosplan \r\nContent-Length: %d \r\n\r\n",
			len);
	if (write(clifd, response, strlen(response)) != strlen(response)) {
		log_message(LOG_FILE, "partial/failed write response");
		return -1;
	}
	if (write(clifd, rescon, strlen(rescon)) != strlen(rescon)) {
		log_message(LOG_FILE, "partial/failed write rescon");
		return -1;
	}
	if (write(clifd, buffer, strlen(buffer)) != strlen(buffer)) {
		log_message(LOG_FILE, "partial/failed write buffer");
		return -1;
	}
	sprintf(temp, "%s get expanded index successfully!", Iam);
	log_message(LOG_FILE, temp);
	return 0;
}

int getfile(char *route, int clifd, char *Iam, int hopcount) {
	char *buf = NULL;
	char response[200] = { 0 };
	int fd;
	struct stat s;
	int file_len;
	char temp[100] = { 0 };

	FILE *fp = NULL;
	size_t len = 0;
	char *line = NULL;
	ssize_t rd;

	fd = open(route, O_RDONLY, S_IRUSR); // | O_APPEND
	if (fd < 0) {
		log_message(LOG_FILE, "open local file error");
		/* try to find another proxy*/
		fp = fopen("../routelist", "r");
		hopcount -= 1;
		while ((hopcount >= 0 && (rd = getline(&line, &len, fp)) != -1)) {
			memset(temp, 0, sizeof(temp));
			sprintf(temp, "redirect to %s for %s", line, Iam);
			log_message(LOG_FILE, temp);
			len -= 1;
			if (redirect(line, strlen(line), route, clifd, hopcount, Iam) == 1) {
				close(clifd);
				fclose(fp);
				return 1;
			}
		}
		fclose(fp);
		memset(temp, 0, sizeof(temp));
		sprintf(temp, "do not find any file on the routelist for %s", Iam);
		log_message(LOG_FILE, temp);
		return show_nf(clifd);
	} else {
		if (stat(route, &s) == 0) {
			file_len = (int) s.st_size;
		} else {
			log_message(LOG_FILE, "handle error");
			/* handle error */
		}
		buf = (char *) malloc(file_len * sizeof(char));
		memset(buf, 0, sizeof(buf));
		memset(response, 0, sizeof(response));
		sprintf(
				response,
				"HTTP/1.1 200 OK \r\nServer:LeosServer 1.0 \r\nContent-Type: text/plain \r\nIam: leosplan \r\nContent-Length: %d \r\n\r\n",
				file_len);
		if (write(clifd, response, strlen(response)) != strlen(response)) {
			log_message(LOG_FILE, "partial/failed write socket");
			return -1;
		}
		while (read(fd, buf, file_len) != 0) {
			//		printf("%d\n", strlen(buf));
			//	printf("Received %ld bytes: %s\n", (long) nread, buf1);
			write(clifd, buf, file_len);
			bzero(buf, sizeof(buf));
		}
		sprintf(temp, "Put %d Bytes to %s successfully!", file_len, Iam);
		log_message(LOG_FILE, temp);
		close(fd);
		free(buf);
		return 0;
	}

}

void log_message(char *filename, char *message) {
	time_t now;
	time(&now);
	char *time_string;
	time_string = asctime(localtime(&now));
	*(time_string + 24) = '\0';
	FILE *logfile;
	logfile = fopen(filename, "a");
	if (!logfile)
		return;
	fprintf(logfile, "[%s]: %s\n", time_string, message);
	fclose(logfile);
}

void signal_handler(int sig) {
	switch (sig) {
	case SIGHUP:
		log_message(LOG_FILE, "hangup signal catched\n");
		break;
	case SIGTERM:
		log_message(LOG_FILE, "terminate signal catched\n");
		exit(0);
		break;
	}
}

int show_nf(int sockfd) {
	char
			responsebd[] =
					"<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n<html><head>\n<title>404 Not Found</title>\n</head><body>\n<h1>Not Found</h1>\n<p>The requested URL /favicon.ico was not found on this server.</p>\n<hr>\n<address>Leos Server at nwprog1.netlab.hut.fi Port 8888</address>\n</body></html>";
	char response[500] = { 0 };
	sprintf(
			response,
			"HTTP/1.1 404 Not Found \r\nServer:LeosServer 1.0 \r\nIam: leosplan \r\nContent-Length: %d \r\nContent-Type: text/html; charset=iso-8859-1 \r\nConnection: close \r\n\r\n",
			strlen(responsebd));
	if (write(sockfd, response, strlen(response)) != strlen(response)) {
		log_message(LOG_FILE, "partial/failed write not found response");
		close(sockfd);
		return -1;
	} else {
		if (write(sockfd, responsebd, strlen(responsebd)) != strlen(responsebd)) {
			log_message(LOG_FILE, "partial/failed write not found body");
			close(sockfd);
			return -1;
		} else {
			close(sockfd);
			return 0;
		}
	}
}
