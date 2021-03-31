#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define HEADER "VERIFICA 2021.0"
#define ERR_MSG "VERIFICA 2021.0 404 Not found"
#define FILE_NAME "FileName"
#define NUM_TO_DRAW "NumberToDraw"

#define MAX_MSG 1024*1024

void errore(const char*, int);

int main (int argc, char* argv[]){
	if (argc != 5){
		printf("USAGE: %s IP_SERVER PORT_SERVER FILE_NAME NUM_TO_DRAW \n", argv[0]);
		return -1;
	}

	char* ip = argv[1];
	int port = atoi(argv[2]);
	char* fileName = argv[3];
	int nToDraw = atoi(argv[4]);

	int socketId = socket(AF_INET, SOCK_STREAM, 0);
	int addrLenght = sizeof(struct sockaddr);

	struct sockaddr_in server;

	server.sin_family = AF_INET;
	inet_aton(ip, &server.sin_addr);
	server.sin_port = htons(port);
	for(int i = 0; i < 8;i++)
		server.sin_zero[i]=0;

	int rc;

	rc = connect(socketId, (struct sockaddr*)&server, (socklen_t)addrLenght);
	if(rc) errore("connect()", -2);

	char buffer[MAX_MSG + 1];
	sprintf(buffer, "%s %s:%s %s:%d", HEADER, FILE_NAME, fileName, NUM_TO_DRAW, nToDraw);
	int bufferLenght = strlen(buffer);

	rc = send(socketId, buffer, bufferLenght, 0);
	if(rc != bufferLenght) errore("send()", -3);

	rc = recv(socketId, buffer, MAX_MSG, 0);
	if(rc < 0) errore("recv()", -4);
	buffer[rc] = '\0';

	if(strcmp(buffer, ERR_MSG) == 0){
		printf("Failed request for class: %s\n", fileName);
		printf("Error: %s\n", (strstr(buffer, HEADER) + strlen(HEADER) + 1));
		return -5;
	}

	printf("1. %s\n", buffer);

	for(int i = 1; i < nToDraw; i++){
		rc = recv(socketId, buffer, MAX_MSG, 0);
		if(rc < 0) errore("recv()", -6);
		buffer[rc] = '\0';
		printf("%d. %s\n", i+1, buffer);
	}

	close(socketId);

	return 0;
}

void errore(const char* s, int n) {
	printf("Error in %s\n", s);
	printf("ERRNO: %d - %s\n", errno, strerror(errno));
	printf("Return code: %d\n", n);
	exit(n);
}