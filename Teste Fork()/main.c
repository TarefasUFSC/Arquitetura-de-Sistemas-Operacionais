#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
    int r;

    r = fork();

    if (r > 0)
    {
        printf("Processo pai\n");
    }
    else if (r == 0)
    {
        printf("Processo filho\n");
    }
    else
    {
        printf("Erro\n");
    }
    return 0;
}