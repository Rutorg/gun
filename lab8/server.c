// Сервер раздает названия обрабатываемых файлов и макс. 
// количество замен.
//
// После того как сервер отослал данные одному клиенту
// он запускает отдельный поток, который ждет ответ этого клиента.
//
// Таким образом, сервер одновременно и отдает задания и собирает 
// результаты.
// 
// После того как сервер раздал все задания он ждет все потоки 
// и завершает работу.
// 

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>


#define REPLACE_NUMBER 100
#define MAX_BUFFER 100

int receiveResult(int acceptedDSC)
{
    printf("Thread started. AcceptedDSC: %d\n", acceptedDSC);
    int receivedNumber = 0;
    int recvLen = recv(acceptedDSC, &receivedNumber, MAX_BUFFER, 0);
    if (recvLen == -1) 
    {
        printf("Receive error\n");
        return -1;
    }
    printf("Thread received: %d\n", receivedNumber);

    close(acceptedDSC);

    return receivedNumber;
}


int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("Usage: file1 file2 ...\n");
        return -1;
    }

    // Создаем сокет TCP. type=SOCK_STREAM
    int socketDSC = socket(AF_INET, SOCK_STREAM, 0);
    if(socketDSC < 0) {
        printf("Socket error\n");
        return -1;
    }

    // Заполняем sockaddr_in
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(12345);
    // Адрес указываем любой
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    // Связываем сокет с адресом
    if(bind(socketDSC, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        printf("bind error\n");
        return -1;
    }

    // Основной цикл
    if (listen(socketDSC, 1) == -1)  {
        printf("listen error\n");
        return -1;
    }

    printf("Listening...\n");
    int FILE_NUM = argc - 1;
    char **files = argv + 1;
    char replaceNumberStr[MAX_BUFFER];
    sprintf(replaceNumberStr, "%d", REPLACE_NUMBER);

    pthread_t threads[FILE_NUM];

    // Цикл отправки названий файлов и кол-ва замен
    for (int i = 0; i < FILE_NUM; i++) {
        int acceptedDSC = accept(socketDSC, 0, 0);
        if (acceptedDSC < 0) {
            printf("Accept error\n");
        }

        printf("Accepted\n");

        char buf[MAX_BUFFER];
        strcpy(buf, files[i]);
        strcat(buf, " ");
        strcat(buf, replaceNumberStr);

        int sendLen = send(acceptedDSC, buf, strlen(buf), 0);
        if (sendLen == -1) {
            printf("Send error\n");
            return -1;
        }

        printf("Sent string: %s\nSent message length: %d\n", buf, sendLen);

        pthread_create(threads + i, NULL, (void *) receiveResult, (void *) (intptr_t)  acceptedDSC);
    }

    for (int i = 0; i < FILE_NUM; i++) {
        int* returned;
        pthread_join(threads[i], (void **) &returned);
    }
    

    close(socketDSC);
    
    return 0;
}
