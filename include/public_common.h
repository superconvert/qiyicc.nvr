#ifndef __PUBLIC_COMMON_20181118074800_H
#define __PUBLIC_COMMON_20181118074800_H

// 屏蔽掉编译错误
#ifdef WIN32
#pragma warning (disable:4786)
#endif

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <dirent.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <sstream>
#include <iostream>  
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <sys/types.h>
#include <list>
#include <map>
#include <string>
#include <deque>
#include <string.h>
#include <unordered_map>

using std::list ;
using std::map ;
using std::string ;
using std::deque ;

// 兼容windowns UNICODE 格式定义
#define _T(x)				x
#define TCHAR			char
#define tstrlen			strlen
#define tstrcpy			strcpy
#define tstrncpy			strncpy
#define tstrcmp			strcmp
#define tstrncmp     		strncmp
#define tstrcat                 	strcat
#define tstrstr         		strstr
#define tprintf			printf
#define tfprintf			fprintf
#define tsprintf			sprintf
#define tstring 			std::string

typedef unsigned int		DWORD ;

typedef enum WeekDay
{
	// 未定义
	wdNone			=	-1,
	// 周日
	wdSun			=	0,
	// 周一
	wdMonday 		=	1,
	// 周二
	wdTuesday 		=	2,
	// 周三
	wdWednesday 	=	3,
	// 周四
	wdThursday 		=	4,
	// 周五
	wdFriday 		=	5,
	// 周六
	wdSaturday 		=	6
}WEEKDAY ;

typedef struct _tagDayPoint
{
	// 年
	int				year ;
	// 月
	int				month ;
	// 日
	int				day ;
}DAYPOINT, *LPDAYPOINT ;

typedef struct _tagTimePoint
{
	// 时
	int				hour ;
	// 分
	int				minute ;
	// 秒
	int				seconds ;

	_tagTimePoint() : hour(0), minute(0), seconds(0)
	{}

	// 获取值
	int getValue()
	{
		return hour * 3600 + minute * 60 + seconds ;
	}		
}TIMEPOINT, *LPTIMEPOINT ;

typedef struct _tagTaskTime
{
	// 日期点
	DAYPOINT	dayPoint ;
	// 时间点
	TIMEPOINT	timePoint ;
	// 周几
	WEEKDAY	weekDay ;	
}TASKTIME, *LPTASKTIME ;

typedef struct _tagExpiredPolicy
{
	// 超时策略
	int			expired ;
	// 路径
	std::string	path ;
}EXPIREDPOLICY, *LPEXPIREDPOLICY ;

typedef struct _tagAlarmParam
{
	// IM 通知
	std::string		im ;
	// 短信通知
	std::string		sms ;
	// 邮件通知
	std::string		email ;
}ALARMPARAM, *LPALARMPARAM ;

typedef struct _tagCameraParam
{
	// 视频大小
	std::string		videoSize ;
	// 视频采样
	std::string		videoSample ;
	// 视频编码 h264
	std::string		videoEncoder ;
	// 音频采样
	std::string		audioSample ;
	// 音频编码aac
	std::string		audioEncoder ;
}CAMERAPARAM, *LPCAMERAPARAM ;

typedef std::deque<int>	QueCameras ;

typedef struct _tagCabinetObject
{
	// NVR ID
	int				pid ;
	// 柜体ID
	int				id ;
	// 柜体名称
	std::string		name ;
	// 柜体纬度
	double			latitude ;
	// 柜体经度
	double			longitude ;
	// 柜体的摄像头列表
	QueCameras		cameras ;
}CABINETOBJECT, *LPCABINETOBJECT ;

typedef std::deque<std::string>		QueProtocols ;

typedef struct _tagCameraObject
{
	// 柜体ID
	int				pid ;
	// 摄像头ID
	int				id ;
	// 策略编号
	int				policy ;
	// 摄像头名称
	std::string		name ;
	// 摄像头类型
	std::string		type ;
	// 验证名
	std::string		authUser ;
	// 验证密码
	std::string		authPassword ;
	// 参数设置
	std::string		osd ;
	// 帧率设置
	std::string		fps ;
	// 默认录制协议索引
	unsigned int		protocolIdx ;
	// 支持的写协议列表"onvif", "rtmp", "http"
	QueProtocols		protocols ;
}CAMERAOBJECT, *LPCAMERAOBJECT ;

