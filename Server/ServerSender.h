//
// Created by marco96 on 17/10/18.
//

#include <stdbool.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>

#include "timerlist.h"


#ifndef IIW2_SERVERSENDER_H
#define IIW2_SERVERSENDER_H

#endif //IIW2_SERVERSENDER_H

/*Struttura che rappresenta il pacchetto da inviare.
 * char data[DATA_SIZE + 1] : buffer di dimensione DATA_SIZE + 1 (per il carattere di terminazione)
 * bool acked : booleano pari a true se il client ha ricevuto il pacchetto, false altrimenti
 * unsigned int seqnum : intero che rappresenta il numero di sequenza del pacchetto
 * bool finished:
 * bool retrasmitted :  booleano pari a true se il pacchetto è stato ritrasmesso, false altrimenti
 */


nodo* phead;

struct snd_pack {

    char data[DATA_SIZE + 1];	//+ 1 per '\0'
    bool acked;
    unsigned int seqnum;
    bool finished;
    bool retransmitted;
};


/*Struttura che contiene al suo interno le informazioni relative al thread che gestisce l'invio dei pacchetti
 * pthread_t timeoutManager :
 * pthread_mutex_t mutex :
 * int fd : file descriptor del file che si v
 * int sock_fd : descriptor del socket
 * struct timeout t : struttura che gestisce il timeout
 * unsigned int send_base : numero di sequenza base
 * unsigned int next_tosend : numero di sequenza del successivo pacchetto da inviare
 */


struct timeout{

    long unsigned EstimatedRTT;
    long unsigned DevRTT;
    long unsigned TIMEOUT;

};


struct  snd_thread_info {

    pthread_t timeoutManager;

    pthread_mutex_t mtx;

    int fd;
    int sock_fd;

    struct timeout t;

    unsigned int send_base;
    unsigned int next_tosend;
    struct snd_pack* buf[N];
    struct sockaddr_in si_other;
};



void sndBufInit(struct snd_thread_info *snd) {

    int i;
    for(i = 0; i < N; i++) {
        snd->buf[i] = NULL;
    }
}


/*
 *
 *
 *
 * char *data : pacchetto da inviare
 * unsigned int seqnum : intero, numero di sequenza del pacchetto
 *
 * Tale funzione
 *
 */
char* addSeqNum(char *data, unsigned int seqnum) {

    if(seqnum >= N){ // Se il numero di sequenza è maggiore di N allora ho finito di
        //trasmettere tutti i pacchetti della finestra e quindi il pacchetto > N viene
        //printf("Ho trasmesso tutti gli N pacchetti della finestra scorrevole. Reimposto la finestra a 0\n");
        char *data_send = malloc(HEADER + 1 + 1);
        if(data_send == NULL) {

            exit(EXIT_FAILURE);
        }

        if(sprintf(data_send, "%d", seqnum) == 0) {   //copio il numero di sequenza convertito in stringa in data_send

            exit(EXIT_FAILURE);
        }

        data_send = strcat(data_send, " ");  	     //aggiungo a data_send uno spazio e il pacchetto dati

        return data_send;
    }

    char *data_send = malloc(HEADER+ 1 + strlen(data) + 1);
    if(data_send == NULL) {

        exit(EXIT_FAILURE);
    }

    if(sprintf(data_send, "%d", seqnum) == 0) {   //copio il numero di sequenza convertito in stringa in data_send

        exit(EXIT_FAILURE);
    }

    data_send = strcat(data_send, " "); 	     //aggiungo a data_send uno spazio e il pacchetto dati
    data_send = strcat(data_send, data);

    return data_send;
}

