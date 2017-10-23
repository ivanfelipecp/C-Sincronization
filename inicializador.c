/*
Sistemas Operativos
ii Semestre 2017

Proyecto # 2
	+ Iván Calvo
	+ Luis Quirós

gcc -pthread -o inicializador inicializador.c && ./inicializador <n>


Donde <n> es la cantidad de memoria a reservar

*/

// Includes
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>  
#include <sys/ipc.h>
#include <sys/shm.h> 
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <pthread.h>

// Defines
#define FILEKEY "/bin/cat"
#define KEY 1300
#define SEM_MEM "Semaforo_Memoria"
#define SEM_BIT "Semaforo_Bitacora"
#define FILE_BIT "bitacora.txt"
#define FILE_MEM "memoria.txt"

void closeSemaphore(char * name){
	sem_t * sem = sem_open(name, 0);
	sem_unlink(name);
	sem_close(sem);
	sem_destroy(sem);
}

int createSharedMemory(int memory){
	int key = ftok(FILEKEY, KEY);
	if(key == -1){
		printf("Error con la llave\n");
		return -1;
	}

	printf("* CREANDO LA MEMORIA COMPARTIDA *\n");
	// Creamos la memoria compartida
	int id_zone = shmget (key, sizeof(int)*memory, 0777 | IPC_CREAT);
	if (id_zone == -1) {
		printf("Error creando la memoria compartida \n");
		return -1; 
	}
	printf("* MEMORIA CREADA CON EXITO *\n");
	printf ("* ID DE LA MEMORIA COMPARTIDA: %i *\n", id_zone);

	int *buffer; /* shared buffer */

	/* we declared to zone to share */
	buffer = shmat (id_zone, (char *)0, 0);
	if (buffer == NULL) {
		printf("Error reservando la memoria compartida\n");
		return -1; 
	}

	printf ("PUNTERO AL BUFFER DE LA MEMORIA COMPARTIDA: %p\n", buffer); 
	int i;
	for (i = 0; i < memory; i++) 
		buffer[i] = -1;

	printf("* CREANDO LOS SEMAFOROS DE MEMORIA Y BITACORA *\n");
	sem_open(SEM_MEM, O_CREAT | O_EXCL, 0644, 1);
	sem_open(SEM_BIT, O_CREAT | O_EXCL, 0644, 1);
	printf("* SEMAFOROS CREADOS CON EXITO *\n");

	// Escribimos en la bitácora
	printf("* ESCRIBIENDO EN ARCHIVOS *\n");
	FILE *f = fopen(FILE_BIT, "w");
	fprintf(f, "CREACION DE LA BITACORA\n");
	fprintf(f,"CANTIDAD DE MEMORIA SOLICITADA: %d \n", memory);
	fflush(f);

	f = fopen(FILE_MEM,"w");
	fprintf(f,"%d\n",memory);
	fflush(f);

	printf("* EJECUTANDO... *\n");
	char c;
	c = getchar();

	/* Free the shared memory */
	shmdt ((char *)buffer);
	shmctl (id_zone, IPC_RMID, (struct shmid_ds *)NULL);

	closeSemaphore(SEM_MEM);
	closeSemaphore(SEM_BIT);

	return 0;
}

int main(int argc, char const *argv[])
{
	if(argc == 2){
		return createSharedMemory(atoi(argv[1]));
	}
	else{
		printf("Tiene que digitar la cantidad de memoria a reservar\n");
		return -1;
	}
	/* code */
	
}