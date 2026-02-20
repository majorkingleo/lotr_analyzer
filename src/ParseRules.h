#pragma once

#include <string>

class ParseRules
{
    static constexpr const wchar_t* RULES_START_REGEX    = L"(^\\s*=\\s*)(.*?)(\\s*=\\s*$)";
    static constexpr const wchar_t* RULES_REGEX_REGEX    = L"(^\\s*regex\\s*:)(.*?)$";
    static constexpr const wchar_t* RULES_ON_MATCH_REGEX = L"(^\\s*on match\\s*:)(.*?)$";

    class TestMatch
    {
        std::wstring m_match;
    public:
        TestMatch(const std::wstring_view & match) 
        : m_match(match) 
        {}

        virtual bool valid() const = 0;
    };

    struct TestRegexMatch : public TestMatch
    {
    public:
        TestRegexMatch(const std::wstring_view & match) 
        : TestMatch(match) 
        {}

        bool valid() const override {
            return true;
        }
    };

    struct Rule {
        std::wstring name;
        std::wstring match;
        std::wstring on_match;
    };

private:
    std::string m_rules_file;    

public:
    ParseRules( const std::string_view & rules_file );

    std::vector<std::shared_ptr<Rule>> parse();

};