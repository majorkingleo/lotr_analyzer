#pragma once

#include "bindtypes.h"
#include "BasicThread.h"

class Grep4Data : public BasicThread
{
public:
    Grep4Data();

    void run() override;

private:
    void process();
    bool grep( const MAIL & mail );
    bool grep( const std::wstring_view & data );
};