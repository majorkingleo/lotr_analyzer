#pragma once

#include "BasicThread.h"
#include "ConfigMailImport.h"
#include "ReadMailFromFile.h"

class ImportMail : public ReadMailFromFile, public BasicThread
{    
    std::set<std::string> m_imported_files{};

public:
    ImportMail();

    void run() override;

private:

    void process();

    std::wstring get_header( const std::vector<std::wstring_view> & content_lines, const std::wstring & header_name );
    void read_already_imported_files();
    bool is_zstd_compressed( const std::string & filename );
};