#include <iostream>
#include <arg.h>
#include <OutDebug.h>
#include <DetectLocale.h>
#include <format.h>
#include "PlaySound.h"
#include "App.h"
#include <filesystem>
#include "Configfile2.h"
#include "ConfigDatabase.h"
#include "bindtypes.h"
#include <dbi.h>
#include <thread>
#include <chrono>
#include "ImportMail.h"
#include <unistd.h>
#include <signal.h>

using namespace Tools;
using namespace std::chrono_literals;

class Co : public ColoredOutput
{
public:
	std::string good( const std::string & message )
	{
		return color_output( ColoredOutput::GREEN, message );
	}

	std::string bad( const std::string & message )
	{
		return color_output( ColoredOutput::BRIGHT_RED, message );
	}
};

static void usage( const std::string & prog )
{
	  std::cerr << "usage: "
				<< prog << "\n";
}

namespace {

template <class TABLE>
struct insert_TABLE_ret
{
	bool inserted {};
	TABLE table {};
};

template <class TABLE>
insert_TABLE_ret<TABLE> insert_TABLE( const std::string & name, const std::string & key, const std::string & value )
{
	TABLE existing_table;

	CPPDEBUG( Tools::format( "%s key=%s value=%s", name, key.c_str(), value.c_str() ) );

	if( StdSqlSelect( *APP.db,
			Tools::format( "select %%%s from %s where `%s` = '%s' ",
					existing_table.get_table_name(),
					existing_table.get_table_name(),
					existing_table.key.get_name(),
					escape( key ) ),
			DBInList<DBBindType>() >> existing_table ) > 0 ) {
		return { false, existing_table };
	}

	TABLE table {};
	table.setHist( BASE::HIST_TYPE::HIST_AN, "broker" );
	table.setHist( BASE::HIST_TYPE::HIST_AE, "broker" );
	table.setHist( BASE::HIST_TYPE::HIST_LO, "broker" );

	table.key = key;
	table.value = value;

	if( !StdSqlInsert( *APP.db, table ) ) {
		CPPDEBUG( Tools::format( "cannot insert into DB: %s", APP.db->get_error() ) );
	}

	APP.db->commit();

	return { true, table };
}

static void insert_config( const std::string & key, const std::string & value, bool force = false )
{
	auto ret = insert_TABLE<CONFIG>( "config", key, value );

	if( !force ) {
		return;
	}

	if( !ret.inserted ) {

		CONFIG & config = ret.table;
		config.setHist( BASE::HIST_TYPE::HIST_AE, "broker" );
		config.value.data = value;
		if( StdSqlUpdate( *APP.db, ret.table, Tools::format( "where `%s` = '%d'", config.idx.get_name(), config.idx.data )) ) {
			APP.db->commit();
		}
	}
}

static void insert_stats( const std::string & key, const std::string & value )
{
	insert_TABLE<STATS>( "stats", key, value );
}

} // namespace

static void insert_default_values()
{
	// insert_config( "brightness", "0.02" );	
}

void my_handler(int s)
{
	APP.quit_request = true;
	printf("Caught signal %d\n",s);
}

void setup_signal_handlers()
{
	struct sigaction sigIntHandler;

	sigIntHandler.sa_handler = my_handler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;

	sigaction(SIGINT, &sigIntHandler, NULL);
}

