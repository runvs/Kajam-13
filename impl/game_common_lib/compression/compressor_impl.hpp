#ifndef JAMTEMPLATE_COMPRESSOR_IMPL_HPP
#define JAMTEMPLATE_COMPRESSOR_IMPL_HPP

#include <compression/compressor_interface.hpp>

class CompressorImpl : public CompressorInterface {
public:
    std::string compress(std::string const& in) override;
    std::string decompress(std::string const& in) override;
};

#endif // JAMTEMPLATE_COMPRESSOR_IMPL_HPP
