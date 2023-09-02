#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <random>
#include <mutex>

#define NUM_FILOSOFOS 5

char estado_filosofos[NUM_FILOSOFOS] = {'P', 'P', 'P', 'P', 'P'};

using namespace std;

struct garfosDisponiveis
{
    mutex *garfo_esquerda;
    mutex *garfo_direita;
};

struct argsFilosofo
{
    int id;
    garfosDisponiveis garfos_disponiveis;
};

void imprime_estado()
{
    while (1)
    {
        for (int i = 0; i < NUM_FILOSOFOS; i++)
        {
            cout << estado_filosofos[i];
        }
        cout << endl;
        this_thread::sleep_for(chrono::seconds(1));
    }
}

void filosofoBehavior(argsFilosofo args)
{
    int id = args.id;
    garfosDisponiveis garfos_disponiveis = args.garfos_disponiveis;

    while (true)
    {
        if (estado_filosofos[id] == 'P')
        {
            // pensa por um tempo aleatorio e depois fica com fome
            int tempo_aleatorio = rand() % 5 + 1;
            this_thread::sleep_for(chrono::seconds(tempo_aleatorio));
            estado_filosofos[id] = 'F';
        }
        else if (estado_filosofos[id] == 'F')
        {
            // verfica se o garfo da esquerda esta disponivel
            if (garfos_disponiveis.garfo_esquerda->try_lock())
            {
                // verifica se o garfo da direita esta disponivel
                if (garfos_disponiveis.garfo_direita->try_lock())
                {

                    // se ambos os garfos estiverem disponiveis, come por um tempo aleatorio
                    estado_filosofos[id] = 'C';
                    int tempo_aleatorio = rand() % 5 + 1;
                    this_thread::sleep_for(chrono::seconds(tempo_aleatorio));

                    // termina de comer e volta a pensar, liberando os garfos
                    estado_filosofos[id] = 'P';
                    garfos_disponiveis.garfo_esquerda->unlock();
                    garfos_disponiveis.garfo_direita->unlock();
                }
                else
                {
                    // se o garfo da direita nao estiver disponivel, libera o garfo da esquerda e segue com fome tentando comer novamente
                    garfos_disponiveis.garfo_esquerda->unlock();
                    continue;
                }
            }
            else
            {
                // se o garfo da esquerda nao estiver disponivel, segue com fome tentando comer novamente
                continue;
            }
        }
    }
}

int main()
{
    srand(time(NULL)); // Inicializa a semente para rand()

    vector<thread> filosofos(NUM_FILOSOFOS);
    vector<mutex> garfos(NUM_FILOSOFOS);

    for (int i = 0; i < NUM_FILOSOFOS; i++)
    {
        argsFilosofo args;
        args.id = i;
        garfosDisponiveis garfos_d;
        garfos_d.garfo_esquerda = &garfos[i];
        garfos_d.garfo_direita = &garfos[(i + 1) % NUM_FILOSOFOS];
        args.garfos_disponiveis = garfos_d;
        cout << "Criando filosofo " << i << " com garfos " << i << " e " << (i + 1) % NUM_FILOSOFOS << endl;
        filosofos[i] = thread(filosofoBehavior, args);
    }

    thread logger(imprime_estado);

    for (int i = 0; i < NUM_FILOSOFOS; i++)
    {
        filosofos[i].join();
    }

    logger.join();

    return 0;
}
