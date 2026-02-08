#include "ImportMail.h"
#include "App.h"
#include <chrono>
#include <thread>
#include <stdexcept>
#include <CpputilsDebug.h>
#include <filesystem>

using namespace std::chrono_literals;
using namespace std::chrono;

ImportMail::ImportMail()
: BasicThread( "ImportMail" )
{

}

void ImportMail::run()
{	
    const ConfigSectionMailImport & cfg_mail_import = Configfile2::get(ConfigSectionMailImport::KEY);

    while( !APP.quit_request ) 
    {               
        APP.db->commit();

        std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
    }
}