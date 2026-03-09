#include "ReadMailFromFile.h"
#include "zstd_util.h"
#include "qp.h"
#include <mimetic/mimetic.h>
#include <zstd.h>
#include <filesystem>
#include <CpputilsDebug.h>
#include <utf8_util.h>

using namespace mimetic;
using namespace Tools;

MAIL ReadMailFromFile::read_mail_from_file( const std::string & filename )
{
    MimeEntity me{};

    if( is_zstd_compressed( filename ) ) {

        std::string decompressed_content_str = decompress_zstd_file_to_string( filename );

        me.load( decompressed_content_str.begin(), decompressed_content_str.end() );
    } else {
        File content( filename );
        me.load( content.begin(), content.end() );
    }

    

    MAIL mail{};
    
    
    mail.from.data      = me.header().field( "From" ).value();
    mail.to.data        = me.header().field( "To" ).value();

    std::string subject  = me.header().field( "Subject" ).value();
    mail.subject.data   = decodeMimeSubject( subject );

    mail.imap_filename.data = std::filesystem::path(filename).filename().string();

    for( const auto & part : me.body().parts() ) {

        std::string content_type = part->header().contentType().str();        

        CPPDEBUG( Tools::format( "processing part with content type '%s'", content_type ) );

        if( part->header().contentType().str().starts_with( "text/plain" ) ) {
            mail.body_text_plain.data = part->body();
            // CPPDEBUG( Tools::format( "found text/plain part in mail file '%s'", filename ) );

            // CPPDEBUG( Tools::format( "found text/plain part in mail file '%s'", filename ) );
            // CPPDEBUG( Tools::format( "mail body: '%s'", mail.body.data ) );
            // CPPDEBUG( Tools::wformat( L"mail body: '%s'", content ) );
        } else if( part->header().contentType().str().starts_with("text/html" ) ) {
            /*
            CPPDEBUG( Tools::format( "found text/html part in mail file '%s'", filename ) );
            CPPDEBUG( Tools::format( "mail body: '%s'", part->body() ) );
            */
           mail.body_text_html.data = part->body();
        } 
    }

    CPPDEBUG( Tools::format( "mail from '%s' to '%s' subject '%s'", mail.from.data, mail.to.data, mail.subject.data ) );

    return mail;
}
/*
std::wstring ReadMailFromFile::get_header( const std::vector<std::wstring_view> & content_lines, const std::wstring & header_name )
{
    const std::wstring header_prefix = header_name + L": ";

    for( const auto & line : content_lines ) {
        if( line.starts_with( header_prefix ) ) {
            return std::wstring( line.substr( header_prefix.size() ) );
        }
    }

    throw std::runtime_error( Tools::format( "header '%s' not found in mail file", Utf8Util::wStringToUtf8( header_name ) ) );

    return {};
}
*/

bool ReadMailFromFile::is_zstd_compressed( const std::string & filename )
{
    std::ifstream file( filename, std::ios::binary );
    if( !file ) {
        throw std::runtime_error( Tools::format( "cannot open file '%s'", filename ) );
    }

    uint32_t file_magic;

    file.read( reinterpret_cast<char*>(&file_magic), sizeof(file_magic) );

    if( !file ) {
        throw std::runtime_error( Tools::format( "cannot read from file '%s'", filename ) );
    }

    return file_magic == ZSTD_MAGICNUMBER;
}
