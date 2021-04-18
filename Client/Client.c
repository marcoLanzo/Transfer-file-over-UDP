//
// Created by valeria on 02/10/18.
//

/*
    Simple udp client
*/
#include<stdio.h> //printf
#include<string.h> //memset
#include<stdlib.h> //exit(0);
#include<arpa/inet.h>
#include<sys/socket.h>
#include <pthread.h>
#include <unistd.h>

#define SERVER "127.0.0.1"
#define BUFLEN 512  //Max length of buffer
#define PORT 7777//The port on which to send data
#define DATA_SIZE 1500
#define HEADER 10
#define TIMEOUT_FILE_RCV 300000
#define MAXIMUM_ATTEMPT 10
#define WR 50
#define WS 50
#define N 100
#define LOSS_PROBABILITY 0.005
#define P 0.005
#define MAX_LINE_SIZE 64
#include "clientfileoperation.h"
#include "ClientReceiver.h"
#include "ClientSender.h"
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <signal.h>
#include "clientreliablemsg.h"




char ** buffer_file;
int global_var = 0;
pthread_mutex_t next;
int Number_file; // variabile in cui memorizzo il numero di file presenti nel File Lista.txt -> cosi al variare
// del numero di file nella lista varia anche questa variabile globale

int segnale = 0;

struct thread_list{
    char *buffer;
    pthread_t tid;
    pthread_mutex_t mutex_thread;
};


void die(char *s)
{
    perror(s);
    exit(1);
}
void* thread_function(void*p)
{
    struct thread_list *tinfo = (struct thread_list *) p;
    pthread_mutex_lock(&tinfo->mutex_thread);
    printf("%s\n",tinfo->buffer);
    pthread_mutex_unlock(&tinfo->mutex_thread);

    pthread_exit(NULL);
}


char ** tokenize_string(char * buffer,char * delimiter){

    int i = 0;
    char **token_vector = malloc(BUFLEN * sizeof(char*));
    token_vector[i] = strtok(buffer,delimiter);
    while(token_vector[i]!= NULL) {
        i++;
        token_vector[i] =strtok(NULL,delimiter);

    }
    //printf("%s",token_vector[1]);
    return token_vector;
}

void handler(int signum)
{
    segnale++;
}

