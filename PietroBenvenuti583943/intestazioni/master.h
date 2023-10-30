#ifndef MASTER_H
#define MASTER_H

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>


#include "../intestazioni/worker.h"
#include "../intestazioni/collector.h"


#define CONVERTI(MS) MS*1000000

typedef void (*funzione) (int);

typedef struct sigaction Sigaction;
typedef struct timespec Timespec;


int master(int, int, char**, int, int, int, char*);


// controllo se il file e' regolare o una directory
// se è un file regolare lo inserisco nella coda
int isType(char*, Threadpool*, int);

// leggo la directory
int leggi_directory(char*, Threadpool*, int);

// setto notifica quando arriva SIGUSR1
void gestore_notifica(int);

// setto termina quando arriva uno dei segnali
void gestore_terminazione(int);

// creo l'azione e la installo
void crea_azione(int, funzione);

#endif
