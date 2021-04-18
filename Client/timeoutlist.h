//
// Created by marco96 on 14/11/18.
//

#ifndef IIW2_TIMEOUTLIST_H
#define IIW2_TIMEOUTLIST_H

#endif //IIW2_TIMEOUTLIST_H
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>



/* Tale struct rappresenta un singolo nodo della lista;
 * unsigned int seq_num : numero di sequenza
 * bool finished : booleano
 * struct timeval timer : struttura di tipo timeval
 * struct node_t *next : puntatore all'elemento successivo della lista
 */
struct node_t {
    unsigned int seq_num;
    bool finished;
    struct timeval timer;
    struct node_t *next;
};

typedef struct node_t nodo;

/* valore di ritorno : struct node_t *
 * Descrizione: Tale funzione contiene al suo interno il procedimento per gestire l'allocazione di un nuovo nodo.
 * Restituisce un puntatore al nuovo nodo allocato.
 */
nodo* alloc_node(void)
{
    nodo* new;
    new = malloc(sizeof(nodo));
    if (new == NULL){
        perror("Error function malloc");
        exit(EXIT_FAILURE);
    }

    return new;
}
/*
 *  param1(nodo* head) : puntatore al primo elemento della coda
 *  param2(unsigned int seq_num) : numero di sequenza
 *  tipo di ritorno : nodo*
 *
 *  Descrizione: Funzione per l'inserimento in coda di un nuovo nodo.
 */

nodo* insert_in_queue(nodo* head,unsigned int seq_num)
{

    nodo* new_node = alloc_node();
    new_node->next = NULL;
    new_node->seq_num = seq_num;
    new_node->finished = 0;
    if(gettimeofday(&new_node->timer, NULL) == -1){  //gettimeofday() riempe la struttura timer del node_t new_node
        perror("Errore in gettimeofday()\n");
        exit(EXIT_FAILURE);
    }
    if (head == NULL){
        return new_node;
    }

    nodo* p;
    for (p=head;p->next != NULL; p = p->next);

    p->next = new_node;
    new_node->next = NULL;

    return head;
}

/* param1(nodo *head) : puntatore al primo elemento
 * Descrizione: tale funzione stampa il contenuto della lista.
 */
void printer_list(nodo*head)
{
    nodo*p;
    for (p = head; p != NULL; p = p->next){ // attento è p!=null
        fprintf(stdout,"%d-> ",p->seq_num);
        //fprintf(stdout,"%d-> ",p->next->seq_num);

    }
    fprintf(stdout,"\n");
}


/* param1(nodo *head) : puntatore al primo elemento
 * valore di ritorno : nodo *
 * Descrizione: tale funzione elimina l'elemento in testa.
 */
nodo * delete_node_in_head(nodo * head){

    nodo * p;
    p = malloc(sizeof(nodo));
    if (p == NULL){
        perror("Error function malloc");
        exit(EXIT_FAILURE);
    }
    if ( head !=NULL){
        p= head;
        head = head->next;
        free(p);
    }
    return head;
}

/* param1(nodo *head) : puntatore al primo elemento
 * param2(unsigned int seq_num) : numero di sequenza
 * param3(struct timeval* t) : puntatore ad una struttura di tipo timeval
 * valore di ritorno : nodo*
 * Descrizione: tale funzione elimina un nodo e restituisce il puntatore dell'elemento in testa.
 */
nodo* remove_nodo(nodo * head,unsigned int seq_num,struct timeval *t){

    nodo * prev;
    nodo * curr;
    curr = malloc(sizeof(nodo));
    if (curr == NULL){
        perror("Error function malloc");
        exit(EXIT_FAILURE);
    }

    for(curr= head,prev = NULL;curr!=NULL;prev = curr,curr=curr->next){
        if(head->seq_num ==seq_num){ //se il valore da eliminare è il primo della lista
            head=head->next;
            *t = curr->timer;
            free(curr);
            break;
        }
        if(curr->seq_num == seq_num){ //se il valore da eliminare non è il primo della ista
            prev->next=curr->next;
            *t = curr->timer;
            free(curr);
            break;

        }
    }
    //free(curr);
    return head;
}