#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "../intestazioni/master.h"

#define DEFAULT_THREAD 4
#define DEFAULT_LUNGHEZZA 8
#define DEFAULT_TEMPO 0


// converto la stringa in numero
long isNumber (char* parola){
    char* stato = NULL;
    long valore = strtol(parola, &stato, 0);
    if (errno == ERANGE)
        return -1;
    if(stato != NULL && *stato ==(char) 0)
        return valore;
    return -2;
}

// controllo se isNumber non mi ha dato errori
void controllaIsNUmber (char* argomento, int ritorno){
    if (ritorno < 0){
        printf("l'argomento %s non e' valido\n", argomento);
        exit(-1);
    }
}

int main(int argc, char** argv){
    int scelta;
    char* nome_directory=NULL;
    int numero_thread = DEFAULT_THREAD;
    int tempo = DEFAULT_TEMPO;
    int lunghezza = DEFAULT_LUNGHEZZA;
    while ((scelta = getopt(argc, argv, "n:q:d:t:"))!=-1){
        switch(scelta){
            case 'd':
                nome_directory = malloc(sizeof(char)*(strlen(optarg)+1));
                memset(nome_directory, '\0', strlen(optarg)+1);
                if(nome_directory==NULL){
                    printf("memoria esaurita\n");
                    return -1;
                }
                strcpy(nome_directory, optarg);
                break;
            case 'n':
                numero_thread = isNumber(optarg);
                controllaIsNUmber(optarg, numero_thread);
                break;
            case 'q':
                lunghezza = isNumber(optarg);
                controllaIsNUmber(optarg, lunghezza);
                break;
            case 't':
                tempo = isNumber(optarg);
                controllaIsNUmber(optarg, tempo);
                break;
            default:
                printf("operazione %c non conosciuta\n", scelta);
        }
    }
    //printf("thread: %d, lunghezza: %d, directory: %s, tempo: %d\n", numero_thread, lunghezza, nome_directory, tempo);
    master(optind, argc, argv, numero_thread, lunghezza, tempo, nome_directory);
//    printf("\n");
    free(nome_directory);
    return 0;
}

