/*
Sistemas Operativos
ii Semestre 2017

Proyecto # 2
	+ Iván Calvo
	+ Luis Quirós

gcc -pthread -o Espia Espia.c && ./Espia

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
#define SIZE 255
#define MSG_WAIT "LOS PROCESOS QUE SE ENCUENTRAN ESPERANDO MEMORIA SON LOS SIGUIENTES"
#define MSG_MEM "LOS PROCESOS QUE SE ENCUENTRAN EN MEMORIA SON LOS SIGUIENTES"
#define MSG_SEARCH "LOS PROCESOS QUE SE ENCUENTRAN BUSCANDO ESPACIO EN MEMORIA SON LOS SIGUIENTES"
#define MSG_DEATH "LOS PROCESOS QUE MURIERON POR NO ENCONTRAR MEMORIA SON LOS SIGUIENTES"
#define MSG_FINISH "LOS PROCESOS QUE YA TERMINARON SON LOS SIGUIENTES"

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
		printf("+ %s\n", temp->text);
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

int getMemorySize(char * filename){
	FILE *f = fopen(filename, "r");
	char * line = NULL;
	size_t len = 0;

	getline(&line, &len, f);
	return atoi(line);
}

void seeMemoryState(char * fileKey, int k, int memory){
	int key = ftok(fileKey, k);
	if (key == -1) { 
		printf("Error with key \n");
		exit(1); 
	}
	int id_zone = shmget (key, sizeof(int)*memory, 0777 | IPC_CREAT);
	if (id_zone == -1) {
		printf("Error with id_zone \n");
		exit(1); 
	}
	printf("* OBSERVANDO EL ESTADO DE LA MEMORIA DESDE EL ESPIA *\n");
	int * buffer;
	int i;

	buffer = shmat (id_zone, (char *)0, 0);
	if (buffer == NULL) { 
		printf("Error reserve shared memory \n");
		exit(1); 
	}

	printf("|");
	for(i = 0; i < memory; i++){
		printf("%d|", buffer[i]);
	}
	printf("\n \n");
}

void see(char * msg, char * filename){
	struct line * head = head = getLines(filename);
	
	if(lenList(&head) == 0)
		printf("NO SE ENCUENTRAN PROCESOS DE LO QUE SOLICITO\n");
	else{
		printf("%s :\n", msg);
		printList(&head);
	}
	printf("\n");
}

void menu(){
	int memory = getMemorySize(FILE_SIZE);
	int choice;
	do{
		printf("#####################\n");
		printf("Bienvenido al menu del espia, digite una opcion\n");
		printf("1 - Ver estado de la memoria\n");
		printf("2 - PID de los procesos que están en memoria en este momento\n");
		printf("3 - PID del único proceso que esté buscando espacio en la memoria\n");
		printf("4 - PID de los procesos que estén esperando por la región critica\n");
		printf("5 - El PID de los procesos que han muerto por no haber espacio suficiente\n");
		printf("6 - El PID de los procesos que ya terminaron su ejecución\n");
		printf("9 - Salir\n");
		scanf("%d",&choice);
		printf("\n");

		switch(choice){
			case 1:
				seeMemoryState(FILEKEY, KEY, memory);
				break;
			case 2:
				see(MSG_MEM,FILE_MEM);
				break;
			case 3:
				see(MSG_SEARCH,FILE_SEARCH);
				break;
			case 4:
				see(MSG_WAIT, FILE_WAIT);
				break;
			case 5:
				see(MSG_DEATH, FILE_DEATH);
				break;
			case 6:
				see(MSG_FINISH, FILE_FINISH);

		}
	} while(choice != 9);
	printf("ADIOS ESPIA!\n");
}

int main(int argc, char const *argv[])
{
	menu();
	return 0;
}