
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include <fcntl.h>
#include <limits.h> /* PATH_MAX */
#include <wait.h>
#include <error.h>
#include <sys/time.h>

#define BUFLEN 512  //Max length of buffer
#define PORT 7777//The port on which to listen for incoming data
#define DATA_SIZE 1500
#define MAX_LINE_SIZE 64
#define N 100 // dimensione finestra scorrevole
#define WS 50
#define WR 50
#define MAX_THREADS 50

#define P 0.001 // Probabilità di perdità del pacchetto
#define HEADER 10 // dimensione header pacchetto udp --> in questo header memorizzo
// il numero di sequenza di ogni pacchetto

#include "serverfileoperation.h"
#include "ServerSender.h"
#include "ServerReceiver.h"
#include "servreliablemsg.h"

int Number_file; // numero di file nella LISTA.txt
int fd;
struct sockaddr_in server;
int sock_main_thread;
int thread_no = 0;
int numero_client = 0;
int numero_client_for_port=0;
/*
 nome struttura: client_info

 Struttura che contiene un puntatore alle informazioni di rete del client nella struttura sockaddr_in* client_addr e un campo per memorizzare
 il comando che tale client ha inviato.

*/
struct client_info{
    int sock;
    int port;
    socklen_t fromlen;
    struct sockaddr_in from;
    char buf[BUFLEN];
    struct sockaddr_in server_addr;
    char * filepath;
};

void die(char *s)
{
    perror(s);
    exit(1);
}


void tokenize_string(char * buffer,char * delimiter,char** tokens){
    char * token;
    int i = 1;
    tokens[0] = strtok(buffer,delimiter);
    while(token!= NULL) {
        token = strtok(NULL,delimiter);
        tokens[i] = token;

        i++;
    }
}
char** alloc_memory(){
    char **buf = malloc(BUFLEN * sizeof(char*));
    if (buf == NULL){
        perror("Error function malloc");
        exit(-1);
    }
    return buf;
}


void read_file_list(char** filelist,char*token, char* cmd,struct client_info* clinf) {
    /*
    char *file_path = malloc(sizeof(char));
    if (file_path == NULL){
        perror("Error function malloc");
        exit(1);
    }
     */
    clinf->filepath = obtain_path(clinf->filepath,token,cmd);
    ssize_t read;
    FILE *file;
    ssize_t len = 0;
    file = fopen(clinf->filepath, "r+");
    if (file == NULL) {
        perror("Error function fopen");
        exit(-1);
    }
    char *segment = malloc(BUFLEN * sizeof(char *));
    int i = 0;
    while (fscanf(file, "%s", segment) != EOF) {
        filelist[i] = segment;
        segment += strlen(segment) + 1;
        i++;
        Number_file = i;
    }

    memset(clinf->filepath,'\0',strlen(clinf->filepath));
}


