#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

// Decodes a Quoted-Printable string (e.g., =3A becomes :)
std::string decodeQuotedPrintable(const std::string& input) {
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
std::string decodeMimeSubject(const std::string& subject) {
    size_t startPos = subject.find("=?");
    if (startPos == std::string::npos) return subject;

    size_t endPos = subject.find("?=", startPos);
    if (endPos == std::string::npos) return subject;

    // Split the inner parts: charset?encoding?text
    std::string inner = subject.substr(startPos + 2, endPos - startPos - 2);
    std::stringstream ss(inner);
    std::string charset, encoding, text;

    std::getline(ss, charset, '?');
    std::getline(ss, encoding, '?');
    std::getline(ss, text, '?');

    if (encoding == "Q" || encoding == "q") {
        return decodeQuotedPrintable(text);
    } else if (encoding == "B" || encoding == "b") {
        return "[Base64 encoding detected - requires a Base64 library]";
    }

    return text;
}

int main() {
    std::string rawSubject = "=?UTF-8?Q?Fwd=3A_Kinoprogramm_Graz_=2830=2E_J=C3=A4nner_-_5=2E_Febr?=";
    
    std::cout << "Raw: " << rawSubject << std::endl;
    std::cout << "Decoded: " << decodeMimeSubject(rawSubject) << std::endl;

    return 0;
}
