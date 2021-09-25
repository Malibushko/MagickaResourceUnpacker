#include  "SegmentedFileDecompressor.h"

#include <zstr.hpp>
#include <filesystem>
#include <fstream>
#include <cassert>
#include <map>
#include <array>

#include "MurmurHash2/MurmurHash2.h"

namespace utility
{
  constexpr size_t COMPRESSED_HEADER_SIZE = 12;
  constexpr size_t COMPRESSED_CHUNK_MAX_SIZE = 65536;
  constexpr size_t BITSQUID_PACKAGE_HEADER_SIZE = 256;

  static constexpr std::array BitsquidResourceNames
  {
      std::pair{"config", ".txt" },
      std::pair{"render_config", ".txt" },
      std::pair{"unit", ".dat"},
      std::pair{"shader_library_group", ".txt"},
      std::pair{"shader_library", ".txt"},
      std::pair{"shader", ".txt"},
      std::pair{"texture", ".txt"},
      std::pair{"material", ".txt"},
      std::pair{"animation", ".txt"},
      std::pair{"animation_curves", ".txt"},
      std::pair{"bones", ".txt"},
      std::pair{"state_machine", ".txt"},
      std::pair{"physics_properties", ".txt"},
      std::pair{"package", ".txt"},
      std::pair{"particles", ".txt"},
      std::pair{"sound_environment", ".txt"},
      std::pair{"font", ".ttf"},
      std::pair{"vaw", ".txt"},
      std::pair{"aul", ".txt"},
      std::pair{"level", ".txt"},
      std::pair{"data", ".txt"},
      std::pair{"shading_environment", ".txt"},
      std::pair{"strings", ".txt"},
      std::pair{"network_config", ".txt"},
      std::pair{"mouse_cursor", ".txt"},
      std::pair{"timpani_bank", ".txt"},
      std::pair{"flow", ".txt"},
      std::pair{"surface_properties", ".txt"},
      std::pair{"baked_lighting", ".txt"},
      std::pair{"mp4", ".txt"},
      std::pair{"ivf", ".txt"},
      std::pair{"bik", ".txt"},
      std::pair{"vector_field", ".txt"},
      std::pair{"cane", ".txt"},
      std::pair{"cane_tilecache", ".txt"},
      std::pair{"entity", ".txt"},
      std::pair{"scene", ".txt"},
      std::pair{"bpa", ".txt"},
      std::pair{"lua", ".lua"},
      std::pair{"script", ".lua"},
      std::pair{"scripts", ".lua"}
  };

  int32_t ZlibDecompress(uint8_t * in_buf, uint32_t in_size, uint8_t * out_buf, uint32_t out_size)
  {
    /* some common variables. */
    int32_t result = 0;
    int32_t tb = 0;
    z_stream z;

    /* fill the stream structure for zlib. */
    z.next_in = (Bytef *)in_buf;
    z.avail_in = (uInt)in_size;
    z.total_in = in_size;
    z.next_out = (Bytef *)out_buf;
    z.avail_out = (uInt)out_size;
    z.total_out = 0;
    z.zalloc = NULL;
    z.zfree = NULL;

    /* initialize the decompression structure, storm.dll uses zlib version 1.1.3. */
    if ((result = inflateInit(&z)) != Z_OK)
    {

      /* something on zlib initialization failed. */
      return result;
    }

    /* call zlib to decompress the data. */
    if ((result = inflate(&z, Z_FINISH)) != Z_STREAM_END)
    {

      /* something on zlib decompression failed. */
      return result;
    }

    /* save transferred bytes. */
    tb = z.total_out;

    /* cleanup zlib. */
    if ((result = inflateEnd(&z)) != Z_OK)
    {

      /* something on zlib finalization failed. */
      return result;
    }

    /* return transferred bytes. */
    return tb;
  }
};

//
// Interface
//

bool SegmentedFileDecompressor::Decompress(
    const std::string & _Folder,
    const std::string & _OutFolder
  )
{
  if (!std::filesystem::exists(_Folder) ||
      !std::filesystem::is_directory(_Folder))
  {
    return false;
  }

  m_Folder = _Folder;

  uint64_t FileCount = 0;

  for (const auto & _ : std::filesystem::directory_iterator(_Folder))
    FileCount++;

  uint64_t FileProcessed = 0;
  for (const auto & File : std::filesystem::directory_iterator(_Folder))
  {
    if (!File.is_directory())
      UnpackBitsquidPackage(ReadSegmentCompressedFile(File.path().string()), _OutFolder);

    std::cout << (float)(++FileProcessed) / FileCount * 100 << "% completed" << std::endl;
  }

  return true;
}

//
// Service
//

