#include "main.h"
#include "watek_glowny.h"

void mainLoop()
{
	srandom(rank); // seed dla generatora
	int tag;	   // tagi dostępne są w util.h
	int perc;	   // deklaracja zmiennej, która będzie decydować, czy proces chce wykonać akcję czy poczeka.
	int reedId, indexInReed;
	int ackReedFull = FALSE;
	while (stan != InFinish)
	{
		switch (stan)
		{

		case REST:
			perc = random() % 100;
			if (perc < 25)
			{
				debug("Perc: %d", perc);
				println("Ubiegam się o trzcinę (sekcję krytyczną)")
					debug("Zmieniam stan na wysyłanie");
				packet_t *pkt = malloc(sizeof(packet_t));
				ackNumReed = 0;
				reqNumReed = 0;
				add_reed_request(rank, lamport, WaitQueueReeds, &reqNumReed); // dodanie samego siebie do kolejki
				changeState(WAIT_REED);
				for (int i = 0; i <= size - 1; i++)
					if (i != rank)
						sendPacket(pkt, i, REQreed);
				
				free(pkt);
			}
			debug("Skończyłem myśleć");
			break;
		case WAIT_REED:
			println("Czekam na wejście do sekcji krytycznej");
			// printf("%d, %d, %d\n",rank, ackNumReed, size - 1);

			if (ackReedFull)
			{
				if (reeds[reedId] == indexInReed) // czy jest nasza kolej aby wejść na trzcinę
				{
					println("Zmieniam stan na ON_REED 1");
					changeState(ON_REED);
					break;
				}
			}

			if (ackNumReed == size - 1)
			{
				printWaitQueueReeds(WaitQueueReeds, reqNumReed);
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
				int reedId = reedIndex % (size / t);
				// Oblicz jako który z kolei powinien dany proces na trzcinę wejść.
				int indexInReed = reedIndex / (size / t);
				printf("%d %d %d\n",rank, reedId, indexInReed);

				if (reeds[reedId] == indexInReed) // czy jest nasza kolej aby wejść na trzcinę
				{
					println("Zmieniam stan na ON_REED 2");
					changeState(ON_REED);
				} else { // jeżeli nie to ustaw zmienną sterującą, aby niepotrzebnie nie obliczać wszystkiego jeszcze raz
					ackReedFull = TRUE;
				}

				
			}

			break;
		// case InWant:
		// 	println("Czekam na wejście do sekcji krytycznej")
		// 		// tutaj zapewne jakiś semafor albo zmienna warunkowa
		// 		// bo aktywne czekanie jest BUE
		// 		if (ackCount == size - 1)
		// 			changeState(InSection);
		// 	break;
		// case InSection:
		// 	// tutaj zapewne jakiś muteks albo zmienna warunkowa
		// 	println("Jestem w sekcji krytycznej")
		// 		sleep(5);
		// 	// if ( perc < 25 ) {
		// 	debug("Perc: %d", perc);
		// 	println("Wychodzę z sekcji krytycznej")
		// 		debug("Zmieniam stan na wysyłanie");
		// 	packet_t *pkt = malloc(sizeof(packet_t));
		// 	pkt->data = perc;
		// 	for (int i = 0; i <= size - 1; i++)
		// 		if (i != rank)
		// 			sendPacket(pkt, (rank + 1) % size, RELEASE);
		// 	changeState(InRun);
		// 	free(pkt);
		// 	//}
		// 	break;

		default:
			break;
		}
		sleep(SEC_IN_STATE);
	}
}
