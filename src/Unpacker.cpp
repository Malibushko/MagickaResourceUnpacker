#include "Unpacker.h"

#include <map>
#include <array>
#include <iostream>
#include <fstream>
#include <filesystem>

#include "reproc/reproc++/include/reproc++/run.hpp"
#include "MurmurHash2/MurmurHash2.h"

namespace utility
{
    static constexpr std::array BitsquidResourceNames
    {
        std::pair{ "config", ".txt" },
        std::pair{ "render_config", ".txt" },
        std::pair{ "unit", ".dat"},
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
        std::pair{"bpa", ".txt"}
    };
}

//
// Interface
//

bool Unpacker::Unpack(
    std::string_view _Path
  ) const
{
    const std::string UnpackedFile = std::string(_Path) + ".dump";

    const char * args[] = { "offzip", "-a", "-1", "-o", _Path.data(), UnpackedFile.data(), NULL };

    reproc::options options;
    options.redirect.discard = true;

    reproc::run(args, options);

    const auto & FileData = ReadFile(UnpackedFile);

    ParseBitsquidHashedResource(FileData);

    std::filesystem::remove(UnpackedFile);

    return true;
}

bool Unpacker::UnpackDirectory(
    std::string_view _Path,
    std::string_view _OutputPath
  ) const
{
    if (!std::filesystem::exists(_Path))
    {
        std::cerr << "Input path does not exist" << std::endl;
        return false;
    }

    if (!_OutputPath.empty() && !std::filesystem::exists(_OutputPath))
    {
        std::cerr << "Output path does not exist" << std::endl;
        return false;
    }

    for (const auto & FileEntry : std::filesystem::directory_iterator(_Path))
    {
        if (std::filesystem::is_directory(FileEntry))
            continue;

        Unpack(FileEntry.path().string());
    }
    return true;
}

void Unpacker::ParseBitsquidHashedResource(
    const std::vector<unsigned char> & _FileData
  ) const
{
    static_assert(sizeof(int32_t) == 4);

    struct BitsquidHeader
    {
        int RecordsCount = 0;
        char pad[0x100];
    } Header;

    std::memcpy(&Header, _FileData.data(), sizeof(BitsquidHeader));

#ifndef NDEBUG
    std::cout << "Records count: " << Header.RecordsCount << std::endl;
#endif

    struct Record
    {
        uint64_t TypeHash;
        uint64_t NameHash;
    };

    std::vector<Record> Records;
    Records.reserve(Header.RecordsCount);

    size_t Offset = sizeof(BitsquidHeader);

    for (int i = 0; i <= Header.RecordsCount; ++i)
    {
        Record Item {};
        std::memcpy(&Item, _FileData.data() + Offset, sizeof(Record));
        Records.push_back(Item);

        Offset += sizeof(Record);
    }

    std::map<uint64_t, std::string> Hashes;

    for (const auto & [Name, Format] : utility::BitsquidResourceNames)
      Hashes[MurmurHash64A(Name, strlen(Name), 0)] = Name;

    int ReadFileCounter = 0;

    while (1)
    {
        if (ReadFileCounter >= Records.size())
            break;

        struct ResourceHeader
        {
            int32_t unknown_0;
            int32_t unknown_1;
            int32_t unknown_2;
            int32_t ResourceSize;
            int32_t unknown_;
            int32_t unknown_4;
            int32_t unknown_5;
            int32_t unknown_6;
            int32_t unknown_7;
        } Item;

        std::memcpy(&Item, _FileData.data() + Offset, sizeof(ResourceHeader));

        std::cout << "Found resource with file size: " << Item.ResourceSize << std::endl;

        Offset += sizeof(ResourceHeader);

        std::vector<unsigned char> Buffer(Item.ResourceSize);
        std::memcpy(Buffer.data(), _FileData.data() + Offset, Buffer.size());

        Offset += Buffer.size();

        if (const auto ResourceName = Hashes.find(Records[ReadFileCounter].TypeHash);
                ResourceName != Hashes.end())
        {
            std::string Name = ResourceName->second;

            if (Name == "unit" || Name == "texture" || Name == "timpani_bank" ||
                    Name == "animation")

            {

            }
            else
            {
                const std::string OutFilePath = "D:\\TempTemp\\" + std::to_string(Records[ReadFileCounter].NameHash) + "." + ResourceName->second;

                std::ofstream(OutFilePath, std::ios::binary).write(reinterpret_cast<char*>(Buffer.data()), Buffer.size());

                std::cout << "Written to " << OutFilePath << std::endl;
            }
        }

        std::cout << "File read " << ReadFileCounter << std::endl;

        ReadFileCounter++;
    }
}

//
// Service
//

std::vector<unsigned char> Unpacker::ReadFile(std::string_view Path) const
{
    std::vector<unsigned char> Data;
    Data.resize(std::filesystem::file_size(Path));

    std::ifstream(Path, std::ios::binary).read(reinterpret_cast<char *>(Data.data()), Data.size());
    return Data;
}
