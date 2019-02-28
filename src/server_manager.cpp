#include <stdio.h>
#include <pwd.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include<algorithm>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <sys/types.h>
#include "markup.h"
#include "web_client.h"
#include "server_manager.h"

extern bool		g_bIsExit ;
CServerManager	ServerManager ;

std::function<int(struct tm &)> localTime = [](struct tm & tm) -> int {	
	tm .tm_year	=	tm .tm_year + 1900 ;
	tm .tm_mon 	= 	tm .tm_mon + 1 ; 
} ;

void printTime(const struct tm & tm)
{
	printLog ( "%04d-%02d-%02d %02d:%02d:%02d", 
		tm .tm_year + 1900, tm .tm_mon + 1, tm .tm_mday, tm .tm_hour, tm .tm_min, tm .tm_sec ) ;
}

int diffDay(struct tm & tm1, struct tm & tm2)
{
	time_t 	tmDay1		=	::mktime ( &tm1 ) ;
	time_t 	tmDay2		=	::mktime ( &tm2 ) ;
	return (int) ::difftime ( tmDay1, tmDay2 ) /86400 ;
}

std::function<int(std::string &, Json::Value &)> verifyJson = [&] (std::string & data, Json::Value & json) -> int { 

	JSONCPP_STRING errs;
	Json::CharReaderBuilder builder;
	builder["collectComments"] = false;	
	nvrShared<Json::CharReader> reader (builder.newCharReader()) ;

	if ( ! reader ->parse ( data .c_str (), data .c_str () + data .size (), &json, &errs ) )
	{		
		return -1 ;
	}

	if ( ! json .isMember ( "code" ) || 200 != json["code"] .asInt () )
	{
		return -2 ;
	}

	if ( ! json .isMember ( "message" ) )
	{
		return -3 ;
	}

	if ( ! json .isMember ( "version" ) )
	{
		return -4 ;
	}

	if ( ! json .isMember ( "data" ) )
	{
		return -5 ;
	}

	return 0 ;
};

std::function<int(Json::Value &, QueTimeSpan &)> getTimeSpan = [&] (Json::Value & json, QueTimeSpan & queTimeSpan) -> int { 

	int nSpanSize = json .size () ;
	
	for ( int nIdx = 0 ; nIdx < nSpanSize ; ++nIdx )
	{
		std::vector<std::string> 	vValue ;
		
		string_split ( json [nIdx] .asString (), "-",  vValue ) ;

		if ( 2 != vValue .size () )
		{
			printLog ( "getTimeSpan format error!!!!!" ) ;
			continue ;
		}

		std::vector<std::string> 	toValue ;
		std::vector<std::string> 	fromValue ;

		string_split ( vValue [1], ":",  toValue ) ;
		string_split ( vValue [0], ":",  fromValue ) ;		

		if ( 3 != fromValue .size () || 3 != toValue .size () )
		{
			printLog ( "getTimeSpan format error1!!!!!" ) ;
			continue ;
		}

		TIMESPAN	tmSpan ;

		tmSpan .to .hour			=	std::stoi (toValue [0]) ;
		tmSpan .to .minute			=	std::stoi (toValue [1]) ;
		tmSpan .to .seconds		=	std::stoi (toValue [2]) ;		
		tmSpan .from .hour		=	std::stoi (fromValue [0]) ;
		tmSpan .from .minute		=	std::stoi (fromValue [1]) ;
		tmSpan .from .seconds		=	std::stoi (fromValue [2]) ;

		queTimeSpan .push_back ( tmSpan ) ;
	}

	return 0 ;
};

/////////////////////////////////////////////////////////////////////////////////////////////
//
// construct/destruct
//
////////////////////////////////////////////////////////////////////////////////////////////////
CServerManager::CServerManager() : m_bInit(false), m_bDemo(false), m_expired(1800), m_bExpired(false)
{
	if ( !m_mutexTask .IsInitialized () )		{	m_mutexTask .Init () ;		}
	if ( !m_mutexPolicy .IsInitialized () )		{	m_mutexPolicy .Init () ;		}
	if ( !m_mutexCabinets .IsInitialized () )	{	m_mutexCabinets .Init () ;	}
}

CServerManager::~CServerManager()
{	
}

