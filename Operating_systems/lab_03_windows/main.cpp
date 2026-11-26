#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>
#include <stdint.h>

#define PROC_CNT 11

HANDLE mutex_num;
HANDLE can_read;
HANDLE can_write;

volatile LONG write_queue = 0;
volatile LONG read_queue = 0;
volatile LONG active_readers = 0;

volatile int num = 'a' - 1;
volatile int sig_flag = 1;

void start_read(void)
{
	InterlockedIncrement(&read_queue);

	if (write_queue || WaitForSingleObject(can_write, 0) == WAIT_OBJECT_0)
		WaitForSingleObject(can_read, INFINITE);

	WaitForSingleObject(mutex_num, INFINITE);

	InterlockedDecrement(&read_queue);
	InterlockedIncrement(&active_readers);

	SetEvent(can_read);
	ReleaseMutex(mutex_num);
}
void stop_read(void)
{
	InterlockedDecrement(&active_readers);

	if (active_readers == 0)
		SetEvent(can_write);
}
void start_write(void)
{
	InterlockedIncrement(&write_queue);
	if (WaitForSingleObject(can_read, 0) == WAIT_OBJECT_0)
		WaitForSingleObject(can_write, INFINITE);
	// WaitForSingleObject(mutex_num, INFINITE);  // ЭТУ СТРОКУ НАДО СКОМПИЛИРОВАТЬ, НО ЗАКОММЕНТИРОВАТЬ ПРИ РЯЗАНОВОЙ
	InterlockedDecrement(&write_queue);
}
void stop_write(void)
{
    // ReleaseMutex(mutex_num);	// ЭТУ СТРОКУ НАДО СКОМПИЛИРОВАТЬ, НО ЗАКОММЕНТИРОВАТЬ ПРИ РЯЗАНОВОЙ
	ResetEvent(can_write);
	if (read_queue > 0)
		SetEvent(can_read);
	else
		SetEvent(can_write); 
}
DWORD WINAPI func_writer(CONST LPVOID lpParams)
{
	intptr_t writer_id = (intptr_t)lpParams;
	while (sig_flag)
	{
		start_write();
		num++;
		if (num > 'z')
			num = 'a';
		printf("Writer %lu -> %c\n", GetCurrentThreadId(), num);
		stop_write();
	}
	return 0;
}
DWORD WINAPI func_reader(CONST LPVOID lpParams)
{
	intptr_t reader_id = (intptr_t)lpParams;
	while (sig_flag)
	{
		start_read();
		printf("Reader %lu -> %c\n", GetCurrentThreadId(), num);
		stop_read();
	}
	return 0;
}
BOOL WINAPI signal_handler(DWORD signal)
{
	if (signal == CTRL_C_EVENT)
	{
		sig_flag = 0;
		printf("Program catch Ctrl-C.\n");
		return TRUE;
	}
	return FALSE;
}
int main(void)
{
	setbuf(stdout, NULL);
	if (!SetConsoleCtrlHandler(signal_handler, TRUE))
	{
		perror("Error: set signal handler.\n");
		exit(EXIT_FAILURE);
	}
	HANDLE pthread[PROC_CNT];
	DWORD thid[PROC_CNT];
	mutex_num = CreateMutex(NULL, FALSE, NULL);
	can_read = CreateEvent(NULL, FALSE, FALSE, NULL);
	can_write = CreateEvent(NULL, TRUE, FALSE, NULL);
	for (int i = 0; i < PROC_CNT; i++)
	{
        if (i % 3 == 0)
		    pthread[i] = CreateThread(NULL, 0, func_writer, (LPVOID)(intptr_t)i, 0, &thid[i]);
        else
        	pthread[i] = CreateThread(NULL, 0, func_reader, (LPVOID)(intptr_t)i, 0, &thid[i]);
	}
	DWORD dw = WaitForMultipleObjects(PROC_CNT, pthread, TRUE, INFINITE);
	if (dw == WAIT_OBJECT_0)
	{
		for (int i = 0; i < PROC_CNT; i++)
			printf("thread %lu finished\n", thid[i]);
	}
	else if (dw == WAIT_TIMEOUT)
		printf("waitThread timeout %lu\n", dw);
	else if (dw == WAIT_FAILED)
		printf("waitThread failed %lu\n", dw);
	else
		printf("unknown %lu\n", dw);
	CloseHandle(mutex_num);
	CloseHandle(can_read);
	CloseHandle(can_write);

	return 0;
}