#include "CheckSingleMail.h"
#include "ReadMailFromFile.h"
#include <CpputilsDebug.h>
#include "HtmlToText.h"
#include <utf8_util.h>
#include "zstd_util.h"

CheckSingleMail::CheckSingleMail( const std::string & filename )
: m_filename(filename)
{
    read_mail( filename );
}


bool CheckSingleMail::read_mail( const std::string & filename )
{
    CPPDEBUG( Tools::format( "Checking mail file '%s'", filename ) );

    ReadMailFromFile read_mail_from_file;

    if( read_mail_from_file.is_zstd_compressed( filename ) ) {
        std::string decompressed_content_str = decompress_zstd_file_to_string( filename );
        CPPDEBUG( Tools::format( "Decompressed content of mail file '%s':\n\n%s\n\n", filename, decompressed_content_str ) );
    }


    m_mail = read_mail_from_file.read_mail_from_file( filename );

        
    CPPDEBUG( Tools::format( "mail from '%s' to '%s' subject '%s'", m_mail->from.data, m_mail->to.data, m_mail->subject.data ) );

    if( m_mail->body_text_plain.data.empty() ) {
        CPPDEBUG( Tools::format( "mail body text/plain is empty" ) );
    } else {
        CPPDEBUG( Tools::format( "mail body text/plain:\n\n%s\n\n", m_mail->body_text_plain.data ) );
    }

    if( m_mail->body_text_html.data.empty() ) {
        CPPDEBUG( Tools::format( "mail body text/html is empty" ) );
    } else {
        CPPDEBUG( Tools::format( "mail body text/html:\n\n%s\n\n", m_mail->body_text_html.data ) );


        HtmlToText html_to_text;
        std::wstring html_converted = html_to_text.convert_from_mail( Utf8Util::utf8toWString( m_mail->body_text_html.data ) );

        CPPDEBUG( Tools::wformat( L"mail body text/html converted to text:\n\n%s\n\n", html_converted ) );
    }

    return true;
}