int main(void)
{
    int mandati;

    struct sockaddr_in si_other;
    int s, i;
    int slen;
    char buf[BUFLEN];
    char message[BUFLEN];
    char* buf_size;
    pthread_t tid[N];
    struct thread_list *tlist;
    char *filepath = malloc(sizeof(char) * BUFLEN);

    struct sigaction act;
    sigset_t set;
    sigfillset(&set);

    act.sa_handler = handler;
    act.sa_mask = set;
    act.sa_flags = 0;


    if (sigaction(SIGINT,&act,NULL) == -1){
        perror("Error fucntion sigaction");
        exit(EXIT_FAILURE);
    }else if (sigaction(SIGQUIT,&act,NULL) == -1){
        perror("Error fucntion sigaction");
        exit(EXIT_FAILURE);
    }

    if (filepath == NULL){
        perror("Error function malloc");
        exit(1);
    }
    char **token_vector;

    buffer_file = malloc(BUFLEN * sizeof(char*));
    if (buffer_file == NULL){
        perror("Error function malloc");
        exit(EXIT_FAILURE);
    }

    if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }

    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);
    slen = sizeof(si_other);
    if (inet_aton(SERVER , &si_other.sin_addr) == 0) {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }
    char* porta_buf = malloc(sizeof(char) * BUFLEN);
    if(porta_buf==NULL){
        perror("Error in malloc\n");
    }


    sprintf(message,"%d",PORT);
     mandati =(int) sendto(s, message, sizeof(message), 0, (struct sockaddr *) &si_other, slen); //manda il comando al server in modo che si prepari a seconda del comando
     if(mandati==-1){
         perror("error in ");
     }
      memset(message,0,strlen(message));
    porta_buf = rcvMsg(s,message,BUFLEN,(struct sockaddr *) &si_other);

    int porta;
    porta=atoi(porta_buf);
    memset(message,0,strlen(message));
    memset(porta_buf,0,strlen(porta_buf));
    //printf("Linked server ip: %s %d\n",inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
    close(s);
    while(1) {

        if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
            die("socket");
        }

        memset((char *) &si_other, 0, sizeof(si_other));
        si_other.sin_family = AF_INET;
        si_other.sin_port = htons(porta);
        slen = sizeof(si_other);

        if (inet_aton(SERVER, &si_other.sin_addr) == 0)
            //serve per convertire in una struttura di indirizzi network(IP) da una struttura di indirizzi dot address
        {
            fprintf(stderr, "inet_aton() failed\n");
            exit(1);
        }
        printf("Linked server ip: %s %d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
        LABEL:
        printf("\n+----------------------------------------------------------------------------------+\n|"
               KYEL"ELENCO  COMANDI:"RESET"                                                                  |");
        printf("\n+----------------------------------------------------------------------------------+\n"
               "| 1) list: elenco dei file presenti nel Server                                     |\n"
               "| 2) get <Filename>: Download del file                                             |\n"
               "| 3) put <Filename>: Upload del file                                               |\n"
               "| 4) exit                                                                          |\n"
               "+----------------------------------------------------------------------------------+");

        printf(KYEL"\nENTER MESSAGE:\t\t"RESET);


        if (fgets(message, BUFLEN, stdin) == NULL && errno != EINTR) {
            perror("Error in fgets");
            exit(1);
        }

        int len;
        len =(int) strlen(message);
        if (len < 3){
            fprintf(stderr, KRED "ATTENZIONE: Hai inserito un comando non valido. Riprova\n"RESET);
            goto LABEL;
        }
        if (message[len - 1] == '\n') {
            message[len - 1] = '\0';
        }
        char string[BUFLEN];
        strcpy(string, message);


        token_vector = tokenize_string(string, " \n");



        mandati = (int)sendto(s, message, sizeof(message), 0, (struct sockaddr *) &si_other, slen);
        if (strncmp(message, "list", 5) == 0) {
            if (mandati == -1) {
                die("sendto()");
            }
            // Il client riceve dal server il numero di file presenti nella Lista.txt
            if (recvfrom(s, &Number_file, sizeof(Number_file), 0, (struct sockaddr *) &si_other, &slen) == -1) {
                die("recvfrom()");
            }
            printf("Number File %d\n", Number_file); // Verifica di aver ottenuto il corretto numelo di elementi

            memset(buf, '\0', BUFLEN);
            //struct sembuf oper;
            int i;
            for (i = 0; i < Number_file; i++) {
                puts("+----------------------------------+");
                memset(buf, 0, BUFLEN);

                tlist = malloc(sizeof(struct thread_list));
                if (tlist == NULL) {
                    printf("Error in malloc");
                    exit(EXIT_FAILURE);
                }


                if (recvfrom(s, buf, sizeof(buf) + 1, 0, (struct sockaddr *) &si_other, &slen) == -1) {
                    die("recvfrom()");
                }
                int s;
                s = i + 1;
                printf("%d) ", s);
                printf("%s\n", buf);
                // Setto la struct di ogni thread
                tlist->buffer = strndup(buf, BUFLEN);
                pthread_mutex_init(&tlist->mutex_thread, NULL);

            }
            puts("+----------------------------------+");

        }
        if (strncmp(token_vector[0], "get", 4) == 0) {
            if (mandati == -1) {
                die("sendto()");
            }
            filepath = malloc(sizeof(char) * BUFLEN);
            memset(filepath, 0, BUFLEN);
            memset(message, 0, BUFLEN);
            if (recvfrom(s, message, sizeof(message), 0, (struct sockaddr *) &si_other, &slen) == -1) {
                die("recvfrom()");
            }
            if (strcmp(message, "Attenzione: file richiesto è vuoto riprova con un altro file!") == 0) {
                printf( KRED"%s\n"RESET, message);
            }
            else if (strcmp(message, "FAIL") == 0) {
                fprintf(stderr, "%s\n", "Il file richiesto non è presente nella lista dei file disponibili del Server");
            }
            else if (token_vector[1]==NULL)  {
                fprintf(stdout,KRED "ATTENZIONE: Devi inserire un file\n"RESET);

            }
            else {
                filepath = obtain_path(filepath, token_vector[1], token_vector[0]); // ottengo path_assoluto file


                int fd = openFile(filepath);
                rcvFile(s, fd, 0, si_other);
                close(fd);

                memset(filepath, '\0', strlen(filepath));

                printf("Ho ricevuto il file %s\n", token_vector[1]);

            }
        }
        if (strncmp(token_vector[0], "put", 4) == 0) {

            char *absolute_path = malloc(sizeof(char) * BUFLEN);
            if (absolute_path == NULL) {
                perror("Error function malloc");
                exit(EXIT_FAILURE);
            }
             if (token_vector[1]==NULL)  {
                fprintf(stdout,KRED "ATTENZIONE: Devi inserire un file\n"RESET);

            }else{
            obtain_path2(filepath, token_vector[1], token_vector[0], absolute_path); // ottengo path_assoluto file
            memset(filepath, '\0', BUFLEN);


            strcpy(filepath, absolute_path);
            free(absolute_path);

            /*rapido controllo per vedere se il file che sto scegliendo di postare ce l'ho davvero in upload
             * perchè altrimenti non posso inviarglielo*/
            char **directoryContent;

            int dirsize = getNumberOfElementsInDir(filepath);

            directoryContent = alloc_memory();

            directoryContent = getContentDirectory(filepath, dirsize);

            bool exists = checkFileInDirectory(token_vector[1], directoryContent, dirsize);

            if (exists == true) {
                /*il file esiste e quindi posso inviarglielo*/

                strcat(filepath, token_vector[1]);
                int fds = openFileForSending(filepath);
                long size = getFileSize(filepath);
                printf("FILESIZECLIENT->%ld\n", size);
                buf_size = malloc(BUFLEN * sizeof(char));
                if (buf_size == NULL) {
                    perror("Error function malloc");
                    exit(1);
                }
                sprintf(buf_size, "%ld", size);
                printf("buf_size -> %s\n", buf_size);
                mandati = (int) sendto(s, buf_size, sizeof(buf_size), 0, (struct sockaddr *) &si_other, slen);
                printf("mandati_size -> %d\n", mandati);
                sendFile(s, fds, si_other);
                close(fds);
                close(s);
                printf("File sent: %s\n", token_vector[1]);
            } else {
                /*il file non esiste, non lo possiedo*/
                printf(KRED"Il file non è presente nella cartella Upload quindi non posso mandarlo\n"RESET);
                sendto(s, "Error_path", sizeof(char) * 11, 0, (struct sockaddr *) &si_other, slen);
            }
            memset(filepath, '\0', BUFLEN * sizeof(char));


             }
        }//Gestione del comando exit
        if (strncmp(token_vector[0], "exit", 5) == 0) {

            printf("Sto segnalando al server l'uscita dal sistema..");
            memset(message, 0, strlen(message));
            //Riceve un msg il cui contenuto è:"Sta terminando questa sessione della socket.\nArrivederci!\n"
            if (recvfrom(s, message, sizeof(message), 0, (struct sockaddr *) &si_other, &slen) == -1) {
                die("recvfrom()");
            }
            printf("%s\n", message);

            exit(0);

        }
        //Casistica in cui il comando mandato dall'utente non viene riconosciuto tra i comandi(GET,PUT,LIST,EXIT)
        if (strncmp(token_vector[0], "list", 5) != 0 && strncmp(token_vector[0], "get", 4) != 0 &&
            strncmp(token_vector[0], "exit", 5) && strncmp(token_vector[0], "put", 4) != 0) {

            memset(message, 0, strlen(message));
            if (recvfrom(s, message, sizeof(message), 0, (struct sockaddr *) &si_other, &slen) == -1) {
                die("recvfrom()");
            }
            printf(KRED"%s\n"RESET, message);

        }
        if (segnale == 1)
        {
            printf("Client identificato da:\n");
            printf(KRED"SOCKET: "RESET);
            printf("%d\n",s);
            printf(KRED"PORTA "RESET);
            printf("%d\n",porta);
            char* quit;
            quit = malloc(BUFLEN * sizeof(char));
            sprintf(quit,"%s","exit");
            mandati = (int)sendto(s,quit, sizeof(quit), 0, (struct sockaddr *) &si_other, slen);
            if (mandati == -1){
                die("Error function sendto");
            }
            printf(KYEL "ARRIVERDERCI\n\n"RESET);
            segnale = 0;
            exit(EXIT_SUCCESS);
        }

    }
        close(s);
        return 0;



}
