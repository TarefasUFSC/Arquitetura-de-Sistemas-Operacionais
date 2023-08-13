#ifndef fs_h
#define fs_h
#include <string>
#include <fstream>
#include <iostream>
// strcpy
#include <string.h>

using namespace std;

typedef struct
{
    uint inicio;
} Header;

typedef struct
{
    bool state;
    char nome[20];
    uint proximo;
} Registro; // 28 bytes

/**
 * @param arquivoDaLista nome do arquivo em disco que contem a lista encadeada
 * @param novoNome nome a ser adicionado apos depoisDesteNome
 * @param depoisDesteNome um nome presente na lista
 */

void readHeader(fstream *file, Header *header)
{

    char lsb, b2, b3, msb;
    file->read((char *)&lsb, 1);
    file->read((char *)&b2, 1);
    file->read((char *)&b3, 1);
    file->read((char *)&msb, 1);

    header->inicio = (int)lsb | (int)b2 << 8 | (int)b3 << 16 | (int)msb << 24;
}

void readRegister(fstream *file, Registro *registro)
{
    char state[4];
    file->read((char *)&state, 4);
    registro->state = (state[0] == 1) ? true : false;

    file->read((char *)&registro->nome, 20);

    char proximo[4];
    file->read((char *)&proximo, 4);
    registro->proximo = (int)proximo[0] | (int)proximo[1] << 8 | (int)proximo[2] << 16 | (int)proximo[3] << 24;
}

void writeRegister(fstream *file, Registro *registro)
{
    char state[4];
    state[0] = (registro->state) ? 1 : 0;
    file->write((char *)&state, 4);

    file->write((char *)&registro->nome, 20);

    char proximo[4];
    proximo[0] = (registro->proximo & 0x000000ff);
    proximo[1] = (registro->proximo & 0x0000ff00) >> 8;
    proximo[2] = (registro->proximo & 0x00ff0000) >> 16;
    proximo[3] = (registro->proximo & 0xff000000) >> 24;
    file->write((char *)&proximo, 4);
}

void printRegister(Registro *registro)
{
    cout << endl
         << "state: " << (registro->state ? "ocupado" : "livre") << endl;
    cout << "nome: " << registro->nome << endl;
    cout << "proximo: " << registro->proximo << endl
         << endl;
}

void adiciona(std::string arquivoDaLista, std::string novoNome, std::string depoisDesteNome)
{
    fstream file;
    file.open(arquivoDaLista, ios::in | ios::out | ios::binary);

    if (!file.is_open())
    {
        cout << "Erro ao abrir o arquivo" << endl;
        exit(EXIT_FAILURE);
    }

    // le o header
    Header header;
    readHeader(&file, &header);
    cout << "header.inicio: " << header.inicio << endl;

    // le o arquivo para achar uma posição livre
    uint newAddr = sizeof(Header);
    file.seekg(newAddr);
    char state[4];
    while (file.read((char *)&state, 4))
    {
        if (state[0] == 0)
            break;
        else
        {
            newAddr += 28;
            file.seekg(newAddr);
        }
    }
    cout << "newAddr: " << newAddr << endl;

    // escreve o novo registro
    Registro registroNovo;
    registroNovo.state = true;
    strcpy(registroNovo.nome, novoNome.c_str());
    registroNovo.proximo = 0;

    file.seekg(newAddr);
    writeRegister(&file, &registroNovo);

    // procura o nome passado na função
    file.seekg(header.inicio);

    bool achouNome = false;
    Registro registroEncontrado;
    uint addrRegistroEncontrado = header.inicio;
    while (!achouNome)
    {
        readRegister(&file, &registroEncontrado);
        if (registroEncontrado.nome == depoisDesteNome)
        {
            achouNome = true;
        }
        else
        {
            // cout << "registro.nome: " << registro.nome << endl;
            file.seekg(registroEncontrado.proximo);
            addrRegistroEncontrado = registroEncontrado.proximo;
        }
    }

    // substitui a informação de proximo do registro encontrado
    printRegister(&registroEncontrado);

    uint addr = registroEncontrado.proximo;
    registroEncontrado.proximo = newAddr;

    file.seekg(addrRegistroEncontrado);
    writeRegister(&file, &registroEncontrado);

    file.seekg(addrRegistroEncontrado);
    readRegister(&file, &registroEncontrado);
    printRegister(&registroEncontrado);

    // atualiza o proximo do novo registro para não quebrar a lista
    file.seekg(newAddr);
    readRegister(&file, &registroNovo);
    printRegister(&registroNovo);

    registroNovo.proximo = addr;
    file.seekg(newAddr);
    writeRegister(&file, &registroNovo);

    file.seekg(newAddr);
    readRegister(&file, &registroNovo);
    printRegister(&registroNovo);

    file.close();
}

#endif