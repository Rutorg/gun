#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>


#define REPLACE_NUMBER 100
#define MAX_BUFFER 100

pthread_mutex_t tasksMutex;

struct task_t
{
    char *fileName;
    size_t replacements;
};

struct task_t* glob_tasks;


int processText(char *sourceText, char *resultText, int fileSize, const int MAX_REPLACES) 
{
	int replaceCounter = 0;
    int currentLineStart = 0;
    int maxReplacementsFlag = 0;
    
    for (size_t i = 0; i < fileSize; i++) {
        resultText[i] = sourceText[i];
        if (!maxReplacementsFlag && resultText[i] == '\n') {
            char lastSymbolInLine = resultText[i - 1];
            
            // if lastSymbolInLine is space then there will be no replaces.
            if (lastSymbolInLine != ' ') {
                for (size_t j = currentLineStart; j < i; j++) {
                    if (resultText[j] == lastSymbolInLine) {
                        resultText[j] = ' ';
                        replaceCounter++;
                        if (replaceCounter == MAX_REPLACES) {
                            maxReplacementsFlag = 1;
                            break;
                        }
                    }
                }
            }
            
            currentLineStart = i + 1;
        }
    }
    
    // process last line
    if (!maxReplacementsFlag && resultText[fileSize - 1] != '\n') {
        char lastSymbolInLine = resultText[fileSize - 1];
        
        // if lastSymbolInLine is space then there will be no replaces.
        if (lastSymbolInLine != ' ') {
            for (size_t j = currentLineStart; j < fileSize; j++) {
                if (resultText[j] == lastSymbolInLine) {
                    resultText[j] = ' ';
                    replaceCounter++;
                }
            }
        }
    } 
    
    
    // printf("%d\n", replaceCounter);
    return replaceCounter;
}

// return type: int.
void* processFile(void* inputFileNameArg)
{
    const int MAX_REPLACES = REPLACE_NUMBER;
    char* inputFileName = (char* ) inputFileNameArg;
    
    // Open read file f1.txt
    int file1DSC = open(inputFileName, O_RDONLY);
	if (file1DSC < 0) {
		printf("Secondary thread: Unable to open input file.\n");
		return (void*) -1;
	}
    // Lock whole file1 for exclusive use
    lockf(file1DSC, F_LOCK, 0);
    
    // ========read whole file1========
    
    // find fileSize of file
    int fileSize = lseek(file1DSC, 0, SEEK_END);
    lseek(file1DSC, 0, SEEK_SET);
    
    // read whole file
    char *fileText = calloc(1, fileSize);
    read(file1DSC, fileText, fileSize);
    
    close(file1DSC);
    // unlock file1
    lockf(file1DSC, F_ULOCK, 0);
    
    // ========end read file1========
    
    
    // fileSize + 1 for \0.
    char *resultText = calloc(1, fileSize + 1);
    // printf("Secondary thread: Calling processText...\n");
    long replaceCounter = processText(fileText, resultText, fileSize, MAX_REPLACES);
    // printf("Secondary thread: Finished processText\n");
    
    // ========write whole file2========
    
    // create name for output file
    char outputFileName[MAX_BUFFER] = "result-";
    strcat(outputFileName, inputFileName);
    
    // open to write file f2.txt
    int file2DSC = open(outputFileName, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
    if (file2DSC < 0) {
		printf("Secondary thread: Unable to open output file.\n");
		return (void*) -1;
	}
    // Lock whole file2 for exclusive use
    lockf(file2DSC, F_LOCK, 0);
    
    write(file2DSC, resultText, fileSize);
    
    close(file2DSC);
    lockf(file2DSC, F_ULOCK, 0);
    
    // ========end write whole file2========
    
    free(fileText);
    free(resultText);

    // Критический участок
    printf("Thread with %s trying to lock\n", inputFileNameArg);
    pthread_mutex_lock(&tasksMutex);
    printf("Thread with %s locked mutex\n", inputFileNameArg);
    sleep(1);
    // Ищем задачу с нулевым именем. В это место запишем нашу решенную задачу.
    for (size_t i = 0; ; i++) {
        if (glob_tasks[i].fileName == 0) {
            glob_tasks[i].fileName = calloc(strlen(inputFileNameArg) + 1, 1);
            strcpy(glob_tasks[i].fileName, inputFileNameArg);
            glob_tasks[i].replacements = replaceCounter;
            break;
        }
    }
    pthread_mutex_unlock(&tasksMutex);
    printf("Thread with %s unlocked mutex\n", inputFileNameArg);

	return (void*) 0;
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        printf("Usage: ./main file1 file2 ...\n");
        exit(-1);
    }
    
    const size_t filesNumber= argc - 1;

    pthread_mutex_init(&tasksMutex, NULL);
    glob_tasks = calloc(sizeof(struct task_t), filesNumber);
    
    
    pthread_t threads[filesNumber];
    for (size_t i = 0; i < filesNumber; i++) {
        pthread_create(threads + i, NULL, processFile, argv[i + 1]);
        printf("MAIN: created %d-thread. File: %s\n", i, argv[i + 1]);
    }
    
    
    int* returned;
    for (size_t i = 0; i < filesNumber; i++) {
        pthread_join(threads[i], (void**) &returned);
    }
    
    for (size_t i = 0; i < filesNumber; i++) {
        printf("Task %i:\n\tfileName: %s\n\tReplacements: %d\n", 
        i, glob_tasks[i].fileName, glob_tasks[i].replacements);
    }
    
    
    
    return 0;
}