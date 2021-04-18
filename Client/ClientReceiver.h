//
// Created by marco96 on 23/10/18.
//

#ifndef IIW2_CLIENTRECEIVER_H
#define IIW2_CLIENTRECEIVER_H

#endif //IIW2_CLIENTRECEIVER_H

// Informazioni del singolo pacchetto con array per i data e il numero di seq. relativo.

struct rcv_info_pack {

    char data[DATA_SIZE + 1];	//+ 1 per '\0'
    unsigned int seqnum;
};

// Informazioni inerenti alla ricezione del file con fd del file ...
struct rcv_info {

    int fd;
    int sock_fd;

    long file_size;

    unsigned int rcv_base; // num.seq base
    struct sockaddr_in si_other;
    struct rcv_info_pack* buf[N];
};
/*
 * param1 (struct rcv_info *rcv_inf): struttura che contiene informazioni inerenti alla ricezione
 * tipo di ritorno : void
 * Descrizione :  funzione che inizializza il buffer di ricezione
 *
 */
void rcvBufInit(struct rcv_info *rcv_inf) {

    int i;
    for(i = 0; i < N; i++) {
        rcv_inf->buf[i] = NULL;
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

    if(ran < (1 - LOSS_PROBABILITY)) {

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

    bool progressbar;
    double pack_percent = 0;
    double complete_percent = 0;
    struct rcv_info *rcv_inf;

    rcv_inf = malloc(sizeof(struct rcv_info));
    if(rcv_inf == NULL){
        perror("Error function malloc");
        exit(1);
    }
    rcvBufInit(rcv_inf);
    rcv_inf->sock_fd = sock_fd;
    rcv_inf->fd=fd;
    rcv_inf->file_size = size;
    rcv_inf->rcv_base = 0;
    rcv_inf->si_other = si_other;

    if(rcv_inf->file_size != 0){						//utilizzo la dimensione del file per creare una progressbar
        pack_percent = (100*DATA_SIZE) / (double) rcv_inf->file_size;
        printf("pack_percent->%ld\n",pack_percent);
        progressbar = 1;
    }
    else
        progressbar = 0;


    setTimeoutRcv(rcv_inf->sock_fd, TIMEOUT_FILE_RCV);
    int slen = sizeof(si_other);
    for (;;) {

        char pack_rcv[HEADER + 1 + DATA_SIZE + 1];
        unsigned int last_pos;

        errno = 0;
        if(recvfrom(rcv_inf->sock_fd, pack_rcv, HEADER + 1 + DATA_SIZE + 1, 0, NULL, 0) < 0) {
            if(errno == EAGAIN || errno == EWOULDBLOCK){ 	//RCV_FILE_TIMEOUT ricezione scaduto
                if(attempt < MAXIMUM_ATTEMPT){		//il mittente non risponde
                    sendAcknowledgement(rcv_inf->sock_fd, ack_to_resend,rcv_inf->si_other);	//testo connessione (ICMP)

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

        struct rcv_info_pack *ip = malloc(sizeof(struct rcv_info_pack));
        if(!ip) {
            fprintf(stderr, "Errore in malloc()\n");
            sleep(5);//evita chiusura brusca terminale
            exit(EXIT_FAILURE);
        }

        ip->seqnum = parseLine(pack_rcv, ip->data);
        ack_to_resend = ip->seqnum;



        if(ip->seqnum >= N){					//ricevuti tutti i pacchetti

            //printf("\nRicevuto pacchetto %d\n", ip->seqnum);
            sendAcknowledgement(rcv_inf->sock_fd, ip->seqnum,rcv_inf->si_other);

            if(close(rcv_inf->sock_fd) == -1) {
                perror("Errore in close()\n");
                sleep(5);//evita chiusura brusca terminale
                exit(EXIT_FAILURE);
            }
            free(ip);
            free(rcv_inf);

            if(progressbar){
                complete_percent = 100;
                printf("\rDownload del file... %d%%",(int)complete_percent);
            }

            if(rcv_inf->file_size != 0)
                fprintf(stdout,"\n\nDownload completato!\n");

            return;
        }

        //printf("Ricevuto pacchetto %d\n", ip->seqnum);

        last_pos = (rcv_inf->rcv_base + WR) % N;

        if(rcv_inf->rcv_base < last_pos) { 			//(1)caso base (la finestra non è spezzata)

            if((ip->seqnum >= rcv_inf->rcv_base) && (ip->seqnum < last_pos)) { //pacchetto nella finestra

                if(rcv_inf->buf[ip->seqnum] == NULL){ 		//pacchetto ancora non ricevuto
                    rcv_inf->buf[ip->seqnum] = ip;

                    sendAcknowledgement(rcv_inf->sock_fd, ip->seqnum,rcv_inf->si_other);

                    if(ip->seqnum == rcv_inf->rcv_base){
                        while(ip != NULL){
                            writeFile(rcv_inf->fd, ip->data);
                            //printf("Scritto pacchetto %d\n", ip->seqnum);

                            if(progressbar){
                                printf("\rDownload del file... %d%%",(int)complete_percent);
                                fflush(stdout);
                                complete_percent += pack_percent;
                            }

                            free(rcv_inf->buf[rcv_inf->rcv_base]);
                            rcv_inf->buf[rcv_inf->rcv_base] = NULL;
                            rcv_inf->rcv_base = (rcv_inf->rcv_base + 1) % N;//aggiorno posizione rcv_base
                            ip = rcv_inf->buf[rcv_inf->rcv_base];
                        }
                    }
                }
                else{						//pacchetto già ricevuto (nella finestra)
                    sendAcknowledgement(rcv_inf->sock_fd, ip->seqnum,rcv_inf->si_other);
                }
            }
            else {							//pacchetto già ricevuto (fuori dalla finestra)
                sendAcknowledgement(rcv_inf->sock_fd, ip->seqnum,rcv_inf->si_other);
            }
        }
        else if (ip->seqnum >= rcv_inf->rcv_base) { 	//(2)caso finestra spezzata (pacchetto fine buffer)

            if(rcv_inf->buf[ip->seqnum] == NULL){ 		//pacchetto ancora non ricevuto
                rcv_inf->buf[ip->seqnum] = ip;

                sendAcknowledgement(rcv_inf->sock_fd, ip->seqnum,rcv_inf->si_other);

                if(ip->seqnum == rcv_inf->rcv_base){
                    while(ip != NULL){
                        writeFile(rcv_inf->fd, ip->data);
                        //printf("Scritto pacchetto n° %i\n", ip->seqnum);

                        if(progressbar){
                            printf("\rDownload del file... %d%%",(int)complete_percent);
                            fflush(stdout);
                            complete_percent += pack_percent;
                        }

                        free(rcv_inf->buf[rcv_inf->rcv_base]);
                        rcv_inf->buf[rcv_inf->rcv_base] = NULL;
                        rcv_inf->rcv_base = (rcv_inf->rcv_base + 1) % N;//aggiorno posizione rcv_base
                        ip = rcv_inf->buf[rcv_inf->rcv_base];
                    }
                }
            }
            else{						//pacchetto già ricevuto (nella finestra)
                sendAcknowledgement(rcv_inf->sock_fd, ip->seqnum,rcv_inf->si_other);
            }
        }
        else if (ip->seqnum < last_pos){		//(3)caso finestra spezzata (pacchetto inizio buffer)

            if(rcv_inf->buf[ip->seqnum] == NULL){ 		//pacchetto ancora non ricevuto
                rcv_inf->buf[ip->seqnum] = ip;

                sendAcknowledgement(rcv_inf->sock_fd, ip->seqnum,rcv_inf->si_other);

                if(ip->seqnum == rcv_inf->rcv_base){
                    while(ip != NULL){
                        writeFile(rcv_inf->fd, ip->data);
                        //printf("Scritto pacchetto n° %i\n", ip->seqnum);

                        if(progressbar){
                            printf("\rDownload del file... %d%%",(int)complete_percent);
                            fflush(stdout);
                            complete_percent += pack_percent;
                        }

                        free(rcv_inf->buf[rcv_inf->rcv_base]);
                        rcv_inf->buf[rcv_inf->rcv_base] = NULL;
                        rcv_inf->rcv_base = (rcv_inf->rcv_base + 1) % N;//aggiorno posizione rcv_base
                        ip = rcv_inf->buf[rcv_inf->rcv_base];
                    }
                }
            }
            else{						//pacchetto già ricevuto (nella finestra)
                sendAcknowledgement(rcv_inf->sock_fd, ip->seqnum,rcv_inf->si_other);
            }
        }
        else { 							//pacchetto già ricevuto (fuori dalla finestra)
            //printf("Pacchetto %d fuori dalla finestra!\n", ip->seqnum);
            sendAcknowledgement(rcv_inf->sock_fd, ip->seqnum,rcv_inf->si_other);
            ack_to_resend = ip->seqnum;
        }
    }
}







