/**
 * Implemente aqui as funções dos sistema de arquivos que simula EXT3
 */

#include "fs.h"
#include "fsclass.h"

#include <fstream>
#include <iostream>
#include <math.h>

using namespace std;

/**
 * @brief Inicializa um sistema de arquivos que simula EXT3
 * @param fsFileName nome do arquivo que contém sistema de arquivos que simula EXT3 (caminho do arquivo no sistema de arquivos local)
 * @param blockSize tamanho em bytes do bloco
 * @param numBlocks quantidade de blocos
 * @param numInodes quantidade de inodes
 */
void initFs(std::string fsFileName, int blockSize, int numBlocks, int numInodes)
{
    FileSystem fs(fsFileName);
    fs.createFileSystem(blockSize, numBlocks, numInodes);
}
/**
 * @brief Adiciona um novo arquivo dentro do sistema de arquivos que simula EXT3. O sistema já deve ter sido inicializado.
 * @param fsFileName arquivo que contém um sistema sistema de arquivos que simula EXT3.
 * @param filePath caminho completo novo arquivo dentro sistema de arquivos que simula EXT3.
 * @param fileContent conteúdo do novo arquivo
 */
void addFile(std::string fsFileName, std::string filePath, std::string fileContent)
{
    FileSystem fs(fsFileName);
    fs.loadSystemVariables();
    fs.addFile(filePath, fileContent);

    // // abre o arquivo
    // fstream file;
    // handleFileOpening(file, fsFileName);

    // int blockSize{}, numBlocks{}, numInodes{}, bitmapSize{}, indexVectorSize{}, blockVectorSize{};
    // handleLoadFileSystemVariables(file, blockSize, numBlocks, numInodes, bitmapSize, indexVectorSize, blockVectorSize);

    // char *bitmap = new char[bitmapSize]{};
    // char *indexVector = new char[indexVectorSize]{};
    // char *dirRaiz = new char[blockSize]{};

    // handleLoadFileSystemSections(file, *bitmap, bitmapSize, *indexVector, indexVectorSize, *dirRaiz, blockSize);

    // INODE inode{};
    // inode.IS_DIR = 0;
    // inode.IS_USED = 1;
    // // pega o nome do arquivo
    // // pega tudo depois do ultimo /

    // string father = getFatherName(filePath);
    // cout << "father: " << father << endl;
    // string fileName = getFileName(filePath);
    // cout << "fileName: " << fileName << endl;

    // for (int i = 0; i < fileName.size(); i++)
    // {
    //     inode.NAME[i] = fileName[i];
    // }
    // inode.SIZE = fileContent.size();

    // // aloca os dados do arquivo no bloco

    // int tamanho = fileContent.size();

    // int qtdBlocosNecessarios = ceil((float)tamanho / (float)blockSize);
    // cout << "qtdBlocosNecessarios: " << qtdBlocosNecessarios << endl;
    // int dataBlocksAddresses[qtdBlocosNecessarios];

    // for (int i = 0; i < qtdBlocosNecessarios; i++)
    // {
    //     int index_livre = findFirstFreeBlockInTheBitmap(bitmap, bitmapSize);
    //     bitmap[(int)(index_livre / 8.0)] |= (1 << (index_livre % 8));
    //     dataBlocksAddresses[i] = index_livre;
    // }

    // populateInodeBlockAddresses(inode, dataBlocksAddresses, qtdBlocosNecessarios, blockSize, indexVectorSize, bitmapSize);

    // changeBitmap(file, bitmap, bitmapSize);

    // // procura um bloco inode livre
    // int index_livre_inode = findFirstFreeInodeBlockAddress(file, indexVector, bitmapSize);
    // changeInodeAtIndex(file, inode, index_livre_inode, bitmapSize);

    // // escreve os dados do arquivo nos blocos
    // char newDataBlocks[qtdBlocosNecessarios * blockSize]{};
    // // converte o fileContent para o vetor de char justificado à direita
    // for (int i = fileContent.size() - 1; i >= 0; i--)
    // {
    //     int offset = (qtdBlocosNecessarios * blockSize) - fileContent.size();
    //     newDataBlocks[i + (offset)] = fileContent[i];
    // }

    // // escreve os dados do arquivo nos blocos
    // writeInAddress(file, 3 + bitmapSize + indexVectorSize + blockSize * dataBlocksAddresses[0], blockSize * qtdBlocosNecessarios, (char *)&newDataBlocks);

    // // tem que achar o inode do pai e incrementar o tamanho dele
    // int fatherPosition = 0;
    // INODE fatherInode = getInodeByNAME(file, father, bitmapSize, indexVectorSize, fatherPosition);
    // // verifica se achou
    // if (fatherInode.IS_USED == 1)
    // {
    //     fatherInode.SIZE++;

    //     // procura o primeiro bloco livre do pai

    //     changeInodeAtIndex(file, fatherInode, fatherPosition, bitmapSize);
    // }
    // else
    // {
    //     cout << "Não achou o pai" << endl;
    // }
    // file.close();
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