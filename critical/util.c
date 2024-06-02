#include "main.h"
#include "util.h"
MPI_Datatype MPI_PAKIET_T;

/*
 * w util.h extern state_t stan (czyli zapowiedź, że gdzieś tam jest definicja
 * tutaj w util.c state_t stan (czyli faktyczna definicja)
 */
// state_t stan = REST; // początkowy stan to REST
state_t stan = InRun; // początkowy stan to REST
extern int lamport;  // lamport jest zainicjaluzowany w main.c

pthread_mutex_t stateMut = PTHREAD_MUTEX_INITIALIZER; // muteks na zmianę stanu

// obsługa tagów
struct tagNames_t
{
    const char *name;
    int tag;
} tagNames[] = {{"pakiet aplikacyjny", APP_PKT}, {"finish", FINISH}, {"potwierdzenie", ACK}, {"prośbę o sekcję krytyczną", REQUEST}, {"zwolnienie sekcji krytycznej", RELEASE}};

const char *const tag2string(int tag)
{
    for (int i = 0; i < sizeof(tagNames) / sizeof(struct tagNames_t); i++)
    {
        if (tagNames[i].tag == tag)
            return tagNames[i].name;
    }
    return "<unknown>";
}

/* tworzy typ MPI_PAKIET_T
 */
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

void inicjuj_typ_req_ack()
{
    int blocklengths[NITEMS_REQ_ACK] = {1, 1};
    MPI_Datatype typy[NITEMS_REQ_ACK] = {MPI_INT, MPI_INT};

    MPI_Aint offsets[NITEMS_REQ_ACK];
    offsets[0] = offsetof(req_ack_message, pid);
    offsets[1] = offsetof(req_ack_message, timestamp);

    MPI_Type_create_struct(NITEMS_REQ_ACK, blocklengths, offsets, typy, &MPI_REQ_ACK);
    MPI_Type_commit(&MPI_REQ_ACK);
}

/* tworzy typ MPI_RELEASE_T */
void inicjuj_typ_release()
{
    int blocklengths[NITEMS_RELEASE] = {1, 1, 1};
    MPI_Datatype typy[NITEMS_RELEASE] = {MPI_INT, MPI_INT, MPI_INT};

    MPI_Aint offsets[NITEMS_RELEASE];
    offsets[0] = offsetof(release_message, pid);
    offsets[1] = offsetof(release_message, timestamp);
    offsets[2] = offsetof(release_message, status);

    MPI_Type_create_struct(NITEMS_RELEASE, blocklengths, offsets, typy, &MPI_RELEASE);
    MPI_Type_commit(&MPI_RELEASE);
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

/* wysyłanie wiadomości req_ack_message */
void sendReqAck(req_ack_message *msg, int destination, int tag)
{
    int freemsg = 0;
    if (msg == 0)
    {
        msg = malloc(sizeof(req_ack_message));
        freemsg = 1;
    }
    msg->pid = rank;

    pthread_mutex_lock(&stateMut);
    msg->timestamp = ++lamport;
    pthread_mutex_unlock(&stateMut);

    MPI_Send(msg, 1, MPI_REQ_ACK, destination, tag, MPI_COMM_WORLD);
    debug("Wysyłam %s do %d\n", tag2string(tag), destination);
    if (freemsg)
        free(msg);
}

/* wysyłanie wiadomości release_message */
void sendRelease(release_message *msg, int destination, int tag)
{
    int freemsg = 0;
    if (msg == 0)
    {
        msg = malloc(sizeof(release_message));
        freemsg = 1;
    }
    msg->pid = rank;

    pthread_mutex_lock(&stateMut);
    msg->timestamp = ++lamport;
    pthread_mutex_unlock(&stateMut);

    MPI_Send(msg, 1, MPI_RELEASE, destination, tag, MPI_COMM_WORLD);
    debug("Wysyłam %s do %d\n", tag2string(tag), destination);
    if (freemsg)
        free(msg);
}

void changeState(state_t newState)
{
    pthread_mutex_lock(&stateMut);
    // if (stan == DEAD)
    if (stan == InFinish)
    {
        pthread_mutex_unlock(&stateMut);
        return;
    }
    stan = newState;
    pthread_mutex_unlock(&stateMut);
}
