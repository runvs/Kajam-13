#ifndef JAMTEMPLATE_COMPRESSION_NONE_HPP
#define JAMTEMPLATE_COMPRESSION_NONE_HPP

#include <compression/compressor_interface.hpp>

class CompressorNone : public CompressorInterface {
public:
    std::string compress(std::string const& in) override;
    std::string decompress(std::string const& in) override;
};

#endif // JAMTEMPLATE_COMPRESSION_NONE_HPP
