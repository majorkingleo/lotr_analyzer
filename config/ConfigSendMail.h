#pragma once

#include "Configfile2.h"

class ConfigSectionSendMail : public Configfile2::Section
{
public:
	static Configfile2::SectionKey<ConfigSectionSendMail> KEY;

	CONFIG_SIMPLE_DECLARE_STR( host );
	CONFIG_SIMPLE_DECLARE_INT( port );
	CONFIG_SIMPLE_DECLARE_STR( from );

public:
	ConfigSectionSendMail( const std::string & name_, Configfile2 *config_file_ );

	static void registerSection( Configfile2 *config_file );
};


