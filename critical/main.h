#ifndef MAINH
#define MAINH
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "util.h"
/* boolean */
#define TRUE 1
#define FALSE 0
#define SEC_IN_STATE 1
// #define ROOT 0

/* tutaj TYLKO zapowiedzi - definicje w main.c */
extern int rank;
extern int size;
extern int ackCount;
extern pthread_t threadKom;
extern int lamport;

//projekt
// extern int p; // pszczółki
// extern int k; // kwiatki
// extern int t; // trzciny
#define t 3 // trzciny, pszczół może być max. t * 3 (bo jedna trzicna to max. 15 jaj, a jedna pszczoła składa 5 jaj)
#define p 9 // pszczółki, <1; t*3>
#define k 3 // kwiatki, dowolna ilość, ale > 0
extern int AckNumFlower; // licznik otrzymanych ACKflower, początkowo 0
extern int AckNumReed; // licznik otrzymanych ACKreed, początkowo 0
extern int layedEggs; // licznik złożonych jaj, początkowo 0, max. 5
#define FULL 0
#define AVAILABLE 1
typedef struct {
    int pid;
    int timestamp;
} request;
extern request WaitQueueFlowers[];
extern request WaitQueueReeds[];

/* macro debug - działa jak printf, kiedy zdefiniowano
   DEBUG, kiedy DEBUG niezdefiniowane działa jak instrukcja pusta

   używa się dokładnie jak printfa, tyle, że dodaje kolorków i automatycznie
   wyświetla rank

   w związku z tym, zmienna "rank" musi istnieć.

   w printfie: definicja znaku specjalnego "%c[%d;%dm [%d]" escape[styl bold/normal;kolor [RANK]
                                           FORMAT:argumenty doklejone z wywołania debug poprzez __VA_ARGS__
                       "%c[%d;%dm"       wyczyszczenie atrybutów    27,0,37
                                            UWAGA:
                                                27 == kod ascii escape.
                                                Pierwsze %c[%d;%dm ( np 27[1;10m ) definiuje styl i kolor literek
                                                Drugie   %c[%d;%dm czyli 27[0;37m przywraca domyślne kolory i brak pogrubienia (bolda)
                                                ...  w definicji makra oznacza, że ma zmienną liczbę parametrów

*/
#ifdef DEBUG
#define debug(FORMAT, ...) printf("%c[%d;%dm [%d]: " FORMAT "%c[%d;%dm\n", 27, (1 + (rank / 7)) % 2, 31 + (6 + rank) % 7, rank, ##__VA_ARGS__, 27, 0, 37);
#else
#define debug(...) ;
#endif

// makro println - to samo co debug, ale wyświetla się zawsze
#define println(FORMAT, ...) printf("%c[%d;%dm ts:[%d] [%d]: " FORMAT "%c[%d;%dm\n", 27, (1 + (rank / 7)) % 2, 31 + (6 + rank) % 7, lamport, rank, ##__VA_ARGS__, 27, 0, 37);

#endif
