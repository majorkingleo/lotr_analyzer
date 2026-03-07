#include "ConfigGlobal.h"

using namespace Tools;

Configfile2::SectionKey<ConfigSectionGlobal> ConfigSectionGlobal::KEY( "global" );

ConfigSectionGlobal::ConfigSectionGlobal( const std::string & name_, Configfile2 *config_file_ )
: Section( name_, config_file_ ),
  LogFile( "LogFile", Configfile2::makePathsAbsolute( "~/.lotr_analyzer.log" ) )
{
	registerValue( &LogFile );
}


void ConfigSectionGlobal::registerSection( Configfile2 *config_file )
{
	config_file->registerSection( new ConfigSectionGlobal(ConfigSectionGlobal::KEY.name, config_file ) );
}
