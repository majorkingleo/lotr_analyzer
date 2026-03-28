#pragma once

#include "bindtypes.h"

class CheckSingleMail
{
    std::string m_filename;
    std::optional<MAIL> m_mail;

public:
    CheckSingleMail( const std::string & filename );
private:
    bool read_mail( const std::string & filename );

    void write_to_file( const std::string & filename, const std::string & content ) const;
};