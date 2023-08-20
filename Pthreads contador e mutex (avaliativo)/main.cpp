/*
Escreva um programa em C onde a thread principal cria outras 128 threads. Deve existir um contador global iniciado em 0. Cada uma das 128 threads deve incrementar o contador 1000 vezes. A thread principal deve aguardar as demais threads terminarem e então imprimir o valor final do contador.

Claramente, existe uma condição de corrida. Elimine-a utilizando um mutex.

Você deve utilizar a biblioteca pthreads.
*/
#include <pthread.h>
#include <stdio.h>

#define NUM_THREADS 128

int contador = 0;

void *incrementa(void *mutex)
{
    pthread_mutex_lock((pthread_mutex_t *)mutex);
    for (int i = 0; i < 1000; i++)
    {
        contador++;
    }
    pthread_mutex_unlock((pthread_mutex_t *)mutex);
    return NULL;
}

int main()
{

    pthread_t threads[NUM_THREADS];
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_create(&threads[i], NULL, incrementa, (void *)&mutex);
    }

    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    printf("%d\n", contador);

    return 0;
}