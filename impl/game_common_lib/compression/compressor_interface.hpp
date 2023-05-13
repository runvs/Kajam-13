#ifndef JAMTEMPLATE_COMPRESSOR_INTERFACE_HPP
#define JAMTEMPLATE_COMPRESSOR_INTERFACE_HPP

#include <string>

class CompressorInterface {
public:
    virtual ~CompressorInterface() = default;

    virtual std::string compress(std::string const& in) = 0;
    virtual std::string decompress(std::string const& in) = 0;
};

#endif // JAMTEMPLATE_COMPRESSOR_INTERFACE_HPP
