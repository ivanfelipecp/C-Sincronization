/*
Sistemas Operativos
ii Semestre 2017

Proyecto # 2
	+ Iván Calvo
	+ Luis Quirós

gcc -pthread -o espia espia.c && ./espia <pasadas> 

Donde <pasadas> es la cantidad de veces a espiar
Donde <segmentos> es la cantidad de segmentos en la que se va dividir la memoria

*/

// Includes
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>  
#include <sys/ipc.h>
#include <sys/shm.h> 
#include <string.h>
#include <unistd.h> 
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <time.h>
#include <pthread.h>

// Defines
#define FILEKEY "/bin/cat"
#define KEY 1300
#define SEM_MEM "Semaforo_Memoria"
#define SEM_BIT "Semaforo_Bitacora"
#define FILE_BIT "bitacora.txt"
#define FILE_MEM "memoria.txt"
#define WAIT_TIME_SEC 10

int getMemorySize(){
	FILE *f = fopen(FILE_MEM, "r");
	char * line = NULL;
	size_t len = 0;

	getline(&line, &len, f);
	return atoi(line);
}

int main(int argc, char const *argv[])
{
	if(argc == 2){
		int key = ftok(FILEKEY, KEY);
		int memory = getMemorySize();
		if (key == -1) { 
			fprintf (stderr, "Error with key \n");
			return -1; 
		}

		/* we create the shared memory */
		int id_zone = shmget (key, sizeof(int)*memory, 0777 | IPC_CREAT);
		if (id_zone == -1) {
			fprintf (stderr, "Error with id_zone \n");
			return -1; 
		}
		
		printf("* EL ESPIA COMENZARÁ A IMPRIMIR LA MEMORIA CADA %d SEGUNDOS *\n",WAIT_TIME_SEC);
		int * buffer;
		int i;
		int cont = 0;
		int n = atoi(argv[1]);
		
		while(cont < n){
			printf("### NUEVO CHEQUEO DE MEMORIA ###\n");
			printf("### CHEQUEOS RESTANTES: %d ###\n", n - cont);
			buffer = shmat (id_zone, (char *)0, 0);
	   		if (buffer == NULL) { 
	      		fprintf (stderr, "Error reserve shared memory \n");
	      		return -1; 
	   		}
	   		printf("|");
	   		for(i = 0; i < memory; i++){
	   			printf("%d|", buffer[i]);
	   		}
	   		printf("\n \n");
	   		printf("### DURMIENDO %d SEGUNDOS ###\n", WAIT_TIME_SEC);
	   		sleep(WAIT_TIME_SEC);
	   		cont++;
		}
	}

	return 0;
}