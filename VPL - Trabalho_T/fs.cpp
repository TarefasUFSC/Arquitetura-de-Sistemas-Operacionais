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

    file.seekg(3);
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
    file.seekg(3);
    file.write(bitmap, bitmapSize);
}

void changeInodeAtIndex(fstream &file, INODE inode, int index, int bitmapSize, int indexVectorSize)
{
    file.seekg(3 + bitmapSize + index * sizeof(INODE));
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
void handleLoadFileSystemSections(fstream &file, char &bitmap, int bitmapSize, char &indexVector, int indexVectorSize, char &dirRaiz, int blockSize)
{
    file.seekg(3);
    file.read((char *)&bitmap, bitmapSize);
    file.read((char *)&indexVector, indexVectorSize);
    file.read((char *)&dirRaiz, blockSize);
    // file.read((char *)&blockVector, blockVectorSize); // ELE CHEGA NO FIM DO ARQUIVO AI BUGA

    cout << "bitmap: " << (int)bitmap << endl;
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

void populateInodeBlockAddresses(INODE &inode, int *dataBlocksAddresses, int qtd, int blockSize, int indexVectorSize, int bitmapSize)
{
    if (qtd > 3)
    {
        // vai ter que usar indireto
    }
    else
    {
        for (int i = 0; i < qtd; i++)
        {
            inode.DIRECT_BLOCKS[i] = dataBlocksAddresses[i];
        }
    }
}

int findFirstFreeInodeBlockAddress(fstream &file, char *indexVector, int bitmapSize)
{
    // Certifique-se de que a posição de leitura está correta
    file.seekg(3 + bitmapSize);

    int index_livre = 0;
    INODE inode{};

    while (1)
    {
        file.read((char *)&inode, sizeof(INODE));

        if (inode.IS_USED == 0)
        {
            cout << "IS_USED: " << (int)inode.IS_USED << endl;
            cout << "IS_DIR: " << (int)inode.IS_DIR << endl;
            cout << "NAME: " << inode.NAME << endl;
            break;
        }
        index_livre++;
    }
    cout << "index_livre_inode: " << index_livre << endl;
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

    int blockSize{}, numBlocks{}, numInodes{}, bitmapSize{}, indexVectorSize{}, blockVectorSize{};
    handleLoadFileSystemVariables(file, blockSize, numBlocks, numInodes, bitmapSize, indexVectorSize, blockVectorSize);

    char *bitmap = new char[bitmapSize]{};
    char *indexVector = new char[indexVectorSize]{};
    char *dirRaiz = new char[blockSize]{};

    handleLoadFileSystemSections(file, *bitmap, bitmapSize, *indexVector, indexVectorSize, *dirRaiz, blockSize);

    INODE inode{};
    inode.IS_DIR = 0;
    inode.IS_USED = 1;
    // pega o nome do arquivo
    // pega tudo depois do ultimo /

    string father = filePath.substr(0, filePath.find_last_of("/")) == "" ? "/" : filePath.substr(0, filePath.find_last_of("/"));
    cout << "father: " << father << endl;
    filePath = filePath.substr(filePath.find_last_of("/") + 1);
    for (int i = 0; i < filePath.size(); i++)
    {
        inode.NAME[i] = filePath[i];
    }
    inode.SIZE = fileContent.size();

    // aloca os dados do arquivo no bloco

    int tamanho = fileContent.size();
    char *dataBlocks = new char[tamanho]{};
    for (int i = 0; i < tamanho; i++)
    {
        dataBlocks[i] = fileContent[i];
    }
    int qtdBlocosNecessarios = ceil((float)tamanho / (float)blockSize);
    cout << "qtdBlocosNecessarios: " << qtdBlocosNecessarios << endl;
    int dataBlocksAddresses[qtdBlocosNecessarios];

    for (int i = 0; i < qtdBlocosNecessarios; i++)
    {
        int index_livre = findFirstFreeBlockInTheBitmap(bitmap, bitmapSize);
        bitmap[(int)(index_livre / 8.0)] |= (1 << (index_livre % 8));
        dataBlocksAddresses[i] = index_livre;
    }

    populateInodeBlockAddresses(inode, dataBlocksAddresses, qtdBlocosNecessarios, blockSize, indexVectorSize, bitmapSize);

    changeBitmap(file, bitmap, bitmapSize);

    // procura um bloco inode livre
    int index_livre_inode = findFirstFreeInodeBlockAddress(file, indexVector, bitmapSize);
    changeInodeAtIndex(file, inode, index_livre_inode, bitmapSize, indexVectorSize);

    // escreve os dados do arquivo nos blocos
    int dados[qtdBlocosNecessarios]{};
    for (int i = 0; i < tamanho; i++)
    {
        int position = (int)floor((float)i / (float)blockSize);
        cout << "i: " << i << endl;
        cout << "position: " << position << endl;
        cout << "dataBlocks[i]: " << dataBlocks[i] << endl;
        dados[position] = (dataBlocks[i] << (8 * (i % blockSize))) | dados[position];
    }

    // escreve os dados do arquivo nos blocos
    for (int i = 0; i < qtdBlocosNecessarios; i++)
    {
        cout << "dataBlocksAddresses[i]: " << dataBlocksAddresses[i] << endl;
        cout << "blockSize: " << blockSize << endl;
        cout << "dados[i]: " << dados[i] << endl;
        file.seekg(3 + bitmapSize + indexVectorSize + blockSize * dataBlocksAddresses[i]);
        file.write((char *)&dados[i], blockSize);
    }

    // escreve o endereço do inode do arquivo novo no diretório raiz
    file.seekg(3 + bitmapSize + indexVectorSize);
    int address = 3 + bitmapSize + sizeof(INODE) * index_livre_inode;
    cout << "address: " << address << endl;
    file.write((char *)&index_livre_inode, sizeof(char));

    file.close();
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