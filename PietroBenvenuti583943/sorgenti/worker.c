#include "../intestazioni/worker.h"


int calcola_file(char* percorso_relativo){
    long valore, somma, contatore, errore;
    int file;
    ssize_t letti;
    somma = contatore = 0;
    file = open(percorso_relativo, O_RDONLY);
    if (file == -1){
        perror("open");
        return -1;
    }

    while((letti = readn(file, &valore, sizeof(long))) > 0){
        somma += valore * contatore;
        contatore++;
    }

    if(letti == -1){
        perror("read");
        return -1;
    }
//    printf("thread: %ld; somma: %ld; percorso relativo: %s\n", pthread_self(), somma, percorso_relativo);
    errore=connetti(strlen(percorso_relativo)+1, somma, percorso_relativo);
    if(errore == -1)
        return -1;
    errore = close(file);
    if(errore == -1){
        perror("close");
        return -1;
    }
    return 0;
}

int inserisci_file(Puntatore_file* testa, char* percorso_relativo){
    Puntatore_file nuovo = malloc(sizeof(Coda_file));
    if(nuovo==NULL){
        printf("memoria esaurita\n");
        return -1;
    }

    nuovo->percorso_relativo = malloc(sizeof(char)*(strlen(percorso_relativo)+1));
    if(nuovo->percorso_relativo == NULL){
        printf("memoria esaurita\n");
        return -1;
    }
    memset(nuovo->percorso_relativo, '\0', strlen(percorso_relativo)+1);
    strcpy(nuovo->percorso_relativo, percorso_relativo);

    nuovo->prossimo = NULL;

    if(*testa == NULL){
        *testa = nuovo;
    } else {
        Puntatore_file corrente = *testa;
        while(corrente->prossimo != NULL)
            corrente = corrente->prossimo;
        corrente->prossimo = nuovo;
    }
    return 0;
}

char* estrai_file(Puntatore_file* testa){
    if(*testa!=NULL){
        Puntatore_file estratto = *testa;

        char* percorso_relativo = malloc(sizeof(char)*(strlen(estratto->percorso_relativo)+1));
        memset(percorso_relativo, '\0', strlen(estratto->percorso_relativo)+1);
        if (percorso_relativo == NULL){
            printf("memoria esaurita\n");
            return NULL;
        }
        strcpy(percorso_relativo, estratto->percorso_relativo);

        *testa = estratto->prossimo;

        free(estratto->percorso_relativo);
        free(estratto);
        return percorso_relativo;
    }
    return NULL;
}

void* prendi_file(void* argomento){
    Threadpool* pool = (Threadpool*) argomento;
    char* percorso_relativo;
    int errore;
    while(pool->uscita != 'u'){
        errore = lock(&pool->mutex);
        if (errore == -1)
            return NULL;
        while(pool->uscita == 'n' && pool->coda == NULL) {
            errore = aspetta(&pool->prodotto, &pool->mutex);
            if (errore == -1)
                return NULL;
        }

        if (pool->uscita == 'u' && pool->coda != NULL){
           pool->uscita='c';
        }

        percorso_relativo = estrai_file(&pool->coda);
        if(percorso_relativo == NULL) {
            pool->uscita = 'u';
            continue;
        }

        pool->conta_celle--;

        stampa_coda(pool->coda);

        errore=sveglia(&pool->consumato);
        if(errore == -1)
            return NULL;

        errore=unlock(&pool->mutex);
        if (errore == -1)
            return NULL;
        errore=calcola_file(percorso_relativo);
        if (errore == -1){
            return NULL;
        }

        free(percorso_relativo);
    }
    sveglia_tutti(&pool->consumato);
    unlock(&pool->mutex);
    return NULL;
}

int lock(pthread_mutex_t* mutex) {
    int errore = pthread_mutex_lock(mutex);
    if (errore != 0){
        printf("lock");
        return -1;
    }
    return 0;
}

