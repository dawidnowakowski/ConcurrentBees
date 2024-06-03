#include "main.h"
#include "watek_komunikacyjny.h"

#define max(a, b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

extern int lamport;
extern int reqNumFlower;
extern int reeds[];
// pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
/* wątek komunikacyjny; zajmuje się odbiorem i reakcją na komunikaty */
void *startKomWatek(void *ptr)
{

    MPI_Status status;
    int is_message = FALSE;
    packet_t pakiet;
    /* Obrazuje pętlę odbierającą pakiety o różnych typach */

    while (stan != InFinish)
    {
        debug("czekam na recv");
        MPI_Recv(&pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        pthread_mutex_lock(&stateMut);
        int oldLamport = lamport;
        lamport = max(pakiet.ts, lamport) + 1;
        pthread_mutex_unlock(&stateMut);

        switch (status.MPI_TAG)
        {

        // projekt
        case REQreed:
            debug("Dostałem REQreed od %d, odsyłam ACKreed", status.MPI_SOURCE);
            int timestamp = pakiet.ts;
            int pid = pakiet.src;
            add_reed_request(pid, timestamp, WaitQueueReeds, &reqNumReed);
            sendPacket(0, status.MPI_SOURCE, ACKreed);
            // printWaitQueueReeds(WaitQueueReeds, reqNumReed);
            // if (reqNumReed == size)
            // {
            //     printWaitQueueReeds(WaitQueueReeds, reqNumReed);
            // }


            break;

        case ACKreed:
            debug("Dostałem ACKreed od %d, mam już %d", status.MPI_SOURCE, ackNumReed);
            if (stan == WAIT_REED)
            {
                ackNumReed++;
            }

            break;

        case REQflower:
            if (stan == REST || stan == WAIT_REED || stan == ON_REED)
            {
                debug("Dostałem REQflower od %d, ale nie mam potrzeby wejść na kwiatek, więc odsyłam ACK", status.MPI_SOURCE);
                sendPacket(0, status.MPI_SOURCE, ACKflower);
            }
            else if (stan == ON_FLOWER)
            {
                debug("Dostałem REQflower od %d, ale jestem na kwiatku, więc dodaję do kolejki", status.MPI_SOURCE);
                add_flower_request(pid, timestamp, WaitQueueFlowers, &reqNumFlower);
                reqNumFlower++;
                // printf("%d req inc 1 %d\n", rank, reqNumFlower);
            }
            else if (stan == WAIT_FLOWER)
            {
                if (pakiet.ts < oldLamport)
                {
                    debug("Dostałem REQflower od %d, czekam na kwiatek, ale mój lamport jest większy, więc odsyłam ACK", status.MPI_SOURCE);
                    sendPacket(0, status.MPI_SOURCE, ACKflower);
                }
                else
                {
                    debug("Dostałem REQflower od %d, czekam na kwiatek, ale mój lamport jest mniejszy lub równy, więc dodaje go do kolejki", status.MPI_SOURCE);
                    add_flower_request(pid, timestamp, WaitQueueFlowers, &reqNumFlower);
                    reqNumFlower++;
                    // printf("%d req inc 2 %d\n", rank, reqNumFlower);
                }
            }

            break;

        case ACKflower:
            
            if (stan == WAIT_FLOWER)
            {
                debug("Dostałem ACKflower od %d i go przyjąłem, mam już %d", status.MPI_SOURCE, ackNumFlower);
                ackNumFlower++;
            } else{
                debug("Dostałem ACKflower od %d ale nie czekam", status.MPI_SOURCE);
            }
            break;

        case RELEASEreed:
            debug("Dostałem RELEASEreed od %d", status.MPI_SOURCE);
            // printf("index %d\n", pakiet.data);
            int index = pakiet.data; // po otrzymaniu indexu zwolnionej trzicny zwiększ wartość na jej indexie o 1. 
            reeds[index]++;
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
