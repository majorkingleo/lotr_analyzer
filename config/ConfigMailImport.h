#pragma once

#include "Configfile2.h"

class ConfigSectionMailImport : public Configfile2::Section
{
public:
	static Configfile2::SectionKey<ConfigSectionMailImport> KEY;

	CONFIG_SIMPLE_DECLARE_STR( ImportDirectory );
	CONFIG_SIMPLE_DECLARE_STR( mail2text );

public:
	ConfigSectionMailImport( const std::string & name_, Configfile2 *config_file_ );

	static void registerSection( Configfile2 *config_file );
};


