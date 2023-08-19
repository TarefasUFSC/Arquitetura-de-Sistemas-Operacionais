/**
 * Implemente aqui as funções dos sistema de arquivos que simula EXT3
 */

#include "fs.h"
#include <fstream>
#include <iostream>
#include <math.h>

using namespace std;

void handleFileOpening(fstream &file, std::string fsFileName)
{
    // abre o arquivo, ou cria se não existir
    file.open(fsFileName, ios::out | ios::in | ios::binary | ios::trunc);

    // verifica se não conseguimos abrir o arquivo
    if (!file.is_open())
    {
        cout << "Não foi possível abrir o arquivo" << endl;
        exit(1);
    }
}

void writeSizesOnHeader(fstream &file, int blockSize, int numBlocks, int numInodes)
{
    // escreve as informações de cabeçalho que foram passadas
    file.seekg(0);
    file.write((char *)&blockSize, sizeof(char));
    file.write((char *)&numBlocks, sizeof(char));
    file.write((char *)&numInodes, sizeof(char));
}

void initializeStorage(fstream &file, char *bitmap, int bitmapSize, char *indexVector, int indexVectorSize, char *dirRaiz, char *blockVector, int blockVectorSize)
{

    file.seekp(3);
    // escreve o bitmap
    file.write(bitmap, bitmapSize);
    // escreve o vetor de indices
    file.write(indexVector, indexVectorSize);
    // deixa livre o tamanho do Dir Raiz
    file.write(dirRaiz, 1);
    // // escreve o vetor de blocos
    file.write(blockVector, blockVectorSize);
}

void changeBitmap(fstream &file, char *bitmap, int bitmapSize)
{
    file.seekp(3);
    file.write(bitmap, bitmapSize);
}

void changeInodeAtIndex(fstream &file, INODE inode, int index, int bitmapSize, int indexVectorSize)
{
    file.seekp(3 + bitmapSize + index * sizeof(INODE));
    file.write((char *)&inode, sizeof(INODE));
}

/**
 * @brief Inicializa um sistema de arquivos que simula EXT3
 * @param fsFileName nome do arquivo que contém sistema de arquivos que simula EXT3 (caminho do arquivo no sistema de arquivos local)
 * @param blockSize tamanho em bytes do bloco
 * @param numBlocks quantidade de blocos
 * @param numInodes quantidade de inodes
 */
void initFs(std::string fsFileName, int blockSize, int numBlocks, int numInodes)
{
    fstream file;
    handleFileOpening(file, fsFileName);

    writeSizesOnHeader(file, blockSize, numBlocks, numInodes);

    // calcula o tamanho do mapa de bits
    int bitmapSize = ceil((float)numBlocks / 8.0);
    // calcula o tamanho do vetor de indices
    int indexVectorSize = sizeof(INODE) * numInodes;
    // calcula o tamanho do Vetor de Blocos
    int blockVectorSize = blockSize * numBlocks;
    // cout << "bitmapSize: " << bitmapSize << endl;
    // cout << "indexVectorSize: " << indexVectorSize << endl;
    // cout << "blockVectorSize: " << blockVectorSize << endl;
    char *bitmap = new char[bitmapSize]{};
    char *indexVector = new char[indexVectorSize]{};
    char *blockVector = new char[blockVectorSize]{};
    char *dirRaiz = new char[1]{};

    initializeStorage(file, bitmap, bitmapSize, indexVector, indexVectorSize, dirRaiz, blockVector, blockVectorSize);

    INODE inode{};
    inode.IS_DIR = 1;
    inode.IS_USED = 1;
    inode.NAME[0] = '/';

    bitmap[0] = 1;
    changeBitmap(file, bitmap, bitmapSize);

    changeInodeAtIndex(file, inode, 0, bitmapSize, indexVectorSize);

    file.close();
}

/**
 * @brief Adiciona um novo arquivo dentro do sistema de arquivos que simula EXT3. O sistema já deve ter sido inicializado.
 * @param fsFileName arquivo que contém um sistema sistema de arquivos que simula EXT3.
 * @param filePath caminho completo novo arquivo dentro sistema de arquivos que simula EXT3.
 * @param fileContent conteúdo do novo arquivo
 */
void addFile(std::string fsFileName, std::string filePath, std::string fileContent) {}

/**
 * @brief Adiciona um novo diretório dentro do sistema de arquivos que simula EXT3. O sistema já deve ter sido inicializado.
 * @param fsFileName arquivo que contém um sistema sistema de arquivos que simula EXT3.
 * @param dirPath caminho completo novo diretório dentro sistema de arquivos que simula EXT3.
 */
void addDir(std::string fsFileName, std::string dirPath) {}

/**
 * @brief Remove um arquivo ou diretório (recursivamente) de um sistema de arquivos que simula EXT3. O sistema já deve ter sido inicializado.
 * @param fsFileName arquivo que contém um sistema sistema de arquivos que simula EXT3.
 * @param path caminho completo do arquivo ou diretório a ser removido.
 */
void remove(std::string fsFileName, std::string path) {}

/**
 * @brief Move um arquivo ou diretório em um sistema de arquivos que simula EXT3. O sistema já deve ter sido inicializado.
 * @param fsFileName arquivo que contém um sistema sistema de arquivos que simula EXT3.
 * @param oldPath caminho completo do arquivo ou diretório a ser movido.
 * @param newPath novo caminho completo do arquivo ou diretório.
 */
void move(std::string fsFileName, std::string oldPath, std::string newPath) {}