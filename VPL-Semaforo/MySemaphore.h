#ifndef MYSEMAPHORE
#define MYSEMAPHORE

#include "AbstractSemaphore.h"

class MySemaphore : public AbstractSemaphore
{
public:
    MySemaphore(int initValue) // chama o construtor do pai
        : AbstractSemaphore(initValue)
    {
    }
    ~MySemaphore() {}

    /**
     * @brief Libera semaforo
     */
    virtual void release() // esse é o signal() do slide 46 e o post() do slide 47
    {
        m_nfree++;
    }

    /**
     * @brief Obtem semaforo
     */
    virtual void acquire() // esse é o wait() do slide 46 e o wait() do slide 47
    {
        while (m_nfree == 0)
        {
            // O comando std::this_thread::yield() sugere ao planejador do sistema que a thread atual renuncie à execução.
            // É usado para otimizar o uso de recursos, permitindo que outras threads sejam priorizadas.
            std::this_thread::yield();
        }
        m_nfree--;
    }
};

#endif