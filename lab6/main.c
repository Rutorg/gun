// Два семафора.
// Один показывает возможность записи в разделяему память.
// Другой показывает возможность чтения из разделяемой памяти.
// 
// Программа создает разделяемую память с помощью mmap
// MAP_SHARED, собственно, чтобы память была разделяемой.
// MAP_ANONYMOUS чтобы не использовать реальный файл.
// 
// 


#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>


#define REPLACE_NUMBER 100
#define MAX_BUFFER 100


int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("Usage: textfile1 textfile2 ...\n");
        exit(-1);
    }
    
    printf("PARENT: Processes: %d\n", argc - 1);
    int pid[argc - 1];
    
    sem_t* semWriteReady = sem_open("/myWrite",  O_CREAT, 0660, 1);
    if (semWriteReady == SEM_FAILED) {
        perror("semaphore /myWrite error sem_open");
        exit(-1);
    }
    
    sem_t* semReadReady = sem_open("/myRead",  O_CREAT, 0660, 0);
    if (semReadReady == SEM_FAILED) {
        perror("semaphore /myRead error sem_open");
        exit(-1);
    }
    
    
    
    // Выделяем разделяемую память.
    // MAP_SHARED, собственно, чтобы память была разделяемой.
    // MAP_ANONYMOUS чтобы не использовать файл.
    char* sharedMemory = mmap(NULL, MAX_BUFFER,
                              PROT_READ | PROT_WRITE,
                              MAP_SHARED | MAP_ANONYMOUS,
                              -1, 0);
    if (sharedMemory == (void*) -1) {
        perror("mmap error");
        exit(-1);
    }
    
    
    
    for (int i = 0; i < argc - 1; i++) {
        // запускаем дочерний процесс
        char arg[20];
        strcpy(arg, argv[i + 1]);
        
        char resultFile[64] = "result-";
        strcat(resultFile, arg);
        
        char replaceNumber[64];
        sprintf(replaceNumber, "%d", REPLACE_NUMBER);
        
        fflush(stdout);
        pid[i] = fork();
        if (pid[i] == 0) {
            // если выполняется дочерний процесс
            
            pid_t prog = fork();
            if (prog == 0) {
                // функция счета
                printf("CHILD: Processing of file %s started. arg: %s, resultFile: %s, replaceNumber: %s\n",
                 argv[i + 1], arg, resultFile, replaceNumber);
                execl("./main-static", "main-static", arg, resultFile, replaceNumber, (char*) NULL);
            }
            
            int returned = 0;
            waitpid(prog, &returned, 0);
            returned = WEXITSTATUS(returned);
            printf("CHILD: %d-process  returned: %d\n", i, returned);
            
            
            // записываем в канал
            char buffer[MAX_BUFFER];
            sprintf(buffer, "%d", returned);
            
            sem_wait(semWriteReady);
            printf("CHILD: %d-process writing in shared. TEXT: %s, LENGTH: %d\n", i, buffer, (int)strlen(buffer) + 1);
            strncpy(sharedMemory, buffer, strlen(buffer));
            sem_post(semReadReady);
            
            exit(0);
        }
        
    }
    
    for (int i = 0; i < argc - 1; i++) {
        char buffer[MAX_BUFFER];
        
        sem_wait(semReadReady);
        strncpy(buffer, sharedMemory, MAX_BUFFER);
        sem_post(semWriteReady);
        
        printf("PARENT: readed from shared: %s\n", buffer);
    }
    
    
    /*ожидание завершения дочернего процесса */
    for (int i = 0; i < argc - 1; i++) {
        waitpid(pid[i], NULL, 0);
    }
    
    sem_close(semWriteReady);
    sem_close(semReadReady);
    
    munmap(sharedMemory, MAX_BUFFER);
    
    return 0;
}