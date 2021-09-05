#include <string>
#include <vector>
#include <string_view>

class Unpacker
{
public: // Interface

    bool Unpack(
          std::string_view _Path
        ) const;

    bool UnpackDirectory(
          std::string_view _Path,
          std::string_view _OutputPath = ""
       ) const;

protected: // Service

    void ParseBitsquidHashedResource(
          const std::vector<unsigned char> & _FileData
        ) const;

    std::vector<unsigned char> ReadFile(std::string_view Path) const;
};
