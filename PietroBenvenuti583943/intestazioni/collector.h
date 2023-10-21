#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>

#include "../intestazioni/conn.h"

struct lista_risultati{
    char* percorso_relativo;
    long somma;
    struct lista_risultati* prossimo;
};

typedef struct lista_risultati Lista_risultati;
typedef Lista_risultati* Puntatore_risultati;

int collector();

char prendi_argomenti(int, Puntatore_risultati*);

int aggiorna(fd_set*, int);

int inserisci_risultato(char*, long, Puntatore_risultati*);

void stampa(Puntatore_risultati);

void distruggi_lista_risultati(Puntatore_risultati*);
