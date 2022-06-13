#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <cstring>

#include "SegmentedFile.h"

#include  "MurmurHash2/MurmurHash2.h"

#define  TEST_COMPRESS
int main(int argc, char ** argv)
{
  /*
  std::string key = "menu_state_news_layout";

  std::cout << MurmurHash64A(key.data(), key.length(), 0) << std::endl;;

  return 0;
  */

#ifdef NDEBUG

  if (argc != 4)
  {
    std::cerr << "Invalid arguments count. Example:\n"
              << argv[0] << " -c FileIn.pack FileOut.lua\n";

    return 1;
  }

#endif

  const char * mode;
  const char * file_in;
  const char * file_out;

#ifndef NDEBUG

#ifdef TEST_COMPRESS

  mode     = "-c";
  file_in  = "D:\\Games\\temp\\unpacked";
  file_out = "D:\\Games\\temp\\f8ff87786a146601";

#else

  mode = "-d";
  file_in = "D:\\Games\\temp\\_f8ff87786a146601";
  file_out = "D:\\Games\\temp\\unpacked";

#endif

#elif

  mode     = argv[1];
  file_in  = argv[2];
  file_out = argv[3];

#endif

  SegmentedFile decompressor;

  if (strcmp(mode, "-c") == 0)
  {
    if (!decompressor.Compress(file_in, file_out))
      std::cerr << "Error\n" << std::endl;
  }
  else if (strcmp(mode, "-d") == 0)
  {
    if (!decompressor.Decompress(file_in, file_out))
      std::cerr << "Error\n" << std::endl;
  }
  
  return 0;
}