typedef struct _tagTimeSpan
{
	// 开始录制时间
	TIMEPOINT		from ;
	// 结束录制时间
	TIMEPOINT		to ;
}TIMESPAN, *LPTIMESPAN ;

typedef std::deque<TIMESPAN>	QueTimeSpan;

typedef struct _tagPolicyObject
{
	// 策略ID
	int				id ;
	// 策略名称
	std::string		name ;
	// 存储空间 0  表示不限制
	int				storage ;
	// 存储文件过期时间0  表示永不过期
	int				expired ;
	// 录像文件时长
	int				segment ;
	// 录像预览图片大小 320x240
	std::string		preview ;
	// 录像文件格式mp4, flv, hls
	std::string		format ;
	// 录像参数设置
	CAMERAPARAM	camera ;
	// 报警参数设置
	ALARMPARAM		alarm ;
	// 录制时间段
	QueTimeSpan		weeks[7] ;
}POLICYOBJECT, *LPPOLICYOBJECT ;

typedef struct _tagTaskParam
{
	// 录制的URL
	std::string		url ;	
	// 录制时长
	std::string		time ;
	// 录制的格式
	std::string		format ;
	// 保存的文件格式
	std::string 		path ;
}TASKPARAM, *LPTASKPARAM ;

typedef enum TaskState
{
	// 初始化
	stateInit	=	0,
	// 开始
	stateBegin	=	1,
	// 空闲
	stateIdle	=	2,
	// 开始
	stateStart	=	3,
	// 暂停
	statePause	=	4,
	// 运行
	stateRun	=	5,
	// 停止
	stateStop	=	6,
	// 结束
	stateEnd	=	7,
}TASKSTATE;

typedef struct _tagTaskObject
{
	// 摄像头编号
	int				camera ;
	// 策略ID
	int				policy ;
	// 对应的进程
	pid_t			child ;
	// 任务参数
	TASKPARAM		param ;
	// 任务时间段
	QueTimeSpan		weeks[7] ;
}TASKOBJECT, *LPTASKOBJECT ;

// 文件大小
typedef struct _tagFileInfo
{
	tstring		sName ;							// 文件名字
	DWORD		nFileSizeHigh; 					// 文件大小
	DWORD		nFileSizeLow;					// 文件大小
	struct tm 	tmCreate ;						// 文件日期
} FILEINFO, *LPFILEINFO ;

