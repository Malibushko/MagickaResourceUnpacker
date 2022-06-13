#pragma once
#include <string>
#include <vector>
#include <map>

class SegmentedFile
{
public: // Interface

  bool Decompress(
      const std::string & _InputFile,
      const std::string & _OutputFolder
	  );

  bool Compress(
      const std::string & _Folder,
      const std::string & _OutputFile
    );

protected: // Service

  std::vector<unsigned char> ReadSegmentCompressedFile(
      const std::string & _FileName
    ) const;

  void WriteFileSegmentCompressed(
      std::vector<unsigned char> & _Data,
      const std::string &          _FileName
    );

  int32_t UnpackBitsquidPackage(
      const std::vector<unsigned char> & _Data,
      const std::string &                _OutPath
    );

  std::string GetFileTypeByHash(
      const uint64_t _TypeHash
    );

protected: // Members
  
  std::map<uint64_t, std::string> m_TypeHashes;
};