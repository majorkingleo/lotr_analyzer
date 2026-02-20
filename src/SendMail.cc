#include "SendMail.h"
#include <CpputilsDebug.h>
#include "App.h"
#include <regex>
#include <string_utils.h>
#include <utf8_util.h>
#include <ConfigRules.h>
#include <ConfigGlobal.h>
#include <filesystem>
#include <format>
#include "MailSender.h"
#include "ConfigSendMail.h"
#include "Configfile2.h"

using namespace Tools;

SendMail::SendMail()
 : BasicThread( "SendMail" )
{

}

void SendMail::run()
{
    try {
        process();
    } catch( const std::exception & e ) {
        CPPDEBUG( Tools::format( "Error in SendMail thread: %s", e.what() ) );
        APP.db->rollback();
    }
}

void SendMail::process()
{
    const ConfigSectionSendMail & cfg_send_mail = Configfile2::get( ConfigSectionSendMail::KEY );
    MailSender sender{ cfg_send_mail.host.value,  static_cast<unsigned short>(cfg_send_mail.port.value) };

    while( !APP.quit_request ) {

        std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
        APP.db->rollback();

        MAIL mailes[100];
        DBInLimit limit(100);
        static const MAIL mail{};

        int count = StdSqlSelect( *APP.db,
    					Tools::format( "select %%%s from %s where %s = 1 and %s = 0 and %s = 1",
                            mail.get_table_name(), 
                            mail.get_table_name(),
                            mail.checked.get_name(),
                            mail.mailed.get_name(),
                            mail.found.get_name() ),
                        DBInArrayList() >> mailes, limit );

        // CPPDEBUG( Tools::format( "Found %d mails to check", count ) );

        if( count < 0 ) {
            throw std::runtime_error( Tools::format( "SqlError: %s", APP.db->get_error() ) );
        }

        if( count == 0 ) {
            continue;
        }

        for( int i = 0; i < count; i++ ) {
            MAIL & mail = mailes[i];
            mail.mailed = 1;

            sender.send( cfg_send_mail.from.value, 
                         { mail.mailto.data }, 
                         mail.subject.data, 
                         mail.body_text_plain.data );

            if( !StdSqlUpdate( *APP.db, mail, Tools::format( "where `%s` = '%d'", mail.idx.get_name(), mail.idx.data ) ) ) {
                throw std::runtime_error( Tools::format( "SqlError: %s", APP.db->get_error() ) );
            }
            APP.db->commit();
        }        

    } // while( !APP.quit_request )
}
