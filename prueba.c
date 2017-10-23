#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct parameter{
	int memory;
	int id;
};


void prueba1(){
	FILE *f = fopen("file.txt", "w");
	if (f == NULL)
	{
		printf("Error opening file!\n");
		exit(1);
	}

	/* print some text */
	const char *text = "Write this to the file";
	fprintf(f, "Some text: %s\n", text);

	/* print integers and floats */
	int i = 1;
	float py = 3.1415927;
	fprintf(f, "Integer: %d, float: %f\n", i, py);

	/* printing single chatacters */
	char c = 'A';
	fprintf(f, "A character: %c\n", c);

	fclose(f); 

}

void prueba2(){
	FILE *f = fopen("memoria.txt", "r");
	char * line = NULL;
	size_t len = 0;

	getline(&line, &len, f);
	printf("file: %d \n", atoi(line));
}

int main(int argc, char const *argv[])
{
	struct parameter p = {1,2};
	struct parameter * pointer = &p;
	pointer->id = 100;
	printf("%d\n", pointer->id);
	return 1;
}