void handle_request(void * p) {
    struct timeval tv1, tv2;
    double total_time;
    struct client_info* clinf = (struct client_info*)p;
    char **tokens;
    char **filetokens;
    char buf[BUFLEN];
    char **bufferDaMandare;
    char *client_size;
    struct timeval read_timeout;

    START:
    memset(clinf->buf,0,sizeof(clinf->buf));
    fflush(stdout);

    int n = recvfrom(clinf->sock, clinf->buf, BUFLEN, 0, (struct sockaddr *)&clinf->from, &clinf->fromlen);
    if(n < 0)
        perror("recvfrom");
    puts("\n");
    gettimeofday(&tv1,NULL);//inizia il tempo dopo la ricezione del comando da parte del client
    printf(KRED"OSSERVA: "RESET);
    printf(KGRN"La porta del client a cui si fa riferimento è %d la cui socket  è %d;"RESET,clinf->port,clinf->sock);

    printf("\nData: %s\n", clinf->buf);
    if (strncmp(clinf->buf, "list", 5) == 0) {
        bufferDaMandare = alloc_memory();
        read_file_list(bufferDaMandare, "Lista.txt", clinf->buf,clinf);
        printf("Number_file %d\n", Number_file);
        if (sendto(clinf->sock, &Number_file, sizeof(Number_file), 0, (struct sockaddr *) &clinf->from, sizeof(clinf->from)) ==
            -1) {
            die("sendto()");
        }
        int h = 0;
        while (bufferDaMandare[h] != NULL) {
            if (h == Number_file){
                break;
            }
            if (sendto(clinf->sock, bufferDaMandare[h], sizeof(bufferDaMandare[h]) + 10, 0,
                       (struct sockaddr *) &clinf->from, sizeof(clinf->from)) ==
                -1) {
                die("sendto()");
            }

            close(clinf->sock);
            clinf->sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            clinf->server_addr.sin_family= AF_INET;
            clinf->server_addr.sin_addr.s_addr = INADDR_ANY;
            clinf->server_addr.sin_port=htons(clinf->port);
            if(bind(clinf->sock, (struct sockaddr *)&clinf->server_addr, sizeof(clinf->server_addr))<0)
                perror("error binding");
            h++;

        }
        close(clinf->sock);
        clinf->sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        clinf->server_addr.sin_family= AF_INET;
        clinf->server_addr.sin_addr.s_addr = INADDR_ANY;
        clinf->server_addr.sin_port=htons(clinf->port);
        if(bind(clinf->sock, (struct sockaddr *)&clinf->server_addr, sizeof(clinf->server_addr))<0)
            perror("error binding");

        gettimeofday(&tv2,NULL);//
        total_time = ((double)(tv2.tv_usec -tv1.tv_usec) /10000 +(double)(tv2.tv_sec - tv1.tv_sec));
        fprintf(stdout,KYEL "TEMPO TOTALE : %lf\n"RESET,total_time);
    }

    if (strncmp(buf, "list", 5) != 0) {
        tokens = alloc_memory();
        tokenize_string(clinf->buf, " \n", tokens);


        if (strcmp(tokens[0], "get") == 0) {
            char **filelist;
            filelist = alloc_memory();
            if (tokens[1] == NULL){
                memset(clinf->buf,'\0',BUFLEN);
                sprintf(clinf->buf,KRED "ATTENZIONE: Devi inserire un file\n"RESET);
                if (sendto(clinf->sock, clinf->buf, sizeof(char*) *BUFLEN, 0,
                           (struct sockaddr *) &clinf->from, sizeof(clinf->from)) ==
                    -1) {
                    die("sendto()");
                }
                goto START;
            }else {
                char *string = strdup(tokens[1]);
                read_file_list(filelist, "Lista.txt", "list",clinf);
                static int i;
                int count=0;
                for (i = 0; i < Number_file; i++) {
                    if (strcmp(filelist[i], string) == 0) {
                        clinf->filepath = obtain_path(clinf->filepath, tokens[1], tokens[0]);
                        int fd = openFile(clinf->filepath);
                        long size = getFileSize(clinf->filepath);
                        if (size == 0) {
                            printf(KRED "ATTENZIONE: " RESET);
                            printf("Il file è vuoto! ");
                            printf("Inserire un comando diverso\n");
                            closeFile(fd);
                            memset(clinf->buf,0,sizeof(char)*BUFLEN);
                            strcpy(clinf->buf,"Attenzione: file richiesto è vuoto riprova con un altro file!");
                            if (sendto(clinf->sock, clinf->buf, sizeof(char*) *BUFLEN, 0,
                                       (struct sockaddr *) &clinf->from, sizeof(clinf->from)) ==
                                -1) {
                                die("sendto()");
                            }

                            close(clinf->sock);
                            clinf->sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
                            clinf->server_addr.sin_family = AF_INET;
                            clinf->server_addr.sin_addr.s_addr = INADDR_ANY;
                            clinf->server_addr.sin_port = htons(clinf->port);

                            if (bind(clinf->sock, (struct sockaddr *) &clinf->server_addr, sizeof(clinf->server_addr)) < 0)
                                perror("error binding");

                            gettimeofday(&tv2,NULL);
                            total_time = ((double)(tv2.tv_usec -tv1.tv_usec) /10000 +(double)(tv2.tv_sec - tv1.tv_sec));
                            fprintf(stdout,KYEL "TEMPO TOTALE : %lf\n"RESET,total_time);
                            goto START;
                        }


                        sendFile(clinf->sock, fd, clinf->from);

                        closeFile(fd);
                        close(clinf->sock);
                        clinf->sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
                        clinf->server_addr.sin_family = AF_INET;
                        clinf->server_addr.sin_addr.s_addr = INADDR_ANY;
                        clinf->server_addr.sin_port = htons(clinf->port);

                        if (bind(clinf->sock, (struct sockaddr *) &clinf->server_addr, sizeof(clinf->server_addr)) < 0)
                            perror("error binding");
                        printf("Ho stampato il file %s\n", filelist[i]);
                        memset(clinf->filepath, '\0', BUFLEN);
                        gettimeofday(&tv2,NULL);//
                        total_time = ((double)(tv2.tv_usec -tv1.tv_usec) /10000 +(double)(tv2.tv_sec - tv1.tv_sec));
                        fprintf(stdout,KYEL "TEMPO TOTALE : %lf\n"RESET,total_time);
                        goto START;
                    }else if (i+1 == Number_file ){
                        //if(strcmp(filelist[i],string)!=0)
                        fflush(stderr);
                        fprintf(stderr, "%s\n", "Non ho trovato il file richiesto");
                        sprintf(clinf->buf,"%s","FAIL");
                        if (sendto(clinf->sock, clinf->buf, sizeof(clinf->buf) + 10, 0,
                                   (struct sockaddr *) &clinf->from, sizeof(clinf->from)) == -1) {
                            die("sendto()");
                        }
                        gettimeofday(&tv2,NULL);//
                        total_time = ((double)(tv2.tv_usec -tv1.tv_usec) /10000 +(double)(tv2.tv_sec - tv1.tv_sec));
                        fprintf(stdout,KYEL "TEMPO TOTALE : %lf\n"RESET,total_time);
                    }
                }
            }
        } else if (strncmp(tokens[0], "put", 4) == 0) {

            if (tokens[1] == NULL) {
                memset(clinf->buf, '\0', BUFLEN);
                sprintf(clinf->buf, KRED "ATTENZIONE: Devi inserire un file\n"RESET);
                if (sendto(clinf->sock, clinf->buf, sizeof(char *) * BUFLEN, 0,
                           (struct sockaddr *) &clinf->from, sizeof(clinf->from)) ==
                    -1) {
                    die("sendto()");
                }
                goto START;
            } else {
                clinf->filepath = obtain_path(clinf->filepath, tokens[1], tokens[0]); // ottengo path_assoluto file

                int fd = creat(clinf->filepath, 0666);
                if (fd == -1) {
                    perror("Error function creat");
                    exit(1);
                }
                close(fd);
                int fds = openFile(clinf->filepath);

                if (fds == -1) {          //file già presente
                    closeFile(fds);
                    printf("\nFile già scaricato o in fase di download...\n");
                    exit(-1);
                }
                printf("file descriptor -> %d\n", fds);

                client_size = malloc(BUFLEN * sizeof(char));
                fflush(stdout);
                memset(client_size, '\0', BUFLEN);
                int receive = recv(clinf->sock, client_size, sizeof(char) * 11, 0);
                if (strcmp(client_size, "Error_path") == 0) {
                    printf("E' stato segnalato dal client un errore nell'apertura del file che voleva mandarmi\n");
                    close(fds);
                    close(clinf->sock);
                    clinf->sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
                    clinf->server_addr.sin_family = AF_INET;
                    clinf->server_addr.sin_addr.s_addr = INADDR_ANY;
                    clinf->server_addr.sin_port = htons(clinf->port);

                    if (bind(clinf->sock, (struct sockaddr *) &clinf->server_addr, sizeof(clinf->server_addr)) < 0)
                        perror("error binding");

                    memset(clinf->filepath, '\0', BUFLEN * sizeof(char));
                    gettimeofday(&tv2, NULL);//
                    total_time = ((double) (tv2.tv_usec - tv1.tv_usec) / 10000 + (double) (tv2.tv_sec - tv1.tv_sec));
                    fprintf(stdout, KYEL "TEMPO TOTALE : %lf\n"RESET, total_time);
                    goto START;
                }
                long size = strtol(client_size, NULL, 10);
                printf("size -> %ld\n", size);

                rcvFile(clinf->sock, fds, size, clinf->from);
                close(fds);
                close(clinf->sock);
                clinf->sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
                clinf->server_addr.sin_family = AF_INET;
                clinf->server_addr.sin_addr.s_addr = INADDR_ANY;
                clinf->server_addr.sin_port = htons(clinf->port);

                if (bind(clinf->sock, (struct sockaddr *) &clinf->server_addr, sizeof(clinf->server_addr)) < 0)
                    perror("error binding");
                bool aggiornato;
                aggiornato = controllo_se_esiste_in_lista(tokens[1]);
                if (aggiornato == false) {
                    aggiorna_lista(tokens[1]);
                    printf("Ho aggiornato la lista dei file disponibili inserendo il file %s\n", tokens[1]);
                } else {
                    printf("File gia presente in lista. Non devo aggiornare la lista dei file disponibili su Server\n");
                }
                memset(clinf->filepath, '\0', BUFLEN * sizeof(char));

                printf("Ho ricevuto il file %s\n", tokens[1]);
                gettimeofday(&tv2, NULL);//
                total_time = ((double) (tv2.tv_usec - tv1.tv_usec) / 10000 + (double) (tv2.tv_sec - tv1.tv_sec));
                fprintf(stdout, KYEL "TEMPO TOTALE : %lf\n"RESET, total_time);
            }

        } else if (strncmp(tokens[0], "exit", 5) == 0) {
            memset(clinf->buf,0,strlen(clinf->buf));
            printf("\nSta terminando questa sessione la cui porta è %d e la socket di riferimento è %d .\n",clinf->port,clinf->sock);
            strcpy(clinf->buf,"\nSta terminando questa sessione della socket.\nArrivederci!");
            if (sendto(clinf->sock, clinf->buf, sizeof(clinf->buf) + 10, 0,
                       (struct sockaddr *) &clinf->from, sizeof(clinf->from)) ==
                -1) {
                die("sendto()");
            }
            close(clinf->sock);
            free(clinf);
            numero_client--;
            gettimeofday(&tv2,NULL);//
            total_time = ((double)(tv2.tv_usec -tv1.tv_usec) /10000 +(double)(tv2.tv_sec - tv1.tv_sec));
            fprintf(stdout,KYEL "TEMPO TOTALE : %lf\n"RESET,total_time);
            pthread_exit(NULL);

        }
        if (strncmp(tokens[0], "list", 5) != 0 || strcmp(tokens[0], "get") != 0 ||
            strncmp(tokens[0], "exit", 5) != 0 || strcmp(tokens[0], "put") != 0 || strcmp(tokens[1],NULL) == 0) {
            memset(clinf->buf,0,strlen(clinf->buf));
            fprintf(stderr,"%s\n","Attenzione : comando non valido!");
            strcpy(clinf->buf,"Attenzione : comando non valido!");
            if (sendto(clinf->sock, clinf->buf, sizeof(clinf->buf) + 10, 0,
                       (struct sockaddr *) &clinf->from, sizeof(clinf->from)) ==
                -1) {
                die("sendto()");
            }
            close(clinf->sock);
            clinf->sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            clinf->server_addr.sin_family= AF_INET;
            clinf->server_addr.sin_addr.s_addr = INADDR_ANY;
            clinf->server_addr.sin_port=htons(clinf->port);

            if(bind(clinf->sock, (struct sockaddr *)&clinf->server_addr, sizeof(clinf->server_addr))<0)
                perror("error binding");
            //exit(EXIT_FAILURE);
            gettimeofday(&tv2,NULL);//
            total_time = ((double)(tv2.tv_usec -tv1.tv_usec) /10000 +(double)(tv2.tv_sec - tv1.tv_sec));
            fprintf(stdout,KYEL "TEMPO TOTALE : %lf\n"RESET,total_time);

        }

        free(tokens);

    }
    goto START;
}