void* thread_send_job(void*p)
{
    struct snd_thread_info* snd = p;
    unsigned int full_pos;
    phead = NULL;


    for (;;) {

        struct snd_pack *pkt = malloc(sizeof(struct snd_pack));
        if(pkt == NULL) {

            exit(EXIT_FAILURE);
        }

        int size_read = readFile(snd->fd, pkt->data);
        pkt->data[size_read] = '\0';




        if (pthread_mutex_lock(&snd->mtx) != 0) {
            exit(EXIT_FAILURE);
        }

        full_pos = (snd->send_base + WS) % N;

        while (full_pos == snd->next_tosend) {	     //controllo se WS è piena

            if (pthread_mutex_unlock(&snd->mtx) != 0) {
                        exit(EXIT_FAILURE);
            }
            usleep(1);

            if (pthread_mutex_lock(&snd->mtx) != 0) {
                 exit(EXIT_FAILURE);
            }
            full_pos = (snd->send_base + WS) % N;
        }

        if (size_read == 0) {						//se leggo 0bytes ho finito il mio compito
            pkt->finished = true;					//segnalo ultimo pacchetto
            pkt->acked = false;
            pkt->retransmitted = false;
            snd->buf[snd->next_tosend] = pkt;			//inserisco comunque ip
            pkt->seqnum = snd->next_tosend + N;			//imposto numero di sequenza fittizio per il ricevitore


            if (pthread_mutex_unlock(&snd->mtx) != 0) {		//lascio lock
                  exit(EXIT_FAILURE);
            }
            pthread_exit(0);
        }


        pkt->seqnum = snd->next_tosend;

        pkt->acked = false;
        pkt->finished = false;
        pkt->retransmitted = false;
        socklen_t slen = sizeof(snd->si_other);
        float ran = random()/RAND_MAX;
        if(ran < (1 - P)) {			//invio pacchetto

            char *data_send = addSeqNum(pkt->data, pkt->seqnum);


            if(sendto(snd->sock_fd, data_send, strlen(data_send) + 1, 0, (struct sockaddr* )&snd->si_other, slen) < 0) {
                perror("Errore in sendto sender()\n");
                exit(EXIT_FAILURE);
            }
            free(data_send);
            printf("Inviato pacchetto %d\n", pkt->seqnum);
            if (pkt->seqnum == 99){
                usleep(2);
                printf("Ho trasmesso tutti gli N pacchetti della finestra scorrevole. Reimposto la finestra a 0\n");
            }
        }

        phead = insert_in_queue(phead, pkt->seqnum);//inserisco nodo nella timeoutlist

        snd->buf[snd->next_tosend] = pkt;
        snd->next_tosend = (snd->next_tosend + 1) % N;

        printf("senderManager: send_base:%d,next_tosend:%d\n,",snd->send_base, snd->next_tosend);

        if (pthread_mutex_unlock(&snd->mtx) != 0) {
            exit(EXIT_FAILURE);
        }
        usleep(1);
    }
}

/* Funzione che ritorna l'intervallo di timeout di ritrasmissione in caso di perdita di pacchetto oppure ACK Non
 * ricevuto entro l'intervallo di tempo.
 *
 * param 1: long unsigned int *EstimatedRTT
 * param 2: long unsigned *DevRTT
 * param 3: long unsigned SampleRTT
 * valore di ritorno: long unsigned
 *
 *
 * Per ottenere il valore di ritorno bisogna prima calcolare il valore aggiornato di EstimatedRTT, una media di valori
 * SampleRTT e ogni qual volta che si ottiene un SampleRTT si effettua un nuovo calcolo.
 * Successivamente oltre ad avere una stima di RTT bisogna avere una misura della sua variabilità, DevRTT.
 * Per ottenere entrambe si utilizzano dei parametri (alfa) e (beta) che per comodità sono stati dati rispettivamente
 * pari a 0.125(alfa) e 0.25 (beta).
 * Inoltre notiamo che SampleRTT non è altro che il calcolo di RTT per un singolo segmento, ossia il tempo che
 * intercorre tra il tempo di invio e la ricezione dell'ACK.
 */


long unsigned estimateTimeout(long unsigned *EstimatedRTT, long unsigned *DevRTT, long unsigned SampleRTT) {

    *EstimatedRTT = 0.875 * (*EstimatedRTT) + 0.125 * SampleRTT;
    *DevRTT = 0.75 * (*DevRTT) + 0.25 * abs(SampleRTT - *EstimatedRTT);
    long unsigned timeoutInterval = (*EstimatedRTT+ 4* (*DevRTT));
    return timeoutInterval;
}
/*
 *
 * Tale funzione viene eseguita dal thread incaricato di ritrasmettere i pacchetti per i quali è scaduto il tempo di
 * timeout.
 *
 *
 *
 */
