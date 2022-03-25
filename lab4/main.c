// Программа создает столько каналов сколько будет дочерних
// процессов, т. е. сколько файлов будет обработано.

// Общение будет вестись только от дочерних процессов к родительскому.

// Дочерний процесс создает свой дочерний, в котором 
// исполняется программа main-static выполняющая собственно алгоритм.
// Далее дочерний процесс получает результат и записывает в канал.
// Далее родительский процесс читает из канала поочередно и выводит.

#include <sys/types.h>
#include <sys/stat.h>
#include <wait.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>


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
    
    // Каналы. Один на каждый дочерний процесс
    int pipes[argc - 1][2];
    for (int i = 0; i < argc; i++) {
        pipe(pipes[i]);
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
            
            printf("CHILD: close pipe on read\n");
            // закрываем чтение из канала.
            close(pipes[i][0]);
            
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
            printf("CHILD: returned: %d\n", returned);
            
            
            // записываем в канал
            char buffer[MAX_BUFFER];
            sprintf(buffer, "%d", returned);
            printf("CHILD: Writing in %d-pipe. TEXT: %s, LENGTH: %d\n", i, buffer, (int)strlen(buffer));
            write(pipes[i][1], buffer, strlen(buffer));
            
            
            // закрываем канал на запись. Канал закрыт.
            close(pipes[i][1]);
            exit(0);
        }
        printf("PARENT: close pipe on write\n");
        // если выполняется родительский процесс
        // закрываем канал на запись.
        close(pipes[i][1]);
    }
    
    
    for (int i = 0; i < argc - 1; i++) {
        char buffer[MAX_BUFFER];
        if (read(pipes[i][0], buffer, MAX_BUFFER) == -1) {
            printf("PARENT: error when reading\n");
        };
        
        printf("PARENT: readed from %d-pipe. %d-process. Result: %s\n", i, i, buffer);
        
        // закрываем канал на чтение. Канал закрыт.
        close(pipes[i][0]);
    }
    
    
    
    /*ожидание завершения дочернего процесса */
    for (int i = 0; i < argc - 1; i++) {
        waitpid(pid[i], NULL, 0);
    }
    
    return 0;
}