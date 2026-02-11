#include "zstd_util.h"
#include <zstd.h>
#include <stdexcept>
#include <vector>
#include <fstream>
#include <CpputilsDebug.h>
#include <format.h>
#include <xml.h>

namespace {

    class AutoFree
    {
    private:
        ZSTD_DStream *m_ptr;

    public:
        AutoFree( ZSTD_DStream * ptr ) 
        : m_ptr(ptr) 
        {

        }
        
        ~AutoFree() 
        { 
            ZSTD_freeDStream(m_ptr);
            m_ptr = nullptr;
        }

        operator ZSTD_DStream*()
        {
            return m_ptr;
        }

        bool operator!() const
        {
            return m_ptr == nullptr;
        }
    };

} // namespace

std::string decompress_zstd_file_to_string(const std::string &path)
{
    std::string in;
    if( !Tools::XML::read_file( path, in ) ) {
        throw std::runtime_error( Tools::format( "cannot read file '%s'", path ) );
    }

    const auto content_size = ZSTD_getFrameContentSize(in.data(), in.size());
    if (content_size != ZSTD_CONTENTSIZE_UNKNOWN && content_size != ZSTD_CONTENTSIZE_ERROR) {
        std::vector<std::byte> out((size_t)content_size);
        size_t r = ZSTD_decompress(out.data(), out.size(), in.data(), in.size());
        if (ZSTD_isError(r)) {
            throw std::runtime_error(Tools::format("zstd decompress error: %s", ZSTD_getErrorName(r)));
        }
        return std::string(reinterpret_cast<const char*>(out.data()), r);
    }

    // streaming fallback
    AutoFree dstream = ZSTD_createDStream();
    if (!dstream) {
        throw std::runtime_error("ZSTD_createDStream failed");
    }

    size_t init = ZSTD_initDStream(dstream);
    if (ZSTD_isError(init)) {
        throw std::runtime_error(std::string("ZSTD_initDStream failed: ") + ZSTD_getErrorName(init));
    }

    std::string out;
    size_t outChunk = ZSTD_DStreamOutSize();
    std::vector<char> outbuf(outChunk);

    ZSTD_inBuffer input = { in.data(), in.size(), 0 };
    
    while (input.pos < input.size) 
    {
        ZSTD_outBuffer output = { outbuf.data(), outbuf.size(), 0 };
        size_t ret = ZSTD_decompressStream(dstream, &output, &input);

        if (ZSTD_isError(ret)) {
            throw std::runtime_error(Tools::format("zstd decompress stream error: %s", ZSTD_getErrorName(ret)));
        }

        if (output.pos) {
            out.append(outbuf.data(), output.pos);
        }
    }

    return out;
}

