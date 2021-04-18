//
// Created by marco96 on 23/10/18.
//

#ifndef IIW2_CLIENTFILEOPERATION_H
#define IIW2_CLIENTFILEOPERATION_H
 
#endif //IIW2_CLIENTFILEOPERATION_H

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
 Il file fileoperation.h contiene tutti quei metodi che vengono utilizzati per gestire i file. A secondo delle varie esigenze si è scelto di
 utilizzare operazioni di alto (eg.fopen, fclose, fgets, fseek, ftell, feof) o basso (eg. open, close, read, write) livello.
*/
 
 
 
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
#include <dirent.h>

#define DATA_SIZE 1500
#define MAX_LINE_SIZE 64
 
int openFile(const char *path) {
 
    int fd;
 
    errno = 0;
    fd = open(path, O_RDWR|O_CREAT|O_TRUNC, 0666);  //O_RDWR = lettura e scrittura, O_CREAT = se non esiste lo creo

    return fd;
}

int openFileForSending(const char *path) {

    int fd;

    errno = 0;
    fd = open(path, O_RDWR, 0666);  //O_RDWR = lettura e scrittura, O_CREAT = se non esiste lo creo
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
        if(v == 0)          //zero indica che sono stati scritti 0 byte
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
    long file_size = 0;
 
    file = fopen(path, "r+");
    if(file == NULL){
        exit(EXIT_FAILURE);
    }
 
    if(fseek(file, 0L, SEEK_END) == -1){        //fseek() permette di modificare l'indicatore di posizone del file
          exit(EXIT_FAILURE);
    }
 
    file_size = ftell(file);            //ftell() restituisce la dimensione del file in byte
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
        if(!feof(f)){                   //lo stream è stato chiuso
            //fprintf(stderr, "Errore in fgets()\n");
            exit(EXIT_FAILURE);
        }
        else
            return NULL;                //non c'è nulla da leggere
    }
 
    if(buf[strlen(buf) - 1] == '\n'){           //aggiungo terminatore
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
        //perror("Errore in fopen()\n");
        exit(EXIT_FAILURE);
    }
 
    bool cond = false;
    char *buf;
 
    buf = malloc(MAX_LINE_SIZE);
    if(!buf) {
        //fprintf(stderr,"Errore in malloc()\n");
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


/* Param1:void
 * Tipo di ritorno: char**
 * Descrizione : tale funzione ha il compito di allocare memoria
 */
char** alloc_memory(){
    char **buf = malloc(BUFLEN * sizeof(char*));
    if (buf == NULL){
        perror("Error function malloc");
        exit(-1);
    }
    return buf;
}

/* Param1(char * dirpathname) : pathname della directory Upload, ovvero della directory che contiene tutti i file che
 * il client può inviare al server.
 * Param2(int dirsize) : numero di elementi contenuti nella directory
 * Param3(char **content) : area di memoria destinata a contenere il nome di tutti i file presenti all'interno della
 * directory.
 * Tipo di ritorno : void
 * Descrizione: utilizzando il pathname della directory di  Upload, e il numero di elementi presenti, ottenuto con la
 * funzione getNumberOfElementsInDir(char *dirpathname), carichiamo in content tutti gli elementi contenuti al suo
 * interno.*/

char ** getContentDirectory(char *dirpathname,int dirsize){

    DIR *directory;
    char **content;
    struct dirent *de;
    int i=0;
    int j=0;
    int count;
    directory = opendir(dirpathname);
    printf("%s\n",dirpathname);
    content = alloc_memory();
    if(directory == NULL ){
        printf("Error in opening directory\n");
        exit(EXIT_FAILURE);
    }
    while((de = readdir(directory)) != NULL) {

        if(content == NULL){
            printf("Error in malloc \n");
            exit(EXIT_FAILURE);
        }
       content[j] = de->d_name;
       //printf("ELemento : %s\n",content[j]);
       j++;

    }
    return content;


}
/*Param1(char *dirpathname) : pathname della directory
 * Tipo di ritorno: int, numero di elementi contenuti nella directory.
 * Tale funzione accede alla directory tramite il dirpathname e con un semplice ciclo si fa restituire il numero
 * di elementi in essa contenuti.
 */
int getNumberOfElementsInDir(char *dirpathname){
    DIR *directory;
    struct dirent *de;
    int i=0;

    directory = opendir(dirpathname);
    if(directory == NULL ){
        printf("Error in opening directory\n");
        exit(EXIT_FAILURE);
    }
    /* Qui controllo numero di elementi presenti all'interno della cartella Upload*/
    while((de = readdir(directory)) != NULL){
        i = i+1;
    }
    return i;
}

/* Param1(char * filename) : nome del file per cui vogliamo sapere la presenza o meno in una directory.
 * Param2(char **dircontent) : lista di tutti gli elementi presenti nella directory
 * Param3(int dirsize) : numero di elementi presenti nella directory.
 * Tipo di ritorno :  bool
 * Descrizione: tale funzione verifica se il nome del file che si vuole caricare su server è davvero presente nella
 * cartella del client; restituisce quindi true se presente, false altrimenti.*/

bool checkFileInDirectory(char *filename,char **dircontent,int dirsize){

    bool trovato = false;
    int i;
    for(i=0;i<dirsize;i++){
        if(strcmp(filename,dircontent[i]) == 0){
     //       printf("%s = %s \n ",filename,dircontent[i]);
            trovato = true;
            break;
        }
        else{
            trovato = false;
        }
    }
    return trovato;
}

char * obtain_path(char*filepath, char*token, char* cmd){
    filepath = malloc(sizeof(char)*BUFLEN);
    if (getcwd(filepath, sizeof(char)*BUFLEN) != NULL) {

    } else {
        perror("getcwd() error");
        exit(EXIT_FAILURE);
    }

    char *ritorno;
    ritorno=malloc(sizeof(char)*BUFLEN);
    ritorno=strstr(filepath,"cmake-build-debug");
    if(ritorno!=NULL) { // Path for Clion

        char *assolute_path = malloc(sizeof(char) * BUFLEN);
        memset(assolute_path, '\0', strlen(assolute_path));
        int lenpath = strlen(filepath);
        strncpy(assolute_path, filepath, lenpath - 17);// 17 sta per cmake-build-debug
        if (strncmp(cmd, "get", 4) == 0)
            strcat(assolute_path, "Client/Download/");
        else if (strncmp(cmd, "put", 4) == 0)
            strcat(assolute_path, "Client/Upload/");
        else if (strncmp(cmd, "list", 5) == 0)
            strcat(assolute_path, "Server/");
        memset(filepath, '\0', BUFLEN);
        strcpy(filepath, assolute_path);
        memset(assolute_path, 0, BUFLEN);

        strcat(filepath,token);
        printf("Current path -> %s\n",filepath);
    }else { // path for terminale
        if (strncmp(cmd, "get", 4) == 0)
            strcat(filepath, "/Download/");
        else if (strncmp(cmd, "put", 4) == 0)
            strcat(filepath, "/Upload/");
        else if (strncmp(cmd, "list", 5) == 0) {
            strcat(filepath, "/");
        }
        strcat(filepath,token);
        printf("Current path -> %s\n", filepath);
    }


    return filepath;
}
/*
 *
 * Usato per la post
 */
void obtain_path2(char *filepath, char *token, char *cmd, char* assolute_path) {

    if (getcwd(filepath, sizeof(char)*BUFLEN) != NULL) {

    } else {
        perror("getcwd() error");
        exit(EXIT_FAILURE);
    }
    printf("filepath -> %s\n",filepath);
    if (strlen(assolute_path) > 1){
        memset(assolute_path,'\0',BUFLEN);
    }
    char *ritorno;
    ritorno=malloc(sizeof(char)*BUFLEN);
    if(ritorno==NULL){
        perror("Error in malloc");
    }
    ritorno=strstr(filepath,"cmake-build-debug");
    if(ritorno!=NULL) { // Path for Clion
        int lenpath = (int)strlen(filepath);
        strncpy(assolute_path, filepath, lenpath - 17);// 17 sta per cmake-build-debug
        memset(ritorno,'\0',sizeof(ritorno));
        ritorno=strstr(assolute_path,"Client/Upload");
        if (ritorno == NULL){
            if (strncmp(cmd, "get", 4) == 0)
                strcat(assolute_path, "Client/Download/");
            else if (strncmp(cmd, "put", 4) == 0 )
                strcat(assolute_path, "Client/Upload/");
            else if (strncmp(cmd, "list", 5) == 0)
                strcat(assolute_path, "Server/");
        }else{
            lenpath = (int)strlen(assolute_path);
            memset(filepath,'\0',sizeof(filepath));
            strncpy(filepath, assolute_path, lenpath - 14);
            memset(assolute_path,'\0',BUFLEN);
            strcpy(assolute_path,filepath);
            if (strncmp(cmd, "get", 4) == 0)
                strcat(assolute_path, "Client/Download/");
            else if (strncmp(cmd, "put", 4) == 0 )
                strcat(assolute_path, "Client/Upload/");
            else if (strncmp(cmd, "list", 5) == 0)
                strcat(assolute_path, "Server/");
        }
    }else { // path for terminale
        if (strncmp(cmd, "get", 4) == 0)
            strcat(filepath, "/Download/");
        else if (strncmp(cmd, "put", 4) == 0)
            strcat(filepath, "/Upload/");
        else if (strncmp(cmd, "list", 5) == 0) {
            strcat(filepath, "/");
        }
        strcpy(assolute_path,filepath);
    }

}
