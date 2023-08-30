#include <stdio.h>
#include <pthread.h>

pthread_t thread_1st_half, thread_2nd_half;
volatile bool is_prime = true;
int cancel_thread = 0;

struct args
{
    long int *number;
    int *begin;
    int *end;
};

void *verify_prime(void *args)
{
    long int *n = ((struct args *)args)->number;
    int *b = ((struct args *)args)->begin;
    int *e = ((struct args *)args)->end;

    for (long int i = *b; i <= *e; i++)
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
    long int x;
    scanf("%ld", &x);

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