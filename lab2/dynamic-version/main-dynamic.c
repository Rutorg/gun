#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <dlfcn.h>

// #define DEBUG

#define MAX_SIZE 1024

int main(int argc, char *argv[])
{
    void *ext_library;
    
    int (*libFunction)(char *sourceText, char *resultText, int fileSize, const int MAX_REPLACES);
    
    ext_library = dlopen("liblab1.so", RTLD_LAZY);
    if (!ext_library) {
        fprintf(stderr,"dlopen() error: %s\n", dlerror());
        return -1;
    }
    printf("Library loaded!\n");
    
    libFunction = dlsym(ext_library, "processText");
    
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
    
    close(file1DSC);
    // unlock file1
    lockf(file1DSC, F_ULOCK, 0);
    
    // ========end read file1========
    
    // fileSize + 1 for \0
    char *resultText = calloc(1, fileSize + 1); 
    const int MAX_REPLACES = atoi(argv[3]);
    printf("Calling function...\n");
    int replaceCounter = (*libFunction)(fileText, resultText, fileSize, MAX_REPLACES);
    printf("Function executed...\n");
    
    // Close library
    dlclose(ext_library);
    
    // printf(resultText);
    // for (size_t i = 0; i < fileSize + 4; i++) {
    //     if (resultText[i] == '\n') {
    //         printf("%d -> \\n\t", resultText[i]);
    // 
    //     } else {
    //         printf("%d -> %c\t", resultText[i], resultText[i]);
    //     }
    // 
    //     if (resultText[i] == '\n') {
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