std::vector<unsigned char> SegmentedFileDecompressor::ReadSegmentCompressedFile(
    const std::string & _FileName
  ) const
{
  const int32_t              FileSize = std::filesystem::file_size(_FileName);
  std::ifstream              FileStream(_FileName, std::ios::binary);
  
  assert(FileStream.is_open() && "[Unexpected]: Cannot open stream");

  // Skip header
  struct Header
  {
    int32_t Version;
    int32_t Magic1;
    int32_t Magic2;
  } Header;

  FileStream.read(reinterpret_cast<char*>(&Header), sizeof(Header));

  std::vector<unsigned char> Data;

  for (uint32_t ReadCount = 0; ReadCount < FileSize; )
  {
    uint32_t CompressedChunkSize = 0;
    FileStream.read(reinterpret_cast<char *>(&CompressedChunkSize), sizeof(CompressedChunkSize));

    std::vector<uint8_t> OutBuffer(66000);

    if (CompressedChunkSize == utility::COMPRESSED_CHUNK_MAX_SIZE)
    {
      FileStream.read(reinterpret_cast<char*>(OutBuffer.data()), CompressedChunkSize);
      std::copy(OutBuffer.begin(), OutBuffer.begin() + CompressedChunkSize, std::back_inserter(Data));
    }
    else
    {
      std::vector<uint8_t> InputBuffer(CompressedChunkSize);
      FileStream.read(reinterpret_cast<char*>(InputBuffer.data()), CompressedChunkSize);

      const int32_t UncompressedSize = utility::ZlibDecompress(InputBuffer.data(), InputBuffer.size(), OutBuffer.data(), utility::COMPRESSED_CHUNK_MAX_SIZE);

      if (UncompressedSize > 0)
        std::copy(OutBuffer.begin(), OutBuffer.begin() + UncompressedSize, std::back_inserter(Data));
    }

    ReadCount += sizeof(int32_t) + CompressedChunkSize;
    FileStream.seekg(ReadCount + utility::COMPRESSED_HEADER_SIZE, std::ios::beg);
  }

  return Data;
}

int32_t SegmentedFileDecompressor::UnpackBitsquidPackage(
    const std::vector<unsigned char> & _Data,
    const std::string                & _OutPath
  )
{
  int32_t Offset = 0;
  auto ReadBytes = [&](auto * _Destination) mutable
  {
    const size_t Size = sizeof(std::decay_t<decltype(*_Destination)>);
    std::memcpy(_Destination, _Data.data() + Offset, Size);
    Offset += Size;
    
    return Offset;
  };

  int32_t RecordsCount = 0;
  ReadBytes(&RecordsCount);

  Offset += 256;

  struct Record
  {
    uint64_t TypeHash;
    uint64_t NameHash;
  };

  std::vector<Record> Records;
  Records.reserve(RecordsCount);

  for (int32_t i = 0; i < RecordsCount; ++i)
  {
    Record Item{};
    ReadBytes(&Item);

    Records.push_back(Item);
  }

  for (int32_t i = 0; i < RecordsCount; ++i)
  {
    struct ResourceInfo
    {
      uint64_t TypeName;
      uint64_t TypeHash;
    } ResourceInfo;

    ReadBytes(&ResourceInfo);

    struct ResourceData
    {
      int32_t _;
      int32_t FileSize;
      int32_t StreamSize;
    };

    int32_t ChunkCount = 0;
    ReadBytes(&ChunkCount);

    Offset += sizeof(int32_t); // Skip unused

    std::vector<ResourceData> ChunksInfo(ChunkCount);
    
    for (int32_t i = 0; i < ChunkCount; ++i)
    {
      ResourceData Chunk;
      ReadBytes(&Chunk);

      ChunksInfo.push_back(Chunk);
    }

    for (const auto & Chunk : ChunksInfo)
    {
      const std::string OutputFileName = _OutPath + "\\" + std::to_string(Records[i].NameHash) + "." + GetFileTypeByHash(Records[i].TypeHash);
      
      std::ofstream OutStream(OutputFileName, std::ios::binary);
      OutStream.write(reinterpret_cast<const char*>(_Data.data() + Offset), Chunk.FileSize);

      Offset += Chunk.FileSize;
    }
  }

  return RecordsCount;
}

std::string SegmentedFileDecompressor::GetFileTypeByHash(
    const uint64_t _TypeHash
  )
{
  if (m_TypeHashes.empty())
  {
    for (const auto & [Type, Format] : utility::BitsquidResourceNames)
      m_TypeHashes[MurmurHash64A(Type, static_cast<int>(strlen(Type)), 0)] = Type;
  }

  if (const auto it = m_TypeHashes.find(_TypeHash); it != m_TypeHashes.end())
    return it->second;

  return std::to_string(_TypeHash);
}
