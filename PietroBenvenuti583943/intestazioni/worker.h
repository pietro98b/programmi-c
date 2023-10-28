

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
#include <signal.h>

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
    pthread_cond_t prodotto;
    pthread_cond_t consumato;
    pthread_t* thread;
    int conta_celle;
    int lunghezza;
    Puntatore_file coda;
    volatile sig_atomic_t uscita; // n = "non uscita", u = "uscita", c = "continua a lavorare (worker)"
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

// aspetto che qualcuno faccia la cond signal
int aspetta(pthread_cond_t*, pthread_mutex_t*);

// sveglio il thread
int sveglia(pthread_cond_t*);

// sveglio tutti i thread
int sveglia_tutti(pthread_cond_t*);

// inizializzo la variabile di condizione
int inizializza_condizione(pthread_cond_t*, pthread_condattr_t*);

// distruggo la variabile di condizione
int distruggi_condizione(pthread_cond_t*);

// stampo la coda per verificare
void stampa_coda(Puntatore_file);
