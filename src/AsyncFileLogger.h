#pragma once

#include "AsyncOutDebug.h"
#include <fstream>

namespace AsyncOut {

class FileLogger : public AsyncOut::Logger
{
    std::string                             m_filename;
    std::ofstream                           m_file;
    std::mutex                              m_file_mutex;    

public:
    FileLogger( const std::string & filename_ );

    void log() override;

    void flush() {
        auto lock = std::lock_guard(m_file_mutex);
        m_file.flush();
    }
};

} // namespace AsyncOut