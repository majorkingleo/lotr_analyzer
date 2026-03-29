#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <CpputilsDebug.h>
#include <format.h>


static bool is_hex_number( const std::string & str )
{
    for( char c : str ) {
        if( !std::isxdigit( static_cast<unsigned char>(c) ) ) {
            return false;
        }
    }
    return true;
}

// Decodes a Quoted-Printable string (e.g., =3A becomes :)
std::string decodeQuotedPrintable(const std::string& input) 
{
    std::string decoded;
    for (size_t i = 0; i < input.length(); ++i) {
        
        if (input[i] == '=' && i + 1 < input.length() && input[i + 1] == '\n') {
            // Soft line break, skip both characters
            ++i; // Skip the newline character
        } else if (input[i] == '=' && i + 1 < input.length() && input[i + 1] == '\r' && i + 2 < input.length() && input[i + 2] == '\n') {
            // Soft line break with CRLF, skip all three characters
            i += 2; // Skip the CR and LF characters
            
        } else if (input[i] == '=' && i + 2 < input.length()) {
            // Convert hex after '=' to char
            std::string hex = input.substr(i + 1, 2);

            if( !is_hex_number( hex ) ) {
                CPPDEBUG( Tools::format( "Invalid hex sequence '%s' in quoted-printable string: %s, aborting", hex, input ) );
                return input; // Return original string if invalid hex sequence is found
                // decoded += '=';
                // continue;
            }
            
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
        decodedSubject += text;
    }

    return decodedSubject;
}
