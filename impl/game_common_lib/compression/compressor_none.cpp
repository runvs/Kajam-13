#include "compressor_none.hpp"

std::string CompressorNone::compress(std::string const& in) { return in; }
std::string CompressorNone::decompress(std::string const& in) { return in; }