int CServerManager::inital()
{
	printLog ( "inital" ) ;
	char buf [ 256 ] = {0};	
	::getcwd ( buf, sizeof(buf) ) ;
	printLog ( "working directory: %s", buf ) ;
	m_workPath = buf ;

	loadConfig () ;	
	printLog ( "----------------   load config finish   ----------------" ) ;
	loadPolicy () ;
	printLog ( "----------------   load policy finish   ----------------" ) ;
	loadCabinet () ;
	printLog ( "----------------   load cabinet finish   ----------------" ) ;
	loadCamera () ;
	printLog ( "----------------   load camera finish   ----------------" ) ;
	
	downloadPolicy () ;
	printLog ( "----------------   download policy finish   ----------------" ) ;
	downloadCabinets () ;
	printLog ( "----------------   download cabinet finish   ----------------" ) ;
	downloadCameras () ;
	printLog ( "----------------   download camera finish   ----------------" ) ;
	
	m_thriftServer .start () ;

	if ( m_bDemo )
	{
		demo () ;
	}

	m_timeExpired .ResetTime () ;
	m_bInit = true ;
	
	return 0 ;
}

void CServerManager::loadConfig()
{
	CMarkup		xmlDoc ;
	
	if ( !xmlDoc .Load ( "config.xml" ) )
	{
		printLog ( "can not read config.xml" ) ;
		return ;
	}

	if ( !xmlDoc .FindElem ( "config" ) )
	{
		printLog ( "this is an bad config file." ) ;
		return ;
	}

	xmlDoc .IntoElem () ;
	xmlDoc .ResetMainPos () ;
	if ( xmlDoc .FindElem ( "work" ) )
	{
		m_workPath = xmlDoc .GetData () ;
		createDirectory ( m_workPath .c_str () ) ;
		printLog ( "data directory: %s", m_workPath .c_str () ) ;
	}

	createDirectory ( getCachePath () .c_str () ) ;
	createDirectory ( getUploadPath () .c_str () ) ;
	createDirectory ( getRecorderPath () .c_str () ) ;

	xmlDoc .ResetMainPos () ;
	if ( xmlDoc .FindElem ( "demo" ) )
	{
		m_bDemo = ( xmlDoc .GetData () == "true" ) ? true : false ;
	}

	xmlDoc .ResetMainPos () ;
	if ( xmlDoc .FindElem ( "recycle" ) )
	{
		m_expired = std::stoi ( xmlDoc .GetData () ) ;
	}

	xmlDoc .ResetMainPos () ;
	if ( xmlDoc .FindElem ( "thrift" ) )
	{
		m_thriftServer .setPort ( std::stoi ( xmlDoc .GetAttrib ( "port" ) ) ) ;
		m_thriftServer .setThread ( std::stoi ( xmlDoc .GetAttrib ( "thread" ) ) ) ;
		m_thriftServer .setSendTimeout ( std::stoi ( xmlDoc .GetAttrib ( "send" ) ) ) ;
		m_thriftServer .setRecvTimeout ( std::stoi ( xmlDoc .GetAttrib ( "recv" ) ) ) ;		
	}

	xmlDoc .ResetMainPos () ;
	if ( xmlDoc .FindElem ( "manager" ) )
	{
		m_webServer = xmlDoc .GetData () ;
	}

	xmlDoc .OutOfElem () ;
}

void CServerManager::writeLog(const std::string & message)
{
	FILE * file = NULL ;
	std::string logFile = m_workPath + "/nvr.log" ;

	if ( ( file = ::fopen ( logFile .c_str (), "a+" ) ) == NULL )
	{
		perror ( "open" ) ;
		return ;
	}

	::fwrite ( message .c_str (), 1 , message .size (), file ) ; 
	::fclose ( file ) ;
}

int CServerManager::unInit()
{
	m_thriftServer .stop () ;	
	m_bInit = false ;
	
	return 0 ;
}

bool CServerManager::isRun()
{
    return true;
}

// 等待函数
void CServerManager::wait()
{
	while ( false == g_bIsExit )
	{ 
		if ( true == m_bInit )
		{
			// 派发任务
			dispatchTask () ;

			// 开启线程，防止阻塞时间过长，影响任务调度			
			if ( m_timeExpired .IsTimeOut ( m_expired * 1000 ) && false == m_bExpired )
			{
				std::thread event( []() -> void { ServerManager .recycleExpired () ; } );
				event .detach () ;				
				m_timeExpired .ResetTime () ;
			}
		}
		::usleep ( 20 ) ;
	}		
}

void CServerManager::dump()
{
}

