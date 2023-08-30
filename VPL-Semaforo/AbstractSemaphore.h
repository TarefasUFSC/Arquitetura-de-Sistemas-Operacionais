#ifndef ABSTRACTSEMAPHORE
#define ABSTRACTSEMAPHORE

#include <chrono>
#include <thread>
#include <vector>
#include <atomic>
#include <mutex>
#include <condition_variable>

class AbstractSemaphore
{

protected:
    int m_nfree;

public:

    /**
    * @brief Construtor
    * @param cap capacidade do semaforo. Por padrao, capacidade eh um (neste caso, funcionara similar a um mutex)
    */
    AbstractSemaphore(int cap = 1) {
        m_nfree = cap;
    }

    /**
    * @brief Obtem capacidade restante do semaforo (isto e, capacidade total menos threads que estao usando o semaforo)
    * @return Numero natural representando a capacidade restante.
    */
    int getFree() const {
        return m_nfree;
    }

    /**
    * @brief Libera semaforo
    */
    virtual void release() = 0;

    /**
    * @brief Obtem semaforo
    */
    virtual void acquire() = 0;
};

#endif