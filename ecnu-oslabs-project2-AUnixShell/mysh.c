#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#define MAXN 1000

char *homePath, curPath[MAXN];
char *newline = "\n";
char error_message[] = "An error has occurred\n";

int getargs(int *argc, char **argv, char *input, int *redpos, int *pipepos) {
    char *in = NULL;
    int i, j;
    int len = strlen(input);
    for (i = 0; i < len; ++i) {
        if (input[i] == '<' || input[i] == '>' || input[i] == '&' || input[i] == '|') {
            for (j = len + 2; j > i; --j)
                input[j] = input[j - 2];
            input[i + 1] = input[i];
            input[i + 2] = ' ';
            input[i] = ' ';
            ++i;
            len += 2;
        }
    }
    in = input;
    for (i = 0; i < MAXN - 1; ++i) {
        if ((argv[i] = strtok(in, " \t\n")) == NULL)
            break;
        if ((!strcmp(argv[i], ">") || !strcmp(argv[i], "<")) && !*redpos)
            *redpos = i;
        if (!strcmp(argv[i], "|"))
            *pipepos = i;
        in = NULL;
    }
    if (i > MAXN - 1) {
        write(STDERR_FILENO, error_message, strlen(error_message));
        return 0;
    }
    *argc = i;
    return 1;
}

void mypipe(int pipepos, int argc, char **argv) {
    int fd[2];
    if (pipe(fd) < 0) {
        write(STDERR_FILENO, error_message, strlen(error_message));
        return;
    }
    int lastpos = pipepos - 1;
    while (lastpos > 0 && strcmp(argv[lastpos], "|")) --lastpos;
    int pid = fork();
    switch (pid) {
    case -1:
        write(STDERR_FILENO, error_message, strlen(error_message));
        break;
    case 0:
        dup2(fd[1], STDOUT_FILENO);
        close(fd[0]);
        close(fd[1]);
        if (lastpos != 0)
            mypipe(lastpos, pipepos, argv);
        else {
            argv[pipepos] = NULL;
            if (execvp(argv[0], argv) == -1)
                write(STDERR_FILENO, error_message, strlen(error_message));
        }
        break;
    default:
        argv[argc] = NULL;
        dup2(fd[0], STDIN_FILENO);
        close(fd[0]);
        close(fd[1]);
        while (wait(NULL) > 0);
        if (execvp(argv[pipepos + 1], &argv[pipepos + 1]) == -1)
            write(STDERR_FILENO, error_message, strlen(error_message));
        break;
    }
}

void execute(int argc, char **argv, int redpos, int pipepos) {
    int isBackGround = strcmp(argv[argc - 1], "&"), stat, i;
    if (!isBackGround)
        argv[--argc] = NULL;
    if (strcmp(argv[0], "cd") == 0) {
        if (argc > 1) {
            if (chdir(argv[1]) == -1) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                exit(1);
            }
            getcwd(curPath, MAXN);
        } else {
            chdir(homePath);
            getcwd(curPath, MAXN);
        }
        return;
    }
    int pid = fork();
    switch (pid) {
    case -1:
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    case 0:
        for (i = 0; i < argc; ++i) {
            if (argv[i][0] == '$' && getenv(&argv[i][1]))
                argv[i] = getenv(&argv[i][1]);
        }
        if (pipepos) {
            if (redpos) {
                if (argc - redpos != 2)
                    write(STDERR_FILENO, error_message, strlen(error_message));
                else {
                    close(STDOUT_FILENO);
                    int fd = open(argv[redpos + 1], O_CREAT | O_TRUNC | O_WRONLY, (S_IRWXU ^ S_IXUSR) | S_IRGRP | S_IROTH);
                    if (fd == -1) {
                        write(STDERR_FILENO, error_message, strlen(error_message));
                        exit(0);
                    }
                    argv[redpos] = NULL;
                    mypipe(pipepos, argc, argv);
                    exit(0);
                }
            } else
                mypipe(pipepos, argc, argv);
            exit(1);
        }
        if (strcmp(argv[0], "wait") == 0)
            exit(0);
        else if (strcmp(argv[0], "pwd") == 0) {
            if (argc > 1) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                exit(0);
            }
            write(STDOUT_FILENO, curPath, strlen(curPath));
            write(STDOUT_FILENO, newline,strlen(newline));
            exit(0);
        } else {
            if (!redpos && execvp(argv[0], argv) == -1) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                exit(0);
            } else {
                if (argc-redpos != 2) {
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    exit(0);
                }
                close(STDOUT_FILENO);
                int fd = open(argv[redpos + 1], O_CREAT | O_TRUNC | O_WRONLY, (S_IRWXU ^ S_IXUSR) | S_IRGRP | S_IROTH);
                if (fd == -1) {
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    exit(0);
                }
                argv[redpos] = NULL;
                if (execvp(argv[0], argv) == -1) {
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    exit(0);
                }
            }
        }
        break;
    default:
        if (strcmp(argv[0], "wait") == 0) {
            if (argc != 1) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                break;
            }
            while (wait(&stat) > 0);
            break;
        } else if(isBackGround)
            waitpid(pid, NULL, 0);
        break;
    }
}

void start(int flag) {
    char *argv[MAXN];
    char input[MAXN];
    while (1) {
        if (flag)
            write(STDOUT_FILENO, "mysh> ", 7);
        int argc = 0, redpos = 0, pipepos = 0;
        int pos = 0, rc;
        while (pos < MAXN - 1 && (rc = read(STDIN_FILENO, &input[pos], 1)) && input[pos] != '\n') ++pos;
        input[pos] = '\0';
        if (pos == 0 && rc == 0)
            break;
        else if (pos > 512) {
            write(STDOUT_FILENO, input, strlen(input));
            write(STDOUT_FILENO, newline, strlen(newline));
            write(STDERR_FILENO, error_message, strlen(error_message));
            continue;
        }
        if (!flag) {
            write(STDOUT_FILENO, input, strlen(input));
            write(STDOUT_FILENO, newline, strlen(newline));
        }
        if (getargs(&argc, argv, input, &redpos, &pipepos) && argc > 0) {
            if (strcmp(argv[0], "exit") == 0) {
                if (argc != 1)
                    write(STDERR_FILENO, error_message, strlen(error_message));
                else break;
            } else execute(argc, argv, redpos, pipepos);
        }
    }
}

int main(int argc, char *argv[]) {
    homePath = getenv("HOME");
    getcwd(curPath, MAXN - 1);
    int i, fd;
    if (argc > 1) {
        for (i = 1; i < argc; ++i) {
            if ((fd = open(argv[i], O_RDONLY)) == -1)
                write(STDERR_FILENO, error_message, strlen(error_message));
            else {
                dup2(fd, STDIN_FILENO);
                start(0);
            }
        }
    } else
        start(1);
    return 0;
}

