#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BFI_MEMSIZE 30000

#define BFI_OK 0
#define BFI_FILE_OPEN 1
#define BFI_FILE_READ 2
#define BFI_MEM 3

typedef struct bfFile{
	FILE *file;
	char *program;
	long index;
	long progIndex;
	long programSize;
	char memory[BFI_MEMSIZE];
} bfFile;

/*
load a file from a path, store the result in res
possible returns:
BFI_OK
BFI_FILE_OPEN
BFI_FILE_READ
BFI_MEM
*/
int bfFileLoad(const char *path, bfFile *res);

/*
clean up
*/
void bfFileDestroy(bfFile *f);

void bfParseOperator(bfFile *f);

/*
parses and executes an operator
*/
void bfFileParse(bfFile *file);

/*
parses loops, no touchy
*/
void bfParseLoop(bfFile *file);

int main(int argc, char **argv){
	if(argc == 1){
		return 0;
	}

	bfFile file;
	bfFileLoad(argv[1], &file);
	bfFileParse(&file);
	bfFileDestroy(&file);

	return 0;
}

//load a file from a path
int bfFileLoad(const char *path, bfFile *res){
	res->file = NULL;
	res->program = NULL;
	res->index = 0;
	res->progIndex = 0;
	res->programSize = 0;

	FILE *f = fopen(path, "r");
	if(f == NULL){
		return BFI_FILE_OPEN;
	}

	fseek(f, 0, SEEK_END);
	long fileSize = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *fData = malloc(fileSize);
	if(fData == NULL){
		fclose(f);
		return BFI_MEM;
	}

	if(fread(fData, 1, fileSize, f) != fileSize){
		fclose(f);
		free(fData);
		return BFI_FILE_READ;
	}

	res->file = f;
	res->program = fData;
	res->programSize = fileSize;
	memset(res->memory, 0, BFI_MEMSIZE);

	return BFI_OK;
}

void bfFileDestroy(bfFile *f){
	fclose(f->file);
	f->file = NULL;

	free(f->program);
	f->program = NULL;
}

void bfParseOperator(bfFile *f){
	switch(f->program[f->progIndex]){
		case '>':
			f->index++;
			break;
		case '<':
			f->index--;
			break;
		case '+':
			f->memory[f->index]++;
			break;
		case '-':
			f->memory[f->index]--;
			break;
		case '.':
			putchar(f->memory[f->index]);
			break;
		case ',':
			f->memory[f->index] = getchar();
			getchar();
			break;
		case '[':
			bfParseLoop(f);
			break;
	}
}

void bfFileParse(bfFile *f){
	for(;f->progIndex < f->programSize; f->progIndex++){
		bfParseOperator(f);
	}
}

void bfParseLoop(bfFile *f){
	f->progIndex++;
	long start = f->progIndex;
	for(;f->progIndex < f->programSize; f->progIndex++){
		if(f->program[f->progIndex] == ']'){
			if(f->memory[f->index]){
				f->progIndex = start;
			}
			else{
				f->progIndex++;
				break;
			}
		}

		bfParseOperator(f);
	}
}