char * lettura_porte(){
    FILE* fd;
    char* disponibili;
    disponibili=malloc(sizeof(char)*BUFLEN);
    if(disponibili==NULL){
        perror("error in malloc\n");
    }
    errno=0;
    fd=fopen("porte_attive.txt","w+");
    if(errno!=0){
        fprintf(stderr,"errore nell'apertura del file\n");
        exit(-1);
    }
    fread(disponibili,sizeof(char),BUFLEN,fd);
    if(fclose(fd)){
        fprintf(stderr,"errore nella chiusura\n");
        exit(-1);
    }
    return disponibili;
}

char* scrivi_su_file(char* s,int n_client){
    FILE* fd;
    char* contenuto_file;
    char*ritorno;
    ritorno=malloc(sizeof(char)*BUFLEN);
    if(ritorno==NULL){
        perror("Error in malloc\n");
    }
    contenuto_file=malloc(sizeof(char)*BUFLEN);
    if(contenuto_file==NULL){
        perror("Error in malloc\n");
    }
    contenuto_file=lettura_porte();
    errno=0;
    fd=fopen("porte_attive.txt","w+");
    if(errno!=0){
        fprintf(stderr,"errore nell'apertura del file\n");
        exit(-1);
    }

    int port=(int)strtol(s,0,10)+n_client;
    int port2=port;
    sprintf(s, "%d", port);
    ritorno = strstr(contenuto_file, s);
    if (ritorno == NULL) {
        fprintf(fd, "%s", s);

    } else {
        RETRY:
        port2++;
        sprintf(s, "%d", port2);
        ritorno = strstr(contenuto_file, s);
        while(ritorno!=NULL)
            goto RETRY;

    }


    if(fclose(fd)){
        fprintf(stderr,"errore nella chiusura\n");
        exit(-1);
    }
    return s;

}

