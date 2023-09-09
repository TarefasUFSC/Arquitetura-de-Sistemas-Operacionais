#include <fstream>
#include <iostream>
#include <math.h>
#include <vector>
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
                    index_livre = i * 4 + j;
                    return index_livre;
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
        uint8_t mask = 0xFF;        // todos os bits são 1
        mask ^= (1 << (index % 8)); // inverte o bit desejado para 0
        bitmap[index / 8] &= mask;  // zera o bit desejado
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
        inode.DIRECT_BLOCKS[(int)ceil((inode.SIZE + 1) / 3)] = data;
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
        // cout << "inserindo o dado: " << data << " no inode: " << inode.NAME << endl;
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
        //  << endl;
        this->writeByteAtBlockAndPosition(last_block_index, inode.SIZE % this->blockSize, data);
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
        INODE inode = this->createDirInode(path);
        int inode_index_livre = this->findFirstFreeInodeBlockIndex();
        // cout << "inode_index_livre: " << inode_index_livre << endl;

        // aloca um bloco para o inode
        int block_index_livre = this->findFirstFreeBlockInTheBitmap();

        // coloca esse bloco no direct block do INODE mas sem aumentar o tamanho
        inode.DIRECT_BLOCKS[0] = (char)block_index_livre;
        // seta o bitmap para 1 nesse bloco
        this->setBitmapAtIndex(block_index_livre);

        // escreve o inode no arquivo
        this->writeInodeAtIndex(inode_index_livre, inode);
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
        this->fsFile.seekg(3 + this->bitmapSize + this->indexVectorSize + 1 + index * this->blockSize);
        this->fsFile.write(data, this->blockSize);
    }
    void writeByteAtBlockAndPosition(int index, int position, char data)
    {
        char *data_block = this->readDataBlockAtIndex(index);
        data_block[position] = data;
        this->writeDataBlockAtIndex(index, data_block);
    }
    void defragmentDir(int index_dir_inode, INODE &dir_inode)
    {
        vector<char> dir_content; // vetor que vai armazenar os dados e depois vai reorganizar em blocos novamente
        for (int i = 0; i < 3; i++)
        {
            char *data_block = this->readDataBlockAtIndex((int)dir_inode.DIRECT_BLOCKS[i]);
            // para cada byte do bloco verifica se o inode correspondente está vazio (IS_USED == 0)
            for (int j = 0; j < this->blockSize; j++)
            {
                int inode_index = (int)data_block[j];
                // pega o inode correspondente
                INODE inode = this->getInodeAtIndex(inode_index);
                // se o inode estiver vazio, não faz nada
                // if (inode.IS_USED == 0)
                // {
                //     continue;
                // }
                // se o inode não estiver vazio, adiciona o dado no vetor dir_content
                // somente se ele ja não estiver no vetor
                bool found = false;
                for (int k = 0; k < dir_content.size(); k++)
                {
                    if (dir_content[k] == inode_index)
                    {
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    dir_content.push_back(inode_index);
                }
            }
        }

        // verifica a quantidade blocos necessários para armazenar o conteudo do dir
        int qtd_necessaria = ceil((float)dir_content.size() / (float)this->blockSize);
        // verifica a quantidade de blocos que o dir esta usando agora
        int qtd_atual = ceil((float)dir_inode.SIZE / (float)this->blockSize);

        // se a quantidade de blocos necessários for menor que a quantidade de blocos que o dir esta usando agora, libera os blocos que não serão mais usados no bitmap
        if (qtd_necessaria < qtd_atual)
        {
            for (int i = qtd_necessaria; i < qtd_atual; i++)
            {
                this->freeBitmapAtIndex((int)dir_inode.DIRECT_BLOCKS[i]);
            }
        }
        if (qtd_atual == 0)
        {
            return;
        }
        // remove do vetor os inodes que não estao mais sendo usados (IS_USED = 0)
        vector<char> dir_content_aux;
        for (int i = 0; i < dir_content.size(); i++)
        {
            INODE inode = this->getInodeAtIndex((int)dir_content[i]);
            if (inode.IS_USED == 1)
            {
                dir_content_aux.push_back(dir_content[i]);
            }
        }
        dir_content = dir_content_aux;
        this->writeByteVectorIntoBlocks(dir_content, dir_inode);
    }
    void writeByteVectorIntoBlocks(vector<char> byte_vector, INODE inode)
    {
        cout << "Escrevendo o vetor de bytes no inode: " << inode.NAME << " com tamanho: " << byte_vector.size() << endl;
        // coloca os dados do vetor nos blocos que ele tem disponivel agora
        for (int i = 0; i < byte_vector.size(); i++)
        {
            cout << "Escrevendo byte: " << i << ", no bloco: " << (int)(i / this->blockSize) << ", na posição: " << (i % this->blockSize) << " | conteudo: " << (int)byte_vector[i] << endl;

            this->writeByteAtBlockAndPosition((int)inode.DIRECT_BLOCKS[(int)(i / 3)], i % this->blockSize, byte_vector[i]);
        }
    }
    vector<char> getBytesFromInodeDirectBlocks(INODE inode)
    {
        vector<char> bytes;
        int qtd_bytes = inode.SIZE;
        int qtd_blocos = ceil((float)qtd_bytes / (float)this->blockSize);
        for (int i = 0; i < qtd_blocos; i++)
        {
            char *data_block = this->readDataBlockAtIndex((int)inode.DIRECT_BLOCKS[i]);
            for (int j = 0; j < this->blockSize; j++)
            {
                if (data_block[j] != 0)
                {
                    bytes.push_back(data_block[j]);
                }
            }
        }

        // o vector de bytes talvez esteja com mais bytes do que o inode tem pq o blocksize buga a conta, eu vou remover os bytes que não fazem parte do inode a partir do fim
        int qtd_bytes_excedentes = bytes.size() - qtd_bytes;
        for (int i = 0; i < qtd_bytes_excedentes; i++)
        {
            bytes.pop_back();
        }
        return bytes;
    }
    string getFileContent(INODE file_inode)
    {
        string content = "";
        for (int i = 0; i < 3; i++)
        {
            if (file_inode.DIRECT_BLOCKS[i] != 0)
            {
                char *data_block = this->readDataBlockAtIndex((int)file_inode.DIRECT_BLOCKS[i]);
                for (int j = 0; j < this->blockSize; j++)
                {
                    if (data_block[j] != 0)
                    {
                        content += data_block[j];
                    }
                }
            }
        }
        return content;
    }
    void removeFileRefFromDir(string path, int file_inode_index)
    {
        // descobre o index do inode do pai no index vector
        int father_dir_inode_index = this->getInodeIndexByName(path);
        INODE father_dir_inode = this->getInodeAtIndex(father_dir_inode_index);

        INODE file_inode = this->getInodeAtIndex(file_inode_index);

        vector<char> dir_content = this->getBytesFromInodeDirectBlocks(father_dir_inode);

        // remove o byte cujo o valor é o index do inode do arquivo
        vector<char> tmp_char_vector;
        for (int i = 0; i < dir_content.size(); i++)
        {
            if (dir_content[i] != file_inode_index)
            {
                tmp_char_vector.push_back(dir_content[i]);
            }
        }
        dir_content = tmp_char_vector;
        int antiga_qtd_necessaria = ceil((float)father_dir_inode.SIZE / (float)this->blockSize);

        cout << "A antiga qtd necessaria é: " << antiga_qtd_necessaria << endl;

        father_dir_inode.SIZE--;

        // recalcula a quantidade de blocos necessários para armazenar o conteudo do dir
        int nova_qtd_necessaria = ceil((float)dir_content.size() / (float)this->blockSize);

        cout << "A nova quantidade necessária é: " << nova_qtd_necessaria << endl;

        // verifica se houve mudança na qtd de blocos necessários
        if ((nova_qtd_necessaria != antiga_qtd_necessaria) && (nova_qtd_necessaria != 0))
        {
            cout << "A quantidade necessária de blocos mudou" << endl;
            ;
            // se houve, libera os blocos que não serão mais usados no bitmap
            for (int i = nova_qtd_necessaria; i < antiga_qtd_necessaria; i++)
            {
                cout << "Liberando no bitmap o bloco: " << (int)father_dir_inode.DIRECT_BLOCKS[i] << endl;
                this->freeBitmapAtIndex((int)father_dir_inode.DIRECT_BLOCKS[i]);
                father_dir_inode.DIRECT_BLOCKS[i] = 0;
            }
        }

        // escreve o inode do pai no arquivo
        this->writeInodeAtIndex(father_dir_inode_index, father_dir_inode);

        // reescreve o dir
        this->writeByteVectorIntoBlocks(dir_content, father_dir_inode);
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
    void addDir(string full_path)
    {
        // cout << "full_path: " << full_path << endl;
        string father_dir_name = this->getFatherDirNameFromFilePath(full_path);
        string dir_name = full_path.substr(full_path.find_last_of("/") + 1, full_path.size());

        this->createDir(dir_name);

        // adiciona o dir na pasta
        this->addFileToDir(father_dir_name, dir_name);
    }
    void remove(string full_path)
    {
        string father_dir_name = this->getFatherDirNameFromFilePath(full_path);
        string file_name = full_path.substr(full_path.find_last_of("/") + 1, full_path.size());

        // descobre o index do inode do pai no index vector
        int father_dir_inode_index = this->getInodeIndexByName(father_dir_name);
        INODE father_dir_inode = this->getInodeAtIndex(father_dir_inode_index);

        // descobre o index do inode do arquivo no index vector
        int file_inode_index = this->getInodeIndexByName(file_name);
        INODE file_inode = this->getInodeAtIndex(file_inode_index);

        // libera no bitmap os blocos do arquivo
        for (int i = 0; i < 3; i++)
        {
            if (file_inode.DIRECT_BLOCKS[i] != 0)
            {
                this->freeBitmapAtIndex((int)file_inode.DIRECT_BLOCKS[i]);
            }
        }

        // escreve o inode do arquivo no arquivo (zerando tudo no inode)
        file_inode = INODE{};
        this->writeInodeAtIndex(file_inode_index, file_inode);
        // remove o arquivo do inode do pai (diminui o tamanho)
        father_dir_inode.SIZE--;

        // defragmenta o dir
        this->defragmentDir(father_dir_inode_index, father_dir_inode);

        // escreve o inode do pai no arquivo
        this->writeInodeAtIndex(father_dir_inode_index, father_dir_inode);
    }
    void move(string old_full_path, string new_full_path)
    {

        cout << "Movendo o arquivo: " << old_full_path << " para: " << new_full_path << endl;
        int a;
        cin >> a;

        // pega o nome do arquivo
        string file_name = old_full_path.substr(old_full_path.find_last_of("/") + 1, old_full_path.size());
        // pega o nome do novo pai
        string new_father_dir_name = this->getFatherDirNameFromFilePath(new_full_path);
        // pega o nome do pai antigo
        string old_father_dir_name = this->getFatherDirNameFromFilePath(old_full_path);

        // pega o inode do arquivo
        int file_inode_index = this->getInodeIndexByName(file_name);
        INODE file_inode = this->getInodeAtIndex(file_inode_index);

        // pega o conteudo do arquivo
        string file_content = this->getFileContent(file_inode);
        // cout << "file_content: " << file_content << endl;

        // insere o indice do inode do arquivo no novo pai
        this->addFileToDir(new_father_dir_name, file_name);

        // remove o indice do inode do arquivo do pai antigo

        cin >> a;
        this->removeFileRefFromDir(old_father_dir_name, file_inode_index);

        cin >> a;
        INODE old_father_dir_inode = this->getInodeAtIndex(this->getInodeIndexByName(old_father_dir_name));
    }
};