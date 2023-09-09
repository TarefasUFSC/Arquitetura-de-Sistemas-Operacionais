#ifndef BANCODEDADOS_H
#define BANCODEDADOS_H

#include <mutex>

class BancoDeDados
{
private:
    int var{0};
    int contagemLeitores{0};
    static BancoDeDados *pinstance;
    std::mutex meutexProtetor;
    BancoDeDados() {}
    BancoDeDados(BancoDeDados const &) = delete;
    void operator=(BancoDeDados const &) = delete;

public:
    static BancoDeDados *getInstance()
    {
        if (pinstance == nullptr)
            pinstance = new BancoDeDados();

        return pinstance;
    }

    /**
     * @brief Le o valor da variavel inteira armazenada no banco de dados
     * @return Valor da variavel inteira lida
     */
    int read();

    /**
     * @brief Incrementa o o valor variavel inteira armazenada no banco de dados
     * @return Novo valor da variavel
     */
    int increment();
};

#endif
