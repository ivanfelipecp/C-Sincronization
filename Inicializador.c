/*
Sistemas Operativos
ii Semestre 2017

Proyecto # 2
	+ Iván Calvo
	+ Luis Quirós

gcc -pthread -o Inicializador Inicializador.c && ./Inicializador <n>

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
#define SEM_TXT_BIT "Semaforo_TXT_Bitacora"
#define SEM_TXT_MEM "Semaforo_TXT_Memoria"
#define SEM_TXT_WAIT "Semaforo_TXT_Wait"
#define SEM_TXT_SEARCH "Semaforo_TXT_Buscando"
#define SEM_TXT_DEATH "Semaforo_TXT_Muertos"
#define SEM_TXT_FINISH "Semaforo_TXT_Wait"
#define FILE_BIT "bitacora.txt"
#define FILE_MEM "memoria.txt"
#define FILE_WAIT "wait.txt"
#define FILE_SEARCH "search.txt"
#define FILE_DEATH "death.txt"
#define FILE_FINISH "finish.txt"
#define FILE_SIZE "size.txt"

void createSemaphore(char * name){
	sem_open(name, O_CREAT | O_EXCL, 0644, 1);
}

void clearFile(char * name){
	FILE *f = fopen(name,"w");
	fclose(f);
}

// filekey, key , memoria a pedir
void createMemory(char * fileKey, int k, int memory){
	int key = ftok(fileKey, k);
	if(key == -1){
		printf("Error con la llave\n");
		exit(1);
	}

	printf("* CREANDO LA MEMORIA COMPARTIDA *\n");
	// Creamos la memoria compartida
	int id_zone = shmget (key, sizeof(int)*memory, 0777 | IPC_CREAT);
	if (id_zone == -1) {
		printf("Error creando la memoria compartida \n");
		exit(1);
	}
	printf("* MEMORIA CREADA CON EXITO *\n");
	printf ("* ID DE LA MEMORIA COMPARTIDA: %i *\n", id_zone);

	int *buffer; /* shared buffer */

	/* we declared to zone to share */
	buffer = shmat (id_zone, (char *)0, 0);
	if (buffer == NULL) {
		printf("Error reservando la memoria compartida\n");
		exit(1);
	}

	printf ("* PUNTERO AL BUFFER DE LA MEMORIA COMPARTIDA: %p\n *", buffer); 
	int i;
	for (i = 0; i < memory; i++) 
		buffer[i] = -1;

	FILE *f = fopen(FILE_SIZE,"w");
	fprintf(f,"%d\n",memory);
	fclose(f);
}

int main(int argc, char const *argv[])
{
	if(argc == 2){
		clearFile(FILE_SIZE);
		clearFile(FILE_FINISH);
		clearFile(FILE_DEATH);
		clearFile(FILE_MEM);
		clearFile(FILE_WAIT);
		clearFile(FILE_SEARCH);
		clearFile(FILE_BIT);
		createMemory(FILEKEY, KEY,atoi(argv[1]));

		createSemaphore(SEM_MEM);
		createSemaphore(SEM_TXT_MEM);
		createSemaphore(SEM_TXT_BIT);
		createSemaphore(SEM_TXT_WAIT);
		createSemaphore(SEM_TXT_SEARCH);
		createSemaphore(SEM_TXT_DEATH);
		createSemaphore(SEM_TXT_FINISH);
	}
	else
		printf("Envie el # de memoria en el argumento\n");
	return 0;
}