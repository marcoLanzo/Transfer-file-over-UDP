//
// Created by marco96 on 23/10/18.
//


#ifndef IIW2_CLIENTRECEIVER_H
#define IIW2_CLIENTRECEIVER_H

#endif
//IIW2_CLIENTRECEIVER_H
//IIW2_CLIENTRECEIVER_H

// Informazioni del singolo pacchetto con array per i data e il numero di seq. relativo.


#define TIMEOUT_FILE_RCV 300000
#define MAXIMUM_ATTEMPT 10

/*
 nome struttura: rcv_info_pack

 La struttura rcv_info_pack contiene le informazioni che vengono ricevute:
 - data, dati ricevuti
 - seqnum, numero di sequenza del pacchetto ricevuto

*/

    struct rcv_info_pack {

    char data[DATA_SIZE + 1];	//+ 1 per '\0'
    unsigned int seqnum;
};

// Informazioni inerenti alla ricezione del file con fd del file ...
/*
struct rcv_info {

    int fd;
    int sock_fd;

    long file_size;

    unsigned int rcv_base; // num.seq base
    struct sockaddr_in si_other;
    struct rcv_info_pack* buf[N];
};
 */
/*
 * param1 (struct rcv_info *rcv_inf): struttura che contiene informazioni inerenti alla ricezione
 * tipo di ritorno : void
 * Descrizione :  funzione che inizializza il buffer di ricezione
 *
 */
void rcvBufInit(struct rcv_info_pack** buf) {

    int i;
    for(i = 0; i < N; i++) {
        buf[i] = NULL;
    }
}

/*
 * param1 (int sock_fd) : intero descrittore del socket
 * param2 (int seq_num) : intero numero di sequenza
 * tipo di ritorno : void
 * Descrizione : Tale funzione serve ad inviare un messaggio di acknowledgement.
 * Innanzitutto si preleva un float in modo randomizzato, successivamente se tale valore è minore di (1-LOSS_PROBABILITY),
 * ovvero non abbiamo una perdita
 */
void sendAcknowledgement(int sock_fd,int seq_num,struct sockaddr_in si_other){
    float ran = random()/RAND_MAX;
    int slen = sizeof(si_other);

    if(ran < (1 - P)) {

        char ACK_tosend[MAX_LINE_SIZE + 1];

        if(sprintf(ACK_tosend, "%d", seq_num) == 0) {  		//copio il numero di sequenza convertito in stringa in ACK_tosend
            perror("Errore in sprintf()\n");
            sleep(5);//evita chiusura brusca terminale
            exit(EXIT_FAILURE);
        }

        if(sendto(sock_fd, ACK_tosend, strlen(ACK_tosend) + 1,0,(struct sockaddr*)&si_other, slen) < 0) {
            perror("Errore in sendto()\n");
            sleep(5);//evita chiusura brusca terminale
            exit(EXIT_FAILURE);

        }

    }

}

/*
 * param1 (int sockfd) : descrittore del socket
 * param2 (long timeout) : long, valore del timeout
 * tipo di ritorno : void
 * Descrizione: tale funzione serve per impostare il timeout in ricezione
 */

void setTimeoutRcv(int sockfd, long timeout){

    struct timeval t;

    if(timeout >= 1000000){			//per evitare errori di range nell'assegnazione
        t.tv_sec = timeout / 1000000;
        t.tv_usec = timeout % 1000000;
    }
    else{
        t.tv_sec = 0;
        t.tv_usec = timeout;
    }
    if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &t, sizeof(t)) == -1){
        perror("Errore in setsockopt()\n");
        exit(EXIT_FAILURE);
    }
}

/*
 * param1 (char *buffer) : buffer
 * param2 (char *data) :
 * tipo di ritorno : unsigned int
 * Descrizione:
 */
unsigned int parseLine(char *buffer,char *data){

    char* p = NULL;
    unsigned int seqnum;

    errno = 0;
    seqnum = (unsigned int) strtoul(buffer ,&p ,0);	//(1)

    if(errno != 0 || *p != ' '){
        perror("Errore in strtoul\n");
        sleep(5);//evita chiusura brusca terminale
        exit(EXIT_FAILURE);
    }
    p++;
    data = strcpy(data, p);

    return seqnum;

}

/*
 *
 *
 *
 */
