#include "zstd_util.h"
#include <zstd.h>
#include <stdexcept>
#include <vector>
#include <fstream>
#include <CpputilsDebug.h>
#include <format.h>
#include <xml.h>


std::string decompress_zstd_file_to_string(const std::string &path)
{
    std::string in;
    if( !Tools::XML::read_file( path, in ) ) {
        throw std::runtime_error( Tools::format( "cannot read file '%s'", path ) );
    }

    unsigned long long content_size = ZSTD_getFrameContentSize(in.data(), in.size());
    if (content_size != ZSTD_CONTENTSIZE_UNKNOWN && content_size != ZSTD_CONTENTSIZE_ERROR) {
        std::vector<std::byte> out((size_t)content_size);
        size_t r = ZSTD_decompress(out.data(), out.size(), in.data(), in.size());
        if (ZSTD_isError(r)) {
            throw std::runtime_error(std::string("zstd decompress error: ") + ZSTD_getErrorName(r));
        }
        return std::string(reinterpret_cast<const char*>(out.data()), r);
    }

    // streaming fallback
    ZSTD_DStream *dstream = ZSTD_createDStream();
    if (!dstream) throw std::runtime_error("ZSTD_createDStream failed");
    size_t init = ZSTD_initDStream(dstream);
    if (ZSTD_isError(init)) {
        ZSTD_freeDStream(dstream);
        throw std::runtime_error(std::string("ZSTD_initDStream failed: ") + ZSTD_getErrorName(init));
    }

    std::string out;
    size_t outChunk = ZSTD_DStreamOutSize();
    std::vector<char> outbuf(outChunk);

    ZSTD_inBuffer input = { in.data(), in.size(), 0 };
    while (input.pos < input.size) {
        ZSTD_outBuffer output = { outbuf.data(), outbuf.size(), 0 };
        size_t ret = ZSTD_decompressStream(dstream, &output, &input);
        if (ZSTD_isError(ret)) {
            ZSTD_freeDStream(dstream);
            throw std::runtime_error(std::string("zstd decompress stream error: ") + ZSTD_getErrorName(ret));
        }
        if (output.pos) {
            out.append(outbuf.data(), output.pos);
        }
    }

    ZSTD_freeDStream(dstream);
    return out;
}