void CServerManager::demo()
{
	CAMERAOBJECT	camera ;
	TASKTIME 		taskTime = getTaskTime () ;

	camera .pid = 1 ;
	camera .id = 1 ;
	camera .policy = 1000000 ;
	camera .name = "camera" ;
	camera .type = "hk" ;
	camera .protocolIdx = 0 ;
	camera .protocols .push_back ( "rtmp://192.168.189.152:1935/vod2/sticker.mp4" ) ;

	m_mutexCameras .Lock () ;
	m_hashCameras [camera .id] = camera ;
	m_mutexCameras .Unlock () ;

	createDirectory ( getCameraPath ( camera .id ) .c_str () ) ;


	POLICYOBJECT	policy ;

	policy .id = camera .policy ;
	policy .name = "default" ;
	policy .storage = 0 ;
	policy .expired = 1 ;
	policy .segment = 60 ;
	policy .preview = "320x240" ;
	policy .format	= "mp4" ;

#if 0
	policy .camera .videoSize ;
	policy .camera .videoSample ;
	policy .camera .videoEncoder = "h264" ;
	policy .camera .audioSample ;
	policy .camera .audioEncoder = "aac" ;
	
	policy .alarm .im = "admin@qiyicc.com" ;
	policy .alarm .sms = "18951668166" ;
	policy .alarm .email = "admin@qiyicc.com" ;

	policy .day .monday ;
	policy .day .tuesday ;
	policy .day .wednesday ;
	policy .day .thursday ;
	policy .day .friday ;
	policy .day .saturday ;
	policy .day .sun ;
#endif
	TIMESPAN	span ;

	span .from .hour		=	taskTime .timePoint .hour ;
	span .from .minute		=	taskTime .timePoint .minute + 1 ;
	span .from .seconds	=	0 ;
	span .to .hour			=	span .from .hour ;
	span .to .minute		=	span .from .minute + 2 ;
	span .to .seconds		=	span .from .seconds ;

	policy .weeks [0] .push_back ( span ) ;
	policy .weeks [1] .push_back ( span ) ;
	policy .weeks [2] .push_back ( span ) ;
	policy .weeks [3] .push_back ( span ) ;
	policy .weeks [4] .push_back ( span ) ;
	policy .weeks [5] .push_back ( span ) ;
	policy .weeks [6] .push_back ( span ) ;

	m_mutexPolicy .Lock () ;
	m_quePolicy .push_back ( policy ) ;
	m_mutexPolicy .Unlock () ;

	createTask ( 1 ) ;	
}

void CServerManager::reloadConfig()
{
}

int CServerManager::loadPolicy()
{
	Json::Value	json ;
	
	if ( 0 != loadCache ( "policy.json", json ) )
	{
		return -1 ;
	}

	printLog ( "load policy cache success!" ) ;
	return processPolicy ( json ) ;
}

int CServerManager::loadCabinet()
{
	Json::Value	json ;
	
	if ( 0 != loadCache ( "cabinet.json", json ) )
	{
		return -1 ;
	}

	printLog ( "load cabinet cache success!") ;
	return processCabinet ( json ) ;
}

int CServerManager::loadCamera()
{
	Json::Value	json ;
	
	if ( 0 != loadCache ( "camera.json", json ) )
	{
		return -1 ;
	}

	printLog ( "load camera cache success!") ;
	return processCamera ( json ) ;
}

int CServerManager::downloadPolicy()
{
	Json::Value	json ;
	std::string	version ;
	std::string	url = "policy?cmd=query&token=" + m_token ;

	m_mutexPolicy .Lock () ;
	version = m_policyVersion ;
	m_mutexPolicy .Unlock () ;

	if ( false == version .empty () )
	{
		url += "&version=" + version ;
	}

	if ( 0 != downloadCommon ( url, json ) )
	{
		return -1 ;		
	}

	if ( version == json["version"] .asString () )
	{
		return 0 ;
	}

	saveCache ( "policy.json", json ) ;
	
	return processPolicy ( json ) ;
}

