#pragma once

#include "bindtypes.h"
#include "BasicThread.h"

class SendMail : public BasicThread
{
public:
    SendMail();

    void run() override;

private:
    void process();

};