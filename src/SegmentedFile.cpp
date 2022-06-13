#include  "SegmentedFile.h"

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
  constexpr uint8_t records_header[] = {0x0D, 0x61, 0xEB, 0x8E, 0x03, 0xEE, 0xD3, 0x92, 0x3D, 0x40, 0x19, 0x7E, 0xD1, 0xB5, 0xD7, 0xBB, 0x62, 0xD2, 0xF5, 0x13, 0x78, 0x25, 0xE1, 0x11, 0xDF, 0xDE, 0x6A, 0x87, 0x97, 0xB4, 0xC0, 0xEA, 0xD1, 0x9F, 0x14, 0x4E, 0xCD, 0x1A, 0xFB, 0xE2, 0xF4, 0x6C, 0x16, 0x55, 0xAA, 0x57, 0x88, 0x0F, 0xE4, 0x26, 0x23, 0xDC, 0x1F, 0xF6, 0xA0, 0xFE, 0x24, 0xD6, 0x32, 0x37, 0xD1, 0xB4, 0x8F, 0xAA, 0xAA, 0x4F, 0x98, 0xF7, 0x42, 0x68, 0x80, 0x31, 0x66, 0x7F, 0x95, 0x77, 0xED, 0x18, 0xBB, 0xC5, 0x44, 0x2C, 0x43, 0x07, 0xEC, 0xC3, 0x39, 0xBA, 0x2D, 0x97, 0x4D, 0x46, 0x39, 0x7D, 0xA3, 0xC8, 0xD7, 0x42, 0x52, 0xFC, 0x2E, 0x2F, 0x5E, 0xA9, 0x44, 0x0A, 0x3A, 0xC4, 0x68, 0xCC, 0xF9, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  static std::array<std::pair<std::string, std::string>, 41> BitsquidResourceNames
  {
      std::pair<std::string, std::string>{"config", "" },
      std::pair<std::string, std::string>{"render_config", "" },
      std::pair<std::string, std::string>{"unit", ""},
      std::pair<std::string, std::string>{"shader_library_group", ""},
      std::pair<std::string, std::string>{"shader_library", ""},
      std::pair<std::string, std::string>{"shader", ""},
      std::pair<std::string, std::string>{"texture", ""},
      std::pair<std::string, std::string>{"material", ""},
      std::pair<std::string, std::string>{"animation", ""},
      std::pair<std::string, std::string>{"animation_curves", ""},
      std::pair<std::string, std::string>{"bones", ""},
      std::pair<std::string, std::string>{"state_machine", ""},
      std::pair<std::string, std::string>{"physics_properties", ""},
      std::pair<std::string, std::string>{"package", ""},
      std::pair<std::string, std::string>{"particles", ""},
      std::pair<std::string, std::string>{"sound_environment", ""},
      std::pair<std::string, std::string>{"font", ""},
      std::pair<std::string, std::string>{"vaw", ""},
      std::pair<std::string, std::string>{"aul", ""},
      std::pair<std::string, std::string>{"level", ""},
      std::pair<std::string, std::string>{"data", ""},
      std::pair<std::string, std::string>{"shading_environment", ""},
      std::pair<std::string, std::string>{"strings", ""},
      std::pair<std::string, std::string>{"network_config", ""},
      std::pair<std::string, std::string>{"mouse_cursor", ""},
      std::pair<std::string, std::string>{"timpani_bank", ""},
      std::pair<std::string, std::string>{"flow", ""},
      std::pair<std::string, std::string>{"surface_properties", ""},
      std::pair<std::string, std::string>{"baked_lighting", ""},
      std::pair<std::string, std::string>{"mp4", ""},
      std::pair<std::string, std::string>{"ivf", ""},
      std::pair<std::string, std::string>{"bik", ""},
      std::pair<std::string, std::string>{"vector_field", ""},
      std::pair<std::string, std::string>{"cane", ""},
      std::pair<std::string, std::string>{"cane_tilecache", ""},
      std::pair<std::string, std::string>{"entity", ""},
      std::pair<std::string, std::string>{"scene", ""},
      std::pair<std::string, std::string>{"bpa", ""},
      std::pair<std::string, std::string>{"lua", ""},
      std::pair<std::string, std::string>{"script", ""},
      std::pair<std::string, std::string>{"scripts", ""}
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

  int32_t ZlibCompress(uint8_t * in_buf, uint32_t in_size, uint8_t * out_buf, uint32_t out_size)
  {
    int32_t result = 0;
    int32_t tb = 0;
    z_stream z;

    z.zalloc = Z_NULL;
    z.zfree = Z_NULL;
    z.opaque = Z_NULL;

    // setup "b" as the input and "c" as the compressed output
    z.next_in = (Bytef *)in_buf;
    z.avail_in = (uInt)in_size;
    z.total_in = in_size;
    z.next_out = (Bytef *)out_buf;
    z.avail_out = (uInt)out_size;
    z.total_out = 0;
    z.zalloc    = NULL;
    z.zfree    = NULL;

    /* initialize the decompression structure, storm.dll uses zlib version 1.1.3. */
    if ((result = deflateInit(&z, Z_DEFAULT_COMPRESSION)) != Z_OK)
    {
      /* something on zlib initialization failed. */
      return result;
    }

    /* call zlib to decompress the data. */
    if ((result = deflate(&z, Z_FINISH)) != Z_STREAM_END)
    {
      /* something on zlib decompression failed. */
      return result;
    }

    /* save transferred bytes. */
    tb = z.total_out;

    /* cleanup zlib. */
    if ((result = deflateEnd(&z)) != Z_OK)
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

bool SegmentedFile::Decompress(
    const std::string & _InputFile,
    const std::string & _OutFolder
  )
{
  if (!std::filesystem::exists(_InputFile))
    return false;

  UnpackBitsquidPackage(ReadSegmentCompressedFile(_InputFile), _OutFolder);
  
  return true;
}

bool SegmentedFile::Compress(
    const std::string & _Folder, 
    const std::string & _OutputFile
  )
{
  if (!std::filesystem::exists(_Folder) || !std::filesystem::is_directory(_Folder))
    return false;

  std::vector<char> Data;

  const auto IsHash = [](const std::string & _Data) -> bool
  {
    return std::all_of(_Data.cbegin(), _Data.cend(), [](char _Char)
    {
      return std::isdigit(_Char);
    });
  };

  struct Record
  {
    uint64_t  TypeHash;
    uint64_t  NameHash;
    uint64_t  ChunkCount;

    struct Chunk
    {
      int32_t _;
      int32_t FileSize;
      int32_t FileSizeHighBits;
    };

    std::vector<Chunk>          Chunks;
    std::vector<unsigned char>  Data;
  };

  std::vector<Record> Records;

  for (auto & DirectoryEntry : std::filesystem::directory_iterator(_Folder))
  {
    if (std::filesystem::is_directory(DirectoryEntry))
      continue;

    Record Record;

    // For now assume names are always hashes
    Record.NameHash = std::stoull(DirectoryEntry.path().filename());

    const std::string Extention     = DirectoryEntry.path().extension().string();
    const std::string ExtentionName = Extention.c_str() + 1;
    
    if (IsHash(ExtentionName))
      Record.TypeHash = std::stoull(ExtentionName);
    else
      Record.TypeHash = MurmurHash64A(Extention.c_str(), ExtentionName.length(), 0);

    // For now assume there's only 1 chunk for each file
    Record.Chunks     = std::vector{Record::Chunk{ 0, (int32_t)std::filesystem::file_size(DirectoryEntry), 0 }};
    Record.ChunkCount = Record.Chunks.size();

    Record.Data.resize(std::filesystem::file_size(DirectoryEntry));
    std::ifstream(DirectoryEntry.path().string(), std::ios::binary).read((char *)Record.Data.data(), Record.Data.size());
    
    Records.push_back(std::move(Record));
  }

  // Sort because Bitsquid sorts it
  std::sort(Records.begin(), Records.end(), [](const Record & lhs, const Record & rhs)
  {
    return std::tie(lhs.TypeHash, lhs.NameHash) < std::tie(rhs.TypeHash, rhs.NameHash);
  });

  std::ofstream OutFile(_OutputFile, std::ios::binary);

  uint32_t RecordsCount = Records.size();

  OutFile.write((const char *)&RecordsCount, sizeof(RecordsCount));

  for (auto record_data : utility::records_header)
  {
    OutFile.write((const char*)&record_data, sizeof(record_data));
  }

  for (const auto & Record : Records)
  {
    OutFile.write((const char *)&Record.TypeHash, sizeof(Record.TypeHash));
    OutFile.write((const char *)&Record.NameHash, sizeof(Record.NameHash));
  }

  for (const auto & Record : Records)
  {
    OutFile.write((const char *)&Record.TypeHash, sizeof(Record.TypeHash));
    OutFile.write((const char *)&Record.NameHash, sizeof(Record.NameHash));
    OutFile.write((const char *)&Record.ChunkCount, sizeof(Record.ChunkCount));
    
    for (const auto & Chunk : Record.Chunks)
    {
      OutFile.write((const char *)&Chunk._, sizeof(Chunk._));
      OutFile.write((const char *)&Chunk.FileSize, sizeof(Chunk.FileSize));
      OutFile.write((const char *)&Chunk.FileSizeHighBits, sizeof(Chunk.FileSizeHighBits));
    }

    for (const auto & Chunk : Record.Chunks)
      OutFile.write((char*)Record.Data.data(), Record.Data.size());
  }

  OutFile.close();

  std::vector<unsigned char> FileData;
  FileData.resize(std::filesystem::file_size(_OutputFile));

  std::ifstream(_OutputFile, std::ios::binary).read((char *)FileData.data(), FileData.size());

  WriteFileSegmentCompressed(FileData, _OutputFile + "_packed");

  return true;
}

//
// Service
//

std::vector<unsigned char> SegmentedFile::ReadSegmentCompressedFile(
    const std::string & _FileName
  ) const
{
  const int32_t FileSize = std::filesystem::file_size(_FileName);
  std::ifstream FileStream(_FileName, std::ios::binary);
  /*
  std::vector<unsigned char> Data_;
  Data_.resize(FileSize);

  FileStream.read((char*)Data_.data(), FileSize);

  return Data_;
  */
  assert(FileStream.is_open() && "[Unexpected]: Cannot open stream");

  struct Header
  {
    int32_t Version;
    int32_t FileSizeUncompressedProbably;
    int32_t FileSizeHighPart;
  } Header;

  // Skip header
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

void SegmentedFile::WriteFileSegmentCompressed(
    std::vector<unsigned char> & _Data,
    const std::string &          _FileName
  )
{
  std::ofstream FileStream(_FileName, std::ios::binary);

  struct Header
  {
    uint32_t Version;
    uint32_t FileSizeUncompressedProbably;
    uint32_t FileSizeHighPart;
  } Header;

  Header.Version           = 0xF0000004; // Always that value
  Header.FileSizeHighPart  = (uint32_t)(_Data.size() * 1.25);
  Header.FileSizeHighPart  = 0;

  FileStream.write((const char *)&Header, sizeof(struct Header));

  int i;

  std::vector<unsigned char> Buffer(utility::COMPRESSED_CHUNK_MAX_SIZE * 2);

  for (i = 0; i < _Data.size(); i += utility::COMPRESSED_CHUNK_MAX_SIZE)
  {
    int size = (i + utility::COMPRESSED_CHUNK_MAX_SIZE >= _Data.size()) ? _Data.size() - i : utility::COMPRESSED_CHUNK_MAX_SIZE;

    int32_t CompressedSize = utility::ZlibCompress(_Data.data() + i, size, Buffer.data(), Buffer.size());

    if (CompressedSize >= utility::COMPRESSED_CHUNK_MAX_SIZE)
    {
      CompressedSize = utility::COMPRESSED_CHUNK_MAX_SIZE;

      FileStream.write((const char *)&CompressedSize, sizeof(int32_t));
      FileStream.write((const char*)(_Data.data() + size), utility::COMPRESSED_CHUNK_MAX_SIZE);
    }
    else
    {
      FileStream.write((const char *)&CompressedSize, sizeof(int32_t));
      FileStream.write((const char *)Buffer.data(), CompressedSize);
    }
  }
}

int32_t SegmentedFile::UnpackBitsquidPackage(
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
      uint64_t NameHash;
    } ResourceInfo;

    ReadBytes(&ResourceInfo);

    struct ResourceData
    {
      int32_t _;
      int32_t FileSize;
      int32_t FileSizeLower;
    };
    
    int64_t ChunkCount = 0;
    ReadBytes(&ChunkCount);

    std::vector<ResourceData> ChunksInfo(ChunkCount);
    
    for (int32_t i = 0; i < ChunkCount; ++i)
    {
      ResourceData Chunk;
      ReadBytes(&Chunk);

      ChunksInfo[i] = Chunk;
    }

    for (const auto & Chunk : ChunksInfo)
    {
      assert(Chunk.FileSize > 0);

      const std::string OutputFileName = _OutPath + "\\" + std::to_string(Records[i].NameHash) + GetFileTypeByHash(Records[i].TypeHash);
      
      std::ofstream OutStream(OutputFileName, std::ios::binary);
      OutStream.write(reinterpret_cast<const char*>(_Data.data() + Offset), Chunk.FileSize);

      Offset += Chunk.FileSize;
    }
  }

  return RecordsCount;
}

std::string SegmentedFile::GetFileTypeByHash(
    const uint64_t _TypeHash
  )
{
  if (m_TypeHashes.empty())
  {
    for (const auto & [Type, Format] : utility::BitsquidResourceNames)
      m_TypeHashes[MurmurHash64A(Type.c_str(), Type.length(), 0)] = Type;
  }

  if (const auto it = m_TypeHashes.find(_TypeHash); it != m_TypeHashes.end())
  {
    for (const auto & [Hash, FileType]: utility::BitsquidResourceNames)
    {
      if (Hash == it->second && !FileType.empty())
        return FileType;
    }
  }

  return "." + std::to_string(_TypeHash);
}
