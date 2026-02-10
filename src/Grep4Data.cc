#include "Grep4Data.h"
#include <CpputilsDebug.h>
#include "App.h"
#include <regex>
#include <string_utils.h>
#include <utf8_util.h>

using namespace Tools;

Grep4Data::Grep4Data()
 : BasicThread( "Grep4Data" )
{

}

void Grep4Data::run()
{
    try {
        process();
    } catch( const std::exception & e ) {
        CPPDEBUG( Tools::format( "Error in Grep4Data thread: %s", e.what() ) );
        APP.db->rollback();
    }
}

void Grep4Data::process()
{
    MAIL filenames[100];
    DBInLimit limit(100);
    MAIL mail{};
    int count = 0;

    while( true ) {
        count = StdSqlSelect( *APP.db,
    					Tools::format( "select %%%s from %s where %s = 0", 
                            mail.get_table_name(), 
                            mail.get_table_name(),
                            mail.checked.get_name() ),
                        DBInArrayList() >> filenames, limit );

        if( count < 0 ) {
            throw std::runtime_error( Tools::format( "SqlError: %s", APP.db->get_error() ) );
        }

        if( count == 0 ) {
            break;
        }

        for( int i = 0; i < count; i++ ) {
            MAIL & mail = filenames[i];
            mail.checked = 1;

            if( grep( mail ) ) {                
                mail.found = 1;
            }

            if( !StdSqlUpdate( *APP.db, mail, Tools::format( "where `%s` = '%d'", mail.idx.get_name(), mail.idx.data ) ) ) {
                throw std::runtime_error( Tools::format( "SqlError: %s", APP.db->get_error() ) );
            }
            APP.db->commit();
        }
    } // while( true )
}

bool Grep4Data::grep( const MAIL & mail )
{
    if( grep( Utf8Util::utf8toWString( mail.subject.data ) ) ) {
        CPPDEBUG( Tools::format( "Found in subject: %s", mail.subject.data ) );
        return true;
    }

    if( grep( Utf8Util::utf8toWString( mail.body_text_plain.data ) ) ) {
        CPPDEBUG( "Found in body_text_plain" );
        return true;
    }

    if( grep( Utf8Util::utf8toWString( mail.body_text_html.data ) ) ) {
        CPPDEBUG( "Found in body_text_html");
        return true;
    }

    return false;
}

bool Grep4Data::grep( const std::wstring_view & data )
{
    std::wregex re( L"Die Zwei Türme", std::regex_constants::icase );

    return std::regex_search( data.begin(), data.end(), re );
}