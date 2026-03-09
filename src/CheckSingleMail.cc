#include "CheckSingleMail.h"
#include "ReadMailFromFile.h"
#include <CpputilsDebug.h>

CheckSingleMail::CheckSingleMail( const std::string & filename )
: m_filename(filename)
{
    read_mail( filename );
}


bool CheckSingleMail::read_mail( const std::string & filename )
{
    CPPDEBUG( Tools::format( "Checking mail file '%s'", filename ) );

    ReadMailFromFile read_mail_from_file;
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
    }

    return true;
}