int CServerManager::processPolicy(Json::Value & json)
{
	Json::Value & jsonPolicies = json ["data"] ;
	int iSize = jsonPolicies .size () ;

	m_mutexPolicy .Lock () ;
	m_quePolicy .clear () ;
	m_policyVersion = json["version"] .asString () ;	
	for ( int nIndex = 0 ; nIndex < iSize ; ++nIndex )
	{
		POLICYOBJECT	policy ;
		
		policy .id 			=	jsonPolicies [nIndex] ["id"] .asInt () ;
		policy .name		=	jsonPolicies [nIndex] ["name"] .asString () ;
		policy .storage	=	jsonPolicies [nIndex] ["storage"] .asInt () ;
		policy .expired	=	jsonPolicies [nIndex] ["expired"] .asInt () ;
		policy .segment	=	jsonPolicies [nIndex] ["segment"] .asInt () ;
		policy .preview	=	jsonPolicies [nIndex] ["preview"] .asString () ;
		policy .format		=	jsonPolicies [nIndex] ["format"] .asString () ;

		CAMERAPARAM & camera = policy .camera ;
		Json::Value & jsonCamera = jsonPolicies [nIndex] ["camera"] ;		
		
		camera .videoSize		=	jsonCamera ["video_size"] .asString () ;
		camera .videoSample	=	jsonCamera ["video_sample"] .asString () ;
		camera .videoEncoder	=	jsonCamera ["video_encoder"] .asString () ;
		camera .audioSample	=	jsonCamera ["audio_sample"] .asString () ;
		camera .audioEncoder	=	jsonCamera ["audio_encoder"] .asString () ;

		ALARMPARAM & alarm = policy .alarm ;
		Json::Value & jsonAlarm = jsonPolicies [nIndex] ["alarm"] ;		

		alarm .im		=	jsonAlarm ["im"] .asString () ;
		alarm .sms	=	jsonAlarm ["sms"] .asString () ;
		alarm .email	=	jsonAlarm ["email"] .asString () ;

		Json::Value & jsonWeek = jsonPolicies [nIndex] ["week"] ; 

		if ( 7 != jsonWeek .size () )
		{
			printLog ( "the policy week format bad.\n%s", json .toStyledString () .c_str () ) ;
			continue ;
		}

		getTimeSpan ( jsonWeek ["sun"], policy .weeks [0] ) ;
		getTimeSpan ( jsonWeek ["monday"], policy .weeks [1] ) ;
		getTimeSpan ( jsonWeek ["tuesday"], policy .weeks[2] ) ;
		getTimeSpan ( jsonWeek ["wednesday"], policy .weeks [3] ) ;
		getTimeSpan ( jsonWeek ["thursday"], policy .weeks [4] ) ;
		getTimeSpan ( jsonWeek ["friday"], policy .weeks [5] ) ;
		getTimeSpan ( jsonWeek ["saturday"], policy .weeks [6] ) ;		
		m_quePolicy .push_back ( policy ) ;
		printLog ( "the policy (%d:%s) has been parsed!", policy .id, policy .name .c_str () ) ;		
	}
	m_mutexPolicy .Unlock () ;
	
	return 0 ;
}

int CServerManager::downloadCabinets()
{
	Json::Value	json ;
	std::string	version ;
	std::string	url = "cabinet?cmd=query&token=" + m_token + "&id=" ;

	m_mutexCabinets .Lock () ;	
	version = m_cabinetVersion ;
	m_mutexCabinets .Unlock () ;

	if ( false == version .empty () )
	{
		url += "&version=" + version ;
	}

	if ( 0 != downloadCommon ( url, json ) )
	{
		return -1 ;		
	}

	if ( version == json["version"] .asString () )
	{
		return 0 ;
	}

	saveCache ( "cabinet.json", json ) ;

	return processCabinet ( json ) ;
}

int CServerManager::processCabinet(Json::Value & json)
{
	Json::Value & jsonCabinets = json ["data"] ;

	int iSize = jsonCabinets .size () ;

	m_mutexCabinets .Lock () ;	
	m_hashCabinets .clear () ;
	m_cabinetVersion = json["version"] .asString () ;
	for ( int nIndex = 0 ; nIndex < iSize ; ++nIndex )
	{
		CABINETOBJECT	cabinet ;

		cabinet .name		=	jsonCabinets [nIndex] ["name"] .asString () ;
		cabinet .pid		=	jsonCabinets [nIndex] ["pid"] .asInt () ;
		cabinet .id		=	jsonCabinets [nIndex] ["id"] .asInt () ;
		cabinet .latitude	=	jsonCabinets [nIndex] ["latitude"] .asFloat () ;
		cabinet .longitude	=	jsonCabinets [nIndex] ["longitude"] .asFloat () ;

		Json::Value & jsonCamera = jsonCabinets [nIndex] ["cameras" ] ;

		int nCameraSize = jsonCamera .size () ;		
		for ( int nIdx = 0 ; nIdx < nCameraSize ; ++nIdx )
		{
			cabinet .cameras .push_back ( jsonCamera [nIdx] .asInt () ) ;
		}
		m_hashCabinets [ cabinet .id ] = cabinet ;
		printLog ( "the cabinet (%d:%s) has been parsed!", cabinet .id, cabinet .name .c_str () ) ;
	}	
	m_mutexCabinets .Unlock () ;
	
	return 0 ;
}

int CServerManager::downloadCameras()
{
	Json::Value	json ;
	std::string	version ;
	std::string	url = "camera?cmd=query&token=" + m_token ;

	m_mutexCameras .Lock () ;	
	version = m_cameraVersion ;
	m_mutexCameras .Unlock () ;

	if ( false == version .empty () )
	{
		url += "&version=" + version ;
	}

	if ( 0 != downloadCommon ( url, json ) )
	{
		return -1 ;		
	}

	if ( version == json["version"] .asString () )
	{
		return 0 ;
	}

	saveCache ( "camera.json", json ) ;

	return processCamera ( json ) ;
}

