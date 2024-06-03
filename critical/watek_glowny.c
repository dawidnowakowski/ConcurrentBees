#include "main.h"
#include "watek_glowny.h"

void mainLoop()
{
	srandom(rank - 100); // seed dla generatora
	int tag;			 // tagi dostępne są w util.h
	int perc;			 // deklaracja zmiennej, która będzie decydować, czy proces chce wykonać akcję czy poczeka.
	int reedId, indexInReed;
	int ackReedFull = FALSE;
	int hasNectar = FALSE;
	while (stan != InFinish)
	{
		switch (stan)
		{

		case REST:
			perc = random() % 100;
			if (perc < CHANCE)
			{
				debug("Perc: %d", perc);
				println("Ubiegam się o trzcinę (sekcję krytyczną)")
					debug("Zmieniam stan na wysyłanie");
				packet_t *pkt = malloc(sizeof(packet_t));
				pkt->ts = lamport;
				pthread_mutex_lock(&stateMut);
				++lamport;
				pthread_mutex_unlock(&stateMut);
				changeState(WAIT_REED);
				for (int i = 0; i <= size - 1; i++)
					// sendPacket(pkt, i, REQreed);
					sendREQ(pkt, i, REQreed);

				free(pkt);
			}
			debug("Skończyłem myśleć");
			break;
		case WAIT_REED:
			// println("Czekam na wejście na trzcinę (do sekcji krytycznej)");
			// printf("%d, %d, %d\n",rank, ackNumReed, size - 1);

			if (ackReedFull)
			{
				if (reeds[reedId] == indexInReed) // czy jest nasza kolej aby wejść na trzcinę
				{
					println("Doczekałem się, zmieniam stan na ON_REED");
					changeState(ON_REED);
					break;
				}
				else if (reeds[reedId] > 2)
				{ // jeżeli > 2 to znaczy, że już zostało żlożone 15 jaj i nie można wejść
					println("Trzcina już jest przepełniona, nie można wejść");
				}
			}

			if (ackNumReed == size)
			{

				// Znajdź indeks requestu o pid == rank
				int reedIndex = -1;
				for (int i = 0; i < size; ++i)
				{
					if (WaitQueueReeds[i].pid == rank)
					{
						reedIndex = i;
						break;
					}
				}

				// Oblicz do której trzciny należy dany proces
				reedId = reedIndex % t;
				// Oblicz jako który z kolei powinien dany proces na trzcinę wejść.
				indexInReed = reedIndex / t;
				// printf("REEDID %d %d %d,%d\n",rank, reedId, indexInReed, reeds[reedId]);

				if (reeds[reedId] == indexInReed) // czy jest nasza kolej aby wejść na trzcinę
				{
					println("Jestem pierwszy w kolejce, zmieniam stan na ON_REED");
					changeState(ON_REED);
				}
				else
				{ // jeżeli nie to ustaw zmienną sterującą, aby niepotrzebnie nie obliczać wszystkiego jeszcze raz
					ackReedFull = TRUE;
				}
			}

			break;

		case ON_REED:
			perc = random() % 100;
			if (perc < CHANCE)
			{
				debug("Perc: %d", perc);
				if (hasNectar)
				{
					layedEggs++;
					hasNectar = FALSE;
					// printf("jajo %d %d\n", rank, layedEggs);

					if (layedEggs == 5)
					{
						println("Składam jajo, Złożyłam już 5 jaj, umieram albo zostaję na trzcinie");

						// wyślij, że zwalniasz trzcinę do wszystkich
						packet_t *pkt = malloc(sizeof(packet_t));
						pthread_mutex_lock(&stateMut);
						pkt->ts = lamport;
						++lamport;
						pthread_mutex_unlock(&stateMut);
						pkt->data = reedId;
						changeState(DEAD);

						for (int i = 0; i <= size - 1; i++)
							sendREQ(pkt, i, RELEASEreed);
						free(pkt);
						println("Umarłam");
					}
					else
					{
						println("Składam jajo, Nie złożyłam wszystkich jaj, zostaję na trzcinie");
					}
				}
				else
				{

					println("Ubiegam się o kwiatek (sekcję krytyczną)")
						debug("Zmieniam stan na wysyłanie");
					packet_t *pkt = malloc(sizeof(packet_t));
					pthread_mutex_lock(&stateMut);
					pkt->ts = lamport;
					++lamport;
					pthread_mutex_unlock(&stateMut);
					changeState(WAIT_FLOWER);
					debug("Wysyłam REQflower");
					for (int i = 0; i <= size - 1; i++)
						if (i != rank)
						{
							sendREQ(pkt, i, REQflower);
						}

					free(pkt);
				}
			}
			break;
		case WAIT_FLOWER:
			// println("Czekam na wejście na kwiatek (do sekcji krytycznej)");
			// printf("pk %d, %d, %d\n", p, k, ackNumFlower);
			if (ackNumFlower >= p - k - deadbees)
			{
				println("Wchodzę na kwiatek (do sekcji krytycznej)");
				changeState(ON_FLOWER);
			}
			break;
		case ON_FLOWER:

			perc = random() % 100;
			if (perc < CHANCE)
			{
				debug("Perc: %d", perc);
				println("Jestem na kwiatku i zbieram nektar");
				hasNectar = TRUE;
				changeState(ON_REED);

				// przy opuszczaniu kwiatka poinformuj wszystkich, którzy czekają, że mogą wejść
				packet_t *pkt = malloc(sizeof(packet_t));
				pthread_mutex_lock(&stateMut);
				pkt->ts = lamport;
				++lamport;
				pthread_mutex_unlock(&stateMut);
				println("Schodzę z kwiatka");
				for (int i = 0; i < reqNumFlower; i++)
				{
					// debug("Wysyłam ACKflower do %d", WaitQueueFlowers[i].pid);
					// printf("ACKflower od %d do %d, %d\n", rank, WaitQueueFlowers[i].pid, reqNumFlower);
					sendPacket(pkt, WaitQueueFlowers[i].pid, ACKflower);
				}
				reqNumFlower = 0; // wyzeruj kolejkę
				ackNumFlower = 0; // wyzeruj liczbę pozwoleń
			}

			break;
		case DEAD:
			return;
			break;

		default:
			break;
		}
		sleep(SEC_IN_STATE);
	}
}
