#include "ImportMail.h"
#include "App.h"
#include <chrono>
#include <thread>
#include <stdexcept>
#include <CpputilsDebug.h>
#include <filesystem>
#include <utf8_util.h>
#include <mimetic/mimetic.h>

using namespace std::chrono_literals;
using namespace std::chrono;
using namespace Tools;
using namespace mimetic;


ImportMail::ImportMail()
: BasicThread( "ImportMail" )
{

}

void ImportMail::run()
{	
    try {
        process();        
    } catch( const std::exception & e ) {
        CPPDEBUG( Tools::format( "Error in ImportMail thread: %s", e.what() ) );
        APP.db->rollback();
    }
}

void ImportMail::process()
{	
    const ConfigSectionMailImport & cfg_mail_import = Configfile2::get(ConfigSectionMailImport::KEY);

    read_already_imported_files();

    while( !APP.quit_request ) 
    {
        for( const auto & entry : std::filesystem::directory_iterator( cfg_mail_import.ImportDirectory.value ) ) {

            const std::string filename = entry.path().string();

            try {

                const std::string imap_filename = std::filesystem::path(filename).filename().string();
                if( m_imported_files.find( imap_filename ) != m_imported_files.end() ) {
                    CPPDEBUG( Tools::format( "Mail file '%s' already imported, skipping", filename ) );
                    continue;
                }

                CPPDEBUG( Tools::format( "Importing mail file '%s'", filename ) );

                MAIL mail = read_mail_from_file( filename );
                mail.setHist( BASE::HIST_TYPE::HIST_AN );
                mail.setHist( BASE::HIST_TYPE::HIST_AE );
                mail.setHist( BASE::HIST_TYPE::HIST_LO );

                if( !StdSqlInsert( *APP.db, mail ) ) {
                    throw std::runtime_error( Tools::format( "cannot insert mail '%s' '%s'", filename, APP.db->get_error() ) );
                }

                APP.db->commit();
                m_imported_files.insert( imap_filename );

            } catch( const std::exception & e ) {
                CPPDEBUG( Tools::format( "Error importing mail '%s': %s", filename, e.what() ) );
            }
        }

        APP.db->commit();

        std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
    }
}

MAIL ImportMail::read_mail_from_file( const std::string & filename )
{    
    std::wstring content;

    if( !m_read_file.read_file( filename, content ) ) {
        throw std::runtime_error( Tools::format( "cannot read mail file '%s': %s", filename, m_read_file.getError() ) );
    }

    MAIL mail{};

    MimeEntity me{};
    me.load( content.begin(), content.end() );
    mail.from.data      = me.header().field( "From" ).value();
    mail.to.data        = me.header().field( "To" ).value();
    mail.subject.data   = me.header().field( "Subject" ).value();
    mail.imap_filename.data = std::filesystem::path(filename).filename().string();

    for( auto & part : me.body().parts() ) {

        CPPDEBUG( Tools::format( "processing part with content type '%s'", part->header().contentType().str() ) );

        if( part->header().contentType().str().starts_with( "text/plain" ) ) {
            mail.body.data = part->body();

            // CPPDEBUG( Tools::format( "found text/plain part in mail file '%s'", filename ) );
            // CPPDEBUG( Tools::format( "mail body: '%s'", mail.body.data ) );
        } /* else if( part->header().contentType().str().starts_with("text/html" ) ) {
            CPPDEBUG( Tools::format( "found text/html part in mail file '%s'", filename ) );
            CPPDEBUG( Tools::format( "mail body: '%s'", part->body() ) );
        } */
    }

    CPPDEBUG( Tools::format( "mail from '%s' to '%s' subject '%s'", mail.from.data, mail.to.data, mail.subject.data ) );

    return mail;
}

std::wstring ImportMail::get_header( const std::vector<std::wstring_view> & content_lines, const std::wstring & header_name )
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

void ImportMail::read_already_imported_files()
{
    MAIL filenames[100];
    DBInLimit limit(100);
    MAIL mail{};
    int count = 0;

    while( true ) {
        count = StdSqlSelect( *APP.db,
    					Tools::format( "select %s from %s", mail.imap_filename.get_name(), mail.get_table_name() ),
                        DBInArrayList() >> filenames, limit );

        if( count < 0 ) {
            throw std::runtime_error( Tools::format( "SqlError: %s", APP.db->get_error() ) );
        }

        if( count == 0 ) {
            break;
        }

        for( int i = 0; i < count; i++ ) {
            m_imported_files.insert( filenames[i].imap_filename.data );
        }        
    }
}