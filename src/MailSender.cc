#include "MailSender.h"

#include <boost/asio.hpp>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iomanip>

using boost::asio::ip::tcp;

// Simple base64 encoder
static std::string base64_encode(const std::string &in) {
    static const char tbl[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    int val = 0, valb = -6;
    for (unsigned char c : in) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(tbl[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) out.push_back(tbl[((val << 8) >> (valb + 8)) & 0x3F]);
    while (out.size() % 4) out.push_back('=');
    return out;
}

struct MailSender::Impl {
    std::string host;
    unsigned short port;
    boost::asio::io_context ioc;

    Impl(const std::string &h, unsigned short p) : host(h), port(p) {}

    void expect_code(tcp::socket &sock, const std::string &expected_prefix) {
        boost::asio::streambuf buf;
        boost::asio::read_until(sock, buf, "\r\n");
        std::istream is(&buf);
        std::string line;
        std::getline(is, line);
        if (line.size() < expected_prefix.size() || line.substr(0,3) != expected_prefix) {
            throw std::runtime_error("SMTP error: " + line);
        }
    }

    std::string read_line(tcp::socket &sock) {
        boost::asio::streambuf buf;
        boost::asio::read_until(sock, buf, "\r\n");
        std::istream is(&buf);
        std::string line;
        std::getline(is, line);
        return line;
    }

    void send_cmd(tcp::socket &sock, const std::string &cmd) {
        std::string tosend = cmd + "\r\n";
        boost::asio::write(sock, boost::asio::buffer(tosend));
    }

    void send_mail(const std::string &from,
                   const std::vector<std::string> &to,
                   const std::string &subject,
                   const std::string &body,
                   const std::string &attachmentPath) {
        tcp::resolver resolver(ioc);
        auto endpoints = resolver.resolve(host, std::to_string(port));
        tcp::socket sock(ioc);
        boost::asio::connect(sock, endpoints);

        // Server greeting
        std::string line = read_line(sock);
        if (line.size() < 3 || line.substr(0,3) != "220") throw std::runtime_error("Bad greeting: " + line);

        // EHLO
        send_cmd(sock, "EHLO localhost");
        // read multiple EHLO lines (250-)
        while (true) {
            line = read_line(sock);
            if (line.size() >= 3 && line.substr(0,3) == "250") {
                if (line.size() > 3 && line[3] == ' ') break;
                else continue;
            } else {
                throw std::runtime_error("EHLO failed: " + line);
            }
        }

        // MAIL FROM
        send_cmd(sock, "MAIL FROM:<" + from + ">");
        expect_code(sock, "250");

        // RCPT TO
        for (const auto &r : to) {
            send_cmd(sock, "RCPT TO:<" + r + ">");
            expect_code(sock, "250");
        }

        // DATA
        send_cmd(sock, "DATA");
        expect_code(sock, "354");

        // Build MIME message
        std::string boundary = "----=_boundary_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
        std::ostringstream msg;
        msg << "From: " << from << "\r\n";
        msg << "To: ";
        for (size_t i=0;i<to.size();++i) { if (i) msg << ", "; msg << to[i]; }
        msg << "\r\n";
        msg << "Subject: " << subject << "\r\n";
        msg << "MIME-Version: 1.0\r\n";
        if (attachmentPath.empty()) {
            msg << "Content-Type: text/plain; charset=utf-8\r\n";
            msg << "\r\n";
            msg << body << "\r\n";
        } else {
            msg << "Content-Type: multipart/mixed; boundary=" << boundary << "\r\n";
            msg << "\r\n";
            msg << "--" << boundary << "\r\n";
            msg << "Content-Type: text/plain; charset=utf-8\r\n\r\n";
            msg << body << "\r\n\r\n";

            // Read attachment
            std::ifstream ifs(attachmentPath, std::ios::binary);
            if (!ifs) throw std::runtime_error("Cannot open attachment: " + attachmentPath);
            std::ostringstream buf;
            buf << ifs.rdbuf();
            std::string data = buf.str();
            std::string b64 = base64_encode(data);

            // Guess filename
            std::string fname;
            auto pos = attachmentPath.find_last_of('/');
            if (pos == std::string::npos) fname = attachmentPath; else fname = attachmentPath.substr(pos+1);

            msg << "--" << boundary << "\r\n";
            msg << "Content-Type: application/octet-stream; name=\"" << fname << "\"\r\n";
            msg << "Content-Transfer-Encoding: base64\r\n";
            msg << "Content-Disposition: attachment; filename=\"" << fname << "\"\r\n\r\n";

            // Break base64 into 76-char lines per RFC
            for (size_t i=0;i<b64.size(); i+=76) {
                msg << b64.substr(i, 76) << "\r\n";
            }
            msg << "\r\n";
            msg << "--" << boundary << "--\r\n";
        }

        // Terminate DATA with single dot on a line
        msg << ".\r\n";

        // Send message
        std::string out = msg.str();
        boost::asio::write(sock, boost::asio::buffer(out));

        // Expect 250
        expect_code(sock, "250");

        // QUIT
        send_cmd(sock, "QUIT");
        // read goodbye
        try { read_line(sock); } catch (...) {}
    }
};

MailSender::MailSender(const std::string &host, unsigned short port) {
    pimpl = new Impl(host, port);
}

MailSender::~MailSender() {
    delete pimpl;
}

void MailSender::send(const std::string &from,
                      const std::vector<std::string> &to,
                      const std::string &subject,
                      const std::string &body,
                      const std::string &attachmentPath) {
    pimpl->send_mail(from, to, subject, body, attachmentPath);
}
