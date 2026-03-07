#include "AsyncFileLogger.h"
#include <stderr_exception.h>
#include <utf8_util.h>
#include <filesystem>

using namespace AsyncOut;
using namespace Tools;

FileLogger::FileLogger( const std::string & filename_ )
: m_filename(filename_)
{
    m_file.open( m_filename, std::ios::out | std::ios::app );

    if( !m_file ) {
        throw STDERR_EXCEPTION( Tools::format( "cannot open log file '%s'", m_filename ) );
    }
}

void FileLogger::log()
{
    auto lock = std::lock_guard(m_file_mutex);

	auto all_messages = popAll();

	for( const auto & m : all_messages ) {

		std::string file_name = std::filesystem::path(m.file).filename().string();
		m_file << file_name;
		m_file << ':' << m.line
 			   << " ";

		std::string message;

		if( const auto str_ptr (std::get_if<std::string>(&m.message)); str_ptr) {
			message = *str_ptr;
		} else {
			message = Utf8Util::wStringToUtf8(std::get<std::wstring>(m.message));
		}

		m_file << message << '\n';
	}
}