int CServerManager::processCamera(Json::Value & json)
{
	Json::Value & jsonCamera = json ["data"] ;

	int iSize = jsonCamera .size () ;

	m_mutexCameras .Lock () ;
	m_hashCameras .clear () ;
	m_cameraVersion = json["version"] .asString () ;
	for ( int nIndex = 0 ; nIndex < iSize ; ++nIndex )
	{
		CAMERAOBJECT	camera ;

		camera .id			=	jsonCamera [nIndex] ["id"] .asInt () ;
		camera .pid			=	jsonCamera [nIndex] ["pid"] .asInt () ;
		camera .name			=	jsonCamera [nIndex] ["name"] .asString () ;
		camera .type			=	jsonCamera [nIndex] ["type"] .asString () ;
		camera .authUser		=	jsonCamera [nIndex] ["auth_name"] .asString () ;
		camera .authPassword	=	jsonCamera [nIndex] ["auth_passwd"] .asString () ;
		camera .osd			=	jsonCamera [nIndex] ["osd"] .asString () ;
		camera .fps			=	jsonCamera [nIndex] ["fps"] .asString () ;
		camera .protocolIdx	=	jsonCamera [nIndex] ["protocol"] .asInt () ;

		Json::Value & protocols = jsonCamera [nIndex] ["protocols"] ;

		int nProtocolSize = protocols .size () ;		
		for ( int nIdx = 0 ; nIdx < nProtocolSize ; ++nIdx )
		{
			camera .protocols .push_back ( protocols [nIdx] .asString () ) ;
		}
		m_hashCameras [ camera .id ] = camera ;
		createDirectory ( getCameraPath ( camera .id ) .c_str () ) ;
		printLog ( "the camera (%d:%s) has been parsed!", camera .id, camera .name .c_str () ) ;
	}
	m_mutexCameras .Unlock () ;

	deque<TASKOBJECT>	queTask ;
	
	// 验证任务的合法性
	m_mutexTask .Lock () ;
	auto taskIter = m_hashTask .begin () ;
	for ( ; taskIter != m_hashTask .end () ; )
	{
		TASKOBJECT & task = (*taskIter) .second ;

		// 摄像头不存在
		m_mutexCameras .Lock () ;
		auto cameraIter = m_hashCameras .find ( task .camera ) ;
		if ( cameraIter == m_hashCameras .end () )
		{
			goto endTask ;
		}
		
		// 策略发生更改
		if ( task .policy != (*cameraIter) .second .policy )
		{			
			goto endTask ;
		}
		m_mutexCameras .Unlock () ;
		taskIter++ ;
		continue ;

	endTask:
		m_mutexCameras .Unlock () ;
		queTask .push_back ( task ) ;
		m_hashTask .erase ( taskIter++ ) ;
	}
	m_mutexTask .Unlock () ;

	for ( auto & stopIter : queTask )
	{
		TASKOBJECT & task = stopIter ;
		
		if ( 0 != task .child )
		{
			stopTask ( task ) ;
		}
	}
	
	return 0 ;
}

int CServerManager::loadCache(const  string & fileName, Json :: Value & json)
{
	std::fstream f ;
	std::string path = getCachePath () + "/" + fileName ;

	f .open ( path .c_str (), std::ios::in ) ;
	if ( !f .is_open () )
	{
		return -1 ;
	}

	std::string response ;
	std::stringstream buffer ;
	
	buffer << f .rdbuf () ;
	f .close () ;
	response = buffer .str () ;

	if ( 0 != verifyJson ( response, json ) )
	{
		return -2 ;
	}

	return 0 ;
}

int CServerManager::saveCache(const  string & fileName, Json :: Value & json)
{
	std::fstream f ;
	std::string path = getCachePath () + "/" + fileName ;

	f .open ( path .c_str (), std::ios::out ) ;
	if ( !f .is_open () )
	{
		return -1 ;
	}

	f << json .toStyledString () ;
	f .close () ;

	return 0 ;
}

int CServerManager::downloadCommon(const std::string & url, Json::Value & json)
{	
	std::string		body ;
	std::string		response ;
	std::string		request = "http://" + m_webServer + "/api/v1/" + url ;
	CHttpClient		webClient ;

	if ( 0 != webClient .posts ( request, body, response, NULL ) )
	{
		printLog ( "download %s failed!", request .c_str () ) ;
		return -1 ;
	}

	if ( 0 != verifyJson ( response, json ) )
	{
		printLog ( "parser %s failed!", request .c_str () ) ;
		printLog ( "the data is : %s", response .c_str () ) ;
		return -2 ;
	}
	
	return 0 ;
}

std::string CServerManager::getCachePath()
{
	return m_workPath + "/cache" ;
}

