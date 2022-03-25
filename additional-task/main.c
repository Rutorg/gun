// В аргументах задается размер поля и количество шариков
// 
// Все шарики появляются в центре поля (или примерно в центре)
//
// За смещение каждого шарика на случайную величину
// отвечает собственный поток
//
// Используется мьютекс для поля и условная переменная для синхронизации.
// Главный поток, который выводит поле, после очистки поля посылает сигнал, чтобы потоки изменили координаты шариков.
//
// Значения координаты шарика x = -1 значит что поток завершился (ушел за границы)
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#define DELTA_TIME_MICROSEC (1000000)
#define SPACE_CHAR ('*')
#define BALL_CHAR ('0')


typedef struct ball_t
{
    int x;
    int y;
} ball_t;

typedef struct ballThread_arg_t
{
    ball_t* ball;
    char* field;
    pthread_mutex_t* fieldMutex;
    pthread_cond_t* fieldIsClearedCond;
    int X_SIZE;
    int Y_SIZE;
} ballThread_arg_t;


void* ballThreadFunc(void* arg)
{
    ballThread_arg_t* ballThread_arg = arg;

    ball_t* ball = ballThread_arg->ball;
    char* field = ballThread_arg->field;

    pthread_mutex_t* fieldMutex = ballThread_arg->fieldMutex;
    pthread_cond_t* fieldIsClearedCond = ballThread_arg->fieldIsClearedCond;

    const int X_SIZE = ballThread_arg->X_SIZE;
    const int Y_SIZE = ballThread_arg->Y_SIZE;


    const int MAX_CHANGE_IN_SECOND = 1;
    const int MAX_CHANGE = MAX_CHANGE_IN_SECOND * DELTA_TIME_MICROSEC / 1000000;
    const int VALUES_NUMBER = MAX_CHANGE_IN_SECOND * 2 + 1;

    const int X_UP_LIMIT = X_SIZE - 1;
    const int X_DOWN_LIMIT = -1;

    const int Y_UP_LIMIT = Y_SIZE;
    const int Y_DOWN_LIMIT = -1;

    // Основной цикл шара
    while (1) {
        int DX = (rand() % VALUES_NUMBER) - MAX_CHANGE;
        int DY = (rand() % VALUES_NUMBER) - MAX_CHANGE;

        int new_x = ball->x + DX;
        int new_y = ball->y + DY;

        if ( (X_DOWN_LIMIT < new_x) && (new_x < X_UP_LIMIT) &&
             (Y_DOWN_LIMIT < new_y) && (new_y < Y_UP_LIMIT) ) 
        {
            ball->x += DX;
            ball->y += DY;
        } else {
            pthread_mutex_lock(fieldMutex);
            ball->x = -1;
            pthread_mutex_unlock(fieldMutex);
            pthread_exit(0);
        }


        // Няпрямую пишем в field
        pthread_mutex_lock(fieldMutex);

        pthread_cond_wait(fieldIsClearedCond, fieldMutex);
        field[(ball->y) * X_SIZE + (ball->x)] = BALL_CHAR;

        pthread_mutex_unlock(fieldMutex);
    }
}

void fieldClear(char* field, const size_t X_SIZE, const size_t Y_SIZE)
{
    // Заполнение пустотой и спец. символы
    for (int i = 0; i < Y_SIZE * X_SIZE; i++) {
        field[i] = SPACE_CHAR;
    }
    
    for (int i = X_SIZE - 1; i < Y_SIZE * X_SIZE; i += X_SIZE) {
        field[i] = '\n';
    }
    field[(Y_SIZE * X_SIZE) - 1] = 0;
}


int main(int argc, char** argv) 
{
    if (argc < 4) {
        printf("Usage: x_size y_size balls_number\n");
        return -1;
    }

    srand(time(NULL));


    // +1 для спец. символов
    const int X_SIZE = atoi(argv[1]) + 1;
    if (X_SIZE < 2) {
        printf("X_SIZE must be > 0\n");
        return -1;
    }
    const int Y_SIZE = atoi(argv[2]);
    if (Y_SIZE < 1) {
        printf("Y_SIZE must be > 0\n");
        return -1;
    }
    const int BALLS_NUMBER = atoi(argv[3]);
    if (BALLS_NUMBER < 1) {
        printf("BALLS_NUMBER must be > 0\n");
        return -1;
    }

    char* field = calloc(Y_SIZE, X_SIZE * sizeof(char));
    fieldClear(field, X_SIZE, Y_SIZE);
    

    // -1, потому что при вычислении центра мы хотим видимый размер
    const int X_CENTER = (X_SIZE - 1) / 2;
    const int Y_CENTER = Y_SIZE / 2;

    ball_t* balls = calloc(BALLS_NUMBER, sizeof(ball_t));
    for (int i = 0; i < BALLS_NUMBER; i++) {
        balls[i].x = X_CENTER;
        balls[i].y = Y_CENTER;
    }
    // Ставим в центре шар. В любом случае там будет.
    field[Y_CENTER * X_SIZE + X_CENTER] = BALL_CHAR;


    pthread_mutex_t fieldMutex;
    pthread_mutex_init(&fieldMutex, NULL);

    pthread_cond_t fieldIsClearedCond;
    pthread_cond_init(&fieldIsClearedCond, NULL);


    pthread_t threads[BALLS_NUMBER];
    ballThread_arg_t ballThread_args[BALLS_NUMBER];

    for (int i = 0; i < BALLS_NUMBER; i++) {
        ballThread_args[i].ball = balls + i;
        ballThread_args[i].field = field;
        ballThread_args[i].fieldMutex = &fieldMutex;
        ballThread_args[i].fieldIsClearedCond = &fieldIsClearedCond;
        ballThread_args[i].X_SIZE = X_SIZE;
        ballThread_args[i].Y_SIZE = Y_SIZE;


        pthread_create(threads + i, NULL, ballThreadFunc, (void*) (ballThread_args + i));
    }

    // Подождем пока потоки не начнут ждать главный поток на всякий случай
    usleep(DELTA_TIME_MICROSEC);
    
    // Главный цикл
    while (1) {
        pthread_mutex_lock(&fieldMutex);

        int ballsRemained = 0;
        for (int i = 0; i < BALLS_NUMBER; i++) {
            if (balls[i].x != -1) {
                ballsRemained++;
            }
        }
        // Множество переносов служат для отображения поля всегда внизу консоли
        printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
        "Balls remained: %d"
        "\n%s\n",
        ballsRemained,
        field);

        fieldClear(field, X_SIZE, Y_SIZE);
        pthread_mutex_unlock(&fieldMutex);
        pthread_cond_broadcast(&fieldIsClearedCond);
        
        usleep(DELTA_TIME_MICROSEC);
    }

    pthread_mutex_destroy(&fieldMutex);
    pthread_cond_destroy(&fieldIsClearedCond);
    free(field);
    free(balls);

    return 0;
}
