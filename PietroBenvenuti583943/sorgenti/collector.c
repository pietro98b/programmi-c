#include "../intestazioni/collector.h"

int collector(){

    int errore;
    Sockaddr_un indirizzo;
    int accettare, comunicare, i, massimo;
    char ritorno='n';
    Puntatore_risultati testa = NULL;
    fd_set lettura, temporanea;
    sigset_t maschera;

    sigemptyset(&maschera);
    sigaddset(&maschera, SIGHUP);
    sigaddset(&maschera, SIGINT);
    sigaddset(&maschera, SIGQUIT);
    sigaddset(&maschera, SIGTERM);
    sigaddset(&maschera, SIGUSR1);
    pthread_sigmask(SIG_BLOCK, &maschera, NULL);

    accettare=fai_socket(AF_UNIX, SOCK_STREAM, 0, &indirizzo);
    if (accettare == -1)
        return -1;

    errore = bind(accettare, (Sockaddr*) &indirizzo, sizeof(indirizzo));
    if(errore == -1){
        perror("bind");
        return -1;
    }

    errore = listen(accettare, SOMAXCONN);
    if (errore == -1){
        perror("listen");
        return -1;
    }

    massimo = accettare;

    FD_ZERO(&lettura);
    FD_SET(accettare, &lettura);

    while(1){
        temporanea = lettura;
        errore = select(massimo+1, &temporanea, NULL, NULL, NULL);
        if (errore == -1){
            perror("select");
            return -1;
        }

        for(i = 0; i<=massimo; i++){
            if (FD_ISSET(i, &temporanea)){
                if (i == accettare){
                    comunicare = accept(accettare, NULL, NULL);
                    if (comunicare == -1){
                        perror("accept");
                        return -1;
                    }
                    FD_SET(comunicare, &lettura);
                    if(massimo < comunicare)
                        massimo = comunicare;
                } else {
                    ritorno=prendi_argomenti(i, &testa);
                    FD_CLR(i, &lettura);
                    massimo = aggiorna(&lettura, massimo);
                    if (ritorno == 'f')
                        break;
                }
            }
        }

        if (ritorno == 'f')
            break;
    }
    close(accettare);
    stampa(testa);
    fflush(stdout);
    distruggi_lista_risultati(&testa);
    unlink(NOME_SOCKET);
    return 0;

}

char prendi_argomenti(int comunicare, Puntatore_risultati* testa){
    long somma;
    size_t lunghezza;
    char* percorso_relativo;
    int errore;

    readn(comunicare, &lunghezza, sizeof(size_t));

    percorso_relativo = malloc(sizeof(char)*lunghezza);
    memset(percorso_relativo, '\0', lunghezza);
    readn(comunicare, percorso_relativo, sizeof(char)*lunghezza);
    if(strcmp(percorso_relativo, "$F")==0) {
        free(percorso_relativo);
        return 'f';
    }
    if(strcmp(percorso_relativo, "$S")==0){
        stampa(*testa);
        fflush(stdout);
        return 'f';
    }

    readn(comunicare, &somma, sizeof(long));

    errore=inserisci_risultato(percorso_relativo, somma, testa);
    if (errore == -1)
        return 'n';

    free(percorso_relativo);

    percorso_relativo = NULL;

    close(comunicare);

    return 'n';
}

int aggiorna(fd_set* insieme, int massimo){
    int i;
    for(i=massimo; i >= 0; i--){
        if(FD_ISSET(i, insieme))
            return i;
    }
    return -1;
}

int inserisci_risultato(char* percorso_relativo, long somma, Puntatore_risultati* testa){
    Puntatore_risultati precedente, corrente;
    Puntatore_risultati nuovo= malloc(sizeof(Lista_risultati));
    if(nuovo == NULL){
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
    nuovo->somma = somma;

    precedente = NULL;
    corrente = *testa;
    while(corrente != NULL && somma > corrente->somma){
        precedente = corrente;
        corrente = corrente->prossimo;
    }
    if(precedente == NULL){
        nuovo->prossimo = *testa;
        *testa = nuovo;
    } else {
        precedente->prossimo = nuovo;
        nuovo->prossimo = corrente;
    }
    return 0;
}

void stampa(Puntatore_risultati testa){
    while(testa != NULL){
        printf("%ld %s\n", testa->somma, testa->percorso_relativo);
        fflush(stdout);
        testa = testa->prossimo;
    }
}

void distruggi_lista_risultati(Puntatore_risultati* testa){
    while(*testa != NULL){
        Puntatore_risultati estratto = *testa;
        *testa = estratto->prossimo;
        free(estratto->percorso_relativo);
        free(estratto);
    }
}