std::string CServerManager::getUploadPath()
{
	return m_workPath + "/upload" ;
}

std::string CServerManager::getRecorderPath()
{
	return m_workPath + "/recorder" ;
}

std::string CServerManager::getChildLogPath(pid_t pid)
{
	return m_workPath + "/child_" + std::to_string ( pid ) + ".log" ;
}

std::string CServerManager::getCameraPath(int id)
{
	return getRecorderPath () + "/" + std::to_string ( id ) ;
}

std::string CServerManager::getCameraFile(int id, const std::string & format)
{
	return getRecorderPath () + "/" + std::to_string ( id ) + "/%Y-%m-%d_%H-%M-%S." + format ;
}

std::string CServerManager::getCurrentTime()
{
	time_t calendar_time = ::time ( NULL ) ;
	// printf ( "calendar_time :%ld ", calendar_time ) ;

	struct tm * tm_local = ::localtime ( &calendar_time ) ;

	std::ostringstream os ;

	os << tm_local->tm_year + 1900 ;
	os << "-" ;
	os << tm_local->tm_mon + 1 ;
	os << "-" ;
	os << tm_local->tm_mday ;
	os << " " ;
	os << tm_local->tm_hour ;
	os << ":" ;
	os << tm_local->tm_min ;
	os << ":" ;
	os << tm_local->tm_sec ;

	return os .str () ;
}

TASKTIME CServerManager::getTaskTime()
{
	time_t calendar_time = ::time ( NULL ) ;
	// printf ( "calendar_time :%ld ", calendar_time ) ;

	// char * calendar_str = ::ctime ( &calendar_time ) ;
	// printf ( "calendar_str  :%s ", calendar_str ) ;

	struct tm * tm_local = ::localtime ( &calendar_time ) ;
#if 0
	printf ( "localtime :year=%d mon=%d mday=%d hour=%d min=%d sec=%d   wday=%d yday=%d isdst=%d ", 
		tm_local->tm_year + 1900, 
		tm_local->tm_mon + 1, 
		tm_local->tm_mday, 
		tm_local->tm_hour, 
		tm_local->tm_min, 
		tm_local->tm_sec,
		tm_local->tm_wday, 
		tm_local->tm_yday, 
		tm_local->tm_isdst ) ;
#endif

	TASKTIME	tt ;

#if 0
	tt .dayPoint .year		=	tm_local->tm_year + 1900 ;
	tt .dayPoint .month		=	tm_local->tm_mon + 1 ;
	tt .dayPoint .day		=	tm_local->tm_mday ;
#endif
	tt .timePoint .hour		=	tm_local->tm_hour ;
	tt .timePoint .minute	=	tm_local->tm_min ;
	tt .timePoint .seconds	=	tm_local->tm_sec ;

	switch ( tm_local ->tm_wday )
	{
	case wdSun 			:	tt .weekDay 	=	wdSun ;			break ;
	case wdMonday		:	tt .weekDay 	=	wdMonday ;		break ;
	case wdTuesday		:	tt .weekDay 	=	wdTuesday ;		break ;
	case wdWednesday	:	tt .weekDay 	=	wdWednesday ;	break ;
	case wdThursday		:	tt .weekDay 	=	wdThursday ;		break ;
	case wdFriday		:	tt .weekDay 	=	wdFriday ;		break ;
	case wdSaturday		:	tt .weekDay 	=	wdSaturday ;		break ;
	default 				:	tt .weekDay 	=	wdNone ;			break ;
	}		

	return tt ;
}

int CServerManager::dispatchTask()
{
	TASKTIME taskTime = getTaskTime () ;
	
	if ( taskTime .weekDay == wdNone )
	{
		return -1 ;
	}

	int nTimePoint = taskTime .timePoint .getValue () ;

	m_mutexTask .Lock () ;
	for ( auto & taskIter : m_hashTask )
	{
		TASKOBJECT & task = taskIter .second ;
		QueTimeSpan & timeSpan = task .weeks [ taskTime .weekDay ] ;

		bool	bStart = false ;
		for ( auto & timeIter : timeSpan )
		{
			TIMESPAN & span = timeIter ;

			if ( nTimePoint >= span .from .getValue () && nTimePoint < span .to .getValue () )
			{
			#if 0
				printLog ( "task hour : %d, minute : %d, seconds : %d",
					taskTime .timePoint .hour, taskTime .timePoint .minute, taskTime .timePoint .seconds ) ;
				printLog ( "span from hour : %d, minute : %d, seconds : %d; to hour : %d, minute : %d, seconds : %d",
					span .from .hour, span .from .minute, span .from .seconds,
					span .to .hour, span .to .minute, span .to .seconds ) ;
			#endif
				bStart = true ;
				break ;
			}
		}
		
		// 判断是否执行新任务
		if ( true == bStart )
		{
			if ( task .child == 0 )	{	startTask ( task ) ;		}
		}
		// 判断是否有任务需要结束
		else
		{
			if ( task .child != 0 )	{	stopTask ( task ) ;		}
		}		
	}
	m_mutexTask .Unlock () ;
	
	return 0 ;
}

