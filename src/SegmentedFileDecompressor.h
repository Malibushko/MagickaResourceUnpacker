#pragma once
#include <string>
#include <vector>
#include <map>

class SegmentedFileDecompressor
{
public: // Interface

  bool Decompress(
      const std::string & _Folder,
      const std::string & _OutputFolder
	);

protected: // Service

  std::vector<unsigned char> ReadSegmentCompressedFile(
      const std::string & _FileName
    ) const;

  int32_t UnpackBitsquidPackage(
      const std::vector<unsigned char> & _Data,
      const std::string &                _OutPath
    );

  std::string GetFileTypeByHash(
      const uint64_t _TypeHash
    );

protected: // Members

	std::string                     m_Folder;
  std::map<uint64_t, std::string> m_TypeHashes;
};