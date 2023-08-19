/*
Escreva um programa que leia um número n>1 do teclado (veja a função scanf) e escreva 1 se o n for primo e 0 caso contrário. Para a verificação, utilize duas threads onde cada thread verifica metade dos possíveis fatores de n. A thread que encontrar um divisor (neste caso, o número não é primo) deve cancelar a outra thread e terminar Para cancelar a outra thread, utilize a função pthread_cancel (ver link).

O código C para leitura do teclado e escrita do resultado na tela já está disponível e você não deve modificá-lo. Lembre de avaliar seu código clicando no botão "avaliar" (botão com checkbox seguido de um zero).*/

#include <stdio.h>
#include <pthread.h>

pthread_t thread_1st_half, thread_2nd_half;
volatile bool is_prime = true;
int cancel_thread = 0;

struct args
{
    int *number;
    int *begin;
    int *end;
};

void *verify_prime(void *args)
{
    int *n = ((struct args *)args)->number;
    int *b = ((struct args *)args)->begin;
    int *e = ((struct args *)args)->end;

    for (int i = *b; i <= *e; i++)
    {
        if (*n % i == 0)
        {
            is_prime = false;
            pthread_cancel(thread_1st_half);
            pthread_cancel(thread_2nd_half);
            break;
        }
    }
    return NULL;
}

int main()
{
    int x;
    scanf("%d", &x);

    // programe aqui suas threads
    int begin_1s_half = 2;
    int end_1s_half = x / 2;

    int begin_2nd_half = x / 2 + 1;
    int end_2nd_half = x - 1;

    args args_1st_half = {&x, &begin_1s_half, &end_1s_half};
    args args_2nd_half = {&x, &begin_2nd_half, &end_2nd_half};

    pthread_create(&thread_1st_half, NULL, verify_prime, (void *)&args_1st_half);
    pthread_create(&thread_2nd_half, NULL, verify_prime, (void *)&args_2nd_half);

    pthread_join(thread_1st_half, NULL);
    pthread_join(thread_2nd_half, NULL);

    printf("%d\n", is_prime);
    return 0;
}