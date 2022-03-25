// Клиент получает строку, в которой
// содержится файл, который нужно обработать и
// максимальное количество замен.
//
// Далее поток создает дочерний процесс,
// в котором вызывается execl main-static.exe с
// нужными аргументами.
//
// Далее клиент посылает полученный результат из
// дочернего процесса


#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>


#define REPLACE_NUMBER 100
#define MAX_BUFFER 100


int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: ip_of_server\n");
        return -1;
    }

    // Инициализируем сокет TCP SOCK_STREAM
    int socketDSC = socket(AF_INET, SOCK_STREAM, 0);
    if(socketDSC < 0) {
        printf("Socket error\n");
        return -1;
    }

    // Структура с адресом
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(12345);
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    if (addr.sin_addr.s_addr == ( in_addr_t)(-1)) 
    {
        printf("IP format error\n");
        return -1;
    }

    if(connect(socketDSC, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        printf("Connect error\n");
        return -1;
    }

    char buf[MAX_BUFFER];
    int recvLen = recv(socketDSC, buf, MAX_BUFFER, 0);
    if (recvLen == -1) 
    {
        printf("Receive error\n");
        return -1;
    }
    buf[recvLen] = 0;
    printf("Received: %s\nReceived message length: %d\n", buf, recvLen);

    char *inputFile = strtok(buf, " ");
    char *replaceNumber = strtok(NULL, " ");
    char outputFile[MAX_BUFFER] = "result-";
    strcat(outputFile, inputFile);

    pid_t prog = fork();
    if (prog == 0) {
        printf("Processing of file %s started. inputFile: %s, resultFile: %s, replaceNumber: %s\n",
            inputFile, inputFile, outputFile, replaceNumber);
        execl("./main-static.exe", "main-static", inputFile, outputFile, replaceNumber, (char*) NULL);
    }

    int returned = 0;
    waitpid(prog, &returned, 0);
    returned = WEXITSTATUS(returned);
    printf("Returned: %d\n", returned);

    int sendedLen = send(socketDSC, &returned, sizeof(returned), 0);
    if (sendedLen == -1) 
    {
        printf("Send error\n");
        return -1;
    }

    printf("Sent returned value\n");


    close(socketDSC);

    return 0;
}