#include "main.h"
#include "watek_glowny.h"

void mainLoop()
{
	srandom(rank); // seed dla generatora
	int tag; // tagi dostępne są w util.h
	int perc; // deklaracja zmiennej, która będzie decydować, czy proces chce wykonać akcję czy poczeka.

	while (stan != InFinish)
	{
		switch (stan)
		{
		case InRun:
			perc = random() % 100;
			if (perc < 25)
			{
				debug("Perc: %d", perc);
				println("Ubiegam się o sekcję krytyczną")
					debug("Zmieniam stan na wysyłanie");
				packet_t *pkt = malloc(sizeof(packet_t));
				pkt->data = perc;
				ackCount = 0;
				for (int i = 0; i <= size - 1; i++)
					if (i != rank)
						sendPacket(pkt, i, REQUEST);
				changeState(InWant);
				free(pkt);
			}
			debug("Skończyłem myśleć");
			break;

		case REST:
			perc = random() % 100;
			if (perc < 25)
			{
				debug("Perc: %d", perc);
				println("Ubiegam się o trzcinę (sekcję krytyczną)")
					debug("Zmieniam stan na wysyłanie");
				packet_t *pkt = malloc(sizeof(packet_t));
				ackNumReed = 0;
				for (int i = 0; i <= size - 1; i++)
					if (i != rank)
						sendPacket(pkt, i, REQreed);
				changeState(WAIT_REED);
				free(pkt);
			}
			debug("Skończyłem myśleć");
		case WAIT_REED:
			println("Czekam na wejście do sekcji krytycznej")
				// tutaj zapewne jakiś semafor albo zmienna warunkowa
				// bo aktywne czekanie jest BUE
				if (ackNumReed == size - 1)
					changeState(InSection);
			break;
		case InWant:
			println("Czekam na wejście do sekcji krytycznej")
				// tutaj zapewne jakiś semafor albo zmienna warunkowa
				// bo aktywne czekanie jest BUE
				if (ackCount == size - 1)
					changeState(InSection);
			break;
		case InSection:
			// tutaj zapewne jakiś muteks albo zmienna warunkowa
			println("Jestem w sekcji krytycznej")
				sleep(5);
			// if ( perc < 25 ) {
			debug("Perc: %d", perc);
			println("Wychodzę z sekcji krytycznej")
				debug("Zmieniam stan na wysyłanie");
			packet_t *pkt = malloc(sizeof(packet_t));
			pkt->data = perc;
			for (int i = 0; i <= size - 1; i++)
				if (i != rank)
					sendPacket(pkt, (rank + 1) % size, RELEASE);
			changeState(InRun);
			free(pkt);
			//}
			break;

		default:
			break;
		}
		sleep(SEC_IN_STATE);
	}
}
