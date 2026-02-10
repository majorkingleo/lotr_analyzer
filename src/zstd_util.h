/* zstd_util.h
 * Small helper to decompress a zstd-compressed file to a std::string.
 */
#ifndef TOOLS_zstd_util_h
#define TOOLS_zstd_util_h

#include <string>


/** Decompress the zstd-compressed file at `path` and return the uncompressed bytes
 *  as a std::string. Throws std::runtime_error on error. */
std::string decompress_zstd_file_to_string(const std::string &path);


#endif // TOOLS_zstd_util_h
