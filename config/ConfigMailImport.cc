#include "ConfigGlobal.h"
#include "cppdir.h"
#include "ConfigMailImport.h"

using namespace Tools;

Configfile2::SectionKey<ConfigSectionMailImport> ConfigSectionMailImport::KEY( "MailImport" );


ConfigSectionMailImport::ConfigSectionMailImport( const std::string & name_, Configfile2 *config_file_ )
: Section( name_, config_file_ ),
  ImportDirectory( "ImportDirectory", "uncompressed" ),
  mail2text( "mail2text", "mail2text.sh" ),
  reformime( "reformime", "reformime" )
{
	registerValue( &ImportDirectory );
	registerValue( &mail2text );
	registerValue( &reformime );
}

void ConfigSectionMailImport::registerSection( Configfile2 *config_file )
{
	config_file->registerSection( new ConfigSectionMailImport(ConfigSectionMailImport::KEY.name, config_file ) );
}








