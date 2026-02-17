#pragma once
#include <regex>
#include <variant>

class CheckRule
{
    struct MailTo
    {
        std::string mail_to;
    };

    struct Rule
    {
        std::string                                 m_name;
        std::regex                                  m_regex;        
        std::variant<std::monostate,Rule,MailTo>    m_next;        
    };

public:
    CheckRule( std::string & file );

    bool open( std::string & file ); 
};


