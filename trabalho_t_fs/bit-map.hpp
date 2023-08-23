#ifndef BITMAP_HPP
#define BITMAP_HPP

#include <fstream>
#include <vector>
#include "constants.hpp"

namespace fs
{
  class BitMap
  {
  private:
    byte posPointer;
    std::vector<byte> data;
    byte modifyBit(byte pos, byte bit, bool value);

  public:
    BitMap();
    BitMap(byte size, byte posPointer);
    BitMap(byte size, byte posPointer, std::fstream *fs);
    byte size();
    byte getFirstFree();
    std::vector<byte> fill(byte qty);
    void occupy(byte posBlock);
    void occupy(byte pos, byte bit);
    void free(byte posBlock);
    void free(byte pos, byte bit);
    void save(std::fstream *fs);
  };
};

#endif /* BITMAP_HPP */