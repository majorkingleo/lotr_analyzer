#include "CheckSingleMail.h"
#include "ReadMailFromFile.h"
#include <CpputilsDebug.h>
#include "HtmlToText.h"
#include <utf8_util.h>
#include "zstd_util.h"
#include <stderr_exception.h>
#include <fstream>

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

     write_to_file( Tools::format( "%s_subject.txt", filename ), 
                    Tools::format( "From: %s\nTo: %s\nSubject: %s\n\n", 
                        m_mail->from.data, m_mail->to.data, m_mail->subject.data ) );    

    if( m_mail->body_text_plain.data.empty() ) {
        CPPDEBUG( Tools::format( "mail body text/plain is empty" ) );
    } else {
         write_to_file( Tools::format( "%s_plain.txt", filename ), m_mail->body_text_plain.data );
    }

    if( m_mail->body_text_html.data.empty() ) {
        CPPDEBUG( Tools::format( "mail body text/html is empty" ) );
    } else {        
        HtmlToText html_to_text;
        std::wstring html_converted = html_to_text.convert_from_mail( Utf8Util::utf8toWString( m_mail->body_text_html.data ) );

        write_to_file( Tools::format( "%s_html.txt", filename ), Utf8Util::wStringToUtf8( html_converted ) );
    }

    return true;
}

void CheckSingleMail::write_to_file( const std::string & filename, const std::string & content ) const
{
    std::ofstream file( filename );
    if( !file ) {
        throw STDERR_EXCEPTION( Tools::format( "Failed to open file '%s' for writing", filename ) );    
    }

    file << content;
}