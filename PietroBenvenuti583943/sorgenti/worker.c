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

    // esco quando uscita è uguale a u
    while(pool->uscita != 'u'){
        errore = lock(&pool->mutex);
        if (errore == -1)
            return NULL;
        while(pool->uscita == 'n' && pool->coda == NULL) {
            errore = aspetta(&pool->prodotto, &pool->mutex);
            if (errore == -1)
                return NULL;
        }

        percorso_relativo = estrai_file(&pool->coda);


        if(percorso_relativo != NULL){
            if (strcmp(pool->ultimo_file, percorso_relativo) == 0)
                pool->uscita = 'u';
        } else
            break;


        pool->conta_celle--;


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
  //  stampa_coda(pool->coda);
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

Threadpool* inizializza(int numero_threads, int lunghezza, char* ultimo_file){
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

    pool->ultimo_file = malloc(sizeof(char)*(strlen(ultimo_file)+1));
    if(pool->ultimo_file == NULL){
        printf("memoria esaurita");
        return NULL;
    }
    memset(pool->ultimo_file, '\0', strlen(ultimo_file)+1);
    strcpy(pool->ultimo_file, ultimo_file);

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
    free(pool->ultimo_file);
    free(pool);
}

int connetti(size_t lunghezza, long somma, char* percorso_relativo){
    int errore;
    int sock;
    Sockaddr_un indirizzo;

    sock = fai_socket(AF_UNIX, SOCK_STREAM, 0, &indirizzo);
    if(sock == -1)
        return -1;

    // inizializzo errno per scrupolo e rifaccio connect quando mi dà errore ed errno è ENOENT o ECONNREFUSED
    do {
        errno = 0;
        errore = connect(sock, (Sockaddr*) &indirizzo, sizeof(indirizzo));
    } while(errore == -1 && (errno == ENOENT || errore == ECONNREFUSED));
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
    fflush(stdout);
}
