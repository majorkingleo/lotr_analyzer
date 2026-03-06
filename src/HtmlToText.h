#pragma once

#include <string>
#include <unordered_map>

class HtmlToText
{
public:
    /// Converts HTML to plain text with entity decoding
    /// @param html Input HTML as std::wstring
    /// @return Plain text as std::wstring with entities converted to UTF-8 characters
    static std::wstring convert(const std::wstring& html);

    static std::wstring convert_from_mail(const std::wstring& html);

private:
    static const std::unordered_map<std::wstring, wchar_t> NAMED_ENTITIES;
    
    /// Decodes a numeric entity (&#123; or &#x1F;)
    static std::wstring decodeNumericEntity(const std::wstring& entity);
    
    /// Converts a Unicode code point to UTF-8 compatible wchar_t
    static wchar_t codePointToChar(unsigned int codePoint);
    
    /// Decodes quoted-printable encoding (e.g., F=C3=BCr -> Für)
    static std::wstring decodeQuotedPrintable(const std::wstring& text);
};
