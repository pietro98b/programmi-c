#include "../intestazioni/master.h"
#include "../intestazioni/worker.h"


int master(int inizio, int fine, char** argv, int numero_thread, int lunghezza, int tempo, char* nome_directory){
    pid_t pid;
    int i, errore;
    Threadpool *pool;
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

    pool = inizializza(numero_thread, lunghezza);
    if (pool == NULL)
        return -1;


    for(i=0; i<numero_thread; i++){
        errore = pthread_create(&pool->thread[i], NULL, prendi_file, pool);
        if (errore != 0){
            printf("errore nella creazione di thread");
            return -1;
        }
    }

    for(i=inizio; i<=fine; i++){
        if(i < fine) {
            errore=isType(argv[i], pool);
            if(errore == -1)
                return -1 ;
        }
        else {
            errore=isType(nome_directory, pool);
            if(errore == -1)
                return -1;
        }
    }

    sleep(tempo);

    pool->uscita = 'u';

    lock(&pool->mutex);
    errore = pthread_cond_broadcast(&pool->cond);
    if (errore != 0){
        printf("broadcast\n");
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

    connetti(2, 0, "(");

    waitpid(pid, NULL, 0);

    return 0;
}
