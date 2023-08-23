#include "file-system.hpp"

#include <cmath>
#include "utils.hpp"
#include <string.h>

namespace fs
{

  FileSystem::FileSystem(std::fstream *fs, byte blockSize, byte numBlocks, byte numInodes) : fs(fs), info(Info{
                                                                                                         blockSize,
                                                                                                         numBlocks,
                                                                                                         numInodes,
                                                                                                         BitMap(ceil(numBlocks / BYTE_BIT_LENGHT), BITMAP_POINTER_POS),
                                                                                                         0x00}),
                                                                                             pointersPosition(PointersPosition{
                                                                                                 (byte)(BITMAP_POINTER_POS + info.bitMap.size()),
                                                                                                 (byte)(BITMAP_POINTER_POS + info.bitMap.size() + numInodes * sizeof(INODE)),
                                                                                                 (byte)(BITMAP_POINTER_POS + info.bitMap.size() + numInodes * sizeof(INODE) + 1)})
  {
    info.bitMap.occupy(0x00);
    fs->seekg(0x00, std::ios_base::beg);
    *fs << info.blockSize;
    *fs << info.numBlocks;
    *fs << info.numInodes;
    info.bitMap.save(fs);
    INODE rootInode = INODE{
        true,
        true,
        "/",
    };
    fs->write((char *)&rootInode, sizeof(INODE));
    INODE emptyInode = INODE{};
    for (byte i = 0; i < numInodes - 1; i++)
    {
      fs->write((char *)&emptyInode, sizeof(INODE));
    }
    fs->write((char *)&info.rootDir, sizeof(byte));
    std::vector<byte> emptyBlock = std::vector<byte>(blockSize, 0x00);
    for (byte i = 0; i < numBlocks; i++)
    {
      for (const auto &blockData : emptyBlock)
      {
        *fs << blockData;
      }
    }
  }

  FileSystem::FileSystem(std::fstream *fs) : fs(fs)
  {
    fs->seekg(0x00, std::ios_base::beg);
    *fs >> info.blockSize;
    *fs >> info.numBlocks;
    *fs >> info.numInodes;
    info.bitMap = BitMap(ceil(info.numBlocks / BYTE_BIT_LENGHT), BITMAP_POINTER_POS, fs);
    pointersPosition = {
        (byte)(BITMAP_POINTER_POS + info.bitMap.size()),
        (byte)(BITMAP_POINTER_POS + info.bitMap.size() + info.numInodes * sizeof(INODE)),
        (byte)(BITMAP_POINTER_POS + info.bitMap.size() + info.numInodes * sizeof(INODE) + 1)};
    fs->seekg(pointersPosition.rootDir, std::ios_base::beg);
    *fs >> info.rootDir;
  }

  void FileSystem::addFile(std::string filePath, std::string fileContent)
  {
    std::vector<std::string> path = split(filePath, '/');
    std::string fileName = path.back();
    byte newInodeNumber = getNewInodePos();
    insertInodeInParentInode(getParentInode(path), newInodeNumber);
    INODE inodeNewFile = INODE{
        true,
        false};
    strcpy(inodeNewFile.NAME, fileName.c_str());
    inodeNewFile.SIZE = (byte)fileContent.length();
    insertDataFile({newInodeNumber, inodeNewFile}, fileContent);
    info.bitMap.save(fs);
  }

  void FileSystem::addDir(std::string dirPath)
  {
    std::vector<std::string> path = split(dirPath, '/');
    std::string dirName = path.back();
    byte newInodeNumber = getNewInodePos();
    insertInodeInParentInode(getParentInode(path), newInodeNumber);
    INODE inodeNewDir = INODE{
        true,
        true};
    strcpy(inodeNewDir.NAME, dirName.c_str());
    createEmptyBlockInInode({newInodeNumber, inodeNewDir});
    info.bitMap.save(fs);
  }

  void FileSystem::remove(std::string excludePath)
  {
    std::vector<std::string> path = split(excludePath, '/');
    std::string excludeName = path.back();
    InodeAndIndex parentInode = getParentInode(path);
    InodeAndIndex excludeInode = getChildInode(excludeName, parentInode);
    reorganizeDirectoryRemovingOneInode(parentInode, excludeInode.index);
    freeAllBlocksFromInode(excludeInode);
    eraseInode(excludeInode.index);
    info.bitMap.save(fs);
  }

  void FileSystem::move(std::string oldPath, std::string newPath)
  {
    std::vector<std::string> oldVectorPath = split(oldPath, '/');
    std::string oldName = oldVectorPath.back();
    InodeAndIndex oldParentInode = getParentInode(oldVectorPath);
    InodeAndIndex inode = getChildInode(oldName, oldParentInode);
    std::vector<std::string> newVectorPath = split(newPath, '/');
    std::string newName = newVectorPath.back();
    if (oldVectorPath.at(oldVectorPath.size() - 2) != newVectorPath.at(newVectorPath.size() - 2))
    {
      reorganizeDirectoryRemovingOneInode(oldParentInode, inode.index);
      InodeAndIndex newParentInode = getParentInode(newVectorPath);
      insertInodeInParentInode(newParentInode, inode.index);
    }
    renameInode(inode, newName);
    info.bitMap.save(fs);
  }

