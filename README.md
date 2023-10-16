#Tesi triennale Andrea Fulcheri - Università degli Studi di Torino 22/23

##Istruzioni per l'immagine Docker

1. L'immagine è disponibile su Docker Hub al link https://hub.docker.com/repository/docker/andreafulcheri/ns3/general precompilata amd64 e arm64

2. E' possibile buildare l'immagine da zero tramite il comando `docker build -t ns3 .` nella cartella docker (a me ci ha messo circa 1 ora)

##Istruzioni per le simulazioni

1. Copiare la cartella tesi dentro la cartella scratch di NS3
2. Eseguire `ns3` per la compilazione
3. Eseguire `ns3 run scratch/tesi/{nome file}`
