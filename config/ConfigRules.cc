#include "ConfigGlobal.h"
#include "cppdir.h"
#include "ConfigRules.h"

using namespace Tools;

Configfile2::SectionKey<ConfigSectionRules> ConfigSectionRules::KEY( "Rules" );


ConfigSectionRules::ConfigSectionRules( const std::string & name_, Configfile2 *config_file_ )
: Section( name_, config_file_ ),
  RulesFile( "RulesFile", "rules.md" )
{
	registerValue( &RulesFile );
}

void ConfigSectionRules::registerSection( Configfile2 *config_file )
{
	config_file->registerSection( new ConfigSectionRules(ConfigSectionRules::KEY.name, config_file ) );
}








