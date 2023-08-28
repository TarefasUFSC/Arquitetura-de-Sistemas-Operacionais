#include <fstream>
#include <iostream>
#include <math.h>

using namespace std;

class FileSystem
{
    // atributos
private:
    fstream fsFile;

public:
    string fsFileName;
    int blockSize;
    int qtdBlocks;
    int qtdInodes;
    int bitmapSize;
    int indexVectorSize;
    int blockVectorSize;

    // metodos
private:
    void loadFileSystem()
    {
        // Tenta abrir o arquivo sem truncar
        this->fsFile.open(fsFileName, ios::out | ios::in | ios::binary);

        // Se não conseguiu abrir, tenta criar o arquivo
        if (!this->fsFile.is_open())
        {
            this->fsFile.clear(); // Limpa os flags de erro
            this->fsFile.open(fsFileName, ios::out | ios::in | ios::binary | ios::trunc);
        }

        // Verifica se ainda não conseguimos abrir o arquivo
        if (!this->fsFile.is_open())
        {
            // cout << "Não foi possível abrir o arquivo" << endl;
            exit(1);
        }
    }
    void writeFSHeader()
    {
        // escreve as informações de cabeçalho que foram passadas
        this->fsFile.seekg(0);
        this->fsFile.write((char *)&this->blockSize, sizeof(char));
        this->fsFile.write((char *)&this->qtdBlocks, sizeof(char));
        this->fsFile.write((char *)&this->qtdInodes, sizeof(char));
    }
    void wipeFS()
    {
        // limpa o arquivo
        int totalSize = 3 + this->bitmapSize + this->indexVectorSize + 1 + this->blockVectorSize;
        char *zero[totalSize]{};
        this->fsFile.seekg(0);
        // escreve o bitmap
        this->fsFile.write((char *)&zero, totalSize);
    }
    INODE populateNameOnInode(INODE inode, string name)
    {
        for (int i = 0; i < name.size(); i++)
        {
            inode.NAME[i] = name[i];
        }
        return inode;
    }
    INODE createDirInode(string path)
    {
        INODE new_inode{};
        new_inode.IS_DIR = 1;
        new_inode.IS_USED = 1;
        new_inode.SIZE = 0;

        new_inode = this->populateNameOnInode(new_inode, path);

        return new_inode;
    }
    INODE createFileInode(string file_name)
    {
        INODE new_inode{};
        new_inode.IS_DIR = 0;
        new_inode.IS_USED = 1;
        new_inode.SIZE = 0;

        new_inode = this->populateNameOnInode(new_inode, file_name);

        return new_inode;
    }
    char *getBitmap()
    {
        char *bitmap = new char[this->bitmapSize];
        this->fsFile.seekg(3);
        this->fsFile.read(bitmap, this->bitmapSize);
        return bitmap;
    }
    int findFirstFreeBlockInTheBitmap()
    {
        char *bitmap = this->getBitmap();
        int index_livre = 0;
        // cout << "bitmapSize: " << this->bitmapSize << endl;
        // descobre a posição do primeiro bit 0 do bitmap
        for (int i = 0; i < this->bitmapSize; i++)
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
    int findFirstFreeInodeBlockIndex()
    {
        // Certifique-se de que a posição de leitura está correta
        this->fsFile.seekg(3 + bitmapSize);

        int index_livre = 0;
        INODE inode{};

        while (1)
        {
            this->fsFile.read((char *)&inode, sizeof(INODE));

            if (inode.IS_USED == 0)
            {
                break;
            }
            index_livre++;
        }
        return index_livre;
    }
    void writeInodeAtIndex(int index, INODE inode)
    {
        this->fsFile.seekg(3 + this->bitmapSize + index * sizeof(INODE));
        this->fsFile.write((char *)&inode, sizeof(INODE));
    }
    void saveBitmap(char *bitmap)
    {
        this->fsFile.seekg(3);
        this->fsFile.write(bitmap, this->bitmapSize);
    }
    void setBitmapAtIndex(int index)
    {
        char *bitmap = this->getBitmap();
        // cout << "old bitmap: " << (int)bitmap[0] << endl;
        bitmap[(int)(index / 8.0)] |= (1 << (index % 8));
        // cout << "new bitmap: " << (int)bitmap[0] << endl;
        this->saveBitmap(bitmap);
    }
    void freeBitmapAtIndex(int index)
    {
        char *bitmap = this->getBitmap();
        bitmap[(int)(index / 8.0)] |= (0 << (index % 8));
        this->saveBitmap(bitmap);
    }
    string getFatherDirNameFromFilePath(string file_path)
    {
        string father = file_path.substr(0, file_path.find_last_of("/")) == "" ? "/" : file_path.substr(0, file_path.find_last_of("/"));
        if (father != "/")
        {
            father = father.substr(1, father.size());
        }
        return father;
    }
    INODE appendDataIntoInode(INODE inode, char data)
    {
        // roda os direct block até achar um vazio e escreve o data
        for (int i = 0; i < 3; i++)
        {
            if (inode.DIRECT_BLOCKS[i] == 0)
            {
                inode.DIRECT_BLOCKS[i] = data;
                break;
            }
        }
        return inode;
    }
    char *readDataBlockAtIndex(int index)
    {
        char *data_block = new char[this->blockSize];
        this->fsFile.seekg(3 + this->bitmapSize + this->indexVectorSize + 1 + index * this->blockSize);
        this->fsFile.read(data_block, this->blockSize);
        return data_block;
    }
    void writeDataBlockAtIndex(int index, char *data)
    {
        this->fsFile.seekg(3 + this->bitmapSize + this->indexVectorSize + 1 + index * this->blockSize);
        this->fsFile.write(data, this->blockSize);
    }
    INODE insertDataIntoInodeBlock(INODE inode, char data)
    {
        // verifica se a quantidade atual + 1 precisa alocar mais um bloco
        int new_qtd_necessaria = ceil((float)(inode.SIZE + 1) / (float)this->blockSize);
        int old_qtd_necessaria = ceil((float)(inode.SIZE) / (float)this->blockSize);
        // cout << "new_qtd_necessaria: " << new_qtd_necessaria << endl;
        // cout << "old_qtd_necessaria: " << old_qtd_necessaria << endl;

        if ((new_qtd_necessaria > old_qtd_necessaria) and !(old_qtd_necessaria == 0 and inode.IS_DIR == 1))
        {
            // aloca mais um bloco
            int index_livre = this->findFirstFreeBlockInTheBitmap();
            // cout << "index_livre: " << index_livre << endl;
            this->setBitmapAtIndex(index_livre);
            inode = this->appendDataIntoInode(inode, (char)index_livre);
        }
        // pega o endereço do ultimo bloco
        int last_block_index = inode.DIRECT_BLOCKS[new_qtd_necessaria - 1];

        // cout << "last_block_index: " << last_block_index << endl;
        // cout << "data: " << data << " | " << (int)data << endl
        // << endl;
        char *data_block = this->readDataBlockAtIndex(last_block_index);
        // encontra o primeiro byte vazio
        int first_empty_byte = 0;
        for (int i = 0; i < this->blockSize; i++)
        {
            if (data_block[i] == 0)
            {
                first_empty_byte = i;
                break;
            }
        }
        // escreve o dado no primeiro byte vazio
        data_block[first_empty_byte] = data;
        // escreve o bloco de dados no arquivo
        this->writeDataBlockAtIndex(last_block_index, data_block);
        inode.SIZE++;
        return inode;
    }
    INODE getInodeAtIndex(int index)
    {
        INODE inode{};
        this->fsFile.seekg(3 + this->bitmapSize + index * sizeof(INODE));
        this->fsFile.read((char *)&inode, sizeof(INODE));
        return inode;
    }
    INODE getInodeByName(string name)
    {
        INODE inode = this->getInodeAtIndex(this->getInodeIndexByName(name));
        return inode;
    }
    int getInodeIndexByName(string name)
    {
        // cout << "buscando inode com nome: " << name << endl;
        this->fsFile.seekg(3 + this->bitmapSize);

        INODE inode{};
        int position = 0;
        bool found = false;
        while (1)
        {
            this->fsFile.read((char *)&inode, sizeof(INODE));
            if (inode.IS_USED == 1)
            {
                if (inode.NAME == name)
                {
                    found = true;
                    break;
                }
            }
            position++;
        }
        if (!found)
        {
            // cout << "Não foi encontrado o inode com o nome: " << name << endl;
            exit(1);
        }
        // cout << "inode encontrado na posição: " << position << endl;
        return position;
    }
    void createDir(string path)
    {
        INODE root_inode = this->createDirInode(path);
        int index_livre = this->findFirstFreeInodeBlockIndex();
        // cout << "index_livre: " << index_livre << endl;
        // escreve o inode no arquivo
        this->writeInodeAtIndex(index_livre, root_inode);
        // seta o bitmap
        this->setBitmapAtIndex(index_livre);
    }
    void createFile(string file_name)
    {
        // cria o inode do arquivo
        INODE file_inode = this->createFileInode(file_name);
        // descobre o index do primeiro bloco livre
        int index_livre = this->findFirstFreeInodeBlockIndex();
        // escreve o inode no arquivo
        this->writeInodeAtIndex(index_livre, file_inode);
    }
    void addFileToDir(string path, string file_name)
    {
        // descobre o index do inode do pai no index vector
        int father_dir_inode_index = this->getInodeIndexByName(path);
        // cout << "father_dir_inode_index: " << father_dir_inode_index << endl;
        INODE father_dir_inode = this->getInodeAtIndex(father_dir_inode_index);

        // descobre o index do inode do arquivo no index vector
        int file_inode_index = this->getInodeIndexByName(file_name);
        // cout << "file_inode_index: " << file_inode_index << endl;
        INODE file_inode = this->getInodeAtIndex(file_inode_index);

        // adiciona o inode do arquivo no inode do pai
        // father_dir_inode.SIZE++;
        // cout << "adicionando o endereço do inode do arquivo no inode do pai" << endl;
        father_dir_inode = this->insertDataIntoInodeBlock(father_dir_inode, (char)file_inode_index);
        // cout << "Endereço adicionado" << endl;

        // escreve o inode do pai no arquivo
        this->writeInodeAtIndex(father_dir_inode_index, father_dir_inode);
        // cout << "inode salvo" << endl;
    }
    void populateFile(string file_name, string content)
    {
        // descobre o index do inode do arquivo no index vector
        int file_inode_index = this->getInodeIndexByName(file_name);
        INODE file_inode = this->getInodeAtIndex(file_inode_index);

        // cout << endl
        //      << "adicionando o conteudo do arquivo no inode do arquivo" << endl;
        for (int i = 0; i < content.size(); i++)
        {
            file_inode = this->insertDataIntoInodeBlock(file_inode, content[i]);
        }

        // escreve o inode do arquivo no arquivo
        this->writeInodeAtIndex(file_inode_index, file_inode);
    }
    void writeBlockAtIndex(int index, char *data)
    {
        this->fsFile.seekg(3 + this->bitmapSize + this->indexVectorSize + index * this->blockSize);
        this->fsFile.write(data, this->blockSize);
    }

public:
    ~FileSystem()
    {
        this->fsFile.close();
    }
    FileSystem(string fs_file_name)
    {
        fsFileName = fs_file_name;
        this->loadFileSystem();
    }
    void loadSystemVariables()
    {
        this->fsFile.seekg(0);
        char tmp{};
        this->fsFile.read((char *)&tmp, sizeof(char));
        this->blockSize = tmp;
        this->fsFile.read((char *)&tmp, sizeof(char));
        this->qtdBlocks = tmp;
        this->fsFile.read((char *)&tmp, sizeof(char));
        this->qtdInodes = tmp;

        this->bitmapSize = ceil((float)this->qtdBlocks / 8.0);
        this->indexVectorSize = sizeof(INODE) * this->qtdInodes;
        this->blockVectorSize = this->qtdBlocks * this->blockSize;
    }
    void createFileSystem(int block_size, int num_blocks, int num_inodes)
    {
        this->blockSize = block_size;
        this->qtdBlocks = num_blocks;
        this->qtdInodes = num_inodes;
        this->bitmapSize = ceil((float)num_blocks / 8.0);
        this->indexVectorSize = sizeof(INODE) * num_inodes;
        this->blockVectorSize = num_blocks * block_size;

        // cria o arquivo só com zeros do tamanho correto
        this->wipeFS();

        // escreve as informações de cabeçalho que foram passadas
        this->writeFSHeader();

        // cria o inode da raiz
        this->createDir("/");
        // seta a primeira pos do bitmap como 1 pra reservar pro /
        this->setBitmapAtIndex(0);
    }

    void addFile(string full_path, string content)
    {
        // cout << "full_path: " << full_path << endl;
        string father_dir_name = this->getFatherDirNameFromFilePath(full_path);
        string file_name = full_path.substr(full_path.find_last_of("/") + 1, full_path.size());
        // cout << "father_dir_name: " << father_dir_name << endl;
        // cout << "file_name: " << file_name << endl;

        // Coloca o arquivo na pasta
        this->createFile(file_name);
        // cout << "criou o arquivo" << endl;
        this->addFileToDir(father_dir_name, file_name);
        // cout << "adicionou o arquivo na pasta" << endl;

        // escreve o conteudo do arquivo no disco
        this->populateFile(file_name, content);
        // cout << "escreveu o conteudo do arquivo no disco" << endl;
    }
};