int main(int argc,char** argv) {

    struct timeval tv1, tv2;
    double total_time;
    /* create the socket */
    char*buffer_port;
    char buf[BUFLEN];
    ssize_t len;
    /* thread variables */
    pthread_t threads[MAX_THREADS];
    puts("Waiting for data");
    int rc = 0;



    /* continuously listen on the specified port */
    while(1){
        sock_main_thread= socket(AF_INET, SOCK_DGRAM, 0);
        if (sock_main_thread < 0)
            perror("Error scoket function");
        size_t length = sizeof(server);
        bzero(&server,length);
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = INADDR_ANY;
        server.sin_port=htons(PORT);

        /* bind port to the IP */
        if(bind(sock_main_thread, (struct sockaddr *)&server, length)<0)
            perror("error binding");
        numero_client++;
        numero_client_for_port++;
        struct client_info* client = malloc(sizeof(struct client_info));
        client->fromlen = sizeof(struct sockaddr_in);

        int n = recvfrom(sock_main_thread, buf, BUFLEN, 0, (struct sockaddr *)&client->from, &client->fromlen);
        gettimeofday(&tv1,NULL);
        if(n < 0)
            perror("recvfrom");
        buffer_port = malloc(sizeof(char) * 6);
        if(buffer_port==NULL){
            perror("Error in malloc\n");
        }
        buffer_port = scrivi_su_file(buf,numero_client_for_port);
        int porta=atoi(buffer_port);
        if (!sndMsg(sock_main_thread,buffer_port,&client->from,client->fromlen))
            return false;

        usleep(5);

        memset(buf,0,strlen(buf));
        client->port = porta;
        printf(KRED"PORTA TRHREAD -> %d\n"RESET,client->port);
        client->sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        size_t length_server = sizeof(client->server_addr);
        bzero(&server,length_server);
        client->server_addr.sin_family= AF_INET;
        client->server_addr.sin_addr.s_addr = INADDR_ANY;
        client->server_addr.sin_port=htons(client->port);
        client->filepath=malloc(sizeof(char)*BUFLEN);
        printf(KRED"SOCKET TRHREAD -> %d\n"RESET,client->sock);
        if(client->filepath==NULL){
            perror("Error function malloc");

        }
        if(bind(client->sock, (struct sockaddr *)&client->server_addr, length_server)<0)
            perror("error binding");

        /* invoke a new thread and pass the recieved request to it */
        printf("Attualmente sono attivi %d client\n",numero_client);
        rc = pthread_create(&threads[thread_no], NULL, handle_request, (void*)client);
        if(rc){
            printf("A request could not be processed\n");
        }
        else{
            thread_no++;
        }
        gettimeofday(&tv2,NULL);
        total_time = ((double)(tv2.tv_usec -tv1.tv_usec) /10000 +(double)(tv2.tv_sec - tv1.tv_sec));
        fprintf(stdout,KYEL "TEMPO TOTALE CREAZIONE E CONNESSIONE: %lf , THREAD I-ESIMO %ld\n"RESET,total_time,threads[thread_no]);
        close(sock_main_thread);

    }

}
