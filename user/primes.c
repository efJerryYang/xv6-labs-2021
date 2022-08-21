#include "kernel/types.h"
#include "user/user.h"

int sieve_primes(int *primes, int n) // maybe can be replaced by while loop
{
    if (n == 1)
        return 0;

    int p[2];
    int p2[2];
    pipe(p);
    pipe(p2);
    if (fork() == 0)
    {
        close(p[1]);
        close(p2[0]);

        int i = 0;
        int j = 0;
        int buf = 0;
        while (read(p[0], &buf, 4) == 4)
        {
            primes[i++] = buf;
            if (buf % primes[0] == 0 && buf != primes[0])
                --i;
            j++;
        }
        close(p[0]);

        fprintf(1, "prime %d\n", primes[0]);
        // fprintf(1, "prime %d \tpid=%d\t", primes[0], getpid());
        // fprintf(1, "numbers:");
        // for (int k = 0; k <= i; k++)
        //     fprintf(1, " %d", primes[k]);
        // fprintf(1, "\n");
        sieve_primes(primes + 1, i + 1);

        // write(p2[1], primes, 4 * (i + 1));
        write(p2[1], "e", 1);
        close(p2[0]);
        exit(0);
    }
    else
    {
        close(p[0]);
        close(p2[1]);

        write(p[1], primes, 4 * n);
        close(p[1]);

        // read(p2[0], primes, 4 * 34);
        char ch;
        read(p2[0], &ch, 1);
        close(p2[0]);
        return 0;
    }
}
int main()
{
    // no argument passed
    // int numbers[34];
    int primes[34];
    int p[2];
    int p2[2];

    for (int i = 0; i <= 33; i++)
        primes[i] = i + 2;

    pipe(p);
    pipe(p2);
    if (fork() == 0)
    {
        close(p[1]);
        close(p2[0]);

        int i = 0;
        int j = 0;
        int buf;
        while (read(p[0], &buf, 4) == 4)
        {
            primes[i++] = buf;
            if (buf % primes[0] == 0 && buf != primes[0])
                --i;
            j++;
        }
        fprintf(1, "prime %d\n", primes[0]);
        // fprintf(1, "prime %d \tpid=%d\t", primes[0], getpid());
        // fprintf(1, "numbers:");
        // for (int k = 0; k <= i; k++)
        //     fprintf(1, " %d", primes[k]);
        // fprintf(1, "\n");
        sieve_primes(primes + 1, i + 1);
        // it seems that can only remain 1 function call here
        // write(p2[1], primes, 4 * (i + 1));
        write(p2[1], "e", 1);
        close(p[0]);
        close(p2[1]);
        exit(0);
    }
    else
    {
        close(p[0]);
        close(p2[1]);

        write(p[1], primes, 4 * 34); // write 34 4-byte integers to pipe write end
        close(p[1]);

        // wait untill last process exit()
        // read(p2[0], primes, 4 * 34);
        char ch;
        read(p2[0], &ch, 1);
        close(p2[0]);
        exit(0);
    }
}