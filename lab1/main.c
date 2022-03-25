#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
// #include <errno.h>
#include <fcntl.h>

// #define DEBUG

#define MAX_SIZE 16384


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


int main(int argc, char *argv[])
{
#ifdef DEBUG
	argv[1] = "f1.txt";
    argv[2] = "f2.txt";
	int replaceNumber = 1000;
    snprintf(argv[3], MAX_SIZE, "%d", replaceNumber);
#else
	if (argc != 4) {    
		printf("Usage: cpW file1 file2 charCount\n");
		return -1;
	}
#endif // DEBUG

    // Open read file f1.txt
    int file1DSC = open(argv[1], O_RDONLY);
	if (file1DSC < 0) {
		printf("Unable to open input file.\n");
		return -1;
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
    fileText[fileSize];
    
    close(file1DSC);
    // unlock file1
    lockf(file1DSC, F_ULOCK, 0);
    
    // ========end read file1========
    
    char *resultText = calloc(1, fileSize);
    const int MAX_REPLACES = atoi(argv[3]);
    int replaceCounter = processText(fileText, resultText, fileSize, MAX_REPLACES);
    
    
    // printf(resultText);
    // for (size_t i = 0; i < fileSize; i++) {
    //     if (resultText[i] == '\n') {
    //         printf("%d -> \\n\t", resultText[i]);
    // 
    //     } else {
    //         printf("%d -> %c\t", resultText[i], resultText[i]);
    //     }
    // 
    //     if (fileText[i] == '\n') {
    //         printf("%d -> \\n\n", fileText[i], fileText[i]);
    //     } else {
    //         printf("%d -> %c\n", fileText[i], fileText[i]);
    //     }   
    // }
    
    // ========write whole file2========
    
    // open to write file f2.txt
    int file2DSC = open(argv[2], O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
    if (file2DSC < 0) {
		printf("Unable to open output file.\n");
		return -1;
	}
    // Lock whole file2 for exclusive use
    lockf(file2DSC, F_LOCK, 0);
    
    write(file2DSC, resultText, fileSize);
    
    close(file2DSC);
    lockf(file2DSC, F_ULOCK, 0);
    
    // ========end write whole file2========
    
    free(fileText);
    free(resultText);

	return replaceCounter;
}