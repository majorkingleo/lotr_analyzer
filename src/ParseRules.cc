#include "ParseRules.h"
#include <CpputilsDebug.h>
#include <read_file.h>
#include <format.h>
#include <stderr_exception.h>
#include <string_utils.h>
#include <utf8_util.h>
#include <regex>

using namespace Tools;

ParseRules::ParseRules( const std::string_view & rules_file )
: m_rules_file(rules_file)
{

}


std::vector<std::shared_ptr<ParseRules::Rule>> ParseRules::parse() 
{   
    std::wstring rules_content;

    if( !READ_FILE.read_file(m_rules_file, rules_content) ) {
        throw STDERR_EXCEPTION( Tools::format( "Failed to read rules file: %s", READ_FILE.getError() ) );
    }

    const std::wregex re_rules_start( RULES_START_REGEX, std::regex_constants::icase );
    const std::wregex re_regex( RULES_REGEX_REGEX, std::regex_constants::icase );
    const std::wregex re_on_match( RULES_ON_MATCH_REGEX, std::regex_constants::icase );

    auto lines = split_string_view( rules_content, L"\n" );

    std::vector<std::shared_ptr<Rule>> rules;
    std::shared_ptr<Rule> current_rule;

    for( unsigned int i = 0; i < lines.size(); ++i ) {

        const auto line = std::wstring(lines[i]);

        if( strip_view( line ).empty() ) {
            continue;
        };                

        // CPPDEBUG( Tools::wformat( L"Checking line: '%s'", line ) );

        /*
        if( std::regex_search( line.begin(), line.end(), re_rules_start ) ) {
            CPPDEBUG( Tools::wformat( L"Found rule start at line: %s", i + 1) );
        }
        */
        std::wsmatch match;

        if( std::regex_search( line, match, re_rules_start ) ) {
            CPPDEBUG( Tools::wformat( L"Found rule start at line: %s", i + 1) );

            for( unsigned int j = 0; j < match.size(); ++j ) {
                CPPDEBUG( Tools::wformat( L"Match[%d]: '%s'", j,  match[j].str() ) );
            }

            if( match.size() > 2 ) {
                current_rule = std::make_shared<Rule>();
                current_rule->name = strip( match[2].str() );
                rules.push_back( current_rule );
            } else {
                CPPDEBUG( Tools::wformat( L"Invalid rule start at line: %s", i + 1) );
            }

            continue;
        } // if


        if( !current_rule ) {
            continue;
        }

        if( std::regex_search( line, match, re_regex ) ) {
            CPPDEBUG( Tools::wformat( L"Found rule regex at line: %s", i + 1) );

            for( unsigned int j = 0; j < match.size(); ++j ) {
                CPPDEBUG( Tools::wformat( L"Match[%d]: '%s'", j,  match[j].str() ) );
            }

            if( match.size() > 2 ) {
                current_rule->match = strip( match[2].str() );
            } else {
                CPPDEBUG( Tools::wformat( L"Invalid rule regex at line: %s", i + 1) );
            }

            continue;
        }

        if( std::regex_search( line, match, re_on_match ) ) {
            CPPDEBUG( Tools::wformat( L"Found rule regex at line: %s", i + 1) );

            for( unsigned int j = 0; j < match.size(); ++j ) {
                CPPDEBUG( Tools::wformat( L"Match[%d]: '%s'", j,  match[j].str() ) );
            }

            if( match.size() > 2 ) {
                current_rule->on_match = strip( match[2].str() );
            } else {
                CPPDEBUG( Tools::wformat( L"Invalid rule regex at line: %s", i + 1) );
            }

            continue;
        }

    }

    return rules;
}