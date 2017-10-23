/*
Sistemas Operativos
ii Semestre 2017

Proyecto # 2
	+ Iván Calvo
	+ Luis Quirós

gcc -pthread -o productor productor.c && ./productor <tipo>

Donde <tipo> es 1 para paginacion o 2 para segmentacion
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
#define MIN_TIME_SEC 20
#define MAX_TIME_SEC 60
#define MIN_SIZE_PAG 1
#define MAX_SIZE_PAG 10
#define MIN_SIZE_SEG 1
#define MAX_SIZE_SEG 5
#define MIN_SPACE_SEG 1
#define MAX_SPACE_SEG 3
#define MIN_WAIT_TIME 30
#define MAX_WAIT_TIME 60
#define MSG_ASIG "ASIGNANDO"
#define MSG_DEASIG "DESASIGNANDO"
#define MSG_OK "SE ENCUENTRA"
#define MSG_FAIL "NO PUDO ENCONTRAR MEMORIA PARA LAS"

struct parameter{
	int memory;
	int id;
};

int getMemorySize();
int getRandom(int minVal, int maxVal);
void run(int mode);
void runPagination();

void writePagination(char * estado, char * accion, int id, int pages){
	FILE *f = fopen(FILE_BIT, "a");
	fprintf(f, "* EL THREAD %d %s %s %d PAGINAS\n",id, estado, accion, pages);
	fflush(f);
}

void writeFragmentation(char * estado, char * accion, int id, int segments, int lines){
	FILE *f = fopen(FILE_BIT, "a");
	fprintf(f, "* EL THREAD %d %s %s %d SEGMENTOS CON %d LINEAS\n",id, estado, accion, segments, lines);
	fflush(f);
}

int getMemorySize(){
	FILE *f = fopen(FILE_MEM, "r");
	char * line = NULL;
	size_t len = 0;

	getline(&line, &len, f);
	return atoi(line);
}

int getRandom(int minVal, int maxVal){
	srand(time(NULL));
	int res = (rand() % (maxVal - minVal + 1)) + minVal;	
	return res;
}

void *threadPagination(void * parameters){
	
	// Variables para el programa
	struct parameter * params = (struct parameter *)parameters;

	int pages = getRandom(MIN_SIZE_PAG, MAX_SIZE_PAG);
	int pause = getRandom(MIN_WAIT_TIME, MAX_WAIT_TIME);
	int key = ftok(FILEKEY, KEY);
	int memory = params->memory;
	int id = params->id;
	FILE *f = fopen(FILE_MEM, "a");
	
	sem_t *sem_b;
	sem_t *sem_m;
	sem_b = sem_open(SEM_BIT, 0);
	sem_m = sem_open(SEM_MEM, 0);
	
	printf("\n ----- THREAD # %d EJECUTANDO PAGINACION CON %d PAGINAS-----\n", id,pages);
	if (key == -1) { 
		printf ("Error con la llave key \n");
		exit(-1); 
   	}

   	// Obtenemos el id de la zona
   	int id_zone = shmget (key, sizeof(int)*memory, 0777 | IPC_CREAT);
   	if (id_zone == -1) {
    	printf ("Error with id_zone \n");
    	exit(-1); 
  	}

  	int *buffer; /* shared buffer */
   	/* we declared to zone to share */
   	buffer = shmat (id_zone, (char *)0, 0);
   	if (buffer == NULL) { 
    	printf("Error obteniendo el buffer \n");
    	exit(-1); 
   	}

   	// buffer
   	int i;
   	int cont = 0;

   	// chequeamos si hay espacio en la memoria
   	sem_post(sem_m);
   	for(i = 0; i < memory; i++){
   		if(buffer[i] == -1)
   			cont++;
   		if(cont == pages)
   			break;
   	}
   	sem_wait(sem_m);

   	// Puede entrar a memoria
   	if(cont == pages){
   		// Escribimos en el archivo
   		printf("\t* ESCRIBIENDO ESTADO EN EL ARCHIVO * \n");
   		sem_post(sem_b);
   		writePagination(MSG_OK,MSG_ASIG,id,pages);
   		sem_wait(sem_b);

   		// Escribimos en la memoria el id del thread
   		printf("\t* ASIGNANDO MEMORIA * \n");
   		

   		cont = 0;
   		sem_post(sem_m);
   		for(i = 0; i < memory; i++){
	   		if(buffer[i] == -1){
	   			buffer[i] = id;
	   			cont++;
	   		}
	   		if(cont == pages)
	   			break;
   		}
   		sem_wait(sem_m);

   		// Dormimos el thread
   		printf("\t* ESPERANDO %d SEGUNDOS PARA CONTINUAR *\n", pause);
   		printf("\n");
   		sleep(pause);

   		// Escribimos en el archivo
   		printf("\t* ESCRIBIENDO ESTADO EN EL ARCHIVO *\n");
   		sem_post(sem_b);
   		writePagination(MSG_OK,MSG_DEASIG,id,pages);
   		sem_wait(sem_b);

   		// Quitamos el id del thread de la memoria
   		printf("* DESASIGNANDO MEMORIA *");
   		cont = 0;
   		sem_post(sem_m);
   		for(i = 0; i < memory; i++){
	   		if(buffer[i] == id){
	   			buffer[i] = -1;
	   		}
	   		if(cont == pages)
	   			break;
   		}
   		sem_wait(sem_m);
   		printf("\n ----- THREAD # %d PAGINACION TERMINADA-----\n", id);
   	}
   	// No hubo campo
   	else{
   		printf("----- NO HAY ESPACIO EN MEMORIA -----\n");
   		printf("* ESCRIBIENDO ESTADO EN EL ARCHIVO *");
   		sem_post(sem_b);
   		writePagination(MSG_FAIL,"",id,pages);
   		sem_wait(sem_b);
   	}
}

