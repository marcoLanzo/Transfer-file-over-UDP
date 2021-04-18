/*
 Il file fileoperation.h contiene tutti quei metodi che vengono utilizzati per gestire i file. A secondo delle varie esigenze si è scelto di
 utilizzare operazioni di alto (eg.fopen, fclose, fgets, fseek, ftell, feof) o basso (eg. open, close, read, write) livello.
*/


//color codes
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
#define RESET "\033[0m"


/*
 nome funzione: openFile

 parametro1: path, tipo: const char*

 valore restituito: int
 
 La funzione openFile() preso come parametro il percorso di un file (path) in caso di successo ritorna il file descriptor associato. Il file
 descriptor è restituito dalla funzione open() che in caso le venga passato un percoso non valido restituisce -1 causando la
 terminazione del programma e la stampa di un opportuno messaggio di errore.

*/

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#define DATA_SIZE 1500
#define MAX_LINE_SIZE 64

int openFile(const char *path) {

	int fd;   

	errno = 0;
	fd = open(path, O_RDWR, 0666);	//O_RDWR = lettura e scrittura, O_CREAT = se non esiste lo creo
	if(fd == -1) {		
		printf("Errore in openFile()\n");
		exit(EXIT_FAILURE);
	}

	return fd;
}




/*
 nome funzione: closeFile

 parametro1: fd, tipo: int

 valore restituito: void
  
 La funzione closeFile permette di chiudere il file descriptor fd.
 In caso di errore della funzione close() si ha la terminazione del programma e la stampa di un opportuno messaggio di errore.

*/
void closeFile(int fd) {

	errno = 0;
	if(close(fd) == -1) {
			exit(EXIT_FAILURE);
	}
}




/*
 nome funzione: readFile

 parametro1: fd, tipo: int

 parametro2: buf, tipo: char*

 valore restituito: int
  
 La funzione readFile permette di leggere dal file riferito dal file descriptor fd al più DATA_SIZE byte e di salvarli nel buffer buf. 
 In caso di successo restituisce il numero di byte letti.
 In caso di errore della funzione read() si ha la terminazione del programma e la stampa di un opportuno messaggio di errore.

*/
int readFile(int fd, char *buf) {

	unsigned long r;
	ssize_t v;
		
	r = 0;
	while(r < DATA_SIZE) {
		errno = 0;
		v = read(fd, buf, DATA_SIZE - r);
		if(v == -1) {
			//perror("Errore in read()\n");
			exit(EXIT_FAILURE);
		}
		if(v == 0)
			return r;

		r += v;
		buf += v;
	}

	return r;
}




/*
 nome funzione: writeFile

 parametro1: fd, tipo: int

 parametro2: buf, tipo: char*

 valore restituito: void
  
 La funzione writeFile() permette di scrivere nel file riferito dal file descriptor fd dim bytes del buffer buf, dove dim rappresenta il
 numero di bytes che compongono il buffer buf. 
 In caso di errore della funzione read() si ha la terminazione del programma e la stampa di un opportuno messaggio di errore.

*/
void writeFile(int fd, char *buf) {

	ssize_t v, dim;
	dim = strlen(buf);
	
	while(dim > 0) {
		errno = 0;
		v = write(fd, buf, dim);

		if(v == -1) {
			exit(EXIT_FAILURE);
		}
		if(v == 0)   		//zero indica che sono stati scritti 0 byte 
			break;  
    
		dim -= v;		
		buf += v;
	}
}




/*
 nome funzione: getFileSize

 parametro1: path, tipo: const char*

 valore restituito: long
  
 Dato il percorso di un file passato come parametro (path), la funzione getFileSize() restituisce la dimensione del file intesa come numero di
 bytes. 
 In caso di errore delle funzioni fopen(), fseek(), ftell() ed fclose() si ha la terminazione del programma e la stampa di un opportuno
 messaggio di errore.

*/
long getFileSize(const char *path) {
	
	FILE *file = NULL;
	long file_size;

	file = fopen(path, "r");
	if(file == NULL){
		exit(EXIT_FAILURE);
	}

	if(fseek(file, 0L, SEEK_END) == -1){		//fseek() permette di modificare l'indicatore di posizone del file
		exit(EXIT_FAILURE);
	}

	file_size = ftell(file); 			//ftell() restituisce la dimensione del file in byte
	if(file_size == -1){
		exit(EXIT_FAILURE);
	}

	fseek(file, 0L, SEEK_SET);

	if(fclose(file) != 0){
		exit(EXIT_FAILURE);
	}

	return file_size;
}




/*
 nome funzione: readLine

 parametro1:buf, tipo: char*

 parametro2:f, tipo: FILE*

 valore restituito: char*
  
 Dato lo stream f associato ad un certo file, la funzione readLine() permette di leggere al più MAX_LINE_SIZE bytes da tale stream e
 di scriverli tramite l'uso della funzione fgets() nel buffer buf che poi viene restituito. Se non c'è nulla da leggere la 
 funzione restituisce NULL.
 In caso di errore della funzione fgets() si ha la terminazione del programma e la stampa di un opportuno  messaggio di errore.

*/
char* readLine(char *buf, FILE *f) {

	if(fgets(buf, MAX_LINE_SIZE, f) == NULL){
		if(!feof(f)){					//lo stream è stato chiuso
			exit(EXIT_FAILURE);
		}
        	else 	
			return NULL;				//non c'è nulla da leggere
	}
	
	if(buf[strlen(buf) - 1] == '\n'){			//aggiungo terminatore
		buf[strlen(buf) - 1] = '\0';
	}
	return buf;	
}




