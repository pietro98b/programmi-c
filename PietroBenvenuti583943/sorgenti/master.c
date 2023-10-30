#include "../intestazioni/master.h"


volatile sig_atomic_t termina = 'n';

volatile sig_atomic_t notifica = 'n';

int master(int inizio, int fine, char** argv, int numero_thread, int lunghezza, int tempo, char* nome_directory){
    pid_t pid;
    int i, errore;
    Threadpool *pool;

    crea_azione(SIGHUP, gestore_terminazione);
    crea_azione(SIGINT, gestore_terminazione);
    crea_azione(SIGQUIT, gestore_terminazione);
    crea_azione(SIGTERM, gestore_terminazione);
    crea_azione(SIGUSR1, gestore_notifica);
    crea_azione(SIGPIPE, SIG_IGN);


    pid = fork();
    switch(pid){
    case -1:
        perror("fork");
        return -1;
    case 0:
        collector();
        return 0;
    default:
        break;
    }

    pool = inizializza(numero_thread, lunghezza, argv[fine-1]);
    if (pool == NULL)
        return -1;

    i=0;
    while(i<numero_thread){
        errore = pthread_create(&pool->thread[i], NULL, prendi_file, pool);
        if (errore != 0){
            printf("errore nella creazione di thread");
            return -1;
        }
        i++;
    }

    i = inizio-1;

    // esco se i è maggiore di o uguale a fine (argc) o termina è settato
    while(i<fine && termina == 'n'){
        // dico al cliente di stampare immediatamente\n
        if(notifica == 's'){
            connetti(3, 0, "$S");
        }
        // chiamo isType quando i è maggiore di argc (inizio) -1 e prendo l'iesimo elemento del vettore
        // altrimenti se è uguale chiamo isType con il nome della directory
        if(i > inizio-1) {
            errore=isType(argv[i], pool, tempo);
            if(errore == -1)
                return -1 ;
        }
        else {
            errore=isType(nome_directory, pool, tempo);
            if(errore == -1)
                return -1;
        }
        i++;
    }

    pool->uscita = 'c';
    lock(&pool->mutex);
    errore = sveglia_tutti(&pool->prodotto);
    if (errore == -1){
        return -1;
    }
    unlock(&pool->mutex);

    for(i=0; i<numero_thread; i++){
        errore = pthread_join(pool->thread[i], NULL);
        if (errore != 0){
            printf("errore nell'attesa di thread");
            return -1;
        }
    }

    distruggi_threadpool(pool);

    connetti(3, 0, "$F");

    waitpid(pid, NULL, 0);

    return 0;
}



int isType(char* percorso_relativo, Threadpool* pool, int tempo){
    Stat statistiche;
    int errore = stat(percorso_relativo, &statistiche);
    if (errore == -1){
        perror("stat");
        return -1;
    }

    if(S_ISDIR(statistiche.st_mode)) {
        errore = leggi_directory(percorso_relativo, pool, tempo);
        if (errore == -1)
            return -1;
    }

    if(S_ISREG(statistiche.st_mode)) {
        errore = lock(&pool->mutex);
        if(errore == -1)
            return -1;

        while(pool->conta_celle == pool->lunghezza) {
            errore = aspetta(&pool->consumato, &pool->mutex);
            if(errore == -1)
                return -1;
        }
        if(pool->conta_celle < pool->lunghezza) {
            errore = inserisci_file(&pool->coda, percorso_relativo);
            if (errore == -1)
                return -1;
            pool->conta_celle++;
        }


        errore=sveglia(&pool->prodotto);
        if (errore == -1)
            return -1;

        errore=unlock(&pool->mutex);
        if (errore == -1)
            return -1;

        struct timespec req;
        struct timespec rem;
        req.tv_sec = 0;
        req.tv_nsec = CONVERTI(tempo);
        memset(&rem, 0, sizeof(rem));

        errore=nanosleep(&req, &rem);
        if (errore == -1 && errno == EINTR){
            nanosleep(&rem, NULL);
        }
    }
    return 0;
}

int leggi_directory(char* nome_directory, Threadpool* pool, int tempo){
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

        if(isType(percorso_relativo, pool, tempo) == -1)
            return -1;

        errno = 0;
    }

    if(errno != 0){
        perror("readdir");
        return -1;
    }

    if(closedir(directory_corrente)==-1){
        perror("closedir");
        return -1;
    }
    free(percorso_relativo);
    return 0;
}

void gestore_terminazione(int numero){
    termina = 's';
}

void gestore_notifica(int numero){
    notifica = 's';
}

void crea_azione(int segnale, funzione gestore){
    Sigaction azione;
    memset(&azione, 0, sizeof(azione));
    azione.sa_handler = gestore;
    int errore=sigaction(segnale, &azione, NULL);
    if (errore == -1){
        printf("fallimento sigaction\n");
        exit(-1);
    }
}

