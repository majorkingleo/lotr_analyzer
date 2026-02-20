#include "Grep4Data.h"
#include <CpputilsDebug.h>
#include "App.h"
#include <regex>
#include <string_utils.h>
#include <utf8_util.h>
#include <ConfigRules.h>
#include <ConfigGlobal.h>
#include <filesystem>
#include <format>

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

    while( !APP.quit_request ) {

        std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
        APP.db->rollback();

        reload_rules_if_needed();

        count = StdSqlSelect( *APP.db,
    					Tools::format( "select %%%s from %s where %s = 0", 
                            mail.get_table_name(), 
                            mail.get_table_name(),
                            mail.checked.get_name() ),
                        DBInArrayList() >> filenames, limit );

        // CPPDEBUG( Tools::format( "Found %d mails to check", count ) );

        if( count < 0 ) {
            throw std::runtime_error( Tools::format( "SqlError: %s", APP.db->get_error() ) );
        }

        if( count == 0 ) {
            continue;
        }

        for( int i = 0; i < count; i++ ) {
            MAIL & mail = filenames[i];
            mail.checked = 1;

            auto rule = grep( mail );

            if( rule ) {
                mail.found = 1;
                mail.mailto = Utf8Util::wStringToUtf8( rule->on_match );
            }

            if( !StdSqlUpdate( *APP.db, mail, Tools::format( "where `%s` = '%d'", mail.idx.get_name(), mail.idx.data ) ) ) {
                throw std::runtime_error( Tools::format( "SqlError: %s", APP.db->get_error() ) );
            }
            APP.db->commit();
        }        

    } // while( !APP.quit_request )
}

const ParseRules::value_type Grep4Data::grep( const MAIL & mail )
{
    if( auto rule = grep( Utf8Util::utf8toWString( mail.subject.data ) ) ) {
        CPPDEBUG( Tools::format( "Found in subject: %s", mail.subject.data ) );
        return rule;
    }

    if( auto rule = grep( Utf8Util::utf8toWString( mail.body_text_plain.data ) ) ) {
        CPPDEBUG( Tools::format( "Found in body_text_plain: %s", mail.body_text_plain.data ) );
        return rule;
    }

    if( auto rule = grep( Utf8Util::utf8toWString( mail.body_text_html.data ) ) ) {
        CPPDEBUG( Tools::format( "Found in body_text_html: %s", mail.body_text_html.data ) );
        return rule;
    }

    return {};
}

const ParseRules::value_type Grep4Data::grep( const std::wstring_view & data )
{
    for( const auto & rule : m_rules ) {
        try {
            const std::wregex re( rule->match );

            if( std::regex_search( data.begin(), data.end(), re ) ) {
                CPPDEBUG( Tools::wformat( L"Rule '%s' matched", rule->name ) );
                return rule;
            }
        } catch( const std::exception & e ) {
            CPPDEBUG( Tools::format( "Error in regex for rule '%s': %s", 
                            Utf8Util::wStringToUtf8(rule->name), 
                            e.what() ) );
        }
    }

    return {};
}

void Grep4Data::reload_rules_if_needed()
{
    const ConfigSectionRules & cfg_rules = Configfile2::get( ConfigSectionRules::KEY );

    auto last_write = std::filesystem::last_write_time( cfg_rules.RulesFile.value );

    if( last_write == std::filesystem::file_time_type::min() ) {
        CPPDEBUG( Tools::format( "Rules file '%s' does not exist!", cfg_rules.RulesFile.value.c_str() ) );
        return;
    }

    if( last_write > m_last_rules_load_time ) {
        CPPDEBUG( Tools::format( "Rules file '%s' has been modified, reloading...", cfg_rules.RulesFile.value.c_str() ) );
        m_rules = ParseRules( cfg_rules.RulesFile.value ).parse();
        m_last_rules_load_time = last_write;

        ParseRules parser( cfg_rules.RulesFile.value );
        m_rules = parser.parse();
    }
}