#include "thrift_handler.h"
#include "public_common.h"
#include "server_manager.h"

nvrWebServiceHandler::nvrWebServiceHandler()
{
}

int32_t nvrWebServiceHandler::notice(const std::string& cmd, const std::string& jsonObject) 
{
	printLog ( "thrift handler cmd : %s", cmd .c_str () )  ;

	if ( "policy" == cmd )			
	{
		std::thread event( []() -> void { ServerManager .downloadPolicy () ; } );
		event .detach () ;
	}
	else if ( "cabinet" == cmd )		
	{
		std::thread event( []() -> void { ServerManager .downloadCabinets () ; } );
		event .detach () ;
	}
	else if ( "camera" == cmd )		
	{
		std::thread event( []() -> void { ServerManager .downloadCameras () ; } );
		event .detach () ;
	}
	else if ( "heartbeat" == cmd )		
	{
	}

	return 0 ;
}

