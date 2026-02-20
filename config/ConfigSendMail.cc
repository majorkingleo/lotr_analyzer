#include "ConfigGlobal.h"
#include "cppdir.h"
#include "ConfigSendMail.h"

using namespace Tools;

Configfile2::SectionKey<ConfigSectionSendMail> ConfigSectionSendMail::KEY( "SendMail" );


ConfigSectionSendMail::ConfigSectionSendMail( const std::string & name_, Configfile2 *config_file_ )
: Section( name_, config_file_ ),
  host( "host", "localhost" ),
  port( "port", 25 ),
  from( "from", "" )
{
	registerValue( &host );
	registerValue( &port );
	registerValue( &from );
}

void ConfigSectionSendMail::registerSection( Configfile2 *config_file )
{
	config_file->registerSection( new ConfigSectionSendMail(ConfigSectionSendMail::KEY.name, config_file ) );
}








