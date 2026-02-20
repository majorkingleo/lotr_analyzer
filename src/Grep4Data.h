#pragma once

#include "bindtypes.h"
#include "BasicThread.h"
#include "ParseRules.h"
#include <chrono>
#include <filesystem>

class Grep4Data : public BasicThread
{
    ParseRules::result_type         m_rules{};
    std::filesystem::file_time_type m_last_rules_load_time{ std::filesystem::file_time_type::min() };

public:
    Grep4Data();

    void run() override;

private:
    void process();
    const ParseRules::value_type grep( const MAIL & mail );
    const ParseRules::value_type grep( const std::wstring_view & data );
    void reload_rules_if_needed();
};