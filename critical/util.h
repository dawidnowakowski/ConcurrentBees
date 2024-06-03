#ifndef UTILH
#define UTILH
#include "main.h"

/* typ pakietu */
typedef struct
{
    int ts; /* timestamp (zegar lamporta */
    int src;
    int data; /* przykładowe pole z danymi; można zmienić nazwę na bardziej pasującą */
} packet_t;

typedef struct {
    int pid;
    int timestamp;
} request;

extern request WaitQueueFlowers[];
extern request WaitQueueReeds[];



/* packet_t ma trzy pola, więc NITEMS=3. Wykorzystane w inicjuj_typ_pakietu */
#define NITEMS 3


/* Typy wiadomości */
/* TYPY PAKIETÓW */
// #define ACK 1
// #define REQUEST 2
// #define RELEASE 3
// #define APP_PKT 4
// #define FINISH 5

#define REQflower 1
#define ACKflower 2
#define REQreed 3
#define ACKreed 4
#define RELEASEreed 5



extern MPI_Datatype MPI_PAKIET_T;


// pakiety należy zainicjować, ponieważ MPI nie jest przystosowane do wysyłania złożonych struktur danych. Zainicjowanie pozwala na przesyłanie nowych typów MPI.
void inicjuj_typ_pakietu();


/* wysyłanie pakietu, skrót: wskaźnik do pakietu (0 oznacza stwórz pusty pakiet), do kogo, z jakim typem */
void sendPacket(packet_t *pkt, int destination, int tag);

void sendREQ(packet_t *pkt, int destination, int tag);

int compare_requests(const void *a, const void *b);

void add_reed_request(int pid, int timestamp, request *WaitQueueReeds, int *current_size);

void add_flower_request(int pid, int timestamp, request *WaitQueueFlowers, int *current_size);

void printWaitQueueReeds(request *WaitQueueReeds, int current_size);

typedef enum
{
    InRun,
    InMonitor,
    InWant,
    InSection,
    InFinish,
    REST,
    WAIT_REED,
    ON_REED,
    WAIT_FLOWER,
    ON_FLOWER,
    DEAD
} state_t;
extern state_t stan;
extern pthread_mutex_t stateMut;
/* zmiana stanu, obwarowana muteksem */
void changeState(state_t);
#endif
