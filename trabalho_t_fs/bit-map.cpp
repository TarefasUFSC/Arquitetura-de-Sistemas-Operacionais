#include "bit-map.hpp"

namespace fs
{
  BitMap::BitMap() : posPointer(0) {}

  BitMap::BitMap(byte size, byte posPointer) : posPointer(posPointer), data(std::vector<byte>(size, (byte)0x00)) {}

  BitMap::BitMap(byte size, byte posPointer, std::fstream *fs) : posPointer(posPointer), data(std::vector<byte>(size))
  {
    for (byte i = 0; i < size; i++)
    {
      *fs >> data.at(i);
    }
  }

  byte BitMap::size()
  {
    return data.size();
  }

  byte BitMap::getFirstFree()
  {
    for (byte i = 0; i < data.size(); i++)
    {
      for (byte j = 0; j < sizeof(long); j++)
      {
        if ((data.at(i) & (1 << j)) == 0)
        {
          return i * sizeof(long) + j;
        }
      }
    }
    return -1;
  }

  std::vector<byte> BitMap::fill(byte qty)
  {
    std::vector<byte> result;
    for (byte i = 0; i < qty; i++)
    {
      byte tmp = getFirstFree();
      occupy(tmp);
      result.push_back(tmp);
    }
    return result;
  }

  byte BitMap::modifyBit(byte pos, byte bit, bool value)
  {
    byte mask = 1 << bit;
    return ((data.at(pos) & ~mask) | (value << bit));
  }

  void BitMap::occupy(byte posBlock)
  {
    data.at(posBlock / sizeof(long)) = modifyBit(
        posBlock / sizeof(long),
        posBlock % sizeof(long),
        1);
  }

  void BitMap::occupy(byte posVec, byte bit)
  {
    data.at(posVec) = modifyBit(posVec, bit, 1);
  }

  void BitMap::free(byte posBlock)
  {
    data.at(posBlock / sizeof(long)) = modifyBit(
        posBlock / sizeof(long),
        posBlock % sizeof(long),
        0);
  }

  void BitMap::free(byte posVec, byte bit)
  {
    data.at(posVec) = modifyBit(posVec, bit, 0);
  }

  void BitMap::save(std::fstream *fs)
  {
    fs->seekg(posPointer, std::ios_base::beg);
    for (const auto &ch : data)
    {
      *fs << ch;
    }
  }
};