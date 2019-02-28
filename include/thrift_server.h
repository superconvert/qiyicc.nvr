#ifndef _THRIFT_SERVER_H
#define _THRIFT_SERVER_H

// 屏蔽掉编译错误
#ifdef WIN32
#pragma warning (disable:4786)
#endif

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "nvrWebService.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thread>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using namespace  ::com::nvr::thrift;

namespace thrift = ::apache::thrift::stdcxx ;

#define nvrShared thrift::shared_ptr

// ********************************************************
//
// class CServerManager 类定义实现
//
// ********************************************************
class CThriftServer
{
public:
	CThriftServer();
	virtual ~CThriftServer();

	// 开启thrift  服务
	int start();
	// 停止thrift 服务
	int stop();

	// 设置端口
	void setPort(int port);
	// 设置线程数
	void setThread(int count);
	// 设置发送超时
	void setSendTimeout(int timeout);
	// 设置接收超时
	void setRecvTimeout(int timeout);
	

protected:
	// 线程运行
	void run();

protected:
	int								mPort ;
	int								mCount ;
	int								mSendTimeout ;
	int								mRecvTimeout ;		
	std::thread						mThread ;	
	nvrShared<TThreadPoolServer> 		mServer ;
};

#endif //

