#pragma once

#include "read_file.h"
#include "bindtypes.h"

class ReadMailFromFile
{
public:
    MAIL read_mail_from_file( const std::string & filename );

    //std::wstring get_header( const std::vector<std::wstring_view> & content_lines, const std::wstring & header_name );
    bool is_zstd_compressed( const std::string & filename );
};