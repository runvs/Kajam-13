#include "logging_compressor.hpp"
#include <string>

LoggingCompressor::LoggingCompressor(
    jt::LoggerInterface& logger, std::shared_ptr<CompressorInterface> decoratee)
    : m_logger(logger)
    , m_decoratee(decoratee)
{
}

std::string LoggingCompressor::compress(std::string const& in)
{
    auto const compressedString = m_decoratee->compress(in);
    m_logger.verbose("compression: " + std::to_string(compressedString.size()) + " / "
            + std::to_string(in.size()),
        { "compression" });
    return compressedString;
}

std::string LoggingCompressor::decompress(std::string const& in)
{
    auto const decompressedString = m_decoratee->decompress(in);
    m_logger.verbose("compression: " + std::to_string(in.size()) + " / "
            + std::to_string(decompressedString.size()),
        { "compression" });
    return decompressedString;
}
