#pragma once

#include "BasicThread.h"
#include "ConfigMailImport.h"
#include "bindtypes.h"
#include <read_file.h>

class ImportMail : public BasicThread
{
    ReadFile              m_read_file{};
    std::set<std::string> m_imported_files{};

public:
    ImportMail();

    void run() override;

private:

    void process();

    MAIL read_mail_from_file( const std::string & filename );

    std::wstring get_header( const std::vector<std::wstring_view> & content_lines, const std::wstring & header_name );
    void read_already_imported_files();
    bool is_zstd_compressed( const std::string & filename );
};