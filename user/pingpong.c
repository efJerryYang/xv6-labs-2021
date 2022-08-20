#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    // no argument passed

    int ping[2];
    int pong[2];
    char byte[1];

    pipe(ping);
    pipe(pong);
    if (fork() == 0)
    {
        close(ping[1]);
        close(pong[0]);

        while (read(ping[0], byte, 1) < 1)
            ;
        printf("%d: received ping\n", getpid());
        write(pong[1], " ", 1);

        close(ping[0]);
        close(pong[1]);
        exit(0);
    }
    else
    {
        close(ping[0]);
        close(pong[1]);

        write(ping[1], " ", 1);
        while (read(pong[0], byte, 1) < 1)
            ;
        printf("%d: received pong\n", getpid());

        close(ping[1]);
        close(pong[0]);
        exit(0);
    }
}
