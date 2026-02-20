#pragma once

#include "Configfile2.h"

class ConfigSectionRules : public Configfile2::Section
{
public:
	static Configfile2::SectionKey<ConfigSectionRules> KEY;

	CONFIG_SIMPLE_DECLARE_STR( RulesFile );

public:
	ConfigSectionRules( const std::string & name_, Configfile2 *config_file_ );

	static void registerSection( Configfile2 *config_file );
};