void runPagination(){
	int id = 0;
	int random;
	int memory = getMemorySize();
	//int * t;

	pthread_t thread;
	struct parameter param = {memory, id};
	struct parameter * pointer = &param;

	printf("* CORRIENDO PAGINACION *\n");
	while(1){
		//pages = getRandom(MIN_SIZE_PAG, MAX_SIZE_PAG);
		//t = malloc(sizeof(int));
		//pointer->id = id;

		if(pthread_create(&thread , NULL ,  threadPagination , (void*) pointer) < 0){
			printf("Error creando el hilo\n");
			exit(1);
		}
		random = getRandom(MIN_WAIT_TIME, MAX_WAIT_TIME);
		printf("* ESPERANDO %d SEGUNDOS PARA EL NUEVO THREAD *\n", random);
		sleep(random);
		pointer->id++;
	}	
}

void *threadFragmentation(void * parameters){
	
	// Variables para el programa
	struct parameter * params = (struct parameter *)parameters;

	int segments = getRandom(MIN_SIZE_PAG, MAX_SIZE_PAG);
	int lines = getRandom(MIN_SPACE_SEG, MAX_SPACE_SEG);
	int pause = getRandom(MIN_WAIT_TIME, MAX_WAIT_TIME);
	int key = ftok(FILEKEY, KEY);
	int memory = params->memory;
	int id = params->id;
	FILE *f = fopen(FILE_MEM, "a");
	
	sem_t *sem_b;
	sem_t *sem_m;
	sem_b = sem_open(SEM_BIT, 0);
	sem_m = sem_open(SEM_MEM, 0);
	
	printf("\n ----- THREAD # %d EJECUTANDO FRAMENTACION CON %d SEGMENTOS Y %d LINEAS-----\n", id,segments,lines);
	if (key == -1) { 
		printf ("Error con la llave key \n");
		exit(-1); 
   	}

   	// Obtenemos el id de la zona
   	int id_zone = shmget (key, sizeof(int)*memory, 0777 | IPC_CREAT);
   	if (id_zone == -1) {
    	printf ("Error with id_zone \n");
    	exit(-1); 
  	}

  	int *buffer; /* shared buffer */
   	/* we declared to zone to share */
   	buffer = shmat (id_zone, (char *)0, 0);
   	if (buffer == NULL) { 
    	printf("Error obteniendo el buffer \n");
    	exit(-1); 
   	}

   	// buffer
   	int i, j;
   	int cont = 0;
   	int temp;
   	// chequeamos si hay espacio en la memoria
   	// segments
   	// lines
   	printf("* CHEQUEANDO SI HAY SEGMENTOS DISPONIBLES *\n");
   	sem_post(sem_m);
   	for(i = 0; i < memory; i++){
   		if(buffer[i] == -1){
   			temp = 0;
   			for(j = i; (temp < lines) && (j < memory); j++){
   				if(buffer[j] == -1)
   					temp++;
   				else
   					break;
   			}
   			if(temp == lines){
   				cont++;
   			}
   			i = j - 1;
   		}
   			
   		if(cont == segments)
   			break;
   	}
   	sem_wait(sem_m);
   	printf("CONT -> %d\n", cont);
   	// Puede entrar a memoria
   	if(cont == segments){
   		// Escribimos en el archivo
   		printf("\t* ESCRIBIENDO ESTADO EN EL ARCHIVO * \n");
   		sem_post(sem_b);
   		writeFragmentation(MSG_OK,MSG_ASIG,id,segments,lines);
   		sem_wait(sem_b);

   		// Escribimos en la memoria el id del thread
   		printf("\t* ASIGNANDO MEMORIA * \n");
   		cont = 0;
   		sem_post(sem_m);
   		for(i = 0; i < memory; i++){
	   		if(buffer[i] == -1){
	   			temp = 0;
	   			for(j = i; (temp < lines) && (j < memory); j++){
	   				if(buffer[j] == -1)
	   					temp++;
	   				else
	   					break;
	   			}
	   			if(temp == lines){
	   				for(j = i; (temp > 0) && (j < memory); j++){
	   					buffer[j] = id;
	   					temp--;
	   				}
	   				i = j - 1;
	   				cont++;
	   			}
	   		}
	   		if(cont == segments)
   				break;
   		}
   		sem_wait(sem_m);

   		// Dormimos el thread
   		printf("\t* ESPERANDO %d SEGUNDOS PARA CONTINUAR *\n", pause);
   		printf("\n");
   		sleep(pause);

   		// Escribimos en el archivo
   		printf("\t* ESCRIBIENDO ESTADO EN EL ARCHIVO *\n");
   		sem_post(sem_b);
   		writeFragmentation(MSG_OK,MSG_DEASIG,id,segments,lines);
   		sem_wait(sem_b);

   		// Quitamos el id del thread de la memoria
   		printf("* DESASIGNANDO MEMORIA *");
   		cont = 0;
   		sem_post(sem_m);
   		for(i = 0; i < memory; i++){
	   		if(buffer[i] == id){
	   			for(j = i; j < memory; j++){
	   				if(buffer[j] == id){
	   					buffer[j] = -1;
	   				}
	   				else
	   					break;
	   			}
	   			cont++;
	   			i = j - 1;
	   		}
	   		if(cont == segments)
	   			break;
   		}
   		sem_wait(sem_m);
   		printf("\n ----- THREAD # %d FRAGMENTACION TERMINADA-----\n", id);
   	}
   	// No hubo campo
   	else{
   		printf("----- NO HAY ESPACIO EN MEMORIA -----\n");
   		printf("* ESCRIBIENDO ESTADO EN EL ARCHIVO *");
   		sem_post(sem_b);
   		writeFragmentation(MSG_FAIL,"",id,segments,lines);
   		sem_wait(sem_b);
   	}
}

