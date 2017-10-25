/*
Sistemas Operativos
ii Semestre 2017

Proyecto # 2
	+ Iván Calvo
	+ Luis Quirós

gcc -pthread -o Finalizador Finalizador.c && ./Finalizador
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

#define FILEKEY "/bin/cat"
#define KEY 1300
#define SEM_MEM "Semaforo_Memoria"
#define SEM_TXT_BIT "Semaforo_TXT_Bitacora"
#define SEM_TXT_MEM "Semaforo_TXT_Memoria"
#define SEM_TXT_WAIT "Semaforo_TXT_Wait"
#define SEM_TXT_SEARCH "Semaforo_TXT_Buscando"
#define SEM_TXT_DEATH "Semaforo_TXT_Muertos"
#define SEM_TXT_FINISH "Semaforo_TXT_Wait"
#define FILE_SIZE "size.txt"

int getMemorySize(char * filename){
	FILE *f = fopen(filename, "r");
	char * line = NULL;
	size_t len = 0;

	getline(&line, &len, f);
	return atoi(line);
}

void deleteSemaphore(char * name){
	sem_t * sem = sem_open(name, 0);
	sem_unlink(name);
	sem_close(sem);
	sem_destroy(sem);
}

void deleteMemory(char * fileKey, int k){
	int key = ftok(fileKey, k);

	int memory = getMemorySize(FILE_SIZE);

	int id_zone = shmget (key, sizeof(int)*memory, 0777 | IPC_CREAT);

	int *buffer;
	buffer = shmat (id_zone, (char *)0, 0);

	shmdt ((char *)buffer);
	shmctl (id_zone, IPC_RMID, (struct shmid_ds *)NULL);
}



int main(int argc, char const *argv[])
{
	deleteMemory(FILEKEY, KEY);
	deleteSemaphore(SEM_MEM);
	deleteSemaphore(SEM_TXT_FINISH);
	deleteSemaphore(SEM_TXT_DEATH);
	deleteSemaphore(SEM_TXT_SEARCH);
	deleteSemaphore(SEM_TXT_WAIT);
	deleteSemaphore(SEM_TXT_MEM);
	deleteSemaphore(SEM_TXT_BIT);
	printf("* MEMORIA Y SEMAFOROS LIBERADOS *\n");
	return 0;
}