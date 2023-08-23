#include "fs.h"
#include "constants.hpp"
#include "file-system.hpp"

/**
 * Implemente aqui as funções dos sistema de arquivos que simula EXT3
 */

/**
 * @brief Inicializa um sistema de arquivos que simula EXT3
 * @param fsFileName nome do arquivo que contém sistema de arquivos que simula EXT3 (caminho do arquivo no sistema de arquivos local)
 * @param blockSize tamanho em bytes do bloco
 * @param numBlocks quantidade de blocos
 * @param numInodes quantidade de inodes
 */
void initFs(std::string fsFileName, int blockSize, int numBlocks, int numInodes)
{
  std::fstream fs(fsFileName, std::fstream::out | std::ios::binary);
  fs::FileSystem fileSystem(&fs, (fs::byte)blockSize,
                            (fs::byte)numBlocks,
                            (fs::byte)numInodes);

  fs.close();
}

/**
 * @brief Adiciona um novo arquivo dentro do sistema de arquivos que simula EXT3. O sistema já deve ter sido inicializado.
 * @param fsFileName arquivo que contém um sistema sistema de arquivos que simula EXT3.
 * @param filePath caminho completo novo arquivo dentro sistema de arquivos que simula EXT3.
 * @param fileContent conteúdo do novo arquivo
 */
void addFile(std::string fsFileName, std::string filePath, std::string fileContent)
{
  std::fstream fs(fsFileName, std::fstream::in | std::fstream::out | std::ios::binary);
  fs::FileSystem fileSystem(&fs);
  fileSystem.addFile(filePath, fileContent);
  fs.close();
}

/**
 * @brief Adiciona um novo diretório dentro do sistema de arquivos que simula EXT3. O sistema já deve ter sido inicializado.
 * @param fsFileName arquivo que contém um sistema sistema de arquivos que simula EXT3.
 * @param dirPath caminho completo novo diretório dentro sistema de arquivos que simula EXT3.
 */
void addDir(std::string fsFileName, std::string dirPath)
{
  std::fstream fs(fsFileName, std::fstream::in | std::fstream::out | std::ios::binary);
  fs::FileSystem fileSystem(&fs);
  fileSystem.addDir(dirPath);
  fs.close();
}

/**
 * @brief Remove um arquivo ou diretório (recursivamente) de um sistema de arquivos que simula EXT3. O sistema já deve ter sido inicializado.
 * @param fsFileName arquivo que contém um sistema sistema de arquivos que simula EXT3.
 * @param path caminho completo do arquivo ou diretório a ser removido.
 */
void remove(std::string fsFileName, std::string path)
{
  std::fstream fs(fsFileName, std::fstream::in | std::fstream::out | std::ios::binary);
  fs::FileSystem fileSystem(&fs);
  fileSystem.remove(path);
  fs.close();
}

/**
 * @brief Move um arquivo ou diretório em um sistema de arquivos que simula EXT3. O sistema já deve ter sido inicializado.
 * @param fsFileName arquivo que contém um sistema sistema de arquivos que simula EXT3.
 * @param oldPath caminho completo do arquivo ou diretório a ser movido.
 * @param newPath novo caminho completo do arquivo ou diretório.
 */
void move(std::string fsFileName, std::string oldPath, std::string newPath)
{
  std::fstream fs(fsFileName, std::fstream::in | std::fstream::out | std::ios::binary);
  fs::FileSystem fileSystem(&fs);
  fileSystem.move(oldPath, newPath);
  fs.close();
}