void runFragmentation(){
	int id = 0;
	int random;
	int memory = getMemorySize();
	//int * t;

	pthread_t thread;
	struct parameter param = {memory, id};
	struct parameter * pointer = &param;

	printf("* CORRIENDO FRAGMENTACION *\n");
	while(1){
		//pages = getRandom(MIN_SIZE_PAG, MAX_SIZE_PAG);
		//t = malloc(sizeof(int));
		//pointer->id = id;

		if(pthread_create(&thread , NULL ,  threadFragmentation , (void*) pointer) < 0){
			printf("Error creando el hilo\n");
			exit(1);
		}
		srand(time(NULL));
		random = getRandom(MIN_WAIT_TIME, MAX_WAIT_TIME);
		printf("* ESPERANDO %d SEGUNDOS PARA EL NUEVO THREAD *\n", random);
		sleep(random);
		pointer->id++;
	}	
}

void run(int mode){
	/*int id = 0;
	int random;
	int handler = atoi(argv[1]);

	pthread_t thread;

	while(1){

	}*/

	if(mode == 1){
		runPagination();
	}
	else{
		runFragmentation();
	}
}




int main(int argc, char const *argv[])
{
	if(argc == 2){
		run(atoi(argv[1]));
	}
	else{
		printf("Digite el tipo de manejo de memoria a usar\n");
	}
	return 0;
	
}