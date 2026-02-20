#include "ParseRules.h"
#include <CpputilsDebug.h>
#include <tools_config.h>
#include <OutDebug.h>
#include <iostream>

using namespace Tools;

int main( int argc, char **argv ) 
{
    try {
        Tools::x_debug = new OutDebug();

        if( argc < 2 ) {
            std::cerr << "Usage: " << argv[0] << " <rules_file>\n";
            return 1;
        }

        ParseRules parse_rules(argv[1]);


        parse_rules.parse();

    } catch (const std::exception &ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 2;
    }
    return 0;
}