  InodeAndIndex FileSystem::getParentInode(std::vector<std::string> path)
  {
    byte parentInodePos = info.rootDir;
    INODE parentInode;
    for (byte i = 1; i < path.size() - 1; i++)
    {
      for (byte j = 0; j < DIRECT_BLOCK_SIZE; j++)
      {
        fs->seekg(pointersPosition.inodeVector + parentInodePos * sizeof(INODE), std::ios_base::beg);
        fs->read((char *)&parentInode, sizeof(INODE));
        byte directBlock = parentInode.DIRECT_BLOCKS[j];
        for (byte k = 0; k < info.blockSize; k++)
        {
          fs->seekg(pointersPosition.blockVector + directBlock * info.blockSize + k, std::ios_base::beg);
          byte inodeSearchPos;
          *fs >> inodeSearchPos;
          fs->seekg(pointersPosition.inodeVector + inodeSearchPos * sizeof(INODE), std::ios_base::beg);
          INODE inodeSearch;
          fs->read((char *)&inodeSearch, sizeof(INODE));
          if (std::string(inodeSearch.NAME) == path.at(i))
          {
            parentInode = inodeSearch;
            parentInodePos = inodeSearchPos;
            j = DIRECT_BLOCK_SIZE;
            break;
          }
        }
      }
    }
    if (parentInodePos == info.rootDir)
    {
      fs->seekg(pointersPosition.inodeVector, std::ios_base::beg);
      fs->read((char *)&parentInode, sizeof(INODE));
    }
    return InodeAndIndex{parentInodePos, parentInode};
  }

  InodeAndIndex FileSystem::getChildInode(std::string inodeName, InodeAndIndex parentInode)
  {
    byte childInodePos;
    INODE childInode;
    for (byte j = 0; j < DIRECT_BLOCK_SIZE; j++)
    {
      byte directBlock = parentInode.inode.DIRECT_BLOCKS[j];
      for (byte k = 0; k < info.blockSize; k++)
      {
        fs->seekg(pointersPosition.blockVector + directBlock * info.blockSize + k, std::ios_base::beg);
        byte inodeSearchPos;
        *fs >> inodeSearchPos;
        fs->seekg(pointersPosition.inodeVector + inodeSearchPos * sizeof(INODE), std::ios_base::beg);
        INODE inodeSearch;
        fs->read((char *)&inodeSearch, sizeof(INODE));
        if (std::string(inodeSearch.NAME) == inodeName)
        {
          childInode = inodeSearch;
          childInodePos = inodeSearchPos;
          j = DIRECT_BLOCK_SIZE;
          break;
        }
      }
    }
    return InodeAndIndex{childInodePos, childInode};
  }

  byte FileSystem::getNewInodePos()
  {
    for (byte i = 0; i < info.numInodes; i++)
    {
      INODE inodeSearch;
      fs->seekg(pointersPosition.inodeVector + i * sizeof(INODE), std::ios_base::beg);
      fs->read((char *)&inodeSearch, sizeof(INODE));
      if (!inodeSearch.IS_USED)
      {
        return i;
      }
    }
    return -1;
  }

  void FileSystem::insertInodeInParentInode(InodeAndIndex parentInode, byte inodeNumber)
  {
    byte directBlockInodePos = floor((double)parentInode.inode.SIZE / (double)info.blockSize);
    byte firstEmptyBlockData = parentInode.inode.SIZE % info.blockSize;
    if (parentInode.inode.SIZE > 0 && firstEmptyBlockData == 0)
    {
      byte newBlockPos = info.bitMap.getFirstFree();
      info.bitMap.occupy(newBlockPos);
      parentInode.inode.DIRECT_BLOCKS[directBlockInodePos] = newBlockPos;
    }
    parentInode.inode.SIZE++;
    fs->seekg(pointersPosition.inodeVector + parentInode.index * sizeof(INODE), std::ios_base::beg);
    fs->write((char *)&parentInode.inode, sizeof(INODE));
    fs->seekg(pointersPosition.blockVector + parentInode.inode.DIRECT_BLOCKS[directBlockInodePos] * info.blockSize + firstEmptyBlockData, std::ios_base::beg);
    *fs << inodeNumber;
  }

