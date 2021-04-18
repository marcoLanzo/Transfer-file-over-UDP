/*
 Il file clientreliablemsg.h contiene i metodi che permettono di mandare un messaggio in modo affidabile.
*/



#define RCV_MSG_TIMEOUT 1000000
#define SND_MSG_TIMEOUT 300000



/*
 nome funzione: rcvMsg

 parametro1: sock,  tipo: int

 parametro2: rcv,  tipo: char*

 parametro3: size,  tipo: unsigned int

 parametro4: servaddr,  tipo: struct sockaddr_in*

 valore restituito: char*

 La funzione rcvMsg() consente di ricevere un messaggio in modo affidabile, dando la conferma al mittente che tale messaggio è stato ricevuto
 correttamente.
 Una volta ricevuto un pacchetto viene mandato al mittente, le cui informazioni di rete si trovano nella struttura servaddr, un ACK per
 informarlo della corretta ricezione. A questo punto la funzione si mette in attesa di un messaggio di FIN, con cui il mittente comunica di
 aver capito che lo scambio è andato a buon fine, e risponde con un FINACK. L'invio e la ricezione dei messaggi avvengono tramite le funzioni
 sendto() e recvfrom(). In caso scada il timeout associato alla recvfrom(), e sia stato già mandato il messaggio FIN, la funzione termina
 restituendo il messaggio ricevuto, altrimenti torna NULL. Il timeout della recvfrom() (RCV_MSG_TIMEOUT) della funzione è impostato ad un
 valore che consente al mittente di inviare almeno per 3 volte un pacchetto in caso non riceva i riscontri ACK e FINACK.
 In caso di errori non gestiti della funzione sendto() o recvfrom() si ha la terminazione del programma e la stampa di un opportuno messaggio
 di errore.

*/
char* rcvMsg(int sock, char *rcv, unsigned int size, struct sockaddr_in *servaddr) {

	bool FIN = 0;
	float ran;

	socklen_t addr_len = sizeof(struct sockaddr);
	socklen_t len = sizeof(*servaddr);

	setRcvTimeout(sock, RCV_MSG_TIMEOUT);	//imposto timeout
	
	//printf("\n\n");	

	for(;;) {
		char pack_rcv[size];
		
		errno = 0;
		if(recvfrom(sock, pack_rcv, size, 0, (struct sockaddr *) servaddr, &addr_len) < 0) {
			if(errno == EAGAIN || errno == EWOULDBLOCK){ //timeout scaduto
				if(FIN == 1)
					return rcv;
				else 
					return NULL;
			}
			else					//errore inatteso in ricezione
				return NULL;
		}
		
		if(strncmp(pack_rcv, "Errore 4", strlen("Errore 4")) == 0 || strncmp(pack_rcv, "Errore 5", strlen("Errore 5")) == 0){
			rcv = strcpy(rcv, pack_rcv);
			return rcv;
		}

		else if(strncmp(pack_rcv, "FIN", strlen("FIN")) == 0) {
			//printf("Ricevuto FIN mando FINACK\n");
			char msg[7] = "FINACK";
			float ran = random()/RAND_MAX;
			FIN = 1;
		
			if(ran < (1 - LOSS_PROBABILITY) ) {
				if(sendto(sock, msg, 7, 0, (struct sockaddr *) servaddr, len) < 0) {
					perror("Errore in sendto() nel rcv_msg()\n");
					exit(EXIT_FAILURE);
				}
				//printf("Inviato pacchetto %s\n", msg);	
			}
			
		}

		else {	
			rcv = strcpy(rcv, pack_rcv);
			
			//printf("Ricevuto pacchetto %s\n", pack_rcv);
			char msg[4] = "ACK";
			float ran = random()/RAND_MAX;

			if(ran < (1 - LOSS_PROBABILITY)) {
				if(sendto(sock, msg, 4, 0, (struct sockaddr *) servaddr, len) < 0) {
					perror("Errore in sendto() nel rcv_msg()\n");
					exit(EXIT_FAILURE);
				}
				//printf("Inviato pacchetto %s\n", msg);	
			}
		}
	}
		
	//printf("\n\n");	
}




/*
 nome funzione: sndMsg

 parametro1: sock,  tipo: int
 
 parametro2: snd_msg,  tipo: char*

 valore restituito: bool
 
 La funzione sndMsg() consente di inviare un messaggio in modo affidabile ricevendo conferma di avvenuta ricezione da parte del ricevitore.
 Una volta inviato il messaggio con il socket connesso, si attende di ricevere il relativo ACK. Ricevuto si passa all'invio del messaggio di
 FIN con cui si comunica al destinatario di aver ricevuto l'ACK e capito quindi che lo scambio è andato a buon fine, attendendo poi anche il
 riscontro di quest'ultimo (FINACK). L'invio e la ricezione dei messaggi avvengono tramite le funzioni sendto() e recvfrom(). 
 Il timeout di ricezione è impostato a SND_MSG_TIMEOUT = 300 ms (vedi relazione): trascorso tale tempo il pacchetto inviato viene considerato
 perso (o perso il riscontro) e re-inviato di nuovo. La sequenza di operazioni si ripete al più tre volte prima di considerare l'invio
 del messaggio non riuscito e restituire false. La funzione restituisce true quando viene ricevuto il FINACK.
 In caso di errore della funzione sendto() si ha la terminazione del programma e la stampa di un opportuno messaggio di errore mentre in caso
 di errore non gestito della recvfrom() la funzione torna false.
 
*/
bool sndMsg(int sock, char *snd_msg) { 
			
	int cont = 0;
	float ran;

	setRcvTimeout(sock, SND_MSG_TIMEOUT);	
	
	//printf("\n\n");	

	for(;;){

		float ran = random()/RAND_MAX;
		
		if(ran < (1 - LOSS_PROBABILITY)) {
			if(sendto(sock, snd_msg, strlen(snd_msg) + 1, 0, NULL, 0) < 0) {
				perror("Errore in sendto() nel snd_msg()\n");
				exit(EXIT_FAILURE);
			}

			//printf("Inviato pacchetto %s\n", snd_msg);
		}
	
		char rcv_msg[7];

		errno = 0;
		if(recvfrom(sock, rcv_msg, 7, 0, NULL, 0) < 0) {
			if(errno == EAGAIN || errno == EWOULDBLOCK){ //timeot scaduto rinvio msg				
				cont++;
				if(cont > 2)			//effettuato terzo tentativo, rinuncio
					return false;

				continue;
			}	
			else					//errore inatteso in ricezione
				return false;		
		}

		cont = 0;
		setRcvTimeout(sock, SND_MSG_TIMEOUT);

		if(strncmp(rcv_msg, "ACK", strlen("ACK")) == 0) {
			//printf("Ricevuto ACK\n");
			char msg[4] = "FIN";
			snd_msg = strncpy(snd_msg, msg, 4);
		}

		else if(strncmp(rcv_msg, "FINACK", strlen("FINACK")) == 0){
			//printf("Ricevuto FINACK\n\n");
			usleep(1000000);
			return true;
		}
		
	}

		
}