void *thread_timeout_job(void *p)
{

    struct snd_thread_info *snd = p;

    snd->timeoutManager = pthread_self();		 //salvo ID thread nella struttura snd_thread_info

    for(;;){

        if (pthread_mutex_lock(&snd->mtx) != 0) {
            exit(EXIT_FAILURE);
        }

        if(phead == NULL) {			//caso timeoutlist vuota
            if (pthread_mutex_unlock(&snd->mtx) != 0) {
                exit(EXIT_FAILURE);
            }
            usleep(1);
            continue;
        }

        struct timeval t1, t2;
        if(gettimeofday(&t1, NULL) == -1){
            exit(EXIT_FAILURE);
        }

        t2 = phead->timer;			//il timeout del pacchetto in testa è l'unico a poter scadere

        time_t diff = ((t1.tv_sec - t2.tv_sec) * 1000000L + t1.tv_usec) - t2.tv_usec;


        if(diff >= snd->t.TIMEOUT){ 		//se timeout scaduto rinvio il pacchetto

            snd->t.TIMEOUT *= 2;		//raddoppio timeout per evitare ritrasmissioni non necessarie
            struct snd_pack*pkt = snd->buf[phead->seq_num % N];

            pkt->retransmitted = true;

            float ran = random()/RAND_MAX;
            int slen = sizeof(snd->si_other);


            if(ran < (1 - P)) {

                char* data_send = addSeqNum(pkt->data, pkt->seqnum);

                errno = 0;
                if(sendto(snd->sock_fd, data_send, strlen(data_send) + 1, 0, (struct sockaddr*)&snd->si_other, slen) < 0) {
                    //perror("Errore in sendto timeout()\n");
                    exit(EXIT_FAILURE);
                }
                free(data_send);

                if( pkt->seqnum >= N){
                    printf("\nScaduto timer pacchetto %d, rinviato\n", pkt->seqnum);
                }
                else {
                    printf("Scaduto timer pacchetto %d, rinviato\n", pkt->seqnum);
                }

            }

            phead = delete_node_in_head(phead);//rimuovo pacchetto re-inviato dalla testa della lista
            phead = insert_in_queue(phead, pkt->seqnum); //inserisco pacchetto re-inviato in coda alla lista


            if (pthread_mutex_unlock(&snd->mtx) != 0) {
                exit(EXIT_FAILURE);
            }
        }
        else {
            if (pthread_mutex_unlock(&snd->mtx) != 0) {
                exit(EXIT_FAILURE);
            }

        }
        usleep(1);
    }
}

/*Funzione incaricata di gestire l'invio del file.
 * param1 : sockfd (int)
 * param2 : fd (int)
 * tipo di ritorno: void
 *
 * Al suo interno viene allocata una struttura di tipo snd_thread_info, passata successivamente alla funzione
 * sndBufInit().
 * Vengono inizializzati i parametri della struct e poi creati due thread, uno che eseguirà il thread_send_job e un
 * altro per
 *
 *
 */

int parseAck(char*ack)
{
    int value;
    char*p;

    errno = 0;

    value = (int) strtoul(ack,&p,10);
    if (errno != 0 && *p != '\0'){
        perror("Error function strtoul");
        exit(EXIT_FAILURE);
    }

    return value;
}
void setRcvTimeout(int sockfd, long unsigned timeout) {

    struct timeval t;

    if(timeout >= 1000000){			//divido il campo secondi da quello in microsecondi
        t.tv_sec = timeout / 1000000;
        t.tv_usec = timeout % 1000000;
    }
    else{
        t.tv_sec = 0;
        t.tv_usec = timeout;
    }
    if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &t, sizeof(t)) == -1){
        //perror("Errore in setsockopt()");
        exit(EXIT_FAILURE);
    }
}

