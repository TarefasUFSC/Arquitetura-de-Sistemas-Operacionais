/**
 * Implemente aqui as funções dos sistema de arquivos que simula EXT3
 */

#include "fs.h"
#include <fstream>
#include <iostream>
#include <math.h>

using namespace std;

void handleFileOpening(fstream &file, const std::string &fsFileName)
{
    // Tenta abrir o arquivo sem truncar
    file.open(fsFileName, ios::out | ios::in | ios::binary);

    // Se não conseguiu abrir, tenta criar o arquivo
    if (!file.is_open())
    {
        file.clear(); // Limpa os flags de erro
        file.open(fsFileName, ios::out | ios::in | ios::binary | ios::trunc);
    }

    // Verifica se ainda não conseguimos abrir o arquivo
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

    INODE new_inode{};
    new_inode.IS_DIR = 1;
    new_inode.IS_USED = 1;
    new_inode.NAME[0] = '/';
    new_inode.SIZE = 0;

    bitmap[0] = 1;
    changeBitmap(file, bitmap, bitmapSize);

    changeInodeAtIndex(file, new_inode, 0, bitmapSize, indexVectorSize);

    file.close();
}

void handleLoadFileSystemVariables(fstream &file, int &blockSize, int &numBlocks, int &numInodes, int &bitmapSize, int &indexVectorSize, int &blockVectorSize)
{
    file.seekg(0);
    file.read((char *)&blockSize, sizeof(char));
    file.read((char *)&numBlocks, sizeof(char));
    file.read((char *)&numInodes, sizeof(char));

    bitmapSize = ceil((float)numBlocks / 8.0);
    indexVectorSize = sizeof(INODE) * numInodes;
    blockVectorSize = blockSize * numBlocks;

    cout << "blockSize: " << blockSize << endl;
    cout << "numBlocks: " << numBlocks << endl;
    cout << "numInodes: " << numInodes << endl;

    cout << "bitmapSize: " << bitmapSize << endl;
    cout << "indexVectorSize: " << indexVectorSize << endl;
    cout << "blockVectorSize: " << blockVectorSize << endl;
}
void handleLoadFileSystemSections(fstream &file, char *bitmap, int bitmapSize, char *indexVector, int indexVectorSize, char *dirRaiz, char *blockVector, int blockVectorSize)
{
    file.seekg(3);
    file.read(bitmap, bitmapSize);
    file.read(indexVector, indexVectorSize);
    file.read(dirRaiz, 1);
    file.read(blockVector, blockVectorSize);
}
int findFirstFreeBlockInTheBitmap(char *bitmap, int bitmapSize)
{
    int index_livre = 0;
    cout << "bitmapsize: " << bitmapSize << endl;
    // descobre a posição do primeiro bit 0 do bitmap
    for (int i = 0; i < bitmapSize; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if ((bitmap[i] & (1 << j)) == 0)
            {
                index_livre = i * 8 + j;
                break;
            }
        }
    }
    return index_livre;
}

/**
 * @brief Adiciona um novo arquivo dentro do sistema de arquivos que simula EXT3. O sistema já deve ter sido inicializado.
 * @param fsFileName arquivo que contém um sistema sistema de arquivos que simula EXT3.
 * @param filePath caminho completo novo arquivo dentro sistema de arquivos que simula EXT3.
 * @param fileContent conteúdo do novo arquivo
 */
void addFile(std::string fsFileName, std::string filePath, std::string fileContent)
{

    // abre o arquivo
    fstream file;
    handleFileOpening(file, fsFileName);

    int blockSize, numBlocks, numInodes, bitmapSize, indexVectorSize, blockVectorSize;
    handleLoadFileSystemVariables(file, blockSize, numBlocks, numInodes, bitmapSize, indexVectorSize, blockVectorSize);

    char *bitmap = new char[bitmapSize]{};
    char *indexVector = new char[indexVectorSize]{};
    char *blockVector = new char[blockVectorSize]{};

    INODE inode{};
    inode.IS_DIR = 0;
    inode.IS_USED = 1;
    for (int i = 0; i < filePath.size(); i++)
    {
        inode.NAME[i] = filePath[i];
    }
    inode.SIZE = fileContent.size();

    // procura no bitmap um bloco livre
    // se não encontrar, retorna erro
    // se encontrar, marca como ocupado

    int index_livre = findFirstFreeBlockInTheBitmap(bitmap, bitmapSize);
    cout << "index_livre: " << index_livre << endl;
}

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