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

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: file textfile1 textfile2 ...\n");
        exit(-1);
    }
    
    // argc = 3;
    // argv[1] = "f1.txt";
    // argv[2] = "f2.txt";
    

    int pid[argc];
    char arg[20];
    for (int i = 1; i < argc; i++) {
        // запускаем дочерний процесс
        strcpy(arg, argv[i]);
        
        char resultFile[64] = "result-";
        strcat(resultFile, arg);
        
        char *replaceNumber[64];
        sprintf(replaceNumber, "%d", REPLACE_NUMBER);
        
        fflush(stdout);
        pid[i] = fork();
        if (pid[i] == 0) {
            // если выполняется дочерний процесс
            // вызов функции счета количества пробелов в файле
            
            printf("blabl\n");

            fflush(stdout);
            printf("dfgdg\n");
            if (execl("./main-static", "main-static", arg, resultFile, replaceNumber, (char*) NULL) < 0) {
                printf("ERROR while start processing file %s\n", argv[i]);
                printf("Something went wrong: %s\n", strerror(errno));
                exit(-2);
                fflush(stdout);
            } else {
                printf("processing of file %s started (pid=%d)\n", argv[i], pid[i]);
            }
            fflush(stdout);
            printf("blabl\n");
            fflush(stdout);
            
        }
        // если выполняется родительский процесс
    }
    sleep(1);

    // ожидание окончания выполнения всех запущенных процессов
    int stat = 0;
    for (int i = 1; i < argc; i++) {
        int status = waitpid(pid[i], &stat, 0);
        if (pid[i] == status) {
            printf("File %s done,  result=%d\n", argv[i], WEXITSTATUS(stat));
        }
    }
    return 0;
}