void proceduraFinale(struct snd_thread_info *snd_ti, struct snd_pack *pkt, char* ACK_torcv) {

    float ran = random()/RAND_MAX;
    int slen = sizeof(snd_ti->si_other);
    if(ran < (1 - P)) {

        char *data_send = addSeqNum(pkt->data, pkt->seqnum);

        if(sendto(snd_ti->sock_fd, data_send, strlen(data_send) + 1, 0,(struct sockaddr*) &snd_ti->si_other , slen) < 0) { //invio pacchetto finale

            exit(EXIT_FAILURE);
        }
        printf("\nInviato pacchetto %d\n", pkt->seqnum);
    }

    phead = insert_in_queue(phead,pkt->seqnum);//inserisco pacchetto finale nella lista, timeoutManager gestirà ritrasmissioni;
    setRcvTimeout(snd_ti->sock_fd, snd_ti->t.TIMEOUT);    //aggiorno timeout

    if (pthread_mutex_unlock(&snd_ti->mtx) != 0) {
         exit(EXIT_FAILURE);
    }

    while(true){					      //attendo notizie dal ricevitore
        if (pthread_mutex_lock(&snd_ti->mtx) != 0) {
            exit(EXIT_FAILURE);
        }

        errno = 0;
        if(recvfrom(snd_ti->sock_fd, ACK_torcv, HEADER+ 1, 0, (struct sockaddr*)&snd_ti->si_other, (unsigned int*)&slen) < 0) {
            printf(" ACK ->%s\n, pkt->seqnum %d\n",ACK_torcv,pkt->seqnum);
            if(errno == EAGAIN || errno == EWOULDBLOCK){ 			//timeout scaduto (pacchetto finale perso)
                printf("Timer scaduto\n");
                setRcvTimeout(snd_ti->sock_fd, 2 * snd_ti->t.TIMEOUT);  //raddoppio timeout per evitare ritrasmissioni inutili
                if (pthread_mutex_unlock(&snd_ti->mtx) != 0) {
                    exit(EXIT_FAILURE);
                }
                usleep(1);
                continue;
            }
            if(errno == ECONNREFUSED){ 					//Il receiver ha chiuso la socket
                break;
            }
            exit(EXIT_FAILURE);
        }

        int final_ACK = parseAck(ACK_torcv);
        if(final_ACK >= N){					//ACK finale ricevuto
            printf("Ricevuto ACK %d\n", final_ACK);
            phead=delete_node_in_head(phead);
            free(snd_ti->buf[snd_ti->send_base]);
            snd_ti->buf[snd_ti->send_base] = NULL;
            break;
        }
        if (pthread_mutex_unlock(&snd_ti->mtx) != 0) {

            exit(EXIT_FAILURE);
        }
    }

    if(pthread_cancel(snd_ti->timeoutManager) != 0){	//cancello thread timeoutManager

        exit(EXIT_FAILURE);
    }


    if (pthread_mutex_unlock(&snd_ti->mtx) != 0) {
        exit(EXIT_FAILURE);
    }

}