// 服务器端未决用户超时结构定义
typedef struct _tagTimeState
{
#ifdef WIN32
	DWORD				tmLogon ;					// 登录时间
#else
	struct timeval		tmLogon ;					// 登录时间
#endif	

	_tagTimeState(bool bTime=false)
	{
		if ( bTime )
		{
			ResetTime () ;				
		}
		else
		{
#ifdef WIN32
			tmLogon			=	0 ;
#else
			tmLogon .tv_sec	=	0 ;
			tmLogon .tv_usec	=	0 ;
#endif					
		}
	}

	_tagTimeState & operator=(const _tagTimeState & rhs)
	{
		tmLogon	=	rhs .tmLogon ;					// 登录时间
		return *this ;
	}

	void ResetTime()
	{
#ifdef WIN32
		tmLogon		=	::GetTickCount () ;		
#else
		// clock 在AS4 (64) 的机器上返回0，不知道什么原因
		::gettimeofday ( &tmLogon, NULL ) ;
#endif
	}

	void ResetTime(int nRemainTime)
	{
#ifdef WIN32
		tmLogon			=	::GetTickCount () - nRemainTime ;		
#else
		// clock 在AS4 (64) 的机器上返回0，不知道什么原因
		::gettimeofday ( &tmLogon, NULL ) ;
		nRemainTime	=	tmLogon .tv_sec * 1000 + tmLogon .tv_usec / 1000 - nRemainTime ;
		tmLogon .tv_sec	=	nRemainTime / 1000 ;
		tmLogon .tv_usec	=	(nRemainTime - tmLogon .tv_sec * 1000) * 1000 ;
#endif

	}

	// 是否超时
	bool IsTimeOut(int nTimeOut)
	{
#ifdef WIN32
		DWORD	tmCurrent 	=	::GetTickCount () ;
		int		duration 	=	tmCurrent - tmLogon ;
#else
		struct timeval	tmCurrent ;
		::gettimeofday ( &tmCurrent, NULL ) ;
		
		int		duration 	=	( tmCurrent .tv_sec - tmLogon .tv_sec ) * 1000 + ( tmCurrent .tv_usec - tmLogon .tv_usec )  / 1000 ;
#endif

		return (duration >= nTimeOut) ? true : false ;
	}

	// 是否超时
	bool IsTimeOut(_tagTimeState & time, int nTimeOut)
	{
#ifdef WIN32
		int	duration = time .tmLogon - tmLogon ;
#else
		
		int	duration = ( time .tmLogon .tv_sec - tmLogon .tv_sec ) * 1000 + ( time .tmLogon .tv_usec - tmLogon .tv_usec )  / 1000 ;
#endif

		return (duration >= nTimeOut) ? true : false ;
	}

	// 获取标示
	DWORD ToToken()
	{
#ifdef WIN32
		return tmLogon ;
#else
		return DWORD(( tmLogon .tv_sec ) * 1000 + ( tmLogon .tv_usec )  / 1000) ;
#endif
	}

	// 持续时间
	int Duration()
	{
#ifdef WIN32
		DWORD	tmCurrent 	=	::GetTickCount () ;
		// 转化为毫秒
		int		duration 	=	tmCurrent - tmLogon ;
#else
		struct timeval	tmCurrent ;
		::gettimeofday ( &tmCurrent, NULL ) ;

		// 转化为毫秒
		int		duration 	=	( tmCurrent .tv_sec - tmLogon .tv_sec ) * 1000 + ( tmCurrent .tv_usec - tmLogon .tv_usec )  / 1000 ;
#endif	
		return duration ;
	}
}TIMESTATE, *LPTIMESTATE ;

typedef std::deque<pid_t>								QueExitTask ;
typedef std::deque<POLICYOBJECT>						QuePolicyObject ;
typedef std::unordered_map<int, CABINETOBJECT>		HashCabinetObject ;
typedef std::unordered_map<int, CAMERAOBJECT>			HashCameraObject ;
typedef std::unordered_map<int, TASKOBJECT>			HashTaskObject ;

static std::string int_str(int id)
{
	std::ostringstream os ;
	os << id ;
	return os .str () ;
}

static int safe_strcpy(TCHAR * strDest, const TCHAR * strSource, size_t count)
{
        size_t length = tstrlen ( strSource ) ;

        if ( length == 0 )
        {
                return 0 ;
        }

        if ( length < count )
        {
                count = length ;
        }
        tstrncpy ( strDest, strSource, count ) ;
        strDest[count] = 0 ;
        return 0 ;
}

static void string_split(const std::string & input, const std::string & find, std::vector<std::string> & vValue)
{
	std::string         strOut ( input ) ;
	
       while ( true )
	{
		size_t pos = strOut .find ( find ) ;
		if ( pos == string::npos )
		{
			if ( false == strOut .empty () )
			{
				vValue .push_back ( strOut ) ;
			}
			break ;
		}

		std::string sHostIP = strOut .substr ( 0, pos ) ;
		strOut .erase ( 0, pos + find .size () ) ;
		vValue .push_back ( sHostIP ) ;
	}
}

