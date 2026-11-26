// Решение Э. Дейкстры задачи «производство-потребление»
#define _POSIX_C_SOURCE 200809L

#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <stdbool.h>

#define SIZE_BUF 5

// семафоры
#define BIN_SEM 0
#define BUFF_FULL 1
#define BUFF_EMPTY 2

#define PROC_CNT 10

// операции над семафорами (уменьшение и увеличение)
// #define P -1
// #define V 1

/* struct sembuf {
    short sem_num; // индекс семафора
    short sem_op; // операция: увеличение, уменьшение или проверка значения
    short sem_flg; // флаги
} */

struct sembuf start_produce[2] = {{BUFF_EMPTY, -1, 0}, {BIN_SEM, -1, 0}};
struct sembuf stop_produce[2] = {{BIN_SEM, 1, 0}, {BUFF_FULL, 1, 0}};
struct sembuf start_consume[2] = {{BUFF_FULL, -1, 0}, {BIN_SEM, -1, 0}};
struct sembuf stop_consume[2] = {{BIN_SEM, 1, 0}, {BUFF_EMPTY, 1, 0}};

volatile bool no_signal = 1;

void setup_timer_us(long microseconds)
{
    struct itimerval timer;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;

    timer.it_value.tv_sec = microseconds / 1000000;
    timer.it_value.tv_usec = microseconds % 1000000;

    if (setitimer(ITIMER_REAL, &timer, NULL) == -1)
    {
        printf("setitimer\n");
        exit(1);
    }
}

void alarm_handler(int sig_num)
{
    printf("pid = %d received signal %d\n", getpid(), sig_num);
    no_signal = false;
}
// ПРОИЗВОДИТЕЛЬ
void producer(void *addr, const int semid)
{

    size_t *idx_prod_p = (size_t *)addr;
    size_t *idx_cons_p = idx_prod_p + 1;
    char *next_letter_p = (char *)(idx_cons_p + 1);
    char *buf = next_letter_p + 1;
    while (no_signal)
    {

        if (semop(semid, start_produce, 2) == -1)
        {
            printf("Error: semop pid = %d, errno %d\n", getpid(), errno);
            exit(1);
        }

        buf[*idx_prod_p] = *next_letter_p;
        printf("Producer %d -> %c\n", getpid(), buf[*idx_prod_p]);
        *next_letter_p = 'a' + (*next_letter_p - 'a' + 1) % 26;
        *idx_prod_p = (*idx_prod_p + 1) % SIZE_BUF;
        if (semop(semid, stop_produce, 2) == -1)
        {
            printf("Error: semop pid = %d, errno %d\n", getpid(), errno);
            exit(1);
        }
    }
    exit(1);
}

void consumer(void *addr, const int semid)
{
    size_t *idx_prod_p = (size_t *)addr;
    size_t *idx_cons_p = idx_prod_p + 1;
    char *next_letter_p = (char *)(idx_cons_p + 1);
    char *buf = next_letter_p + 1;
    while (no_signal)
    {

        if (semop(semid, start_consume, 2) == -1)
        {
            printf("Error: semop pid = %d, errno %d\n", getpid(), errno);
            exit(1);
        }
        printf("Consumer %d %c ->\n", getpid(), buf[*idx_cons_p]);
        *idx_cons_p = (*idx_cons_p + 1) % SIZE_BUF;
        if (semop(semid, stop_consume, 2) == -1)
        {
            printf("Error: semop pid = %d, errno %d\n", getpid(), errno);
            exit(1);
        }
    }
    exit(1);
}

int main(void)
{
    setbuf(stdout, NULL);

    if (signal(SIGALRM, alarm_handler) == SIG_ERR)
    {
        perror("Error: signal SIGALRM.\n");
        exit(EXIT_FAILURE);
    }
    // setup_timer_us(2000);
    key_t shmkey = ftok("file.txt", 3);
    if (shmkey == (key_t)-1)
    {
        perror("Error: ftok.\n");
        exit(1);
    }

    // права доступа
    int perms = S_IRWXU | S_IRWXG | S_IRWXO;
    int shmid = shmget(shmkey, (SIZE_BUF + 1) * sizeof(char) + sizeof(size_t *) * 2, perms | IPC_CREAT);
    if (shmid == -1)
    {
        perror("Error: shmget.\n");
        exit(1);
    }

    void *addr = shmat(shmid, NULL, 0);
    if (addr == (void *)-1)
    {
        perror("Error: shmat.\n");
        exit(1);
    }

    size_t *idx_prod_p = (size_t *)addr;
    size_t *idx_cons_p = idx_prod_p + 1;
    char *next_letter_p = (char *)(idx_cons_p + 1);
    *idx_cons_p = 0;
    *idx_prod_p = 0;
    *next_letter_p = 'a';
    key_t semkey = ftok("file.txt", 4);
    if (semkey == (key_t)-1)
    {
        perror("Error: ftok.\n");
        exit(1);
    }

    int semid = semget(semkey, 3, IPC_CREAT | perms);
    if (semid == -1)
    {
        perror("Error: semget.\n");
        exit(1);
    }

    int rc_bin_sem = semctl(semid, BIN_SEM, SETVAL, 1);
    if (rc_bin_sem == -1)
    {
        perror("Error: semctl.\n");
        exit(1);
    }
    int rc_buff_empty = semctl(semid, BUFF_EMPTY, SETVAL, SIZE_BUF);
    if (rc_buff_empty == -1)
    {
        perror("Error: semctl.\n");
        exit(1);
    }
    int rc_buff_full = semctl(semid, BUFF_FULL, SETVAL, 0);
    if (rc_buff_full == -1)
    {
        perror("Error: semctl.\n");
        exit(1);
    }

    pid_t cpid[PROC_CNT];
    for (size_t i = 0; i < PROC_CNT; ++i)
    {
        cpid[i] = fork();
        if (cpid[i] == -1)
        {
            perror("Error: fork producer.\n");
            exit(1);
        }
        else if (cpid[i] == 0)
        {
            setup_timer_us(2000);
            if (i % 3)
                producer(addr, semid);
            else
                consumer(addr, semid);
        }
    }

    for (size_t i = 0; i < PROC_CNT; ++i)
    {
        int wait_status;
        pid_t res_waitpid;
        do
        {
            res_waitpid = waitpid(cpid[i], &wait_status, WUNTRACED | WCONTINUED);
        } while (res_waitpid == -1 && errno == EINTR);
        if (res_waitpid == -1)
        {
            perror("Error: waitpid.\n");
            exit(EXIT_FAILURE);
        }
        if (WIFEXITED(wait_status))
            printf("%d exited, status=%d, errno %d\n", cpid[i], WEXITSTATUS(wait_status), errno);
        else if (WIFSIGNALED(wait_status))
            printf("%d killed by signal %d, errno %d\n", cpid[i], WTERMSIG(wait_status), errno);
        else if (WIFSTOPPED(wait_status))
            printf("%d stopped by signal %d, errno %d\n", cpid[i], WSTOPSIG(wait_status), errno);
    }
    if (shmdt(addr) == -1)
    {
        printf("Error: shmdt.\n");
        exit(1);
    }

    if (semctl(semid, 0, IPC_RMID, NULL) == -1)
    {
        printf("Error: delete semafor.\n");
        exit(1);
    }
    if (shmctl(shmid, IPC_RMID, NULL) == -1)
    {
        printf("Error: delete segment.\n");
        exit(1);
    }
    return 0;
}