void thread_ack_job(void* p)
{
    struct snd_thread_info *snd_ti = p;
    long unsigned SampleRTT;
    struct timeval *t1, *t2;
    t1 = malloc(sizeof(struct timeval));
    t2 = malloc(sizeof(struct timeval));
    int slen = sizeof(snd_ti->si_other);

    for(;;){
        char ACK_torcv[HEADER + 1];
        unsigned int ACK_num;

        if (pthread_mutex_lock(&snd_ti->mtx) != 0) {
            //perror("Errore in pthread_mutex_lock()\n");
            exit(EXIT_FAILURE);
        }

        if(snd_ti->send_base == snd_ti->next_tosend) { 			//nessun ack da ricevere

            if (pthread_mutex_unlock(&snd_ti->mtx) != 0) {
                //perror("Errore in pthread_mutex_unlock()\n");
                exit(EXIT_FAILURE);
            }
            usleep(1);

            if (pthread_mutex_lock(&snd_ti->mtx) != 0) {
                //perror("Errore in pthread_mutex_lock()\n");
                exit(EXIT_FAILURE);
            }
        }

        struct snd_pack *pkt = snd_ti->buf[snd_ti->send_base];

        while(pkt == NULL) {					      //??

            if (pthread_mutex_unlock(&snd_ti->mtx) != 0) {
                //perror("Errore in pthread_mutex_unlock()\n");
                exit(EXIT_FAILURE);
            }
            usleep(1);

            if (pthread_mutex_lock(&snd_ti->mtx) != 0) {
                //perror("Errore in pthread_mutex_lock()\n");
                exit(EXIT_FAILURE);
            }
            pkt = snd_ti->buf[snd_ti->send_base];
        }

        if(pkt->finished){//ack tutti ricevuti
            printf("Ho ricevuto tutti gli ack\n");
            proceduraFinale(snd_ti, pkt, ACK_torcv);
            return;
        }

         setRcvTimeout(snd_ti->sock_fd, snd_ti->t.TIMEOUT);//aggiorno timeout


        errno = 0;
        if(recvfrom(snd_ti->sock_fd, ACK_torcv, HEADER + 1, 0, (struct sockaddr*)&snd_ti->si_other, (unsigned int*)&slen) < 0) {
            if(errno == EAGAIN || errno == EWOULDBLOCK){
                if (pthread_mutex_unlock(&snd_ti->mtx) != 0) {
                    //perror("Errore in pthread_mutex_unlock()\n");
                    exit(EXIT_FAILURE);
                }
                usleep(1); //dormo in modo che il timeout rimandi gli eventuali pacchetti persi visto che non ricevo ACK
                continue;
            }
            if(errno == ECONNREFUSED){ 					//Il receiver ha chiuso la socket
                return;
            }
            //perror("Errore in recvfrom()\n");
            exit(EXIT_FAILURE);
        }


        if(parseAck(ACK_torcv) == -1) {  				//scarto ack (vedi implementazione receiver)
            if (pthread_mutex_unlock(&snd_ti->mtx) != 0) {
                //perror("Errore in pthread_mutex_unlock()\n");
                exit(EXIT_FAILURE);
            }
            usleep(1);
            continue;
        }

        ACK_num = (unsigned int) parseAck(ACK_torcv);

        if(snd_ti->send_base < (snd_ti->send_base + WS) % N) { 		//(1)caso base (la finestra non è spezzata)
            if((ACK_num >= snd_ti->send_base) && (ACK_num < snd_ti->send_base + WS)) { //ACK nella finestra

                if((snd_ti->buf[ACK_num])->acked == true){		//ACK già ricevuto...scarto l'ACK
                    if (pthread_mutex_unlock(&snd_ti->mtx) != 0) {
                        //perror("Errore in pthread_mutex_unlock()\n");
                        exit(EXIT_FAILURE);
                    }
                    usleep(1);
                    continue;
                }

                (snd_ti->buf[ACK_num])->acked = true;

                if(gettimeofday(t1, NULL) == -1){
                    //perror("Errore in gettimeofday()\n");
                    exit(EXIT_FAILURE);
                }

                phead = remove_nodo(phead, ACK_num,t2);
                //printf("struct timer t2->tv_sec: %ld\n",t2->tv_sec);
                if(!(snd_ti->buf[ACK_num])->retransmitted){	//se pacchetto non ritrasmesso
                    SampleRTT = (unsigned long) ((t1->tv_sec - t2->tv_sec) * 1000000L + t1->tv_usec) - t2->tv_usec;
                    snd_ti->t.TIMEOUT = estimateTimeout(&snd_ti->t.EstimatedRTT, &snd_ti->t.DevRTT, SampleRTT);
                    printf("Pacchetto %d non ritrasmesso\n",pkt->seqnum);
                }

                printf("Ricevuto ACK %d, Timeout = %lu\n", ACK_num, snd_ti->t.TIMEOUT);

                if(snd_ti->send_base == ACK_num){	//se l'ack ricevuto è quello in pos send_base aggiorno send_base
                    while((snd_ti->buf[snd_ti->send_base])->acked) {

                        free(snd_ti->buf[snd_ti->send_base]);
                        snd_ti->buf[snd_ti->send_base] = NULL;
                        snd_ti->send_base = (snd_ti->send_base + 1) % N;
                        if(snd_ti->buf[snd_ti->send_base] == NULL )
                            break;
                    }
                }
            }
        }
        else if (ACK_num >= snd_ti->send_base) { 		//(2)caso finestra spezzata (pacchetto fine buffer)

            if((snd_ti->buf[ACK_num])->acked == true){		//ACK già ricevuto...scarto l'ACK
                if (pthread_mutex_unlock(&snd_ti->mtx) != 0) {
                    //perror("Errore in pthread_mutex_unlock()\n");
                    exit(EXIT_FAILURE);
                }
                usleep(1);
                continue;
            }

            (snd_ti->buf[ACK_num])->acked = true;

            if(gettimeofday(t1, NULL) == -1){
                //perror("Errre in gettimeofday()\n");
                exit(EXIT_FAILURE);
            }

            phead = remove_nodo(phead,ACK_num,t1);
            if(!(snd_ti->buf[ACK_num])->retransmitted){	//se pacchetto non ritrasmesso
                SampleRTT = (unsigned long) ((t1->tv_sec - t2->tv_sec) * 1000000L + t1->tv_usec) - t2->tv_usec;
                snd_ti->t.TIMEOUT = estimateTimeout(&snd_ti->t.EstimatedRTT, &snd_ti->t.DevRTT, SampleRTT);
                printf("Pacchetto %d non ritrasmesso\n",pkt->seqnum);
            }

            printf("Ricevuto ACK %d, Timeout = %lu\n", ACK_num, snd_ti->t.TIMEOUT);

            if(snd_ti->send_base == ACK_num){	         //se l'ack ricevuto è quello in pos send_base aggiorno send_base
                while((snd_ti->buf[snd_ti->send_base])->acked) {

                    free(snd_ti->buf[snd_ti->send_base]);
                    snd_ti->buf[snd_ti->send_base] = NULL;
                    snd_ti->send_base = (snd_ti->send_base + 1) % N;
                    if(snd_ti->buf[snd_ti->send_base] == NULL)
                        break;
                }
            }

        }
        else if (ACK_num < (snd_ti->send_base + WS) % N){       //(3)caso finestra spezzata (pacchetto inizio buffer)

            if((snd_ti->buf[ACK_num])->acked == true){		//ACK già ricevuto...scarto l'ACK
                if (pthread_mutex_unlock(&snd_ti->mtx) != 0) {
                    //perror("Errore in pthread_mutex_unlock()\n");
                    exit(EXIT_FAILURE);
                }
                usleep(1);
                continue;
            }

            (snd_ti->buf[ACK_num])->acked = true;

            if(gettimeofday(t1, NULL) == -1){
                //perror("Errore in gettimeofday()\n");
                exit(EXIT_FAILURE);
            }

            phead=remove_nodo(phead,ACK_num,t2);
            if(!(snd_ti->buf[ACK_num])->retransmitted){	//se pacchetto non ritrasmesso
                SampleRTT = (unsigned long) ((t1->tv_sec - t2->tv_sec) * 1000000L + t1->tv_usec) - t2->tv_usec;
                snd_ti->t.TIMEOUT = estimateTimeout(&snd_ti->t.EstimatedRTT, &snd_ti->t.DevRTT, SampleRTT);
                printf("Pacchetto %d non ritrasmesso\n",pkt->seqnum);
            }

            printf("Ricevuto ACK %d, Timeout = %lu\n", ACK_num, snd_ti->t.TIMEOUT);

            if(snd_ti->send_base == ACK_num){	      //se l'ack ricevuto è quello in pos send_base aggiorno send_base
                while((snd_ti->buf[snd_ti->send_base])->acked) {

                    free(snd_ti->buf[snd_ti->send_base]);
                    snd_ti->buf[snd_ti->send_base] = NULL;
                    snd_ti->send_base = (snd_ti->send_base + 1) % N;
                    if(snd_ti->buf[snd_ti->send_base] == NULL)
                        break;
                }
            }
        }

        printf("SampleRTT:%lu,EstimatedRTT:%lu,DevRTT:%lu,TIMEOUT:%lu\n\n",SampleRTT, snd_ti->t.EstimatedRTT, snd_ti->t.DevRTT, snd_ti->t.TIMEOUT);
        if (pthread_mutex_unlock(&snd_ti->mtx) != 0) {
            //perror("Errore in pthread_mutex_unlock()\n");
            exit(EXIT_FAILURE);
        }
        usleep(1);
    }
}

