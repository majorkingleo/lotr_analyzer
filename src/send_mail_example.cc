#include "MailSender.h"
#include <iostream>

int main(int argc, char **argv) {
    try {
        MailSender sender("localhost", 25);
        std::string from = "sender@example.com";
        std::vector<std::string> to = {"recipient@example.com"};
        std::string subject = "Test mail from MailSender";
        std::string body = "Hello,\n\nThis is a test message sent via MailSender.\n";
        std::string attachment = ""; // set path to a file to attach

        if (argc > 1) attachment = argv[1];

        sender.send(from, to, subject, body, attachment);
        std::cout << "Mail sent successfully\n";
    } catch (const std::exception &ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 2;
    }
    return 0;
}
