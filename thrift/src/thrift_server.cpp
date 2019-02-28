#include <thrift/concurrency/ThreadManager.h>
#include <thrift/concurrency/PosixThreadFactory.h>
#include "thrift_server.h"
#include "thrift_handler.h"
#include "public_common.h"

using namespace ::apache::thrift::concurrency;

CThriftServer::CThriftServer() : mPort(9090), mCount(32)
{}

CThriftServer::~CThriftServer()
{}

/*
http://thrift.apache.org/lib/cpp

shared_ptr<TSSLSocketFactory> getSSLSocketFactory() {
  shared_ptr<TSSLSocketFactory> factory(new TSSLSocketFactory());
  // client: load trusted certificates
  factory->loadTrustedCertificates("my-trusted-ca-certificates.pem");
  // client: optionally set your own access manager, otherwise,
  //         the default client access manager will be loaded.

  factory->loadCertificate("my-certificate-signed-by-ca.pem");
  factory->loadPrivateKey("my-private-key.pem");
  // server: optionally setup access manager
  // shared_ptr<AccessManager> accessManager(new MyAccessManager);
  // factory->access(accessManager);
  ...
}

client code sample

shared_ptr<TSSLSocketFactory> factory = getSSLSocketFactory();
shared_ptr<TSocket> socket = factory.createSocket(host, port);
shared_ptr<TBufferedTransport> transport(new TBufferedTransport(socket));
...
server code sample

shared_ptr<TSSLSocketFactory> factory = getSSLSocketFactory();
shared_ptr<TSSLServerSocket> socket(new TSSLServerSocket(port, factory));
shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory));
...


TSimpleServer 		– 简单的单线程服务模型，常用于测试
TThreadPoolServer 	– 多线程服务模型，使用标准的阻塞式IO。
TNonblockingServer 	– 多线程服务模型，使用非阻塞式IO（需使用TFramedTransport数据传输方式）

*/

void CThriftServer::run()
{
	printLog ( "thrift server running begin." ) ;
	mServer ->serve () ;
	printLog ( "thrift server running end." ) ;
}

int CThriftServer::start()
{
	nvr_shared<nvrWebServiceHandler> handler(new nvrWebServiceHandler());
	nvr_shared<TProcessor> processor(new nvrWebServiceProcessor(handler));
	nvr_shared<TServerTransport> serverTransport(new TServerSocket(mPort));
	nvr_shared<TTransportFactory> transportFactory(new TBufferedTransportFactory());
	nvr_shared<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
       nvr_shared<ThreadManager> threadManager(ThreadManager::newSimpleThreadManager(mCount));
       nvr_shared<PosixThreadFactory> threadFactory (new PosixThreadFactory());

	threadManager->threadFactory(threadFactory);
	threadManager->start();	
	mServer .reset ( new TThreadPoolServer(processor, serverTransport, transportFactory, protocolFactory, threadManager) ) ;
	mThread = std::thread ( &CThriftServer::run, this ) ;
	
	printLog ( "thrift server start." ) ;

	return 0 ;
}

int CThriftServer::stop()
{
	mServer ->stop () ;
	mThread .join () ;
	printLog ( "thrift server stop." ) ;
	
	return 0 ;
}

void CThriftServer::setPort(int port)
{
	mPort = port ;
}

void CThriftServer::setThread(int count)
{
	mCount = count ;
}