void sendFile(int sockfd,int fd,struct sockaddr_in si_other)
{
    pthread_t tid;

    //puts("PASSO1: INIZIALIZZO STRUCT SND_THREAD_INFO");
    struct snd_thread_info* snd = malloc(sizeof(struct snd_thread_info));
    if (snd == NULL){
        perror("Error function malloc");
        exit(EXIT_FAILURE);
    }

    sndBufInit(snd);

    snd->send_base = 0;
    snd->next_tosend = 0;
    snd->fd = fd;
    snd->sock_fd = sockfd;
    snd->t.EstimatedRTT = 0;
    snd->t.DevRTT = 0;
    snd->t.TIMEOUT = 1000000;
    snd->si_other = si_other;

    //printf("fd: %d\n",snd->fd);

    if(pthread_mutex_init(&snd->mtx,NULL) != 0) {
        //perror("Errore in pthread_mutex_init()");
        exit(EXIT_FAILURE);
    }

    if(pthread_create(&tid , NULL, thread_send_job, snd) != 0){
        //perror("Errore in pthread_create()");
        exit(EXIT_FAILURE);
    }

    if(pthread_create(&tid, NULL, thread_timeout_job, snd) != 0){
        //perror("Errore in pthread_create()");
        exit(EXIT_FAILURE);
    }

    // Lavoro thread_main
    thread_ack_job(snd);

}