/*
 nome funzione: checkFileName

 parametro1: filename, tipo: char*

 parametro2: path_list, tipo: char*

 valore restituito: bool

 La funzione checkFileName() permette di verificare se la stringa filename occorre nel file con percorso path_list ed in tal caso resituisce
 true.
 In caso di errore delle funzioni fopen(), fgets(), fclose() si ha la terminazione del programma e la stampa di un opportuno  messaggio di
 errore.

*/
bool checkFileName(char* filename, char* path_list){ 
	
	FILE *list = NULL;			
	list = fopen(path_list, "r");
	if(list == NULL){

		exit(EXIT_FAILURE);
	}	

	bool cond = false;
	char *buf;

	buf = malloc(MAX_LINE_SIZE);
	if(!buf) {
		exit(EXIT_FAILURE);
	}
	
	for(;;) {
		buf = readLine(buf, list);
		if(!buf)	
			break;
		if(strncmp(filename, buf, strlen(filename) ) == 0){
			cond=true;
			break;
		}
	}
	free(buf);

	if(fclose(list) != 0){
		//fprintf(stderr, "Errore in fclose()\n");
		exit(EXIT_FAILURE);
	}

	return cond;
}
char* obtain_path(char*file_path,char*token,char* cmd){

    file_path = malloc(sizeof(char)*BUFLEN);
    if (getcwd(file_path, sizeof(file_path)*BUFLEN) != NULL) {
    } else {
        perror("getcwd() error");
        exit(EXIT_FAILURE);
    }
	//printf("file-path -> %s\n", file_path);
    char *ritorno;
    ritorno=malloc(sizeof(char)*BUFLEN);
    ritorno=strstr(file_path,"cmake-build-debug");
    if(ritorno!=NULL) { // Path da Clion
        char *assolute_path = malloc(sizeof(char) * BUFLEN);
        memset(assolute_path, '\0', BUFLEN);
        int lenpath = strlen(file_path);
        strncpy(assolute_path, file_path, lenpath - 17);// 17 sta per cmake-build-debug;
        if (strncmp(cmd, "get", 4) == 0)
            strcat(assolute_path, "Server/Server_file/");
        else if (strncmp(cmd, "put", 4) == 0)
            strcat(assolute_path, "Server/Server_file/");
        else if (strncmp(cmd, "list", 5) == 0)
            strcat(assolute_path, "Server/");
        memset(file_path, '\0', BUFLEN);
        strcpy(file_path, assolute_path);
        //free(assolute_path);
        strcat(file_path, token);
        //printf("Current_path -> %s\n", file_path);
    }else { // Path da terminale
	//	printf("file-path -> %s\n", file_path);
        if (strncmp(cmd, "get", 4) == 0)
            strcat(file_path, "/Server_file/");
        else if (strncmp(cmd, "put", 4) == 0)
            strcat(file_path, "/Server_file/");
		else if (strncmp(cmd, "list", 5) == 0){
			strcat(file_path,"/");
		}
		strcat(file_path, token);
		//printf("file-path -> %s\n", file_path);
    }

    return file_path;
}


void aggiorna_lista(char* disponibili){
    char* filepath;
    filepath = malloc(sizeof(char) * BUFLEN);
    filepath = obtain_path( filepath,"Lista.txt", "list");
	FILE* fd;
	errno=0;
	fd=fopen(filepath,"a+");
	if(errno!=0){
		fprintf(stderr,"errore nell'apertura del file\n");
		exit(-1);
	}
	strcat(disponibili,"\n");
	fprintf(fd,"%s",disponibili);
	//fwrite(disponibili,sizeof(char),strlen(disponibili),fd);
	if(fclose(fd)){
		fprintf(stderr,"errore nella chiusura\n");
		exit(-1);
	}
	return;
}

char * lettura_elementi_lista() {
	char* filepath;
	filepath=malloc(sizeof(char)*BUFLEN);
	filepath = obtain_path( filepath,"Lista.txt", "list");
	ssize_t read;
	FILE *file;
	ssize_t len = 0;
	char *filelist;
	filelist=malloc(sizeof(char)*BUFLEN);
	file = fopen(filepath, "r+");
	if (file == NULL) {
		perror("Error function fopen");
		exit(-1);
	}
	char *segment = malloc(BUFLEN * sizeof(char *));
	int i = 0;
	while (fscanf(file, "%s", segment) != EOF) {
		strcat(filelist, segment);
		segment += strlen(segment) + 1;

	}

	return filelist;

}

bool controllo_se_esiste_in_lista(char * disponibili){
    int q;
    char*array_elem;
    array_elem=malloc(sizeof(char )*BUFLEN);
    array_elem=lettura_elementi_lista();
	char *ritorno;
	ritorno=malloc(sizeof(char)*BUFLEN);

	ritorno=strstr(array_elem,disponibili);
	if(ritorno!=NULL){
            return true;
	}


    return false;
}