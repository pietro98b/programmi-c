#ifndef COLLECTOR_H
#define COLLECTOR_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <signal.h>

#include "../intestazioni/conn.h"

struct lista_risultati{
    char* percorso_relativo;
    long somma;
    struct lista_risultati* prossimo;
};

typedef struct lista_risultati Lista_risultati;
typedef Lista_risultati* Puntatore_risultati;

int collector();

// prendo i valori dal client
char prendi_argomenti(int, Puntatore_risultati*);

// aggiorna i descrittori di file pronti
int aggiorna(fd_set*, int);

// inserisco i risultati in una lista ordinata
int inserisci_risultato(char*, long, Puntatore_risultati*);

// stampo la lista
void stampa(Puntatore_risultati);

// distruggo la lista dei risultati
void distruggi_lista_risultati(Puntatore_risultati*);
#endif
