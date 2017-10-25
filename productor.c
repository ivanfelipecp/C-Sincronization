/*
Sistemas Operativos
ii Semestre 2017

Proyecto # 2
	+ Iván Calvo
	+ Luis Quirós

gcc -pthread -o Productor Productor.c && ./Productor <tipo>

Donde <tipo> es 1 para paginacion o 2 para segmentacion
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
#define PAGINATION 0
#define FRAGMENTATION 1
#define SIZE 255
#define ASIG "asigno"
#define DESAIG "desasigno"
#define PAGS "paginas"
#define SEGS "segmentos"
#define FAIL "no encontro memoria para las"

/*struct parameter{
	int memory;
	int algorithm;
}*/

struct line{
	char text[SIZE];
	struct line * next;
};

void insertLine(struct line ** head, char text[SIZE]){
	//strcpy(text, t);
	struct line * temp = (*head);
	if(temp == NULL){
		temp = malloc(sizeof(struct line));
		//temp->text = text;
		strcpy(temp->text, text);
		temp->next = NULL;
		(*head) = temp;
	}
	else{
		while(temp->next != NULL)
			temp = temp->next;
		temp->next = malloc(sizeof(struct line));
		strcpy(temp->next->text, text);
		temp->next->next = NULL;
	}
}

void removeLine(struct line ** head, char text[SIZE]){
	struct line * temp = (*head);
	if(strcmp(temp->text,text) == 0){
		(*head) = (*head)->next;
	}
	else{
		while(temp != NULL){
			if(strcmp(temp->next->text, text) == 0)
				break;
			temp = temp->next;
		}
		if(temp != NULL){
			struct line * tmp = temp->next;
			temp->next = tmp->next;
			tmp->next = NULL;
			free(tmp);
		}
	}
}

void printList(struct line ** head){
	struct line * temp = (*head);
	while(temp != NULL){
		printf("%s\n", temp->text);
		temp = temp->next;
	}
}

void freeList(struct line ** head){
	struct line * temp;

	while((*head) != NULL){
		temp = (*head);
		(*head) = (*head)->next;
		free(temp);
	}
}

int lenList(struct line ** head){
	struct line * temp = (*head);
	int cont = 0;
	while(temp != NULL){
		cont++;
		temp = temp->next;
	}
	return cont;
}

char* strip(char * buff){
	char buffer[SIZE];
	strcpy(buffer,buff);
	int i;
	for(i = 0; i < SIZE; i++){
		if(buffer[i] == '\n'){
			buffer[i] = '\0';
			break;
		}
	}
	strcpy(buff,buffer);
}

struct line * getLines(char * filename){
	int n = SIZE;
	struct line * head = NULL;
	char buffer[n];
	size_t nread;

	FILE* file = fopen(filename, "r");
	while(fgets(buffer, n, (FILE*) file)) {
		strcpy(buffer,strip(buffer));
    	insertLine(&head, buffer);
	}
    //fflush(file);
    fclose(file);    
    return head;
}

void writeFile(struct line ** head, char * filename){
	struct line * temp = (*head);
	FILE* file = fopen(filename, "w");
	//char * back = "\n";
	while(temp != NULL){
		//if(!(strcmp(temp->text,back)))
		fprintf(file, "%s\n", temp->text);
		temp = temp->next;
	}
	fclose(file);
}

void addFile(int id, char * filename){
	char name[SIZE];
	sprintf(name,"%d",id);

	struct line * head = getLines(filename);
	insertLine(&head, name);
	writeFile(&head, filename);
	freeList(&head);
}

void removeFile(int id, char * filename){
	char name[SIZE];
	sprintf(name,"%d",id); 

	struct line * head = getLines(filename);
	removeLine(&head, name);
	writeFile(&head, filename);
	freeList(&head);
}

char* timestamp() {
  time_t ltime; /* tiempo calendario */
  ltime = time(NULL); /* obtener tiempo actual */
  return asctime(localtime(&ltime));
}

