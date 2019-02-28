#ifndef _SERVERMANAGER_H
#define _SERVERMANAGER_H

// 屏蔽掉编译错误
#ifdef WIN32
#pragma warning (disable:4786)
#endif

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <json/json.h>
#include "jmutex.h"
#include "thrift_server.h"
#include "public_common.h"

// ********************************************************
//
// class CServerManager 类定义实现
//
// ********************************************************
class CServerManager
{
public:
	CServerManager();
	virtual ~CServerManager();

	int inital();
	int unInit();
	bool isRun();

	// 打印最基本信息
	void dump();
	// 执行等待动作
	void wait();	
	// 测试数据
	void demo();
	// 装载配置
	void loadConfig();
	// 写日志
	void writeLog(const std::string & message);
	
	// 加载策略
	int loadPolicy();
	// 加载柜体
	int loadCabinet();
	// 加载摄像头
	int loadCamera();
	// 下载策略
	int downloadPolicy();
	// 下载柜体列表
	int downloadCabinets();
	// 下载摄像头列表
	int downloadCameras();
	// 加载缓存
	int loadCache(const std::string & fileName, Json::Value & json);
	// 数据保存
	int saveCache(const std::string & fileName, Json::Value & json);
	// 下载公共
	int downloadCommon(const std::string & url, Json::Value & json);
	// 解析policy
	int processPolicy(Json::Value & json);
	// 解析柜体
	int processCabinet(Json::Value & json);
	// 解析摄像头
	int processCamera(Json::Value & json);

	// 更新配置信息
	void reloadConfig();

	// 录制任务退出
	int taskExit(pid_t pid, int status);

	// 获取缓存路径
	std::string getCachePath();
	// 获取上传路径
	std::string getUploadPath();
	// 获取录制路径
	std::string getRecorderPath();
	// 获取子进程日志
	std::string getChildLogPath(pid_t pid);
	// 获取摄像头地址
	std::string getCameraPath(int id);
	// 获取摄像头录像
	std::string getCameraFile(int id, const std::string & format);
	// 获取当前日期
	std::string getCurrentTime();
	// 获取当前日期周几
	TASKTIME getTaskTime();	

public:

	// 派发任务
	int dispatchTask();	
	// 创建一个录制任务
	int createTask(int id);
	// 执行一个任务
	int startTask(TASKOBJECT & taskObject);
	// 停止任务
	int stopTask(TASKOBJECT & taskObject);
	// 回收过期文件
	int recycleExpired();


protected:
	bool						m_bInit ;
	bool						m_bDemo ;
	// 配置文件的路径
	std::	string				m_workPath ;
	std::string				m_logPath ;
	std::string				m_childPath ;

	// 管理服务器地址
	std::string				m_token ;
	std::string				m_webServer ;
	int						m_expired ;
	bool						m_bExpired ;
	TIMESTATE				m_timeExpired ;
	CThriftServer				m_thriftServer ;

	JMutex					m_mutexPolicy ;
	std::string				m_policyVersion ;
	QuePolicyObject			m_quePolicy ;

	JMutex					m_mutexCabinets ;
	std::string				m_cabinetVersion ;
	HashCabinetObject			m_hashCabinets ;

	JMutex					m_mutexCameras ;
	std::string				m_cameraVersion ;
	HashCameraObject			m_hashCameras ;

	JMutex					m_mutexTask ;
	HashTaskObject			m_hashTask ;
};

extern CServerManager		ServerManager ;

#endif
