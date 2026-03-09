#include "ImportMail.h"
#include "App.h"
#include <chrono>
#include <thread>
#include <stdexcept>
#include <CpputilsDebug.h>
#include <utf8_util.h>
#include <xml.h>
#include <filesystem>

using namespace std::chrono_literals;
using namespace std::chrono;
using namespace Tools;


ImportMail::ImportMail()
: ReadMailFromFile(), BasicThread( "ImportMail" )
{

}

void ImportMail::run()
{	
    try {
        process();        
    } catch( const std::exception & e ) {
        CPPDEBUG( Tools::format( "Error in ImportMail thread: %s", e.what() ) );
        APP.db->rollback();
        APP.quit_request = true;
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
                    // CPPDEBUG( Tools::format( "Mail file '%s' already imported, skipping", imap_filename ) );
                    continue;
                }

                CPPDEBUG( Tools::format( "Importing mail file '%s'", filename ) );

                MAIL mail = read_mail_from_file( filename );
                mail.setHist( BASE::HIST_TYPE::HIST_AN );
                mail.setHist( BASE::HIST_TYPE::HIST_AE );
                mail.setHist( BASE::HIST_TYPE::HIST_LO );

                if( !StdSqlInsert( *APP.db, mail ) ) {
                    throw std::runtime_error( Tools::format( "cannot insert mail '%s' '%s' subject: '%s'", 
							filename, 
							APP.db->get_error(),
							mail.subject.str() ) );
                }

                APP.db->commit();
                m_imported_files.insert( imap_filename );

            } catch( const std::exception & e ) {
                CPPDEBUG( Tools::format( "Error importing mail '%s': %s", filename, e.what() ) );
            }
        }

        APP.db->rollback();

        std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
    }
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
    } // while( true )
}
