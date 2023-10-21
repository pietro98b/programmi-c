#include "../intestazioni/worker.h"
#include "../intestazioni/conn.h"



int isType(char* percorso_relativo, Threadpool* pool){
    Stat statistiche;
    int errore = stat(percorso_relativo, &statistiche);
    if (errore == -1){
        perror("stat");
        return -1;
    }

    if(S_ISDIR(statistiche.st_mode)) {
        errore = leggi_directory(percorso_relativo, pool);
        if (errore == -1)
            return -1;
    }

    if(S_ISREG(statistiche.st_mode)) {

        errore = lock(&pool->mutex);
        if(errore == -1)
            return -1;

        if(pool->conta_celle < pool->lunghezza) {
            errore = inserisci_file(&pool->coda, percorso_relativo);
            if (errore == -1)
                return -1;
            pool->conta_celle++;
        }

        errore=pthread_cond_signal(&pool->cond);
        if (errore == -1) {
            perror("cond signal\n");
            return -1;
        }

        errore=unlock(&pool->mutex);
        if (errore == -1)
            return -1;
    }
    return 0;
}

int leggi_directory(char* nome_directory, Threadpool* pool){
    DIR* directory_corrente;
    Dirent* file_corrente;
    char* percorso_relativo = malloc(sizeof(char)*MAX_NOME_FILE);
    if (percorso_relativo == NULL) {
        printf("memoria esaurita\n");
        return -1;
    }

    directory_corrente = opendir(nome_directory);
    if (directory_corrente == NULL){
        perror("opendir");
        return -1;
    }
    errno = 0;

    while((file_corrente = readdir(directory_corrente))!=NULL){
        if(strcmp(file_corrente->d_name, ".") == 0 || strcmp(file_corrente->d_name, "..")==0)
            continue;

        memset(percorso_relativo, '\0', MAX_NOME_FILE);
        strcpy(percorso_relativo, nome_directory);
        strcat(percorso_relativo, "/");
        strcat(percorso_relativo, file_corrente->d_name);

        if(isType(percorso_relativo, pool) == -1)
            return -1;

        errno = 0;
    }

    if(errno != 0){
        perror("readdir");
        return -1;
    }
    closedir(directory_corrente);
    free(percorso_relativo);
    return 0;
}

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
    printf("somma: %ld; percorso relativo: %s\n", somma, percorso_relativo);
    errore=connetti(strlen(percorso_relativo)+1, somma, percorso_relativo);
    if(errore == -1)
        return -1;
    close(file);
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
    while(pool->uscita == 'n'){
        errore = lock(&pool->mutex);
        if (errore == -1)
            return NULL;
        while(pool->uscita == 'n' && pool->coda == NULL) {
            errore = pthread_cond_wait(&pool->cond, &pool->mutex);
            if (errore != 0){
                printf("cond wait\n");
                return NULL;
            }
        }

        if (pool->uscita == 'u'){
            continue;
        }

        percorso_relativo = estrai_file(&pool->coda);

        pool->conta_celle--;

        errore=unlock(&pool->mutex);
        if (errore == -1)
            return NULL;

        errore=calcola_file(percorso_relativo);
        if (errore == -1){
            return NULL;
        }

        free(percorso_relativo);
    }

    errore=unlock(&pool->mutex);
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
    int errore;
    Threadpool* pool = malloc(sizeof(Threadpool));
    if(pool == NULL){
        printf("memoria esaurita\n");
        return NULL;
    }

    errore = pthread_cond_init(&pool->cond, NULL);
    if (errore != 0){
        printf("init cond\n");
        return NULL;
    }

    errore = pthread_mutex_init(&pool->mutex, NULL);
    if(errore != 0){
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
        free(estratto);
    }
}

void distruggi_threadpool(Threadpool* pool){
    distruggi_coda_file(&pool->coda);

    pthread_cond_destroy(&pool->cond);
    pthread_mutex_destroy(&pool->mutex);

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
    if(strcmp(percorso_relativo, "(") == 0)
        return 0;

    writen(sock, &somma, sizeof(long));
    return 0;
}