void writeBinnacle(int id, char * action, int memory, char * kind){
	FILE *file = fopen(FILE_BIT,"a");
	fprintf(file,"El thread %d %s %d %s a las %s\n",id, action, memory, kind, timestamp());
	fclose(file);
}


int getMemorySize(char * filename){
	FILE *f = fopen(filename, "r");
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

void * threadPagination(void * memory_size){
	//(void * memory_size)
	//struct parameter * params = (struct parameter *)parameters;

	int memory = *((int *)memory_size);
	//int memory = memory_size;
	int key = ftok(FILEKEY, KEY);
	int pages = getRandom(MIN_SIZE_PAG, MAX_SIZE_PAG);
	int pause = getRandom(MIN_TIME_SEC, MAX_TIME_SEC);
	int id = (int)pthread_self();
	int id_zone;
	int * buffer;
	int i;
	int cont;

	sem_t * sem_mem;
	sem_t * sem_bit;
	sem_t * sem_waits;
	sem_t * sem_search;
	sem_t * sem_death;
	sem_t * sem_finish;
	sem_t * sem_m;
	
	sem_mem = sem_open(SEM_MEM,0);
	sem_m = sem_open(SEM_TXT_MEM,0);
	sem_bit = sem_open(SEM_TXT_BIT,0);
	sem_waits = sem_open(SEM_TXT_WAIT,0);
	sem_search = sem_open(SEM_TXT_SEARCH,0);
	sem_death = sem_open(SEM_TXT_DEATH,0);
	sem_finish = sem_open(SEM_TXT_FINISH,0);
	
	printf("\n ----- THREAD #%d VA A COMENZAR EL PROCESO-----\n", id);
	printf("\n ----- THREAD #%d SOLICITO %d PAGINAS-----\n", id,pages);
	if (key == -1) { 
		printf ("Error con la llave key \n");
		exit(1); 
   	}

   	// Obtenemos el id de la zona
   	id_zone = shmget (key, sizeof(int)*memory, 0777 | IPC_CREAT);
   	if (id_zone == -1) {
    	printf ("Error with id_zone \n");
    	exit(1); 
  	}

   	/* we declared to zone to share */
   	buffer = shmat (id_zone, (char *)0, 0);
   	if (buffer == NULL) { 
    	printf("Error obteniendo el buffer \n");
    	exit(-1); 
   	}
   	// Comienzo del algoritmo
   	// Escribimos en el archivo de espera
   	sem_post(sem_waits);
   	addFile(id,FILE_WAIT);

   	// BUSCAMOS MEMORIA
   	printf("\n ----- THREAD #%d VA A BUSCAR MEMORIA-----\n", id);
   	cont = 0;
   	sem_post(sem_mem);
   	for(i = 0; i < memory; i++){
   		if(buffer[i] == -1)
   			cont++;
   		if(cont == pages)
   			break;
   	}
   	sem_wait(sem_mem);

   	// Escribimos en el archivo de espera
   	removeFile(id,FILE_WAIT);
   	sem_wait(sem_waits);

   	if(cont == pages){
   		printf("\n ----- THREAD #%d VA POSICIONARSE EN MEMORIA-----\n", id);
   		sem_post(sem_search);
   		addFile(id,FILE_SEARCH);

   		cont = 0;
   		sem_post(sem_mem);
   		for(i = 0; i < memory; i++){
	   		if(buffer[i] == -1){
	   			buffer[i] = id;
	   			cont++;
	   		}
	   		if(cont == pages)
	   			break;
   		}
   		sem_wait(sem_mem);

   		removeFile(id,FILE_SEARCH);
   		sem_wait(sem_search);

   		// Guardamos en la bitacora
   		sem_post(sem_bit);
   		writeBinnacle(id, ASIG, pages, PAGS);
   		sem_wait(sem_bit);

   		// Nos quedamos en memoria
   		sem_post(sem_m);
   		addFile(id,FILE_MEM);

   		printf("\n ----- THREAD #%d EN PAUSA DE %d SEGS-----\n", id,pause);
   		sleep(pause);

   		printf("\n ----- THREAD #%d VA A QUITARSE DE MEMORIA-----\n", id);
   		cont = 0;
   		sem_post(sem_mem);
   		for(i = 0; i < memory; i++){
	   		if(buffer[i] == id){
	   			buffer[i] = -1;
	   		}
	   		if(cont == pages)
	   			break;
   		}
   		sem_wait(sem_mem);

   		removeFile(id,FILE_MEM);
   		sem_wait(sem_m);
   		
   		// Escribimos terminado
   		sem_post(sem_finish);
   		addFile(id,FILE_FINISH);
   		sem_wait(sem_finish);
   		printf("\n ----- THREAD #%d TERMINO EXITOSAMENTE-----\n", id);
   	}
   	else{
   		printf("\n ----- THREAD #%d NO ENCONTRO ESPACIO EN MEMORIA-----\n", id);
   		sem_post(sem_bit);
   		writeBinnacle(id, FAIL, pages, PAGS);
   		sem_wait(sem_bit);

   		sem_post(sem_death);
   		addFile(id,FILE_DEATH);
   		sem_wait(sem_death);
   	}
}

void * threadSegmentation(void * memory_size){
	//(void * memory_size)
	//struct parameter * params = (struct parameter *)parameters;

	int memory = *((int *)memory_size);
	//printf("MEMORIA %d\n", memory);
	//int memory = memory_size;
	int key = ftok(FILEKEY, KEY);
	int segments = getRandom(MIN_SIZE_SEG, MAX_SIZE_SEG);
	int lines = getRandom(MIN_SPACE_SEG, MAX_SPACE_SEG);
	int pause = getRandom(MIN_TIME_SEC, MAX_TIME_SEC);
	int id = (int)pthread_self();
	int id_zone;
	int * buffer;
	int i, j, temp;
	int cont;

	sem_t * sem_mem;
	sem_t * sem_bit;
	sem_t * sem_waits;
	sem_t * sem_search;
	sem_t * sem_death;
	sem_t * sem_finish;
	sem_t * sem_m;
	
	sem_mem = sem_open(SEM_MEM,0);
	sem_m = sem_open(SEM_TXT_MEM,0);
	sem_bit = sem_open(SEM_TXT_BIT,0);
	sem_waits = sem_open(SEM_TXT_WAIT,0);
	sem_search = sem_open(SEM_TXT_SEARCH,0);
	sem_death = sem_open(SEM_TXT_DEATH,0);
	sem_finish = sem_open(SEM_TXT_FINISH,0);
	
	printf("\n ----- THREAD #%d VA A COMENZAR EL PROCESO-----\n", id);
	printf("\n ----- THREAD #%d SOLICITO %d SEGMENTOS CON %d LINEAS -----\n", id,segments,lines);
	if (key == -1) { 
		printf ("Error con la llave key \n");
		exit(1); 
   	}

   	// Obtenemos el id de la zona
   	id_zone = shmget (key, sizeof(int)*memory, 0777 | IPC_CREAT);
   	if (id_zone == -1) {
    	printf ("Error with id_zone \n");
    	exit(1); 
  	}

   	/* we declared to zone to share */
   	buffer = shmat (id_zone, (char *)0, 0);
   	if (buffer == NULL) { 
    	printf("Error obteniendo el buffer \n");
    	exit(-1); 
   	}
   	// Comienzo del algoritmo
   	// Escribimos en el archivo de espera
   	sem_post(sem_waits);
   	addFile(id,FILE_WAIT);

   	// BUSCAMOS MEMORIA
   	printf("\n ----- THREAD #%d VA A BUSCAR MEMORIA-----\n", id);
   	cont = 0;
   	sem_post(sem_mem);
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
   	sem_wait(sem_mem);

   	// Escribimos en el archivo de espera
   	removeFile(id,FILE_WAIT);
   	sem_wait(sem_waits);

   	if(cont == segments){
   		printf("\n ----- THREAD #%d VA POSICIONARSE EN MEMORIA-----\n", id);
   		printf("\n ----- THREAD #%d SOLICITO %d SEGMENTOS CON %d LINEAS-----\n", id,segments,lines);
   		sem_post(sem_search);
   		addFile(id,FILE_SEARCH);

   		cont = 0;
   		sem_post(sem_mem);
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
   		sem_wait(sem_mem);

   		removeFile(id,FILE_SEARCH);
   		sem_wait(sem_search);

   		// Guardamos en la bitacora
   		sem_post(sem_bit);
   		writeBinnacle(id, ASIG, segments, PAGS);
   		sem_wait(sem_bit);

   		// Nos quedamos en memoria
   		sem_post(sem_m);
   		addFile(id,FILE_MEM);

   		printf("\n ----- THREAD #%d EN PAUSA DE %d SEGS-----\n", id,pause);
   		sleep(pause);

   		printf("\n ----- THREAD #%d VA A QUITARSE DE MEMORIA-----\n", id);
   		cont = 0;
   		sem_post(sem_mem);
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
   		sem_wait(sem_mem);

   		removeFile(id,FILE_MEM);
   		sem_wait(sem_m);
   		
   		// Escribimos terminado
   		sem_post(sem_finish);
   		addFile(id,FILE_FINISH);
   		sem_wait(sem_finish);
   		printf("\n ----- THREAD #%d TERMINO EXITOSAMENTE-----\n", id);
   	}
   	else{
   		printf("\n ----- THREAD #%d NO ENCONTRO ESPACIO EN MEMORIA-----\n", id);
   		sem_post(sem_bit);
   		writeBinnacle(id, FAIL, segments, PAGS);
   		sem_wait(sem_bit);

   		sem_post(sem_death);
   		addFile(id,FILE_DEATH);
   		sem_wait(sem_death);
   	}
}

void runSegmentation(){
	int random;
	int memory = getMemorySize(FILE_SIZE);
	int * mem;
	mem = malloc(sizeof(int));
	*mem = memory;
	

	printf("* CORRIENDO SEGMENTACION *\n");
	while(1){
		pthread_t thread;

		if(pthread_create(&thread , NULL ,  threadSegmentation , (void*) mem) < 0){
			printf("Error creando el hilo\n");
			exit(1);
		}
		random = getRandom(MIN_WAIT_TIME, MAX_WAIT_TIME);
		printf("* ESPERANDO %d SEGUNDOS PARA EL NUEVO THREAD *\n", random);
		sleep(random);
	}	
}

void runPagination(){
	int random;
	int memory = getMemorySize(FILE_SIZE);
	int * mem;

	pthread_t thread;

	printf("* CORRIENDO PAGINACION *\n");
	while(1){
		mem = malloc(sizeof(int));
		*mem = memory;

		if(pthread_create(&thread , NULL ,  threadPagination , (void*) mem) < 0){
			printf("Error creando el hilo\n");
			exit(1);
		}
		random = getRandom(MIN_WAIT_TIME, MAX_WAIT_TIME);
		printf("* ESPERANDO %d SEGUNDOS PARA EL NUEVO THREAD *\n", random);
		sleep(random);
	}	
}

void run(int mode){
	if(mode == 1){
		runPagination();
	}
	else{
		runSegmentation();
	}
}

int main(int argc, char const *argv[])
{
	if(argc == 2){
		int a = atoi(argv[1]); 
		if((a==1) || (a==2))
			run(a);
		else
			printf("El argumento debe ser 1 o 2\n");
	}
	else{
		printf("Digite el tipo de manejo de memoria a usar\n");
	}
	return 0;
	
}