int unlock(pthread_mutex_t* mutex) {
    int errore = pthread_mutex_unlock(mutex);
    if (errore != 0){
        printf("unlock");
        return -1;
    }
    return 0;
}

Threadpool* inizializza(int numero_threads, int lunghezza){
    int errore1, errore2;
    Threadpool* pool = malloc(sizeof(Threadpool));
    if(pool == NULL){
        printf("memoria esaurita\n");
        return NULL;
    }

    errore1 = inizializza_condizione(&pool->prodotto, NULL);
    errore2 = inizializza_condizione(&pool->consumato, NULL);
    if(errore1 == -1 || errore2 == -1)
        return NULL;

    errore1 = pthread_mutex_init(&pool->mutex, NULL);
    if(errore1 != 0){
        printf("init mutex");
        return NULL;
    }

    pool->thread = malloc(sizeof(pthread_t)*numero_threads);
    if(pool->thread == NULL){
        printf("memoria esaurita");
        return NULL;
    }

    pool->coda = NULL;
    pool->conta_celle = 0;
    pool->lunghezza = lunghezza;
    pool->uscita = 'n';

    return pool;
}

void distruggi_coda_file(Puntatore_file* testa){
    while(*testa != NULL){
        Puntatore_file estratto = *testa;
        *testa = estratto->prossimo;
        free(estratto->percorso_relativo);
        free(estratto);
    }
}

void distruggi_threadpool(Threadpool* pool){
    int errore1, errore2;

    distruggi_coda_file(&pool->coda);

    errore1=distruggi_condizione(&pool->prodotto);
    errore2=distruggi_condizione(&pool->consumato);
    if(errore1 == -1 || errore2 == -1)
        exit(-1);

    errore1=pthread_mutex_destroy(&pool->mutex);

    free(pool->thread);

    free(pool);
}

int connetti(size_t lunghezza, long somma, char* percorso_relativo){
    int errore;
    int sock;
    Sockaddr_un indirizzo;

    sock = fai_socket(AF_UNIX, SOCK_STREAM, 0, &indirizzo);
    if(sock == -1)
        return -1;

    errore = connect(sock, (Sockaddr*) &indirizzo, sizeof(indirizzo));
    if (errore == -1){
        perror("connect");
        return -1;
    }

    writen(sock, &lunghezza, sizeof(size_t));
    writen(sock, percorso_relativo, sizeof(char)*lunghezza);
    if(strcmp(percorso_relativo, "$F") == 0) // f sta per fine
        return 0;
    if(strcmp(percorso_relativo, "$S") == 0) // s sta per stampa
        return 0;

    writen(sock, &somma, sizeof(long));
    return 0;
}

int aspetta(pthread_cond_t* condizione, pthread_mutex_t* mutex){
    int errore = pthread_cond_wait(condizione, mutex);
    if(errore != 0){
        printf("cond wait\n");
        return -1;
    }
    return 0;
}

int sveglia (pthread_cond_t* condizione){
    int errore = pthread_cond_signal(condizione);
    if (errore != 0){
        printf("cond signal\n");
        return -1;
    }
    return 0;
}

int sveglia_tutti(pthread_cond_t* condizione){
    int errore = pthread_cond_broadcast(condizione);
    if(errore != 0){
        printf("cond broadcast\n");
        return -1;
    }
    return 0;
}

int inizializza_condizione(pthread_cond_t* condizione, pthread_condattr_t* attributi){
    int errore = pthread_cond_init(condizione, attributi);
    if (errore != 0){
        printf("init cond\n");
        return -1;
    }
    return 0;
}

int distruggi_condizione(pthread_cond_t* condizione){
    int errore = pthread_cond_destroy(condizione);
    if(errore != 0){
        printf("cond destroy\n");
        return -1;
    }
    return 0;
}

void stampa_coda(Puntatore_file testa){
    while(testa != NULL){
        printf("%s -> ", testa->percorso_relativo);
        testa = testa->prossimo;
    }
    printf("NULL\n");
}
