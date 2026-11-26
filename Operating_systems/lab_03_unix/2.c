#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>

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

#define ACTIVE_WRITER 0
#define ACTIVE_READER 1
#define WAITING_WRITERS 2
#define WAITING_READERS 3

#define WRITERS_CNT 4
#define READERS_CNT 5

volatile int no_signal = 1;

void signal_handle(int sig_num)
{
    no_signal = 0;
    printf("pid = %d received signal: %d\n", getpid(), sig_num);
}

void start_read(int semid)
{
    struct sembuf turn[] = {{WAITING_READERS, 1, 0}};
    if (semop(semid, turn, 1) == -1)
    {
        printf("semop\n");
        exit(1);
    }

    struct sembuf start[] = {{WAITING_WRITERS, 0, 0}, {ACTIVE_WRITER, 0, 0}, {WAITING_READERS, -1, 0}, {ACTIVE_READER, 1, 0}};
    if (semop(semid, start, 4) == -1)
    {
        printf("semop\n");
        exit(1);
    }
}

void stop_read(int semid)
{
    struct sembuf stop[] = {{ACTIVE_READER, -1, 0}};
    if (semop(semid, stop, 1) == -1)
    {
        printf("semop\n");
        exit(1);
    }
}

void start_write(int semid)
{
    struct sembuf wait[] = {{WAITING_READERS, 0, 0}, {WAITING_WRITERS, 1, 0}};
    if (semop(semid, wait, 2) == -1)
    {
        printf("semop\n");
        exit(1);
    }

    struct sembuf start[] = {{ACTIVE_READER, 0, 0}, {ACTIVE_WRITER, 0, 0}, {WAITING_WRITERS, -1, 0}, {ACTIVE_WRITER, 1, 0}};
    if (semop(semid, start, 4) == -1)
    {
        printf("semop\n");
        exit(1);
    }
}

void stop_write(int semid)
{
    struct sembuf stop[] = {{ACTIVE_WRITER, -1, 0}};
    if (semop(semid, stop, 1) == -1)
    {
        printf("semop\n");
        exit(1);
    }
}

void writer(char *addr, int semid)
{
    while (no_signal)
    {
        start_write(semid);
        ++(*addr);
        if ((*addr) > 'z')
            (*addr) = 'a';
        printf("Writer %d %c\n", getpid(), *addr);
        stop_write(semid);
    }
    exit(0);
}

void reader(char *addr, int semid)
{
    while (no_signal)
    {
        start_read(semid);
        printf("Reader %d %c\n", getpid(), *addr);
        stop_read(semid);
    }
    exit(0);
}

int main()
{
    if (signal(SIGALRM, signal_handle) == SIG_ERR)
    {
        printf("signal\n");
        exit(1);
    }
    int perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    key_t semkey = ftok("file.txt", 1);
    if (semkey == -1)
    {
        printf("ftok\n");
        exit(1);
    }
    int semid = semget(semkey, 4, IPC_CREAT | perms);
    if (semid == -1)
    {
        printf("semget\n");
        exit(1);
    }
    int rc_active_writer = semctl(semid, ACTIVE_WRITER, SETVAL, 0);
    int rc_active_readers = semctl(semid, ACTIVE_READER, SETVAL, 0);
    int rc_turn_writers = semctl(semid, WAITING_WRITERS, SETVAL, 0);
    int rc_turn_readers = semctl(semid, WAITING_READERS, SETVAL, 0);
    if (rc_active_writer == -1 || rc_active_readers == -1 || rc_turn_writers == -1 || rc_turn_readers == -1)
    {
        printf("semctl\n");
        exit(1);
    }
    key_t shmkey = ftok("file.txt", 2);
    if (shmkey == -1)
    {
        printf("ftok\n");
        exit(1);
    }
    int shmid = shmget(shmkey, sizeof(size_t), IPC_CREAT | perms);
    if (shmid == -1)
    {
        printf("shmget\n");
        exit(1);
    }
    char *addr = shmat(shmid, NULL, 0);
    if (addr == (char *)-1)
    {
        printf("shmat\n");
        exit(1);
    }
    *addr = 'a' - 1;

    for (size_t i = 0; i < WRITERS_CNT; ++i)
    {
        int child_pid;
        if ((child_pid = fork()) == -1)
        {
            printf("fork\n");
            exit(1);
        }
        else if (child_pid == 0)
        {
            setup_timer_us(2000);
            writer(addr, semid);
        }
    }
    for (size_t i = 0; i < READERS_CNT; ++i)
    {
        int child_pid;
        if ((child_pid = fork()) == -1)
        {
            printf("fork\n");
            exit(1);
        }
        else if (child_pid == 0)
        {
            setup_timer_us(2000);
            reader(addr, semid);
        }
    }
    for (int i = 0; i < READERS_CNT + WRITERS_CNT; ++i)
    {
        int status;
        pid_t wpid = wait(&status);

        if (wpid == -1)
        {
            printf("wait\n");
            exit(1);
        }
        printf("%d\n", wpid);
        if (WIFEXITED(status))
        {
            printf("exited, status=%d\n", WEXITSTATUS(status));
        }
        else if (WIFSIGNALED(status))
        {
            printf("killed by signal %d\n", WTERMSIG(status));
        }
        else if (WIFSTOPPED(status))
        {
            printf("stopped by signal %d\n", WSTOPSIG(status));
        }
    }

    if (shmdt(addr) == -1)
    {
        printf("shmdt\n");
        exit(1);
    }
    if (shmctl(shmid, IPC_RMID, NULL) == -1)
    {
        printf("shmctl\n");
        exit(1);
    }
    if (semctl(semid, 0, IPC_RMID) == -1)
    {
        printf("semctl\n");
        exit(1);
    }
    return 0;
}