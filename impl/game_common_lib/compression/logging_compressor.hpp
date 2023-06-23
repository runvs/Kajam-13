#ifndef JAMTEMPLATE_LOGGING_COMPRESSOR_HPP
#define JAMTEMPLATE_LOGGING_COMPRESSOR_HPP

#include <compression/compressor_interface.hpp>
#include <log/logger_interface.hpp>
#include <memory>

class LoggingCompressor : public CompressorInterface {
public:
    LoggingCompressor(jt::LoggerInterface& logger, std::shared_ptr<CompressorInterface> decoratee);

    std::string compress(std::string const& in) override;
    std::string decompress(std::string const& in) override;

private:
    jt::LoggerInterface& m_logger;
    std::shared_ptr<CompressorInterface> m_decoratee;
};

#endif // JAMTEMPLATE_LOGGING_COMPRESSOR_HPP
