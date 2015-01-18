/*
 * libhttpserver.c
 *
 *  Created on: 11.3.2012
 *      Author: liup1
 */
#include "libhttpserver.h"

void daemonize() {
	time_t now;
	char *time_string;
	int i, lfp;
	char str[100];
	// get the time stamp, and transfer it to the ascii time string
	time(&now);
	time_string = asctime(localtime(&now));
	*(time_string + 24) = '\0';

	if (getppid() == 1)
		return; // already a daemon
	i = fork();
	if (i < 0)
		exit(1); // fork error
	if (i > 0)
		exit(0); // parent exits

	// child (daemon) continues
	setsid(); //obtain a new process group
	for (i = getdtablesize(); i >= 0; --i)
		close(i); // close all descriptors
		i = open("/dev/null", O_RDWR);
		dup(i);
		dup(i); // handle standart I/O
	umask(027); // set newly created file permissions
	if (access(RUNNING_DIR, 0) != 0)
		mkdir(RUNNING_DIR, S_IRWXU );
	chdir(RUNNING_DIR); // change running directory
	lfp = open(LOCK_FILE, O_RDWR | O_CREAT, 0640);
	if (lfp < 0) {
		printf("open error\n");
		exit(1); // can not open
	}
	if (lockf(lfp, F_TLOCK, 0) < 0) {
		printf("lock error\n");
		exit(0); // can not lock
	}
	// first instance continues

	sprintf(str, "[%s]: Server daemon process created, pid= %d\n", time_string,
			getpid());
	write(lfp, str, strlen(str)); // record pid to lockfile
	signal(SIGCHLD, SIG_IGN); // ignore child
	signal(SIGTSTP, SIG_IGN); // ignore tty signals
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGHUP, signal_handler); // catch hangup signal
	signal(SIGTERM, signal_handler); // catch kill signal
}

int service(int clifd) {
	char *slen = NULL;
	char route[100] = { 0 };
	char Iam[100] = { 0 };
	char length[100] = { 0 };
	char buf1[65536] = { 0 };
	char temp[80] = { 0 };
	struct stat s;
	int file_len = 0;
	int mode = 88;
	int len;
	int fg = 0;
	int i;
	int fd;
	while (read(clifd, buf1, 65536) != 0) {
		// for the first buffered content, look up the header
		if (fg == 0) {
			// if received GET request, find and save filename
			if (strstr(buf1, "GET ")) {
				slen = strstr(buf1, "GET /") + strlen("GET /");
				for (i = 0; i < strlen(slen); i++) {
					if (*(slen + i) == ' ' && *(slen + i + 1) == 'H')
						break;
					route[i] = *(slen + i);
				}
				mode = 0;
				// for get index function
				if (strstr(buf1, "GET /index")) {
					mode = 2;
				}
			}
			// if received PUT request, find and save filename, content length
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
				mode = 1;
			}
			// find and save Iam header
			if (strstr(buf1, "Iam: ")) {
				slen = strstr(buf1, "Iam: ") + strlen("Iam: ");
				for (i = 0; i < strlen(slen); i++) {
					if (*(slen + i) == '\r' && *(slen + i + 1) == '\n')
						break;
					Iam[i] = *(slen + i);
				}
			}
			// move the pointer to the beginning of body
			slen = strstr(buf1, "\r\n\r\n") + strlen("\r\n\r\n");

			switch (mode) {
			case 2:
				// mode 2, get index
				return getlist(clifd, Iam);
			case 1:
				// mode 1, PUT request received, save content to local disk
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
				// mode 0, GET a specified file
				return getfile(route, clifd, Iam);
			default:
				// default, illegal request
				sprintf(temp, "illegal request from %s!", Iam);
				log_message(LOG_FILE, temp);
				return -1;

			}
		}
		// finalize the rest PUT function
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
	close(fd);
	close(clifd);
	sprintf(temp, "Get %d Bytes from %s successfully!", len, Iam);
	log_message(LOG_FILE, temp);
	return 0;
}

int getlist(int clifd, char *Iam) {
	DIR *d;
	struct dirent *dirp;
	char *p = NULL;
	char temp[80] = { 0 };
	char response[200] = { 0 };
	char rescon[10000] = { 0 };
	int len;
	// open the local directory
	d = opendir("./");
	p = rescon;
	// save the local file list to rescon
	while ((dirp = readdir(d)) != NULL) {
		strcpy(p, dirp -> d_name);
		p = p + strlen(dirp -> d_name);
		*p = '\n';
		p++;
	}
	p = '\0';
	len = strlen(rescon);
	// send response header
	sprintf(
			response,
			"HTTP/1.1 200 OK \r\nServer:LeosServer 1.0 \r\nContent-Type: text/plain \r\nIam: leosplan \r\nContent-Length: %d \r\n\r\n",
			len);
	if (write(clifd, response, strlen(response)) != strlen(response)) {
		log_message(LOG_FILE, "partial/failed write");
		return -1;
	}
	// send response body(rescon)
	if (write(clifd, rescon, strlen(rescon)) != len) {
		log_message(LOG_FILE, "partial/failed write");
		return -1;
	}
	sprintf(temp, "%s get index successfully!", Iam);
	log_message(LOG_FILE, temp);

	return 0;
}

int getfile(char *route, int clifd, char *Iam) {
	char *buf = NULL;
	char response[200] = { 0 };
	int fd;
	struct stat s;
	int file_len;
	char temp[80] = { 0 };
	// open local file
	fd = open(route, O_RDONLY, S_IRUSR);
	if (fd < 0) {
		log_message(LOG_FILE, "open local file error");
		return -1;
	}
	// get local file length
	if (stat(route, &s) == 0) {
		file_len = (int) s.st_size;
	} else {
		log_message(LOG_FILE, "handle error");
		/* handle error */
	}
	buf = (char *) malloc(file_len * sizeof(char));
	memset(buf, 0, sizeof(buf));
	memset(response, 0, sizeof(response));
	// send response header
	sprintf(
			response,
			"HTTP/1.1 200 OK \r\nServer:LeosServer 1.0 \r\nContent-Type: text/plain \r\nIam: leosplan \r\nContent-Length: %d \r\n\r\n",
			file_len);
	if (write(clifd, response, strlen(response)) != strlen(response)) {
		log_message(LOG_FILE, "partial/failed write");
		return -1;
	}
	// send body
	if (read(fd, buf, file_len) != file_len) {
		log_message(LOG_FILE, "read local file error");
		return -1;
	}
	write(clifd, buf, file_len);
	sprintf(temp, "Put %d Bytes to %s successfully!", file_len, Iam);
	log_message(LOG_FILE, temp);
	close(fd);
	free(buf);
	return 0;

}

void log_message(char *filename, char *message) {
	time_t now;
	char *time_string;
	// get time string
	time(&now);
	time_string = asctime(localtime(&now));
	*(time_string + 24) = '\0';
	FILE *logfile;
	// open file to write log information
	logfile = fopen(filename, "a");
	if (!logfile)
		return;
	// save log information to LOGFILE as a certain format
	fprintf(logfile, "[%s]: %s\n", time_string, message);
	fclose(logfile);
}

void signal_handler(int sig) {
	switch (sig) {
	case SIGHUP:
		// if hangup signal catched
		log_message(LOG_FILE, "hangup signal catched\n");
		break;
	case SIGTERM:
		// if terminate signal catched
		log_message(LOG_FILE, "terminate signal catched\n");
		exit(0);
		break;
	}
}