int CServerManager::taskExit(pid_t pid, int status)
{
#if 0
	if (WIFEXITED(stat))        
	{
		printf("Child exited with code %d\n", WEXITSTATUS(status)); 
	}
	else if (WIFSIGNALED(stat))
	{
		printf("Child terminated abnormally, signal %d\n", WTERMSIG(status));
	}
#endif
	
	m_mutexTask .Lock () ;
	for ( auto & iter : m_hashTask )
	{
		TASKOBJECT & task = iter .second ;

		if ( task .child != pid )
		{
			continue ;
		}
		task .child = 0 ;
		printLog ( "task (%d:%d) exit end", pid, status ) ;
		break ;
	}
	m_mutexTask .Unlock () ;	

	std::string childLog =  getChildLogPath ( pid ) ;
	::unlink ( childLog .c_str () ) ;
	
	return 0 ;
}

int CServerManager::createTask(int id)
{
	m_mutexCameras .Lock () ;
	auto iter = m_hashCameras .find ( id ) ;
	if ( iter == m_hashCameras .end () )
	{
		m_mutexCameras .Unlock () ;
		printLog ( "camera (%d) create task failed1 ", id ) ;
		return -1 ;
	}

	CAMERAOBJECT camera = (*iter) .second ;
	m_mutexCameras .Unlock () ;

	// 校验协议列表和索引
	if ( camera .protocolIdx >= camera .protocols .size () || camera .protocolIdx < 0 )
	{
		printLog ( "camera (%d) create task failed2 ", id ) ;
		return -2 ;
	}

	bool 			bfind ( false ) ;
	TASKOBJECT		taskObject ;

	taskObject .child		=	0 ;
	taskObject .camera	=	camera .id ;
	taskObject .policy		=	camera .policy ;
	taskObject .param .url 	=	camera .protocols .at ( camera .protocolIdx ) ;

	m_mutexPolicy .Lock () ;
	for ( auto & policyIter : m_quePolicy )
	{
		POLICYOBJECT & policy = policyIter ;
		if ( policy .id != camera .policy )
		{			
			continue ;
		}

		for ( int i = 0 ; i < 7 ; i++ )
		{
			taskObject .weeks [i] = policy .weeks [i] ;
		}
		bfind = true ;
		taskObject .param .time = std::to_string ( policy .segment ) ;
		taskObject .param .format = policy .format ;
		taskObject .param .path = getCameraFile ( camera .id, policy .format ) ;
		break ;
	}
	m_mutexPolicy .Unlock () ;

	if ( false == bfind )
	{
		printLog ( "camera (%d) create task failed3 ", id ) ;
		return -3 ;
	}

	m_mutexTask .Lock () ;
	auto  taskIter = m_hashTask .find ( camera .id ) ;
	if ( taskIter != m_hashTask .end () )
	{
		(*taskIter) .second = taskObject ;
	}
	else
	{
		m_hashTask [camera .id] = taskObject ;
	}
	m_mutexTask .Unlock () ;

	printLog ( "camera (id: %d, policy: %d) create task success.", id, camera .policy ) ;

	return 0 ;
}

