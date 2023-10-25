#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

#include "../intestazioni/conn.h"

#define MAX_NOME_FILE 255

struct coda_file {
    char* percorso_relativo;
    struct coda_file* prossimo;
};

typedef struct stat Stat;
typedef struct dirent Dirent;
typedef struct coda_file Coda_file;

typedef Coda_file* Puntatore_file;

typedef struct threadpool {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    pthread_t* thread;
    int conta_celle;
    int lunghezza;
    Puntatore_file coda;
    int uscita;
} Threadpool;


// calcolo la formula data dal progetto
int calcola_file(char*);

// inserisco il file nella coda
int inserisci_file(Puntatore_file*, char*);

// estraggo il file dalla coda
char* estrai_file(Puntatore_file*);

// prendo il file dalla coda e lo calcolo
void* prendi_file(void*);

// inizializzo le variabili legate alla coda
Threadpool* inizializza(int, int);

int lock(pthread_mutex_t*);

int unlock(pthread_mutex_t*);

// libero la memoria dalla coda
void distruggi_coda_file(Puntatore_file*);

// libero la memoria dal pool
void distruggi_threadpool(Threadpool*);

// mando i valori al server
int connetti(size_t, long, char*);
