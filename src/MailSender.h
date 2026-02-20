// Simple SMTP mail sender using Boost.Asio
#pragma once

#include <string>
#include <vector>

class MailSender {
public:
    // Connect to SMTP server on host:port (usually localhost:25) and send mail
    // from: sender email, to: list of recipient emails, subject, body (plain text), attachment file path (optional)
    // Throws std::runtime_error on error
    MailSender(const std::string &host = "localhost", unsigned short port = 25);
    MailSender( const MailSender& ) = delete;
    ~MailSender();

    void send(const std::string &from,
              const std::vector<std::string> &to,
              const std::string &subject,
              const std::string &body,
              const std::string &attachmentPath = "");

private:
    class Impl;
    Impl *pimpl;
};