int CServerManager::startTask(TASKOBJECT & taskObject)
{	
#if 1
	char * argv [] = { 
		"ffmpeg", 
		"-i", 
		(char *) taskObject .param .url .c_str (), 
		"-vcodec", "copy", 
		"-acodec", "copy", 
		"-f", "segment", 
		"-strftime", "1", 
		"-segment_time", 
		(char *) taskObject .param .time .c_str (), 
		"-segment_format", 
		(char *) taskObject .param .format .c_str (), 
		(char *) taskObject .param .path .c_str (),
		0 } ;
#endif

	int i = 0 ;
	while ( argv[i] != NULL )
	{
		writeLog ( argv[i] ) ;
		writeLog ( " " ) ;
		i++ ;
	}
	writeLog ( "\n" ) ;

	pid_t pid = ::fork () ;

	if ( pid < 0)
	{
		printLog ( "start camera (%d) task (%d) failed.", taskObject .camera, pid ) ;
		return -1 ;
	}
	else if ( 0 == pid )
	{
		/////////////////////////////////////////////////////////////////////////////////
		//		
		// fork 多线程程序，只fork 当前调用fork 的线程到子进程，但文件描述符是所有的
		//
		/////////////////////////////////////////////////////////////////////////////////
		for ( int i = 3 ; i < ::sysconf ( _SC_OPEN_MAX ) ;  i++ )
		{
			::close ( i ) ;
		}

		std::string childLog = getChildLogPath ( getpid () ) ;

		::freopen( "/dev/null", "r", stdin );
		::freopen ( childLog .c_str (), "a+", stdout ) ; 
		::freopen ( childLog .c_str (), "a+", stderr ) ; 
		
		printf ( "child process start.\n" ) ;
		
		// ffmpeg -i rtmp://192.168.189.152:1935/media/b.mp4 
		//            -vcodec copy 
		//            -acodec copy 
		//            -f segment 
		//            -strftime 1 
		//            -segment_time 10 
		//            -segment_format mp4 
		//            %Y-%m-%d_%H-%M-%S.mp4
		
		int ret = ::execvp ( argv[0], argv ) ;		
		if ( -1 == ret )
		{
			printf ( "execvp child process failed.\n" ) ;
		}
		printf ( "exiting child process ----\n" ) ;
		exit(0);
	}

	taskObject .child = pid ;		
	printLog ( "start camera (%d) task (%d) success.", taskObject .camera, pid ) ;

	return 0 ;
}

int CServerManager::stopTask(TASKOBJECT & taskObject)
{
	::kill ( taskObject .child, SIGKILL ) ;		
	printLog ( "stop camera (%d) task (%d) success.", taskObject .camera, taskObject .child ) ;
	
	return 0 ;
}

int CServerManager::recycleExpired()
{
	m_bExpired = true ;

	// printLog ( "recycle expired execute!" ) ;

	std::deque<EXPIREDPOLICY>	queCameras ;
	
	m_mutexCameras .Lock () ;
	for ( auto & cameraIter : m_hashCameras )
	{
		EXPIREDPOLICY	expired ;
		CAMERAOBJECT & 	camera = cameraIter .second ;

		expired .expired = camera .policy ;
		expired .path 	= getCameraPath ( camera .id ) ;
		queCameras .push_back ( expired ) ;
	}
	m_mutexCameras .Unlock () ;

	m_mutexPolicy .Lock () ;
	for ( auto & cameraPolicy : queCameras )
	{
		bool bfind = false ;
		for ( auto & policyIter : m_quePolicy )
		{
			POLICYOBJECT & policy = policyIter ;
			if ( cameraPolicy .expired != policy .id )
			{
				continue ;
			}			
			bfind = true ;
			cameraPolicy .expired = policy .expired ;
			break ;
		}

		if ( false == bfind )
		{
			cameraPolicy .expired = -1 ;
		}
	}
	m_mutexPolicy .Unlock () ;

	std::deque<tstring> queDirName ;	
	std::string sPath 	= getRecorderPath () ;

	recursionDir ( sPath .c_str (), NULL, queDirName ) ;

	auto dirIter = queDirName .begin () ;
	for ( ; dirIter != queDirName .end () ; )
	{
		// 查找此目录是否存在
		auto tmpIter = std::find_if ( std::begin ( queCameras ), std::end ( queCameras ), 
			[&](const EXPIREDPOLICY & value){ return ( (sPath + "/" + *dirIter) == value .path ); } ) ;
		// 存在则删除
		if ( tmpIter != queCameras .end () )
		{
			dirIter = queDirName .erase ( dirIter ) ;
			continue ;
		}
		dirIter ++ ;		
	}

	// 回收不存在的摄像头的目录
	for ( auto & dirIter : queDirName )
	{
		std::string command = "rm -rf " + sPath + "/" + dirIter ;
		::system ( command .c_str () ) ;
		printLog ( command .c_str () ) ;
	}
	queDirName .clear () ;

	// 回收过期的视频文件
	for ( auto & expiredIter : queCameras )
	{
		if ( expiredIter .expired == 0 )
		{
			continue ;
		}

		std::deque<FILEINFO>  queFileInfo ;

		recursionFile ( expiredIter .path .c_str (), NULL, queFileInfo ) ;
		
		for ( auto & fileInfo : queFileInfo )
		{
			time_t 		calendar_time = ::time ( NULL ) ;
			struct tm * 	tm_local = ::localtime ( &calendar_time ) ;

			std::string sFilePath = expiredIter .path + "/" + fileInfo .sName ;
			int nDay = diffDay ( *tm_local, fileInfo .tmCreate ) ;			
			if ( nDay > expiredIter .expired )
			{				
				::unlink ( sFilePath .c_str () ) ;
				printLog ( "rm %s", sFilePath .c_str () ) ;
			}
		}
	}

	m_bExpired = false ;
	
	return 0 ;
}

