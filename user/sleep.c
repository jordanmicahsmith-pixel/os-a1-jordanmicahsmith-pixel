#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int is_all_digits(char *s)
{
    if (*s == 0) return 0;
    for (int i = 0; s[i] != 0; i++)
    {
        if (s[i] < '0' || s[i] > '9') return 0;
    }
    return 1;
}

int main(int argc, char *argv[])
{
    if (argc != 2 || atoi(argv[1]) < 0 || !is_all_digits(argv[1]))
    {
        fprintf(2, "usage: sleep <ticks>\n");
        exit(1);
    }
    pause(atoi(argv[1]));
    exit(0);
}