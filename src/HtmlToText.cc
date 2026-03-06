#include "HtmlToText.h"
#include <regex>
#include <cctype>
#include <string_utils.h>
#include <CpputilsDebug.h>
#include <format.h>
#include <codecvt>
#include <locale>
#include "qp.h"
#include <utf8_util.h>

using namespace Tools;

// Named HTML entities map
const std::unordered_map<std::wstring, wchar_t> HtmlToText::NAMED_ENTITIES = {
    {L"nbsp", L' '},
    {L"lt", L'<'},
    {L"gt", L'>'},
    {L"amp", L'&'},
    {L"quot", L'"'},
    {L"apos", L'\''},
    {L"cent", L'¢'},
    {L"pound", L'£'},
    {L"yen", L'¥'},
    {L"euro", L'€'},
    {L"copy", L'©'},
    {L"reg", L'®'},
    {L"deg", L'°'},
    {L"times", L'×'},
    {L"divide", L'÷'},
    {L"zwnj", L'\u200C'},  // Zero-width non-joiner
    {L"zwj", L'\u200D'},   // Zero-width joiner
};

std::wstring HtmlToText::decodeNumericEntity(const std::wstring& entity)
{
    try {
        if (entity.empty()) return L"";
        
        unsigned int codePoint = 0;
        if (entity[0] == L'x' || entity[0] == L'X') {
            // Hexadecimal entity: &#x1A;
            codePoint = std::stoul(entity.substr(1), nullptr, 16);
        } else {
            // Decimal entity: &#123;
            codePoint = std::stoul(entity, nullptr, 10);
        }
        
        return std::wstring(1, codePointToChar(codePoint));
    } catch (...) {
        return L"";
    }
}

wchar_t HtmlToText::codePointToChar(unsigned int codePoint)
{
    // For wchar_t, we can directly use the code point value for valid Unicode characters
    if (codePoint <= 0x10FFFF) {
        return static_cast<wchar_t>(codePoint);
    }
    return L'?';
}

#if 0
std::wstring HtmlToText::decodeQuotedPrintable(const std::wstring& text)
{
    std::string result;
    
    // Convert wstring to UTF-8 string for byte processing
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::string input = converter.to_bytes(text);
    
    for (size_t i = 0; i < input.length(); ++i) {
        if (input[i] == '=' && i + 2 < input.length()) {
            // Try to decode =XX where XX is hex
            char hex[3] = {input[i+1], input[i+2], '\0'};
            char* endptr = nullptr;
            unsigned long value = std::strtoul(hex, &endptr, 16);
            
            if (endptr == hex + 2 && value <= 0xFF) {
                // Valid hex sequence
                result.push_back(static_cast<char>(value));
                i += 2;
            } else {
                result.push_back(input[i]);
            }
        } else {
            result.push_back(input[i]);
        }
    }
    
    // Convert back to wstring
    return converter.from_bytes(result);
}
#else

std::wstring HtmlToText::decodeQuotedPrintable(const std::wstring& text)
{
    std::string input = Utf8Util::wStringToUtf8(text);
    std::string decoded = ::decodeQuotedPrintable(input);
    return Utf8Util::utf8toWString(decoded);
}

#endif
std::wstring HtmlToText::convert(const std::wstring& html)
{
    std::wstring result = html;
    
    // Decode quoted-printable encoding (e.g., F=C3=BCr -> Für)
    result = decodeQuotedPrintable(result);
    
    // Remove script and style tags with content
    std::wregex scriptRegex(L"<script[^>]*>.*?</script>", std::regex::icase);
    result = std::regex_replace(result, scriptRegex, L"");
    
    std::wregex styleRegex(L"<style[^>]*>.*?</style>", std::regex::icase);
    result = std::regex_replace(result, styleRegex, L"");
    
    // Remove HTML comments
    std::wregex commentRegex(L"<!--.*?-->", std::regex::icase);
    result = std::regex_replace(result, commentRegex, L"");
    
    // Remove all HTML tags (replace with space to separate adjacent text)
    std::wregex tagRegex(L"<[^>]+>");
    result = std::regex_replace(result, tagRegex, L" ");
    
    // Decode named entities (e.g., &nbsp; &lt; &gt; &amp;)
    std::wregex namedEntityRegex(L"&([a-zA-Z]+);");
    std::wsmatch match;
    std::wstring::const_iterator searchStart(result.cbegin());
    std::wstring decoded;
    
    while (std::regex_search(searchStart, result.cend(), match, namedEntityRegex)) {
        decoded.append(searchStart, match[0].first);
        
        std::wstring entityName = match[1].str();
        auto it = NAMED_ENTITIES.find(entityName);
        if (it != NAMED_ENTITIES.end()) {
            decoded.push_back(it->second);
        } else {
            // Unknown entity, keep as is
            decoded.append(match[0].str());
        }
        
        searchStart = match[0].second;
    }
    decoded.append(searchStart, result.cend());
    result = decoded;
    
    // Decode numeric entities (e.g., &#123; or &#x1F;)
    std::wregex numericEntityRegex(L"&#(x?[0-9a-fA-F]+);");
    searchStart = result.cbegin();
    decoded.clear();
    
    while (std::regex_search(searchStart, result.cend(), match, numericEntityRegex)) {
        decoded.append(searchStart, match[0].first);
        
        std::wstring entityContent = match[1].str();
        std::wstring decodedEntity = decodeNumericEntity(entityContent);
        decoded.append(decodedEntity);
        
        searchStart = match[0].second;
    }
    decoded.append(searchStart, result.cend());
    result = decoded;
    
    // Replace multiple whitespace with single space
    std::wregex whitespaceRegex(L"\\s+");
    result = std::regex_replace(result, whitespaceRegex, L" ");
    
    // Trim leading and trailing whitespace
    size_t start = result.find_first_not_of(L" \t\n\r\f\v");
    size_t end = result.find_last_not_of(L" \t\n\r\f\v");
    if (start != std::wstring::npos) {
        result = result.substr(start, end - start + 1);
    } else {
        result.clear();
    }
    
    return result;
}


std::wstring HtmlToText::convert_from_mail(const std::wstring& html)
{
    auto sl = split_string_view( html, L"\n" );
 
    std::wstring combined;

    for( unsigned int i = 0; i < sl.size(); ++i ) {
        std::wstring_view line = sl[i];

        if( line.ends_with( L"=" ) ) {
            line.remove_suffix( 1 );
        }

        combined.append( line );
    }
    
    return convert(combined);
}