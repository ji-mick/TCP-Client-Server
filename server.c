#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>

#define HEADER "VERIFICA 2021.0"
#define ERR_MSG "VERIFICA 2021.0 404 Not found"
#define FILE_NAME "FileName"
#define NUM_TO_DRAW "NumberToDraw"

#define MAX_MSG 1024*1024
#define LOOPBACK "127.0.0.1"
#define MAX_CON 50

typedef struct {
	int connectionId;
	struct sockaddr_in client;
} threadParams;

void* demon(void*);				// Errors: -10
int estraiAlunno(int);		
char* cercaAlunno(char*, int);
int contaRighe(char*);			// Errors: -20
void errore(const char*, int);

int main(int argc, char* argv[]) {
	if(argc != 2) {
		printf("USAGE: %s PORT\n", argv[0]);
		errore("input", -1);
	}

	int port = atoi(argv[1]);

	int socketId = socket(AF_INET, SOCK_STREAM, 0);
	int addrLenght = sizeof(struct sockaddr);

	struct sockaddr_in mySelf, client;

	mySelf.sin_family = AF_INET;
	inet_aton(LOOPBACK, &mySelf.sin_addr);
	mySelf.sin_port = htons(port);
	for(int i = 0; i < 8; i++)
		mySelf.sin_zero[i] = 0;

	int rc = bind(socketId, (struct sockaddr*) &mySelf, (socklen_t) addrLenght);
	if(rc) errore("bind()", -2);

	rc = listen(socketId, MAX_CON);
	if(rc) errore("listen()", -3);

	int connectionId = accept(socketId, (struct sockaddr*) &client, (socklen_t*) &addrLenght);
	if(connectionId < 0) errore("accept()", -4);

	threadParams params = {connectionId, client};
	pthread_t threadId;
	rc = pthread_create(&threadId, NULL, demon, (void*)&params);
	if(rc) errore("pthread_create()", -5);

	pthread_join(threadId, NULL);

	shutdown(connectionId, SHUT_RDWR);
	close(socketId);

	return 0;
}

void* demon(void* params) {
	threadParams* p = (threadParams*) params;

	int connectionId = p->connectionId;
	struct sockaddr_in client = p->client;

	int rc;

	char buffer[MAX_MSG + 1];

	rc = recv(connectionId, buffer, MAX_MSG, 0);									// buffer = VERIFICA 2021.0 FileName:5ainf NumberToDraw:7
	if(rc < 0) errore("recv()", -10);
	buffer[rc] = '\0'; 																// buffer = VERIFICA 2021.0 FileName:5ainf NumberToDraw:7\0

	char* fileName = strdup(strstr(buffer, FILE_NAME) + strlen(FILE_NAME) + 1); 	// fileName = 5ainf NumberToDraw:7\0

	/*
		char* strstr(char* STRINGA, char* SOTTOSTRINGA);
		Return value: 
			puntatore alla prima occorenza di SOTTOSTRINGA all'interno di STRINGA
	*/	
	*(strstr(fileName, NUM_TO_DRAW) - 1)= '\0';	
	printf("%s\n", fileName);									// fileName = 5ainf\0NumberToDraw:7\0

	// Controllo per file leggibile
	if(access(fileName, R_OK) != 0){
		rc = send(connectionId, ERR_MSG, strlen(ERR_MSG), 0);
		if(rc != strlen(ERR_MSG)) errore("send()", -12);
		return NULL;
	}

	int nToDraw = atoi(strstr(buffer, NUM_TO_DRAW) + strlen(NUM_TO_DRAW) + 1); 		// strstr -> 5ainf numStudenti:7\0
																					// atoi("7\0")															 		
																					// nToDraw = 2
	
	int alreadyDrawnedList[1000];
	int nRows = contaRighe(fileName); // free()
	int draw, j;

	srand(time(NULL));
	for(int i = 0; i < nToDraw; i++) {

		bool alreadyDrawned = true;
		while(alreadyDrawned) {

			alreadyDrawned = false;
			draw = estraiAlunno(nRows);
			printf("numero estratto:%d\n", draw);
			j = 0;

			while(!alreadyDrawned && j < nToDraw){

				if(alreadyDrawnedList[j] == draw)
					alreadyDrawned = true;

				j++;
			}			

		}


		alreadyDrawnedList[i] = draw;
		char* student = cercaAlunno(fileName, draw);
		printf("student = '%s'\n", student);
		rc = send(connectionId, student, strlen(student), 0);
		if(rc != strlen(student)) errore("send()", -11);
	}
	free(fileName);
	return NULL;
}

int estraiAlunno(int nRows) {
	return (rand() % nRows + 1);
}

char* cercaAlunno(char* fileName, int nToDraw) {
	char draw[MAX_MSG];
	char utility[MAX_MSG];
	int rowCount = 1;
	FILE* fp = fopen(fileName, "r");
	if (fp==NULL) {
        printf("Errore nell'apertura del file");
	    exit(1);
   	}

	while(!feof(fp)) {
		if(rowCount == nToDraw){
			fgets(draw, 1024, fp);
		}
		fgets(utility, 1024, fp);
		rowCount++;
	}
	
	fclose(fp);
	
	/*
		char* strdup(char* STRINGA)
		Return value:
			puntatore a una copia di STRINGA allocata dinamicamente tramite malloc()

		Si ritorna un strdup(draw) anzichè draw perchè l'ambito (scope) della variabile termina nel momento in cui la funzione giunge alla fine
		IMPORTANTE: ricordarsi di fare la free() alla fine
	*/ 
	return strdup(draw); // free()
}

int contaRighe(char* fileName){
	FILE* fp = fopen(fileName, "r");
	if(fp == NULL) errore("fopen()", -20);

	int rowCount = 1;

	while(!feof(fp)){
		if(getc(fp)=='\n') rowCount++;
	}

	fclose(fp);
	return rowCount;
}

void errore(const char* s, int n) {
	printf("Error in %s\n", s);
	printf("ERRNO: %d - %s\n", errno, strerror(errno));
	printf("Return code: %d\n", n);
	exit(n);
}

/*
Stateless (TS)		Funzione che a diverse chiamate restituisce risultati indipendenti uno dall'altro
Stateful (non TS)	Funzione che a diverse chiamate restituisce risultati dipendenti dalle sue chiamate precedenti perchè ha un suo stato interno
*/