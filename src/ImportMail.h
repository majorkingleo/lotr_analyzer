#pragma once

#include "BasicThread.h"
#include "ConfigMailImport.h"
#include "bindtypes.h"

class ImportMail : public BasicThread
{
public:
    ImportMail();

    void run() override;

private:
    MAIL read_mail_from_file( const std::string & filename );

};