#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <CpputilsDebug.h>
#include <format.h>

// Decodes a Quoted-Printable string (e.g., =3A becomes :)
std::string decodeQuotedPrintable(const std::string& input) 
{
    std::string decoded;
    for (size_t i = 0; i < input.length(); ++i) {
        if (input[i] == '=' && i + 2 < input.length()) {
            // Convert hex after '=' to char
            std::string hex = input.substr(i + 1, 2);
            char c = static_cast<char>(std::stoul(hex, nullptr, 16));
            decoded += c;
            i += 2;
        } else if (input[i] == '_') {
            // RFC 2047 specific: underscores represent spaces
            decoded += ' ';
        } else {
            decoded += input[i];
        }
    }
    return decoded;
}

// Parses the MIME encoded-word: =?charset?encoding?text?=
std::string decodeMimeSubject(const std::string& subject) 
{
    size_t pos = 0;
    std::string decodedSubject;

    while(true) {

        size_t startPos = subject.find("=?", pos);
        if (startPos == std::string::npos) {
            if( pos == 0 ) {                
                return subject;
            }
            return decodedSubject;
        }

        size_t endPos = subject.find("?=", startPos);
        if (endPos == std::string::npos) {
            if( pos == 0 ) {
                return subject;
            }
            return decodedSubject;
        }

        // Split the inner parts: charset?encoding?text
        std::string inner = subject.substr(startPos + 2, endPos - startPos - 2);
        std::stringstream ss(inner);
        std::string charset;
        std::string encoding;
        std::string text;

        std::getline(ss, charset, '?');
        std::getline(ss, encoding, '?');
        std::getline(ss, text, '?');

        if (encoding == "Q" || encoding == "q") {
            text = decodeQuotedPrintable(text);
        } else if (encoding == "B" || encoding == "b") {
            return "[Base64 encoding detected - requires a Base64 library]";
        }

        pos = endPos + 2;
        CPPDEBUG( Tools::format( "pos: %zu", pos ) );
        decodedSubject += text;
    }

    return decodedSubject;
}
