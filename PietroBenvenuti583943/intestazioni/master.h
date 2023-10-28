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


int master(int, int, char**, int, int, int, char*);


// controllo se il file e' regolare o una directory
int isType(char*, Threadpool*, int);

// leggo la directory
int leggi_directory(char*, Threadpool*, int);

void gestore_notifica(int);

void gestore_terminazione(int);

void crea_azione(int, funzione);

#endif
