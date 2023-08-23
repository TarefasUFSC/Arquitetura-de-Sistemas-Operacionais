#ifndef FYLE_SYSTEM_HPP
#define FYLE_SYSTEM_HPP

#include <vector>
#include <fstream>
#include "fs.h"
#include "constants.hpp"
#include "bit-map.hpp"

namespace fs
{

  typedef struct
  {
    byte index;
    INODE inode;
  } InodeAndIndex;

  typedef struct
  {
    byte blockSize;
    byte numBlocks;
    byte numInodes;
    BitMap bitMap;
    byte rootDir;
  } Info;

  typedef struct
  {
    byte inodeVector;
    byte rootDir;
    byte blockVector;
  } PointersPosition;

  class FileSystem
  {
  private:
    std::fstream *fs;
    Info info;
    PointersPosition pointersPosition;
    InodeAndIndex getParentInode(std::vector<std::string> path);
    InodeAndIndex getChildInode(std::string inodeName, InodeAndIndex parentInode);
    byte getNewInodePos();
    void insertInodeInParentInode(InodeAndIndex parentInode, byte newInodeNumber);
    void insertDataFile(InodeAndIndex fileInode, std::string fileContent);
    void createEmptyBlockInInode(InodeAndIndex dirInode);
    void freeAllBlocksFromInode(InodeAndIndex inode);
    void eraseInode(byte inodePos);
    void reorganizeDirectoryRemovingOneInode(InodeAndIndex dirInode, byte excludeInodePos);
    void renameInode(InodeAndIndex inode, std::string newName);
    std::vector<byte> getAllChildInodeButOne(InodeAndIndex dirInode, byte excludeInodePos);
    void overrideBlocksData(InodeAndIndex dirInode, std::vector<byte> blockSaveData);

  public:
    FileSystem(std::fstream *fs);
    FileSystem(std::fstream *fs, byte blockSize, byte numBlocks, byte numInodes);
    void addFile(std::string filePath, std::string fileContent);
    void addDir(std::string dirPath);
    void remove(std::string excludePath);
    void move(std::string oldPath, std::string newPath);
  };
}

#endif /* FYLE_SYSTEM_HPP */