/*
 Il file clientreliablemsg.h contiene i metodi per mandare un messaggio in modo affidabile.
*/



#define RCV_MSG_TIMEOUT 1000000
#define SND_MSG_TIMEOUT 300000



/*
 nome funzione: rcvMsg

 parametro1: sock,  tipo: int

 parametro2: rcv,  tipo: char*

 parametro3: size,  tipo: unsigned int

 parametro4: addr,  tipo: struct sockaddr_in

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
char* rcvMsg(int sock, char* rcv, unsigned int size, struct sockaddr_in *addr) {

	bool FIN = 0;
	float ran;

	socklen_t addr_len = sizeof(struct sockaddr);
	socklen_t len = sizeof(*addr);

	setRcvTimeout(sock, RCV_MSG_TIMEOUT);

	//printf("\n\n");	

	for(;;) {
		char pack_rcv[size];
		
		errno = 0;
		if(recvfrom(sock, pack_rcv, size, 0, (struct sockaddr *) addr, &addr_len) < 0) {
			if(errno == EAGAIN || errno == EWOULDBLOCK){	//scaduto timeout
				if(FIN == 1)
					return rcv;
				else 
					return NULL;
			}
			else					//errore inatteso in ricezione
				return NULL;
		}


		if(strncmp(pack_rcv, "FIN", strlen("FIN")) == 0) {
			
			//printf("Ricevuto FIN mando FINACK\n");
			char msg[7] = "FINACK";
			float ran = random()/RAND_MAX;
			FIN = 1;
		
			if(ran < (1 - P) ) {
				if(sendto(sock, msg, 7, 0, (struct sockaddr *) addr, len) < 0) {
					//perror("Errore in sendto(FINACK) nel rcv_msg()\n");
					exit(EXIT_FAILURE);
				}
				//printf("Inviato pacchetto %s\n\n", msg);	
			}
			
		}
		else {	
			rcv = strcpy(rcv, pack_rcv);
			
			//printf("Ricevuto pacchetto %s\n", pack_rcv);
			char msg[4] = "ACK";
			float ran = random()/RAND_MAX;

			if(ran < (1 - P)) {
				if(sendto(sock, msg, 4, 0, (struct sockaddr *) addr, len) < 0) {
					//perror("Errore in sendto(ACK) nel rcv_msg()\n");
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

 parametro3: snd_addr,  tipo: struct sockaddr_in*

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
bool sndMsg(int sock, char* snd_msg, struct sockaddr_in *snd_addr,socklen_t msg) {
	
	struct sockaddr_in rcv_addr;				
	socklen_t rcv_addr_len = sizeof(struct sockaddr);
	int cont = 0;
	float ran;

	setRcvTimeout(sock, SND_MSG_TIMEOUT);
		
	//printf("\n\n");	

	for(;;){

		float ran = random()/RAND_MAX;
		
		if(ran < (1 - P)) {
			if (sendto(sock, snd_msg, sizeof(snd_msg), 0, (struct sockaddr *) snd_addr,msg) ==
				-1) {
				perror("sendto()");
			}
			//printf("Inviato paccheto %s\n", snd_msg);
		}

		char rcv_msg[7];

		while(true){

			errno = 0;
			if(recvfrom(sock, rcv_msg, 7, 0, (struct sockaddr *) &rcv_addr, &rcv_addr_len) < 0) {
				if(errno == EAGAIN || errno == EWOULDBLOCK){ //timeout scaduto rinvia msg
					cont++;
					if(cont > 2)			//effettuato terzo tentativo, rinuncio
						return false;

					continue;
				}
				else					//errore inatteso in ricezione
					return false;
			}

			if(rcv_addr.sin_addr.s_addr != snd_addr->sin_addr.s_addr || ntohs(rcv_addr.sin_port) != ntohs(snd_addr->sin_port))				//scarto pacchetto estraneo
				continue;
			else
				break;
		}

		cont = 0;
		setRcvTimeout(sock, SND_MSG_TIMEOUT);

		if(strncmp(rcv_msg, "ACK", strlen("ACK")) == 0) {
			//printf("Ricevuto ACK\n");
			char msg[4] = "FIN";
			snd_msg = strncpy(snd_msg, msg, 4);
		}
		else if(strncmp(rcv_msg, "FINACK", strlen("FINACK")) == 0){
			//printf("Ricevuto FINACK\n");
			usleep(1000000);
			return true;
		}
		else{	//problema di sincronizzazione termino processo figlio

			exit(EXIT_FAILURE);
		}
	}
	//printf("\n\n");
}