  void FileSystem::insertDataFile(InodeAndIndex fileInode, std::string fileContent)
  {
    byte blocksNeeded = ceil((double)fileContent.length() / (double)info.blockSize);
    std::vector<std::string> blocksDvivided;
    for (byte i = 0; i < fileContent.length(); i = i + info.blockSize)
    {
      blocksDvivided.push_back(fileContent.substr(i, info.blockSize));
    }
    for (int i = 0; i < blocksNeeded; i++)
    {
      byte newPos = info.bitMap.getFirstFree();
      info.bitMap.occupy(newPos);
      fileInode.inode.DIRECT_BLOCKS[i] = newPos;
      for (int j = 0; j < std::min(info.blockSize, (byte)blocksDvivided.at(i).size()); j++)
      {
        fs->seekg(pointersPosition.blockVector + newPos * info.blockSize + j, std::ios_base::beg);
        *fs << blocksDvivided.at(i).at(j);
      }
    }
    fs->seekg(pointersPosition.inodeVector + fileInode.index * sizeof(INODE), std::ios_base::beg);
    fs->write((char *)&fileInode.inode, sizeof(INODE));
  }

  void FileSystem::createEmptyBlockInInode(InodeAndIndex dirInode)
  {
    byte newBlockPos = info.bitMap.getFirstFree();
    info.bitMap.occupy(newBlockPos);
    dirInode.inode.DIRECT_BLOCKS[0] = newBlockPos;
    fs->seekg(pointersPosition.inodeVector + dirInode.index * sizeof(INODE), std::ios_base::beg);
    fs->write((char *)&dirInode.inode, sizeof(INODE));
  }

  void FileSystem::freeAllBlocksFromInode(InodeAndIndex inode)
  {
    byte numBit2Free = ceil((long)inode.inode.SIZE / (long)info.blockSize);
    if (numBit2Free == 0 && inode.inode.IS_DIR)
    {
      numBit2Free++;
    }
    for (int i = 0; i < numBit2Free; i++)
    {
      info.bitMap.free(inode.inode.DIRECT_BLOCKS[i]);
    }
  }

  void FileSystem::reorganizeDirectoryRemovingOneInode(InodeAndIndex dirInode, byte excludeInodePos)
  {
    std::vector<byte> newParentBlockData = getAllChildInodeButOne(dirInode, excludeInodePos);
    byte poslastInode = floor((double)dirInode.inode.SIZE / (double)info.blockSize);
    byte posLastBlockData = dirInode.inode.SIZE % info.blockSize;
    if (posLastBlockData == 1 && poslastInode != 0)
    {
      info.bitMap.free(dirInode.inode.DIRECT_BLOCKS[poslastInode]);
      dirInode.inode.DIRECT_BLOCKS[poslastInode] = 0x00;
    }
    dirInode.inode.SIZE--;
    overrideBlocksData(dirInode, newParentBlockData);
    fs->seekg(pointersPosition.inodeVector + dirInode.index * sizeof(INODE), std::ios_base::beg);
    fs->write((char *)&dirInode.inode, sizeof(INODE));
  }

  void FileSystem::eraseInode(byte inodePos)
  {
    INODE excludeInode = INODE{};
    fs->seekg(pointersPosition.inodeVector + inodePos * sizeof(INODE), std::ios_base::beg);
    fs->write((char *)&excludeInode, sizeof(INODE));
  }

  void FileSystem::renameInode(InodeAndIndex inode, std::string newName)
  {
    strcpy(inode.inode.NAME, newName.c_str());
    fs->seekg(pointersPosition.inodeVector + inode.index * sizeof(INODE), std::ios_base::beg);
    fs->write((char *)&inode.inode, sizeof(INODE));
  }

  std::vector<byte> FileSystem::getAllChildInodeButOne(InodeAndIndex dirInode, byte excludeInodePos)
  {
    std::vector<byte> response = std::vector<byte>();
    byte counter = 0;
    for (byte i = 0; i < DIRECT_BLOCK_SIZE; i++)
    {
      for (byte j = 0; j < info.blockSize; j++)
      {
        if (dirInode.inode.SIZE == counter)
        {
          i = DIRECT_BLOCK_SIZE;
          break;
        }
        byte tmpData;
        fs->seekg(pointersPosition.blockVector + dirInode.inode.DIRECT_BLOCKS[i] * info.blockSize + j, std::ios_base::beg);
        *fs >> tmpData;
        if (tmpData != excludeInodePos && tmpData != 0x00)
        {
          response.push_back(tmpData);
        }
        counter++;
      }
    }
    return response;
  }

  void FileSystem::overrideBlocksData(InodeAndIndex dirInode, std::vector<byte> blockSaveData)
  {
    byte counter = 0;
    for (byte i = 0; i < DIRECT_BLOCK_SIZE; i++)
    {
      for (byte j = 0; j < info.blockSize; j++)
      {
        if (counter == dirInode.inode.SIZE)
        {
          i = DIRECT_BLOCK_SIZE;
          break;
        }
        fs->seekg(pointersPosition.blockVector + dirInode.inode.DIRECT_BLOCKS[i] * info.blockSize + j, std::ios_base::beg);
        *fs << blockSaveData.at(counter);
        counter++;
      }
    }
  }

}