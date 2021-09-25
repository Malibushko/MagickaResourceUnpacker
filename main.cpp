#include <iostream>
#include <map>
#include <vector>
#include <string>

#include "SegmentedFileDecompressor.h"

#include  "MurmurHash2/MurmurHash2.h"
int main(int argc, char ** argv)
{
  SegmentedFileDecompressor decompressor;
  decompressor.Decompress("D:\\Steam\\steamapps\\common\\Magicka 2\\data", "C:\\Temp");
  
  return 0;
}
