#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <sched.h>

int a;
int b;

FILE *listfd;

pthread_t *pthreads;
int pthreadsidx;

pid_t *pings;
int pingsidx;

char *trim(char *str)
{
    size_t len = 0;
    char *frontp = str;
    char *endp = NULL;

    if (str == NULL)
    {
        return NULL;
    }
    if (str[0] == '\0')
    {
        return str;
    }

    len = strlen(str);
    endp = str + len;

    while (isspace((unsigned char)*frontp))
    {
        ++frontp;
    }
    if (endp != frontp)
    {
        while (isspace((unsigned char)*(--endp)) && endp != frontp)
        {
        }
    }

    if (frontp != str && endp == frontp)
        *str = '\0';
    else if (str + len - 1 != endp)
        *(endp + 1) = '\0';

    endp = str;
    if (frontp != str)
    {
        while (*frontp)
        {
            *endp++ = *frontp++;
        }
        *endp = '\0';
    }

    return str;
}

void ctrlc_handler()
{

    for (int i = 0; i < pingsidx; i++)
    {
        kill(pings[i], SIGINT);
    }
    free(pings);
    free(pthreads);
    exit(0);
}

void *exec_target(char *args)
{
    pings[pingsidx] = fork();
    if (!pings[pingsidx])
    {
        char *argv[1000];
        char argz[strlen(args)];
        args = trim(args);
        strcat(argz, (char *)args);

        char *token = strtok(argz, " ");
        int i = 0;

        while (token != NULL)
        {
            char *str = (char *)malloc(sizeof(char) * strlen(token));
            strcpy(str, token);
            argv[i] = str;
            token = strtok(NULL, " ");
            ++i;
        }
        execvp(argv[0], argv);
    }
    ++pingsidx;
}

int main(int argc, const char *argv[])
{
    char filename[1000];
    memset(filename, '\0', 1000);
    pthreadsidx = 0;
    a = 0;
    b = 0;
    signal(SIGINT, ctrlc_handler);
    strcpy(filename, argv[1]);
    int y = atoi(argv[2]);
    pings = (int *)malloc(y * sizeof(int));
    pthreads = (pthread_t *)malloc(y * sizeof(pthread_t));
    for (int j = 0; j < y; ++j)
    {
        pings[j] = -1;
    }
    pingsidx = 0;
    while (true)
    {
        const char t[3] = ";";
        char buf[10000];
        listfd = fopen(filename, "r+");
        int cnt = 0;
        b = 0;
        for (int c = getc(listfd); c != EOF; c = getc(listfd))
        {
            if (c == '\n')
            {
                cnt = cnt + 1;
            }
        }
        if (cnt < a)
        {
            fclose(listfd);
            sleep(1);
            continue;
        }
        if(a > y) {
            printf("Max processes met!\n");
            sleep(60);
            continue;
        }
        rewind(listfd);
        while (fgets(buf, 10000, listfd) != NULL)
        {
            if (a > b)
            {
                ++b;
                continue;
            }
            if (a >= cnt)
            {
                break;
            }
            int t;
            if ((t = pthread_create(&pthreads[pthreadsidx], NULL, exec_target, (char *)buf)))
            {
                printf("Thread creation failed: %d\n", t);
            }
            else
            {
                printf("%s\n", buf);
            }
            pthread_join(pthreads[pthreadsidx], NULL);
            ++pthreadsidx;
            ++b;
            ++a;
            sleep(1);
        }
        fclose(listfd);
        sleep(1);
    }
    return 0;
}