#include "check_rule.h"
#include <xml.h>
#include <read_file.h>

using namespace Tools;

CheckRule::CheckRule( std::string & file )
{
    open( file );
}


bool CheckRule::open( std::string & file )
{
    std::wstring buffer;
    if( !READ_FILE.read_file( file, buffer ) ) {
        return false;
    }

    

}