int main( int argc, char **argv )
{
	setup_signal_handlers();

	Co co;

	std::list<std::thread> threads;

	try {
		Arg::Arg arg( argc, argv );

		arg.addPrefix( "-" );
		arg.addPrefix( "--" );

		arg.setMinMatch(1);

		Arg::OptionChain oc_info;
		arg.addChainR( &oc_info );
		oc_info.setMinMatch( 1 );
		oc_info.setContinueOnMatch( false );
		oc_info.setContinueOnFail( true );

		Arg::FlagOption o_help( "h" );
		o_help.addName( "help" );
		o_help.setDescription( "Show this page" );
		oc_info.addOptionR( &o_help );

		Arg::FlagOption o_version( "v" );
		o_version.addName( "version" );
		o_version.setDescription( "Show replace version number" );
		oc_info.addOptionR( &o_version );


		Arg::OptionChain oc_main;
		arg.addChainR( &oc_main );
		oc_main.setMinMatch( 0 );
		oc_main.setContinueOnMatch( true );
		oc_main.setContinueOnFail( true );

		Arg::FlagOption o_debug("debug");
		o_debug.setDescription("print debugging messages");
		o_debug.setRequired(false);
		oc_main.addOptionR( &o_debug );

		Arg::FlagOption o_create_sql("create-sql");
		o_create_sql.setDescription("print create sql script");
		o_create_sql.setRequired(false);
		oc_main.addOptionR( &o_create_sql );

		Arg::FlagOption o_with_drop_table("with-drop-table");
		o_with_drop_table.setDescription("add drop table statements");
		o_with_drop_table.setRequired(false);
		oc_main.addOptionR( &o_with_drop_table );


		/*
		Arg::StringOption o_enqueue_animation("enqueue-animation");
		o_enqueue_animation.setDescription("add a animation file to queue");
		o_enqueue_animation.setRequired(false);
		o_enqueue_animation.setMinValues(1);
		o_enqueue_animation.setMaxValues(1);
		oc_main.addOptionR( &o_enqueue_animation );
		*/
		DetectLocale dl;

		const unsigned int console_width = 80;

		if( !arg.parse() )
		{
			std::cout << "failed\n";

			if( o_version.getState() )
			{
				std::cout << format("%s version %s\n", argv[0], VERSION);
				return 0;
			} else {
				usage(argv[0]);
				std::cout << arg.getHelp(5,20,30, console_width ) << std::endl;
				return 1;
			}
		}

		if( o_help.getState() ) {
			usage(argv[0]);
			std::cout << arg.getHelp(5,20,30, console_width ) << std::endl;
			return 0;
		}


		if( o_debug.getState() ) {
			Tools::x_debug = new OutDebug();
		}

		if( o_create_sql.getState() ) {			
			std::cout << create_sql( o_with_drop_table.getState() ) << std::endl;
		}

		Configfile2::createDefaultInstaceWithAllModules("~/.lotr-analyzer.ini")->read(true);
		const ConfigSectionDatabase 	& cfg_db 			= Configfile2::get(ConfigSectionDatabase::KEY);
	
		std::chrono::steady_clock::time_point retry_logon_until{};

		retry_logon_until = std::chrono::steady_clock::now() + std::chrono::seconds(cfg_db.retry_db_timeout.value);


		while( !APP.db ) {

			APP.reconnect_db = [&cfg_db]() {
				APP.db.connect( cfg_db.Host,
								cfg_db.UserName,
								cfg_db.Password,
								cfg_db.Instance,
								Database::DB_MYSQL );
			};

			APP.reconnect_db();

			if( !APP.db->valid() ) {
				if( retry_logon_until < std::chrono::steady_clock::now() ) {
					throw STDERR_EXCEPTION( Tools::format( "cannot connect to database: '%s'", APP.db->get_error()));
				} else {
					CPPDEBUG( Tools::format( "cannot connect to database: '%s' retrying...", APP.db->get_error()) );
					APP.db.dispose();
					std::this_thread::sleep_for(500ms);
					continue;
				}
			}
		}

		/*
		if( o_enqueue_chunk.isSet() ) {
			for( const auto & file : *o_enqueue_chunk.getValues() ) {
				if( !std::filesystem::exists(file) ) {
					throw STDERR_EXCEPTION( Tools::format( "file '%s' does not exists", file ) );
				}

				PLAY_QUEUE_CHUNKS pqc {};
				pqc.file = std::filesystem::absolute( file ).string();
				pqc.setHist(BASE::HIST_TYPE::HIST_AN);
				pqc.setHist(BASE::HIST_TYPE::HIST_AE);
				pqc.setHist(BASE::HIST_TYPE::HIST_LO);

				if( !StdSqlInsert( *APP.db, pqc ) ) {
					throw STDERR_EXCEPTION( Tools::format( "cannot enqueue file '%s' '%s'", file, APP.db->get_error() ) );
				}
				APP.db->commit();
			}

			return 0;
		}
		*/


		insert_default_values();

		
		threads.emplace_back([]() {
			auto token = APP.db.get_dispose_token();
			ImportMail import_mail;
			import_mail.run();			
		});		
		

		for( auto & t : threads ) {
			t.join();
		}

		return 0;


	} catch( std::exception & err ) {
		std::cerr << co.bad("error: ") << err.what() << std::endl;
		return 10;
	}

	return 0;
}
