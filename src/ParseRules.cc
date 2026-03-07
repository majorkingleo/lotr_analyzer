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


ParseRules::result_type ParseRules::parse() 
{   
    std::wstring rules_content;

    if( !READ_FILE.read_file(m_rules_file, rules_content) ) {
        throw STDERR_EXCEPTION( Tools::format( "Failed to read rules file: %s", READ_FILE.getError() ) );
    }

    static const std::wregex re_rules_start( RULES_START_REGEX, std::regex_constants::icase );
    static const std::wregex re_regex( RULES_REGEX_REGEX, std::regex_constants::icase );
    static const std::wregex re_on_match( RULES_ON_MATCH_REGEX, std::regex_constants::icase );
    static const std::wregex re_email_md( RULES_EMAIL_MD_REGEX, std::regex_constants::icase );

    auto lines = split_string_view( rules_content, L"\n" );

    result_type rules;
    value_type  current_rule;

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
            CPPDEBUG( Tools::wformat( L"Found rule regex at line: %s: '%s'", i + 1, line ) );

            for( unsigned int j = 0; j < match.size(); ++j ) {
                CPPDEBUG( Tools::wformat( L"Match[%d]: '%s'", j,  match[j].str() ) );
            }

            if( match.size() > 2 ) {

                current_rule->match = strip( match[2].str() );
                current_rule->match = strip( current_rule->match, L"`" );

            } else {
                CPPDEBUG( Tools::wformat( L"Invalid rule regex at line: %s", i + 1) );
            }

            continue;
        }

        if( std::regex_search( line, match, re_on_match ) ) {
//            CPPDEBUG( Tools::wformat( L"Found rule regex at line: %s", i + 1) );
/*
            for( unsigned int j = 0; j < match.size(); ++j ) {
                CPPDEBUG( Tools::wformat( L"Match[%d]: '%s'", j,  match[j].str() ) );
            }
*/
            if( match.size() > 2 ) {
                current_rule->on_match = strip( match[2].str() );

                auto emails = split_and_strip_simple( current_rule->on_match, L"," );

                for( auto & email : emails ) {
                    //CPPDEBUG( Tools::wformat( L"Found email in on_match at line: %s: '%s'", i + 1, email ) );


                    // extract email from on_match if it is in markdown format
                    // eg: [kingleo@borger.co.at](mailto:kingleo@borger.co.at)
                    std::wsmatch match2;
                    if( std::regex_search( email, match2, re_email_md ) ) {
  //                      CPPDEBUG( Tools::wformat( L"Found email in on_match at line: %s", i + 1) );
/*
                        for( unsigned int j = 0; j < match2.size(); ++j ) {
                            CPPDEBUG( Tools::wformat( L"MatchMdEmail[%d]: '%s'", j,  match2[j].str() ) );
                        }
*/
                        if( match2.size() > 1 ) {
                            email = strip( match2[1].str(), L"[]" );
                        }
                    }
                } // for
                              
                current_rule->on_match = IterableToCommaSeparatedWString( emails );
                CPPDEBUG( Tools::wformat( L"Rule on_match after email extraction at line: %s: '%s'", i + 1, current_rule->on_match ) );

            } else {
                CPPDEBUG( Tools::wformat( L"Invalid rule regex at line: %s", i + 1) );
            }

            continue;
        }
    }

    result_type res;

    for( const auto & rule : rules ) {
        if( rule->name.empty() || rule->match.empty() || rule->on_match.empty() ) {
            CPPDEBUG( Tools::wformat( L"Skipping invalid rule: name='%s' match='%s' on_match='%s'", rule->name, rule->match, rule->on_match ) );
            continue;
        }

        res.push_back( rule );
    }

    return res;
}