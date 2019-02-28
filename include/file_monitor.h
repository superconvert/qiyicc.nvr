#ifndef FILEMONITOR_H
#define FILEMONITOR_H

#include<map>
#include<vector>
#include<iostream>

using namespace std;

enum EventType
{
	CREATE,
	DELETE,
	MODIFY
};

typedef struct 
{
    string 		path;		//路径
    EventType 	type;		//事件类型
    int 		fileType;	//0表示文件夹,1表示文件
} Event ;

typedef struct 
{
    string 		path;		//文件路径
    int 		expire;		//过期时间
} FileInfo;

typedef void (*ListenerProc)(const Event *)	;

/////////////////////////////////////////////////////////////////
//
// class CFileMonitor
//
/////////////////////////////////////////////////////////////////
class CFileMonitor
{
public:
	CFileMonitor(std::string path, ListenerProc proc, int expire=180);
	virtual ~CFileMonitor();

	int start();

protected:
	void watchFolder(const std::string & path);
	void deleteFile();

private:
	int m_fd ;						//句柄
	long m_expire ;					//文件过期时间
	std::string m_path ;				//监控根目录	
	std::map<int,string> monitorMap;		//监控列表	
	std::vector<FileInfo *> fileList;
	ListenerProc listener ;
};

#endif // FILEMONITOR_H
