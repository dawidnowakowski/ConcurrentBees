#include "main.h"
#include "util.h"
MPI_Datatype MPI_PAKIET_T;
MPI_Datatype MPI_REQ_ACK;
MPI_Datatype MPI_RELEASE;
/*
 * w util.h extern state_t stan (czyli zapowiedź, że gdzieś tam jest definicja
 * tutaj w util.c state_t stan (czyli faktyczna definicja)
 */
// state_t stan = REST; // początkowy stan to REST
state_t stan = REST; // początkowy stan to REST
extern int lamport;   // lamport jest zainicjaluzowany w main.c

pthread_mutex_t stateMut = PTHREAD_MUTEX_INITIALIZER; // muteks na zmianę stanu

// obsługa tagów
struct tagNames_t
{
    const char *name;
    int tag;
} tagNames[] = {{"Request flower", REQflower},{"Acknowledge flower", ACKflower},{"Request reed", REQreed},{"Acknowledge reed", ACKreed},{"Release reed", RELEASEreed}};

// {{"pakiet aplikacyjny", APP_PKT}, {"finish", FINISH}, {"potwierdzenie", ACK}, {"prośbę o sekcję krytyczną", REQUEST}, {"zwolnienie sekcji krytycznej", RELEASE}};

const char *const tag2string(int tag)
{
    for (int i = 0; i < sizeof(tagNames) / sizeof(struct tagNames_t); i++)
    {
        if (tagNames[i].tag == tag)
            return tagNames[i].name;
    }
    return "<unknown>";
}

/* tworzy typ MPI_PAKIET_T*/
void inicjuj_typ_pakietu()
{
    /* Stworzenie typu */
    /* Poniższe (aż do MPI_Type_commit) potrzebne tylko, jeżeli
       brzydzimy się czymś w rodzaju MPI_Send(&typ, sizeof(pakiet_t), MPI_BYTE....
    */
    /* sklejone z stackoverflow */
    int blocklengths[NITEMS] = {1, 1, 1};
    MPI_Datatype typy[NITEMS] = {MPI_INT, MPI_INT, MPI_INT};

    MPI_Aint offsets[NITEMS];
    offsets[0] = offsetof(packet_t, ts);
    offsets[1] = offsetof(packet_t, src);
    offsets[2] = offsetof(packet_t, data);

    MPI_Type_create_struct(NITEMS, blocklengths, offsets, typy, &MPI_PAKIET_T);

    MPI_Type_commit(&MPI_PAKIET_T);
}

/* opis patrz util.h */
void sendPacket(packet_t *pkt, int destination, int tag)
{
    int freepkt = 0;
    if (pkt == 0)
    {
        pkt = malloc(sizeof(packet_t));
        freepkt = 1;
    }
    pkt->src = rank;

    pthread_mutex_lock(&stateMut);
    pkt->ts = ++lamport;
    pthread_mutex_unlock(&stateMut);

    MPI_Send(pkt, 1, MPI_PAKIET_T, destination, tag, MPI_COMM_WORLD);
    debug("Wysyłam %s do %d\n", tag2string(tag), destination);
    if (freepkt)
        free(pkt);
}

void sendREQ(packet_t *pkt, int destination, int tag)
{
    pkt->src = rank;
    MPI_Send(pkt, 1, MPI_PAKIET_T, destination, tag, MPI_COMM_WORLD);
    debug("Wysyłam %s do %d z TS %d, pid %d\n", tag2string(tag), destination, pkt->ts, pkt->src);
}

void changeState(state_t newState)
{
    pthread_mutex_lock(&stateMut);
    if (stan == DEAD)
    {
        pthread_mutex_unlock(&stateMut);
        return;
    }
    stan = newState;
    pthread_mutex_unlock(&stateMut);
}

// funkcja do porównania dwóch requestów (po ts i pid)
int compare_requests(const void *a, const void *b)
{
    const request *req1 = (const request *)a;
    const request *req2 = (const request *)b;

    // Sortowanie rosnące po timestamp
    if (req1->timestamp != req2->timestamp)
    {
        return req1->timestamp - req2->timestamp;
    }
    // Sortowanie malejące po pid, gdy timestampy są równe
    return req1->pid - req2->pid;
}

// funkcja dodająca nowy request do tablicy WaitQueueReeds, current_size powinen być ackNumReed
void add_reed_request(int pid, int timestamp, request *WaitQueueReeds, int *current_size)
{
    request new_request = {pid, timestamp};

    WaitQueueReeds[*current_size] = new_request;
    
    (*current_size)++;

    // posortuj tablicę po dodaniu nowego elementu
    qsort(WaitQueueReeds, *current_size, sizeof(request), compare_requests);
}

void add_flower_request(int pid, int timestamp, request *WaitQueueFlowers, int *current_size)
{
    // Sprawdzenie, czy request o danym pid już istnieje w kolejce
    for (int i = 0; i < *current_size; i++) {
        if (WaitQueueFlowers[i].pid == pid) {
            return;
        }
    }

    // Jeśli request o danym pid nie istnieje, dodajemy nowy request
    request new_request = {pid, timestamp};
    WaitQueueFlowers[*current_size] = new_request;
    (*current_size)++;
}

void printWaitQueueReeds(request *WaitQueueReeds, int current_size)
{
    
    char buff[1024];
    buff[0]='\0';
    for (int i = 0; i < current_size; i++)
    {   
        char temp[100];
        snprintf(temp,sizeof(temp),"Request %d: PID = %d, Timestamp = %d\n", i, WaitQueueReeds[i].pid, WaitQueueReeds[i].timestamp);
        strncat(buff, temp, sizeof(buff)-strlen(buff)-1);
    }
    printf("WaitQueueReeds %d:\n", rank);
    printf("%s",buff);
}
