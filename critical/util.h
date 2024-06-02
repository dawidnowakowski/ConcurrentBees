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

typedef struct
{
    int pid;
    int timestamp;
} req_ack_message;

typedef struct
{
    int pid;
    int timestamp;
    int status; // FULL or AVAILABLE
} release_message;
/* packet_t ma trzy pola, więc NITEMS=3. Wykorzystane w inicjuj_typ_pakietu */
#define NITEMS 3
#define NITEMS_REQ_ACK 2
#define NITEMS_RELEASE 3

/* Typy wiadomości */
/* TYPY PAKIETÓW */
#define ACK 1
#define REQUEST 2
#define RELEASE 3
#define APP_PKT 4
#define FINISH 5

#define REQflower 1
#define ACKflower 2
#define REQreed 3
#define ACKreed 4
#define REALEASEreed 5

extern MPI_Datatype MPI_PAKIET_T;
extern MPI_Datatype MPI_REQ_ACK;
extern MPI_Datatype MPI_RELEASE;

// pakiety należy zainicjować, ponieważ MPI nie jest przystosowane do wysyłania złożonych struktur danych. Zainicjowanie pozwala na przesyłanie nowych typów MPI.
void inicjuj_typ_pakietu();
void inicjuj_typ_req_ack();
void inicjuj_typ_release();

/* wysyłanie pakietu, skrót: wskaźnik do pakietu (0 oznacza stwórz pusty pakiet), do kogo, z jakim typem */
void sendPacket(packet_t *pkt, int destination, int tag);
void sendPacket(packet_t *pkt, int destination, int tag);
void sendReqAck(req_ack_message *msg, int destination, int tag);
void sendRelease(release_message *msg, int destination, int tag);

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
