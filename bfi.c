#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BFI_MEMSIZE 30000

#define BFI_OK 0
#define BFI_FILE_OPEN 1
#define BFI_FILE_READ 2
#define BFI_MEM 3
#define BFI_OUT_OF_BOUNDS 4
//the brainfuck program has an error in it
#define BFI_PROG_ERROR 5

typedef struct bfFile{
	char *program;
	long memoryIndex;
	long programIndex;
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
int bfLoad(const char *path, bfFile *res);

void bfDestroy(bfFile *f);

/*
parses and executes an operator at program[prodIndex]
*/
int bfInterpretOperator(bfFile *f);

int bfInterpret(bfFile *file);

void bfLoop(bfFile *file);

int main(int argc, char **argv){
	if(argc == 1){
		printf("proper usage: \n\tfor a single file: %s file.bf \n\tfor multiple files: %s file1.bf file2.bf file3.bf ...\n", argv[0], argv[0]);
		return 0;
	}

	for(int i = 1; i < argc; i++){
		bfFile file;
		int ret = bfLoad(argv[i], &file);
		if(ret != BFI_OK){
			switch(ret){
				case BFI_FILE_OPEN:
					fprintf(stderr, "failed to open file: %s\n", argv[i]);
					break;
				case BFI_FILE_READ:
					fprintf(stderr, "failed to read file: %s\n", argv[i]);
					break;
				case BFI_MEM:
					fprintf(stderr, "failed to allocate memory for file: %s\n", argv[i]);
					break;
			}
			continue;
		}

		if(bfInterpret(&file) != BFI_OK){
			fprintf(stderr, "error in file: %s\n", argv[i]);
		}

		bfDestroy(&file);
	}

	return 0;
}

//load a file from a path
int bfLoad(const char *path, bfFile *res){
	res->program = NULL;
	res->memoryIndex = 0;
	res->programIndex = 0;
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

	fclose(f);

	res->program = fData;
	res->programSize = fileSize;
	memset(res->memory, 0, BFI_MEMSIZE);

	return BFI_OK;
}

void bfDestroy(bfFile *f){
	free(f->program);
	f->program = NULL;
}

int bfInterpretOperator(bfFile *f){
	switch(f->program[f->programIndex]){
		case '>':
			if(f->memoryIndex >= BFI_MEMSIZE){
				fprintf(stderr, "program attempted to move memory pointer higher than the upper bounds of %u\n", BFI_MEMSIZE);
				return BFI_OUT_OF_BOUNDS;
			}
			f->memoryIndex++;
			break;
		case '<':
			if(f->memoryIndex <= 0){
				fputs("program attempted to move memory pointer below 0\n", stderr);
				return BFI_OUT_OF_BOUNDS;
			}
			f->memoryIndex--;
			break;
		case '+':
			f->memory[f->memoryIndex]++;
			break;
		case '-':
			f->memory[f->memoryIndex]--;
			break;
		case '.':
			putchar(f->memory[f->memoryIndex]);
			break;
		case ',':
			f->memory[f->memoryIndex] = getchar();
			break;
		case '[':
			bfLoop(f);
			break;
	}

	return BFI_OK;
}

int bfInterpret(bfFile *f){
	for(;f->programIndex < f->programSize; f->programIndex++){
		if(bfInterpretOperator(f) != BFI_OK){
			//print 8 before and 8 after the current programIndex
			int bytesBefore = 8;
			int bytesAfter = 8;
			if(f->programIndex < 7){
				bytesBefore = f->programIndex;
			}
			if((f->programIndex + bytesAfter) > f->programSize){
				bytesAfter = f->programSize-f->programIndex;
			}
			fprintf(stderr, "error at character %d:\n\t%.*s\n", f->programIndex, bytesBefore+bytesAfter, &f->program[f->programIndex-bytesBefore]);
			fprintf(stderr, "\t%*s\n", bytesBefore+1, "^");
			return BFI_PROG_ERROR;
		}
	}
	return BFI_OK;
}

void bfLoop(bfFile *f){
	f->programIndex++;
	long start = f->programIndex;
	for(;f->programIndex < f->programSize; f->programIndex++){
		if(f->program[f->programIndex] == ']'){
			if(f->memory[f->memoryIndex]){
				f->programIndex = start;
			}
			else{
				f->programIndex++;
				break;
			}
		}

		bfInterpretOperator(f);
	}
}
