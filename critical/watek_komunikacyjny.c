#include "main.h"
#include "watek_komunikacyjny.h"

#define max(a, b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

extern int lamport;
// pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
/* wątek komunikacyjny; zajmuje się odbiorem i reakcją na komunikaty */
void *startKomWatek(void *ptr)
{

    MPI_Status status;
    int is_message = FALSE;
    packet_t pakiet;
    /* Obrazuje pętlę odbierającą pakiety o różnych typach */
    // while (stan != DEAD)
    while (stan != InFinish)
    {
        debug("czekam na recv");
        MPI_Recv(&pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        pthread_mutex_lock(&stateMut);
        lamport = max(pakiet.ts, lamport) + 1;
        pthread_mutex_unlock(&stateMut);

        switch (status.MPI_TAG)
        {
        // case REQUEST:
        //     debug("Ktoś coś prosi. A niech ma!")
        //         sendPacket(0, status.MPI_SOURCE, ACK);
        //     break;
        // case ACK:
        //     debug("Dostałem ACK od %d, mam już %d", status.MPI_SOURCE, ackCount);
        //     ackCount++; /* czy potrzeba tutaj muteksa? Będzie wyścig, czy nie będzie? Zastanówcie się. */
        //     break;

        // projekt
        case REQreed:
            debug("Dostałem REQreed od %d", status.MPI_SOURCE, ackNumReed);
            if (stan == REST || stan == WAIT_REED){ //odeślij ACKreed i wstaw request do tablicy WaitQueueReeds
                int timestamp = pakiet.ts;
                int pid = pakiet.src;
                add_request(pid, timestamp, WaitQueueReeds, &ackNumReed);
                sendPacket(0, status.MPI_SOURCE, ACKreed);
            break;


        case ACKreed:
            debug("Dostałem ACKreed od %d, mam już %d", status.MPI_SOURCE, ackNumReed);
            if (stan == WAIT_REED){
                ackNumReed++;
            }
            
            break;

        case REQflower:
            debug("Dostałem REQflower od %d", status.MPI_SOURCE, ackNumReed);
            if (stan == REST || stan == WAIT_REED){ //odeślij ACKflower
                sendPacket(0, status.MPI_SOURCE, ACKflower);
            }
            
            break;
        
        case ACKflower:
            debug("Dostałem ACKreed od %d, mam już %d", status.MPI_SOURCE, ackNumFlower);
            break;

        case RELEASEreed:
            debug("Dostałem RELEASEreed od %d", status.MPI_SOURCE);
            break;

        default:
            break;
        }
    }
}


// rudy
// stan REST: wydaje mi się, że mamy błąd w algorytmie i w stanie REST jako reakcja na REQreed powinniśmy odsyłać ACK, ale też wstawiać request do tablicy. Dodałem obsługę tego, ale przemyśl czy dobrze gadam. Poza tym stan REST powinien być skończony

// stan wait_reed jest w trakcie. Trzeba zrobić obsługę REQreed - trzeba zmienić funkcję add_request tak aby nie dodawała requesta, jeżeli już otrzymaliśmy wcześniej od tego kogoś wiadomość. Można to też obejść pewnie inaczej zmieniając startegię wysyłania wiadomości. Do przemyślenia.

// reszta stanów nie tknięta. W tych dwóch wyżej wymienionych też mogą być jakieś braki
