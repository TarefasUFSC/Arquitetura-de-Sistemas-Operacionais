#include "BancoDeDados.h"

/**
 * @brief Le o valor da variavel inteira armazenada no banco de dados
 * @return Valor da variavel inteira lida
 */
int BancoDeDados::read()
{
    int valorLido;
    this->meutexProtetor.lock();
    valorLido = this->var;
    this->meutexProtetor.unlock();
    return valorLido;
}

/**
 * @brief Incrementa o o valor variavel inteira armazenada no banco de dados
 * @return Novo valor da variavel
 */
int BancoDeDados::increment()
{
    int novoValor;
    this->meutexProtetor.lock();
    this->var++;
    novoValor = this->var;
    this->meutexProtetor.unlock();
    return novoValor;
}