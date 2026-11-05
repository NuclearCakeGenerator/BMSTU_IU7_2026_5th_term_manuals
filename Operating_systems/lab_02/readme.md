Структура рабочей директории:

```
.
├── 1.c
├── 2.c
├── 3.c
├── 4.c
├── 5.c
├── build
│   ├── 1.exe
│   ├── 2.exe
│   ├── 3.exe
│   ├── 4.exe
│   ├── 5.exe
│   ├── my_prog_1.exe
│   └── my_prog_2.exe
├── my_prog_1.c
├── my_prog_2.c
└── .vscode
    └── tasks.json
```

`N.c` и `N.exe` - исходный код и исполняемый файл для
соответствующей подзадачи.

`my_prog_N.c` - вспомогательные программы для 3-ей
подзадачи.

Компиляция исходного кода в исполняемый файл сочетанием
`ctrl + shift + B` с открытым кодом в редакторе.

[Первое упоминание](https://t.me/c/2204134426/92437/96241) того, как сдавать лабу.

Исходные коды:

`1.c`:

```c
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

int main(void)
{
    pid_t childpid[2];
    for (size_t i = 0; i < 2; i++)
    {
        childpid[i] = fork();
        if (childpid[i] == -1)
        {
            perror("Can't fork.\n");
            exit(1);
        }
        else if (childpid[i] == 0)
        {
            printf("Child: PID = %d, PPID = %d, group = %d\n",
                   getpid(), getppid(), getpgrp());
            if (i == 1)
            {
                sleep(2);
                printf("Child after sleep: PID = %d, PPID = %d, group = %d\n",
                       getpid(), getppid(), getpgrp());
            }
            exit(0);
        }
    }
    printf("Parent: PID = %d, group = %d, children = %d, %d\n",
           getpid(), getpgrp(), childpid[0], childpid[1]);
    exit(0);
}
```

`2.c`:

```c
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

int main(void)
{
    pid_t childpid[2];
    int status;
    for (size_t i = 0; i < 2; i++)
    {
        childpid[i] = fork();
        if (childpid[i] == -1)
        {
            perror("Can't fork.\n");
            exit(1);
        }
        else if (childpid[i] == 0)
        {
            printf("Child №%ld: PID = %d, PPID = %d, group = %d\n",
                   i + 1, getpid(), getppid(), getpgrp());
            if (i)
                sleep(30);
            exit(0);
        }
    }
    printf("Parent: PID = %d, group = %d, children = %d, %d\n",
           getpid(), getpgrp(), childpid[0], childpid[1]);
    for (size_t i = 0; i < 2; i++)
    {
        pid_t terminated_pid = wait(&status);
        if (terminated_pid == -1)
        {
            perror("Wait failed");
            exit(1);
        }
        if (WIFEXITED(status))
            printf("Child №%ld exited with code %d\n", i + 1, WEXITSTATUS(status));
        else if (WIFSIGNALED(status))
            printf("Child №%ld terminated, recieved signal %d\n", i + 1, WTERMSIG(status));
        else if (WIFSTOPPED(status))
            printf("Child №%ld stopped, recieved signal %d\n", i + 1, WSTOPSIG(status));
    }
    exit(0);
}
```

`3.c`:

```c
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

int main(void)
{
    const char *programs[2] = {"./my_prog_1.exe", "./my_prog_2.exe"};
    pid_t childpid[2];
    for (size_t i = 0; i < 2; i++)
    {
        childpid[i] = fork();
        if (childpid[i] == -1)
        {
            perror("Can't fork.\n");
            exit(1);
        }
        else if (childpid[i] == 0)
        {
            printf("Child PID: %d\n", getpid());
            int rc = execlp(programs[i], programs[i], NULL);
            if (rc == -1)
            {
                perror("Can't exec.\n");
                exit(1);
            }
        }
        else
        {
            printf("Parent PID: %d\n", getpid());
        }
    }
    for (size_t i = 0; i < 2; i++)
    {
        int status;
        pid_t child_pid = wait(&status);
        if (child_pid == -1)
        {
            perror("Can't wait.\n");
            exit(1);
        }
        if (WIFEXITED(status))
            printf("Child №%ld exited with code %d\n", i + 1, WEXITSTATUS(status));
        else if (WIFSIGNALED(status))
            printf("Child №%ld terminated, received signal %d\n", i + 1, WTERMSIG(status));
        else if (WIFSTOPPED(status))
            printf("Child №%ld stopped, received signal %d\n", i + 1, WSTOPSIG(status));
    }
    exit(0);
}
```

`my_prog_1.c`:

```c
#include <stdio.h>

int main(void)
{
    int num;
    printf("Input integer: ");
    if (scanf("%d", &num) != 1)
    {
        return 1;
    }
    int ans = 0;
    for (int i = 1; i <= num; ++i)
    {
        ans += i;
    }
    printf("Sum of numbers from 0 to %d : %d\n", num, ans);
    return 0;
}
```

`my_prog_2.c`

```c
#include <stdio.h>

int main(void)
{
    char ch;
    printf("Input char: ");
    if (scanf("%c", &ch) != 1)
    {
        return 1;
    }
    printf("Char: %c\nASCII (dec): %d\nASCII (hex): %x\n", ch, ch, ch);
    return 0;
}
```

`4.c`:

```c
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <err.h>

int main()
{
    pid_t child_pid[2];
    int status;
    int bytes_read;
    char buffer[16];
    pid_t childpid;
    int fd[2];
    char *msg[] = {"aaa", "bbbbbbbbbb"};
    if (pipe(fd) == -1)
    {
        perror("Can't pipe\n");
        exit(1);
    }
    for (int i = 0; i < 2; i++)
    {
        if ((child_pid[i] = fork()) == -1)
        {
            perror("Can't fork");
            exit(1);
        }
        else if (child_pid[i] == 0)
        {
            printf("Child PID: %d\n", getpid());
            if (close(fd[0]) == -1)
                err(EXIT_FAILURE, "close");
            if (write(fd[1], msg[i], (strlen(msg[i]) + 1)) == -1)
            {
                perror("Can't write");
                exit(1);
            }
            exit(0);
        }
        else
        {
            printf("Parent PID: %d\n", getpid());
        }
    }

    if (close(fd[1]) == -1)
        err(EXIT_FAILURE, "close");
    for (int i = 0; i < 3; i++)
    {
        bytes_read = read(fd[0], buffer, sizeof(buffer) - 1);
        if (bytes_read >= 0)
        {
            buffer[bytes_read] = '\0';
            printf("%d: %s\n", i + 1, buffer);
        }
        else if (bytes_read == -1)
        {
            printf("Empty");
        }
    }

    if (close(fd[0]) == -1)
        err(EXIT_FAILURE, "close");

    for (int i = 0; i < 2; i++)
    {

        childpid = waitpid(child_pid[i], &status, 0);
        if (childpid == -1)
        {
            perror("Can't wait");
            exit(1);
        }
        if (WIFEXITED(status))
            printf("Child №%d exited with code %d\n", i + 1, WEXITSTATUS(status));
        else if (WIFSIGNALED(status))
            printf("Child №%d terminated, received signal %d\n", i + 1, WTERMSIG(status));
        else if (WIFSTOPPED(status))
            printf("Child №%d stopped, received signal %d\n", i + 1, WSTOPSIG(status));
    }
    exit(0);
}
```

`5.c`:

```c
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>

bool fl = 0;

void sig_handler(int signal)
{
    fl = 1;
    printf("PID %d, catch sig: %d\n", getpid(), signal);
}

int main()
{
    pid_t cpid[2];
    int wstatus;
    pid_t w;
    int bytes_read;
    char buffer[16];
    int fd[2];
    const char *msg[] = {"aaa", "bbbbbbbbbb"};

    if (signal(SIGINT, sig_handler) == SIG_ERR)
    {
        perror("Can`t signal");
        exit(1);
    }
    sleep(2);
    if (pipe(fd) == -1)
    {
        perror("Can't pipe\n");
        exit(1);
    }
    for (int i = 0; i < 2; i++)
    {
        if ((cpid[i] = fork()) == -1)
        {
            perror("Can't fork");
            exit(1);
        }
        else if (cpid[i] == 0)
        {
            if (fl)
            {
                printf("%d\n", getpid());
                if (close(fd[0]) == -1)
                {
                    perror("Can`t close");
                    exit(1);
                }
                if (write(fd[1], msg[i], (strlen(msg[i]) + 1)) == -1)
                {
                    perror("Can`t write");
                    exit(1);
                }
            }
            exit(0);
        }
        else
        {
            printf("%d\n", getpid());
        }
    }

    if (close(fd[1]) == -1)
    {
        perror("Can`t close");
        exit(1);
    }
    for (int i = 0; i < 2; i++)
    {
        bytes_read = read(fd[0], buffer, sizeof(buffer) - 1);
        if (bytes_read >= 0)
        {
            buffer[bytes_read] = '\0';
            printf("%d: %s\n", i + 1, buffer);
        }
        else if (bytes_read == -1)
        {
            printf("Can`t read");
        }
    }
    for (int i = 0; i < 2; i++)
    {
        w = waitpid(cpid[i], &wstatus, 0);
        if (w == -1)
        {
            perror("Can`t waitpid");
            exit(1);
        }
        if (WIFEXITED(wstatus))
        {
            printf("%d exited, status=%d\n", w, WEXITSTATUS(wstatus));
        }
        else if (WIFSIGNALED(wstatus))
        {
            printf("%d killed by signal %d\n", w, WTERMSIG(wstatus));
        }
        else if (WIFSTOPPED(wstatus))
        {
            printf("%d stopped by signal %d\n", w, WSTOPSIG(wstatus));
        }
    }
    exit(0);
}
```

`tasks.json`:

```json
{
  "version": "2.0.0",
  "tasks": [
    {
      "type": "shell",
      "label": "Build current C file",
      "dependsOn": ["Create build directory"],
      "command": "gcc",
      "args": [
        "-g",
        "-std=c99",
        "${file}",
        "-o",
        "./build/${fileBasenameNoExtension}.exe"
      ],
      "problemMatcher": ["$gcc"],
      "presentation": {
        "echo": true,
        "reveal": "silent",
        "focus": false,
        "panel": "shared",
        "showReuseMessage": true,
        "clear": false
      },
      "group": {
        "kind": "build",
        "isDefault": true
      }
    },
    {
      "type": "shell",
      "label": "Create build directory",
      "command": "mkdir",
      "args": ["-p", "./build"],
      "problemMatcher": [],
      "presentation": {
        "echo": true,
        "reveal": "silent",
        "focus": false,
        "panel": "shared",
        "showReuseMessage": false,
        "clear": true
      }
    }
  ]
}
```