static bool createDirectory(const TCHAR * szPath)
{
        if ( szPath == NULL || tstrlen ( szPath ) <= 0 )
        {
                return false ;
        }

#ifdef WIN32

        tstring sPath = ToplevelStudio::string_replace ( szPath, _T("/"), _T("\\") ) ;

        WIN32_FIND_DATA         wfd ;
        HANDLE                          hFind = FindFirstFile ( sPath .c_str (), &wfd ) ;
        if ( (hFind != INVALID_HANDLE_VALUE) && (wfd .dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
        {
                FindClose ( hFind ) ;
                return true ;
        }
        FindClose ( hFind ) ;

        if ( _T('\\') != (*sPath .rbegin ()) )
        {
                sPath += _T('\\') ;
        }

        bool bSuccess = false;
        tstring strTemp;
        std::vector<tstring> vPath;

        for ( uint32_t i = 0 ; i < sPath .size () ; ++i )
        {
                if ( sPath .at ( i ) != _T('\\') )
                {
                        strTemp += sPath .at ( i );
                }
                else
                {
                        vPath .push_back ( strTemp ) ;
                        strTemp += _T('\\') ;
                }
        }

        std::vector<tstring>::const_iterator vIter ;
        for ( vIter = vPath .begin () ; vIter != vPath .end () ; vIter++ )
        {
                if ( TRUE == ::CreateDirectory ( (*vIter) .c_str (), NULL ) || 183 == ::GetLastError () )
                {
                        bSuccess = true ;
                }
                else
                {
                        bSuccess = false ;
                        ToplevelStudio::Log .Error ( FILELINE, _T("createDirectory - %s failed ( %d )!"), sPath .c_str (), ::GetLastError () ) ;
                }
        }

        return bSuccess ;
		
#else

        char szNewPath [260] = {0} ;
        safe_strcpy ( szNewPath, szPath, 256 ) ;

        int len = ::strlen ( szNewPath ) ;
        if ( szNewPath [ len - 1] != '/' )
        {
                ::strcat ( szNewPath, "/" ) ;
        }

        len += 1 ;

        for ( int i = 1 ; i < len ; i++ )
        {
                if ( szNewPath [i] == '/' )
                {
                        szNewPath [i] = 0 ;
                        if ( ::access ( szNewPath, R_OK ) != 0 )
                        {
                                if ( ::mkdir ( szNewPath, 0755 ) == -1 )
                                {
                                        if ( errno == EEXIST )
                                        {
                                                continue ;
                                        }
                                        return false ;
                                }
                        }
                        szNewPath [i] = '/' ;
                }
        }
		
#endif

        return true ;
}

// 是否是根目录
static bool isRootPath(const TCHAR * szPath)
{
	TCHAR		szRoot [32]	=	{0} ;
	tsprintf ( szRoot, _T("%c:\\"), szPath [0] ) ; 
	return ( tstrcmp ( szRoot, szPath ) == 0 ) ; 
}

// 遍历一个文件夹中的所有目录
static int recursionDir(const TCHAR * pszPath, const TCHAR * pszHook, deque<tstring> & listDirName)
{
	if ( pszPath == NULL )
	{
		return -1 ;
	}

#ifdef WIN32
	TCHAR		szFind [ 258 ]	=	{0} ;

	safe_strcpy ( szFind, pszPath, tstrlen ( pszPath ) > 256 ? 256 : tstrlen ( pszPath ) ) ; 

		if ( !isRootPath ( szFind ) )
		{
			tstrcat ( szFind, _T("\\") ) ; 
		}

	tstrcat ( szFind, _T("*.*") ) ; // 找所有文件 

	WIN32_FIND_DATA 	wfd ;
	HANDLE 				hFind = ::FindFirstFile ( szFind, &wfd ) ;
	// 如果没有找到或查找失败 
	if ( hFind == INVALID_HANDLE_VALUE ) 
	{
		return -2 ;
	}

	do
	{
		// 过滤这两个目录 
		if ( wfd .cFileName [0] == _T('.') ) 
		{
			continue ;
		}

		if ( (wfd .dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FALSE )
		{
			continue ;
		}

		if ( pszHook && tstrstr ( wfd .cFileName, pszHook ) )
		{
			listDirName .push_back ( tstring ( wfd .cFileName ) ) ;
		}
		else if ( NULL == pszHook )
		{
			listDirName .push_back ( tstring ( wfd .cFileName ) ) ;
		}
		
		// 对文件进行操作 
	}while ( ::FindNextFile ( hFind, &wfd ) ) ;

	::FindClose ( hFind ) ;
#else
	struct stat 		info ;
	struct dirent *	dirent ;
	DIR * 			dir 				= 	NULL ;

	if ( ( ::lstat ( pszPath, &info ) ) != 0 ) 
	{
		return -2 ;
       }
		
	dir = ::opendir ( pszPath ) ;
	if ( dir == NULL )
	{
		return -3 ;
	}
	
	// 读取目录里的所有内容
	while ( ( dirent = ::readdir ( dir ) ) != 0 )
	{
		char		szFilename [ 512 ]	=	{0} ;		
		tsprintf ( szFilename, "%s/%s", pszPath, dirent->d_name ) ;
		
		if ( ::stat ( szFilename, &info ) != 0 ) 
		{
			continue ;
		}

		if ( ::strcmp ( dirent->d_name, "..") == 0 || ::strcmp ( dirent->d_name, ".") == 0 )
		{
			continue ;
		}

		//"<td> 文件大小</td>"
		if (S_ISREG(info.st_mode))					{}
		// 如果是目录，则迭代获取大小
		else if ( S_ISDIR ( info.st_mode ) )			
		{
			if ( pszHook && ::strstr ( dirent->d_name, pszHook ) )
			{
				listDirName .push_back ( dirent->d_name ) ;
			}				
			else if ( NULL == pszHook )
			{
				listDirName .push_back ( dirent->d_name ) ;
			}
		}
		// "<td>链接</td>"
		else if (S_ISLNK(info.st_mode))				{}
		// "<td>字符设备</td>" 
		else if (S_ISCHR(info.st_mode))				{}
		// "<td>块设备</td>"
		else if (S_ISBLK(info.st_mode))				{}
		// "<td>FIFO</td>"
		else if (S_ISFIFO(info.st_mode))				{}
		// "<td>Socket</td>"
		else if (S_ISSOCK(info.st_mode))			{}
		// "<td>(未知)</td>"
		else										{}		

	}

	::closedir ( dir ) ;
#endif

	return 0 ;	

}


// 遍历一个文件夹中的所有文件
static int recursionFile(const TCHAR * pszPath, const TCHAR * pszFilter, deque<FILEINFO> & listFileInfo)
{
	if ( pszPath == NULL )
	{
		return -1 ;
	}

#ifdef WIN32
	TCHAR	szFind [ 258 ]	=	{0} ;
	tstring	sPath = string_replace ( pszPath, _T("/"), _T("\\") ) ;

	safe_strcpy ( szFind, sPath .c_str (), sPath .size () > 256 ? 256 : sPath .size () ) ; 

		if ( !isRootPath ( szFind ) )
		{
			tstrcat ( szFind, _T("\\") ) ; 
		}

	tstrcat ( szFind, _T("*.*") ) ; // 找所有文件 

	WIN32_FIND_DATA 	wfd ;
	HANDLE 				hFind = ::FindFirstFile ( szFind, &wfd ) ;
	// 如果没有找到或查找失败 
	if ( hFind == INVALID_HANDLE_VALUE ) 
	{
		return -2 ;
	}

	do
	{
		// 过滤这两个目录 
		if ( wfd .cFileName [0] == _T('.') ) 
		{
			continue ;
		}

		if ( wfd .dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
		{
			continue ;
		}

		if ( pszFilter && tstrstr ( wfd .cFileName, pszFilter ) )
		{
			continue ;
		}

		FILEINFO		fileInfo ;
		SYSTEMTIME	time ;
		
		::FileTimeToSystemTime ( &wfd .ftCreationTime, &time ) ;

		/*
		char		Date[512] = {0} ;

		::memset ( szDate, 0, 512 ) ;
		::sprintf ( szDate, "%04d-%02d-%02d %02d:%02d:%02d", time .wYear, 
			time .wMonth, time .wDay, time .wHour, time .wMinute, time .wSecond ) ;
		*/

		fileInfo .sName				=	tstring ( wfd .cFileName ) ;		
		fileInfo .nFileSizeLow			=	wfd .nFileSizeLow ;
		fileInfo .nFileSizeHigh			=	wfd .nFileSizeHigh ;

		fileInfo .tmCreate .tm_year		=	time .wYear - 1900 ;
		fileInfo .tmCreate .tm_mon		=	time .wMonth - 1 ;
		fileInfo .tmCreate .tm_mday		=	time .wDay ;
		fileInfo .tmCreate .tm_hour		=	time .wHour ;
		fileInfo .tmCreate .tm_min		=	time .wMinute ;
		fileInfo .tmCreate .tm_sec		=	time .wSecond ;		
		
		listFileInfo .push_back ( fileInfo ) ;
		// 对文件进行操作 
	}while ( ::FindNextFile ( hFind, &wfd ) ) ;

	::FindClose ( hFind ) ;
#else
	struct stat 		info ;
	struct dirent *	dirent ;
	DIR * 			dir 			= 	NULL ;

	if ( ( ::lstat ( pszPath, &info ) ) != 0 ) 
	{
		return -2 ;
       }
		
	dir = ::opendir ( pszPath ) ;
	if ( dir == NULL )
	{
		return -3 ;
	}
	
	// 读取目录里的所有内容
	while ( ( dirent = ::readdir ( dir ) ) != 0 )
	{
		TCHAR		szFilename [ 512 ]	=	{0} ;		
		tsprintf ( szFilename, "%s/%s", pszPath, dirent->d_name ) ;
		
		if ( ::stat ( szFilename, &info ) != 0 ) 
		{
			continue ;
		}

		if ( ::strcmp ( dirent->d_name, "..") == 0 || ::strcmp ( dirent->d_name, ".") == 0 )
		{
			continue ;
		}

		//"<td> 文件大小</td>"
		if ( S_ISREG ( info.st_mode ) )
		{
			if ( pszFilter && ::strstr ( dirent->d_name, pszFilter ) )
			{
				continue ;
			}

			FILEINFO		fileInfo ;
			struct tm *	time = ::localtime ( &info .st_mtime ) ;

			/*
			time->tm_year += 1900 ;
			time->tm_mon ++ ;
			sDate += outputString ( FILELINE, "%04d-%02d-%02d %02d:%02d:%02d", time->tm_year, 
				time->tm_mon, time->tm_mday, time ->tm_hour, time ->tm_min, time ->tm_sec ) ;
			*/
			
			fileInfo .sName		=	dirent->d_name ;
			fileInfo .tmCreate		=	* time ;
			fileInfo .nFileSizeLow	=	info .st_size ;				
			listFileInfo .push_back ( fileInfo ) ;
		}
		// 如果是目录，则迭代获取大小
		else if ( S_ISDIR ( info.st_mode ) )			{}
		// "<td>链接</td>"
		else if ( S_ISLNK ( info.st_mode ) )			{}
		// "<td>字符设备</td>" 
		else if ( S_ISCHR ( info.st_mode ) )			{}
		// "<td>块设备</td>"
		else if ( S_ISBLK ( info.st_mode ) )			{}
		// "<td>FIFO</td>"
		else if ( S_ISFIFO ( info.st_mode ) )			{}
		// "<td>Socket</td>"
		else if ( S_ISSOCK(info.st_mode ) )			{}
		// "<td>(未知)</td>"
		else										{}		

	}

	::closedir ( dir ) ;
#endif

	return 0 ;	
}

static void printLog(const char * sFmt, ...)
{
#define BIGLOGSIZEX 	2056
#define SMALLLOGSIZE 2048

	va_list			marker ; 		
	char 			sBuf [ BIGLOGSIZEX ] ;		
	time_t   			tmNow;   
	struct tm *		newtime ;	

	::time ( &tmNow ) ;   
	newtime   =  ::localtime(&tmNow); 		
	::memset ( sBuf, 0, BIGLOGSIZEX ) ;
	::strftime ( sBuf, 128, "%Y-%m-%d %H:%M:%S manager: ", newtime ) ;	
		
	va_start ( marker, sFmt ) ; 			
	// 2018-11-18 11:48:00 manager: 
	// |         19			|      9      | 1 |
#ifdef WIN32
	::_vsnprintf ( sBuf + 29, SMALLLOGSIZE, sFmt, marker ) ;
#else
	::vsnprintf ( sBuf + 29, SMALLLOGSIZE, sFmt, marker ) ;
#endif
	va_end ( marker ) ;

	std::cout << sBuf << std::endl ;
	
	return;
}


#endif // __PUBLIC_COMMON_20181118074800_H

