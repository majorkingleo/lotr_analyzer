#include "ReadMailFromFile.h"
#include "zstd_util.h"
#include "qp.h"
#include <zstd.h>
#include <filesystem>
#include <CpputilsDebug.h>
#include <utf8_util.h>
#include "HtmlToText.h"

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
    CPPDEBUG( Tools::format( "subject: '%s'", mail.subject.data ) );

    mail.imap_filename.data = std::filesystem::path(filename).filename().string();

    for( const auto & part : me.body().parts() ) {
        handle_mail_part( part, mail );
    }

    if( mail.body_text_plain.data.empty() && mail.body_text_html.data.empty() ) {
        CPPDEBUG( Tools::format( "mail body is empty, trying to get body from main entity" ) );
        handle_mail_part( &me, mail );
    }

    CPPDEBUG( Tools::format( "mail from '%s' to '%s' subject '%s'", mail.from.data, mail.to.data, mail.subject.data ) );

    return mail;
}


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

std::string ReadMailFromFile::decodeBase64( const std::string & encoded )
{
    mimetic::Base64::Decoder decoder;
    std::string decoded;
    decoder.process( encoded.begin(), encoded.end(), std::back_inserter(decoded) );
    return decoded;
}

void ReadMailFromFile::handle_mail_part( const mimetic::MimeEntity * part, MAIL & mail )
{
    const std::string content_type = part->header().contentType().str();
    const std::string content_transfer_encoding = part->header().contentTransferEncoding().str();

    CPPDEBUG( Tools::format( "processing part with content type '%s' encoding '%s'", 
        content_type, content_transfer_encoding ) );

    if( part->header().contentType().str().starts_with( "text/plain" ) ) {
        mail.body_text_plain.data = part->body();

        if( content_transfer_encoding == "quoted-printable" ) {
            try {
                mail.body_text_plain.data = decodeQuotedPrintable( mail.body_text_plain.data );
            } catch( const std::exception & e ) {
                CPPDEBUG( Tools::format( "Failed to decode quoted-printable content: %s", e.what() ) );
            }
        } else if( content_transfer_encoding == "base64" ) {
            mail.body_text_plain.data = decodeBase64( mail.body_text_plain.data );
        }

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

        if( content_transfer_encoding == "quoted-printable" ) {
            try {
                mail.body_text_html.data = decodeQuotedPrintable( mail.body_text_html.data );
            } catch( const std::exception & e ) {
                CPPDEBUG( Tools::format( "Failed to decode quoted-printable content: %s", e.what() ) );
            }
        } else if( content_transfer_encoding == "base64" ) {
            mail.body_text_html.data = decodeBase64( mail.body_text_html.data );
        }
    }

    // repair html mail sent as text/plain with html content
    if( mail.body_text_plain.data.find( "<span" ) != std::string::npos || 
        mail.body_text_plain.data.find( "<div" ) != std::string::npos ) {
        CPPDEBUG( Tools::format( "mail body text/plain contains html tags, cleaning up" ) );
        mail.body_text_plain.data = Utf8Util::wStringToUtf8( HtmlToText::convert_from_mail( Utf8Util::utf8toWString( mail.body_text_plain.data ) ) );
    }
}