void rcvFile(int sock_fd,int fd,long size,struct sockaddr_in si_other){
    unsigned int attempt = 0;
    int ack_to_resend = -1;
    /*
    bool progressbar;
    double pack_percent = 0;
    double complete_percent = 0;
    */
    struct rcv_info_pack *buf[N];
    unsigned int rcv_base = 0;
    struct rcv_info_pack *ip;
    rcvBufInit(buf);
    setTimeoutRcv(sock_fd, TIMEOUT_FILE_RCV);
    int slen = sizeof(si_other);
    for (;;) {

        char pack_rcv[HEADER + 1 + DATA_SIZE + 1];
        unsigned int last_pos;

        errno = 0;
        if(recvfrom(sock_fd, pack_rcv, HEADER + 1 + DATA_SIZE + 1, 0, (struct sockaddr *) &si_other, &slen) < 0) {
           if(errno==EBADF){
               close(sock_fd);
           }
            if(errno == EAGAIN || errno == EWOULDBLOCK){ 	//RCV_FILE_TIMEOUT ricezione scaduto
                if(attempt < MAXIMUM_ATTEMPT){//il mittente non risponde
                    printf("Timer Scaduto -> Mittente non risponde\n");
                    sendAcknowledgement(sock_fd, ack_to_resend,si_other);	//testo connessione (ICMP)

                    attempt++;
                    continue;
                }
                fprintf(stderr, "\n\nIl server non mostra segni di attività!\n");
                fprintf(stderr, "\nImpossibile completare download!\n");
                return;
            }
            if(errno == ECONNREFUSED){		//connessione interrotta
                fprintf(stderr, "\n\nConnessione interrotta!\n");
                fprintf(stderr, "\nImpossibile completare download!\n");
                return;
            }					//errore inatteso
            perror("\nErrore inatteso della funzione recvfrom(). Programma terminato\n");
            sleep(5);//evita chiusura brusca terminale
            exit(EXIT_FAILURE);
        }

        attempt = 0;

        ip = malloc(sizeof(struct rcv_info_pack ));
        if(ip == NULL) {
            fprintf(stderr, "Errore in malloc()\n");
            sleep(5);//evita chiusura brusca terminale
            exit(EXIT_FAILURE);
        }

        ip->seqnum = parseLine(pack_rcv, ip->data);
        ack_to_resend = ip->seqnum;
        printf("Ricevuto pacchetto %d\n", ip->seqnum);


        if(ip->seqnum >= N){					//ricevuti tutti i pacchetti

            printf("Ho ricevuto tutti i pacchetti\n");
            sendAcknowledgement(sock_fd, ip->seqnum,si_other);
            sendAcknowledgement(sock_fd, ip->seqnum,si_other);
            sendAcknowledgement(sock_fd, ip->seqnum,si_other);
            return;
        }

        //printf("Ricevuto pacchetto %d\n", ip->seqnum);

        last_pos = (rcv_base + WR) % N;

        if(rcv_base < last_pos) { 			//(1)caso base (la finestra non è spezzata)

            if((ip->seqnum >= rcv_base) && (ip->seqnum < last_pos)) { //pacchetto nella finestra

                if(buf[ip->seqnum] == NULL){ 		//pacchetto ancora non ricevuto
                    buf[ip->seqnum] = ip;

                    sendAcknowledgement(sock_fd, ip->seqnum,si_other);

                    if(ip->seqnum == rcv_base){
                        while(ip != NULL){
                            writeFile(fd, ip->data);
                            printf("Scritto pacchetto %d\n", ip->seqnum);
                            rcvBufInit(buf);
                            buf[rcv_base] = NULL;
                            rcv_base = (rcv_base + 1) % N;//aggiorno posizione rcv_base
                            ip = buf[rcv_base];
                        }
                    }
                }
                else{						//pacchetto già ricevuto (nella finestra)
                    sendAcknowledgement(sock_fd, ip->seqnum,si_other);
                }
            }
            else {							//pacchetto già ricevuto (fuori dalla finestra)
                sendAcknowledgement(sock_fd, ip->seqnum,si_other);
            }
        }
        else if (ip->seqnum >= rcv_base) { 	//(2)caso finestra spezzata (pacchetto fine buffer)

            if(buf[ip->seqnum] == NULL){ 		//pacchetto ancora non ricevuto
                buf[ip->seqnum] = ip;

                sendAcknowledgement(sock_fd, ip->seqnum,si_other);

                if(ip->seqnum == rcv_base){
                    while(ip != NULL){
                        writeFile(fd, ip->data);
                        printf("Scritto pacchetto n° %i\n", ip->seqnum);
                        rcvBufInit(buf);
                        buf[rcv_base] = NULL;
                        rcv_base = (rcv_base + 1) % N;//aggiorno posizione rcv_base
                        ip = buf[rcv_base];
                    }
                }
            }
            else{						//pacchetto già ricevuto (nella finestra)
                sendAcknowledgement(sock_fd, ip->seqnum,si_other);
            }
        }
        else if (ip->seqnum < last_pos){		//(3)caso finestra spezzata (pacchetto inizio buffer)

            if(buf[ip->seqnum] == NULL){ 		//pacchetto ancora non ricevuto
                buf[ip->seqnum] = ip;

                sendAcknowledgement(sock_fd, ip->seqnum,si_other);

                if(ip->seqnum == rcv_base){
                    while(ip != NULL){
                        writeFile(fd, ip->data);
                        printf("Scritto pacchetto n° %i\n", ip->seqnum);
                        rcvBufInit(buf);
                        buf[rcv_base] = NULL;
                        rcv_base = (rcv_base + 1) % N;//aggiorno posizione rcv_base
                        ip = buf[rcv_base];
                    }
                }
            }
            else{						//pacchetto già ricevuto (nella finestra)
                sendAcknowledgement(sock_fd, ip->seqnum,si_other);
            }
        }
        else { 							//pacchetto già ricevuto (fuori dalla finestra)
            printf("Pacchetto %d fuori dalla finestra!\n", ip->seqnum);
            sendAcknowledgement(sock_fd, ip->seqnum,si_other);
            ack_to_resend = ip->seqnum;
        }
    }
}







