#include<time.h>
#include <stdio.h>
#include<stdlib.h>
#include<string.h>
#include <dirent.h>
#include<iostream>
#include <unistd.h>
#include<sys/types.h>
#include<sys/inotify.h>
#include "file_monitor.h"

#define EVENT_SIZE  		( sizeof (struct inotify_event) )
#define BUFFER_LEN     	( 1024 * ( EVENT_SIZE + 16 ) )

CFileMonitor::CFileMonitor(std::string path, ListenerProc proc, int expire)
{
	m_expire = 60;//expire*24*60*60;
	listener = proc ;
	
	m_fd = ::inotify_init () ;
	if ( m_fd < 0 )
	{
		std::cout<<"innotify_init failed"<< std::endl;
	}

	watchFolder ( path ) ;
}

CFileMonitor::~CFileMonitor()
{
	//移除监视
	for (auto & iter : monitorMap )
	{
		::inotify_rm_watch ( m_fd, iter .first ) ; 
	}
	monitorMap .clear () ;
	::close ( m_fd ) ;
}

int CFileMonitor::start()
{
	bool bRun = true ;
	char buffer [ BUFFER_LEN ] ;	
	
	while ( bRun )
	{
		int length=0,i=0;
		length = ::read ( m_fd, buffer, BUFFER_LEN ) ;
		if ( length < 0 )
		{
			std::cout << "read none" << std::endl;
		}

		while ( i < length )
		{
			struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];
			if ( event->len &&event->name[0]!='.'&&event->name[strlen(event->name)-1]!='~') //同时过滤隐藏文件和临时文件
			{
				if ( event->mask & IN_CREATE )
				{
					Event *args=new Event;
					if ( event->mask & IN_ISDIR )
					{
						string path=monitorMap[event->wd]+"/"+event->name;
						args->path=path;
						args->fileType=0;
						args->type=CREATE;
						watchFolder ( path ) ;
					}
					else
					{
						string path=monitorMap[event->wd]+"/"+event->name;
						args->path=path;
						args->fileType=1;
						args->type=CREATE;

						time_t now  = ::time ( NULL ) ;
						FileInfo * fileInfo=new FileInfo();//将文件信息添加到监控列表中
						fileInfo->path=path;
						fileInfo->expire=now + m_expire ; //设置过期时间

						if ( ::strcmp ( event->name, "stop" ) == 0 )//如果新建文件名为stop则退出程序
						{
							bRun = false ;
							fileInfo->expire=now;//设置过期时间
						}

						fileList.push_back(fileInfo);
					}

					if ( listener != NULL )
					{
						listener(args);
					}
				}
				else if ( event->mask & IN_DELETE )
				{
					Event *args=new Event;
					if ( event->mask & IN_ISDIR )
					{
						string path=monitorMap[event->wd]+"/"+event->name;
						args->path=path;
						args->fileType=0;
						args->type=DELETE;
						::inotify_rm_watch ( m_fd, event->wd ) ;//移除监视
						monitorMap.erase(event->wd);//
					}
					else 
					{
						string path=monitorMap[event->wd]+"/"+event->name;
						args->path=path;
						args->fileType=1;
						args->type=DELETE;
					}

					if ( listener != NULL )
					{
						listener(args);
					}
				}
				else if ( event->mask & IN_MODIFY )
				{
					Event *args=new Event;
					if ( event->mask & IN_ISDIR )
					{
						string path=monitorMap[event->wd]+"/"+event->name;
						args->path=path;
						args->fileType=0;
						args->type=MODIFY;
					}
					else 
					{
						string path=monitorMap[event->wd]+"/"+event->name;
						args->path=path;
						args->fileType=1;
						args->type=MODIFY;
					}
					
					if(listener!=NULL)
					{
						listener(args);
					}
				}
			}

			i += EVENT_SIZE + event->len;
		}

		deleteFile();
	}

	return 0;
}

//监控子目录
void CFileMonitor::watchFolder(const std::string & path)
{
	int wd=::inotify_add_watch ( m_fd, path.c_str(), IN_MODIFY|IN_CREATE|IN_DELETE ) ;
	//添加到监视列表
	monitorMap [ wd ] = path ;
	
	struct dirent* ent = NULL ;
	DIR * pDir = ::opendir ( path .c_str () ) ;

	if ( pDir==NULL )
	{
		std::cout << "invalid path" << std::endl ;
		return;
	}

	while ( NULL != (ent = ::readdir ( pDir ) ) )
	{
		if ( ent->d_type == 4 )
		{
			if ( ent->d_name[0] != '.' )
			{
				std::string subPath = path + "/" + ent ->d_name ;
				watchFolder ( subPath ) ;
			}
		}
		else
		{
			if ( listener != NULL )
			{
				int len = ::strlen ( ent->d_name ) ;
				if ( ent->d_name[0]!='.'&&ent->d_name[len-1] != '~' )
				{
					string filename=path+"/"+ent->d_name;
					Event *args=new Event;
					args->path=filename;
					args->fileType=1;
					args->type=CREATE;
					
					time_t now = ::time ( NULL ) ;
					FileInfo * fileInfo=new FileInfo();//将文件信息添加到监控列表中
					fileInfo->path=filename;
					fileInfo->expire=now+ m_expire ; //设置过期时间
					fileList.push_back(fileInfo);
					listener(args);
				}
			}
		}
	}
}

void CFileMonitor::deleteFile()
{
	time_t now = ::time ( NULL ) ;

	auto iter = fileList .begin () ;
	for (  ; iter != fileList .end () ; )
	{
		if ( (*iter) ->expire <= now )
		{
			if ( !::remove ( (*iter) ->path .c_str () ) )
			{
				std::cout << (*iter)->path << " expired has been deleted" << std::endl ;
				iter = fileList .erase ( iter ) ;
			}
			else
			{
				std::cout << "delete " << (*iter)->path << " failure" << std::endl ;
				iter++ ;
			}
		}
		else
		{
			iter++ ;
		}
	}
}


