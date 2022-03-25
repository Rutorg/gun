// Программа создает столько каналов сколько будет дочерних
// процессов, т. е. сколько файлов будет обработано.

// Общение будет вестись только от дочерних процессов к родительскому.

// Дочерний процесс создает свой дочерний, в котором 
// исполняется программа main-static выполняющая собственно алгоритм.
// Далее дочерний процесс получает результат и записывает в канал.
// Далее родительский процесс читает из канала поочередно и выводит.

#include <unistd.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>


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
    
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = 64;
    attr.mq_curmsgs = 0;
    
    // Очередь. Одна на всех
    mqd_t myQueue = mq_open("/my-queue", O_RDONLY | O_CREAT, S_IRUSR | S_IWUSR, &attr);
    if (myQueue == -1) {
        perror("Parent: mq_open (parent)");
        exit (1);
    }
    printf("PARENT: queue created\n");
    
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
            
            mqd_t childQueue = mq_open("/my-queue", O_WRONLY);
            if (myQueue == -1) {
                perror("Child: mq_open (child)");
                exit (1);
            }
            
            printf("CHILD: %d-process opened queue\n", i);
            
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
            printf("CHILD: %d-process Writing in queue. TEXT: %s, LENGTH: %d\n", i, buffer, (int)strlen(buffer) + 1);
            
            
            if (mq_send(childQueue, buffer, strlen(buffer) + 1, 0) == -1) {
                perror ("Child: Not able to send message to parent");
            }
            
            mq_close(childQueue);
            
            exit(0);
        }
        
    }
    
    
    for (int i = 0; i < argc - 1; i++) {
        char buffer[MAX_BUFFER];
        if (mq_receive(myQueue, buffer, MAX_BUFFER, NULL) == -1) {
            perror("PARENT: error when recieving\n");
        }
        
        printf("PARENT: readed. Result: %s\n", buffer);
    }
    
    mq_close(myQueue);
    mq_unlink("/my-queue");
    
    /*ожидание завершения дочернего процесса */
    for (int i = 0; i < argc - 1; i++) {
        waitpid(pid[i], NULL, 0);
    }
    
    return 0;
}