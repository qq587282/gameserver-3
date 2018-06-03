﻿
#include "KernalServerBase.h"
#include "KernalGuid.h"

void *KernalServerBaseWorker( void *arg )
{
	KernalWorkerThreadArg *pWorkerThreadArg = ( KernalWorkerThreadArg* )arg;
	pWorkerThreadArg->pServerBase->worker( pWorkerThreadArg->arg );
	return ((void *)0);
}

void *KernalServerBaseEpollWorker( void *arg )
{
	KernalServerBase *pServerBase = (KernalServerBase*)arg;
	pServerBase->epollWroker();
	return ((void *)0);
}

void *KernalServerBaseHeartBeatWorker( void *arg )
{
	KernalServerBase *pServerBase = (KernalServerBase*)arg;
	pServerBase->heartbeatWorker();
	return ((void *)0);
}

KernalServerBase::KernalServerBase()
	:m_quit( NULL ),
	 m_threadNum( 0 )
{

}

KernalServerBase::~KernalServerBase()
{

}

int KernalServerBase::listen( const char *ip, int port )
{
	return m_Epoll.listen( ip, port );
}

int KernalServerBase::connect( const char *ip, int port, int &sfd, bool addToEpoll )
{
	return m_Epoll.connect( ip, port, sfd );
}

int KernalServerBase::listenHttp( const char *ip, int port )
{
	return m_Epoll.listenHttp( ip, port );
}

int KernalServerBase::connectHttp( const char *ip, int port )
{
	return m_Epoll.connectHttp( ip, port );
}

void KernalServerBase::init( const char *configPath )
{
	srand( (unsigned)time(NULL) );
	
	m_Epoll.create();

	m_EpollThread.init(KernalServerBaseEpollWorker, this);
	m_EpollThread.detach();

	m_HeartBeatThread.init(KernalServerBaseHeartBeatWorker, this);
	m_HeartBeatThread.detach();

	m_Config.init( configPath );
	guidInit();
	onInit();

	if( m_threadNum <= 0 )
	{
		m_threadNum = 1;
	}
	
	// 创建工作线程管道
	for( int i = 1; i <= m_threadNum; ++i )
	{
		m_Epoll.createWorkerPipe( i );
	}
	
	// 创建工作线程
	for( int i = 1; i <= m_threadNum; ++i )
	{
		KernalWorkerThreadArg *pWorkerThreadArg = new KernalWorkerThreadArg();
		pWorkerThreadArg->pServerBase = this;
		pWorkerThreadArg->arg = i;
		m_WorkThreadArgs.push_back( pWorkerThreadArg );
		KernalThread *pThread = new KernalThread();
		pThread->init(KernalServerBaseWorker, pWorkerThreadArg);
		m_WorkThreads.push_back( pThread );
	}
}

void KernalServerBase::uninit()
{
	m_Epoll.release();
}

void KernalServerBase::run()
{
	onRun();
	for( auto iter = m_WorkThreads.begin(); iter != m_WorkThreads.end(); ++iter )
	{
		(*iter)->join();
	}
	onExit();
}

void KernalServerBase::epollWroker()
{
	KernalRequestMsg result;
	while( !m_quit )
	{
		KernalSocketMessageType type = m_Epoll.handleMessage( result );
		switch( type )
		{
			case KernalSocketMessageType_SOCKET_DATA: // 如果接收到数据则加入到队列中
			{
				pushMsg( NETWORK_DATA, result.netType, result.data, result.size, result.id );		
				break;
			}
			case KernalSocketMessageType_SOCKET_CONNECT:// 连接 socket
			{
				pushMsg( NETWORK_CONNECT, result.netType, NULL, 0, result.id );
				break;
			}
			case KernalSocketMessageType_SOCKET_CLOSE: // 关闭连接
			{
				pushMsg( NETWORK_CLOSE, result.netType, NULL, 0, result.id );
				break;
			}
			default:
			{

			}
		}
	}
}

void KernalServerBase::worker( int arg )
{
	m_Timer.initThreadTimer();
	pthread_setspecific(m_Epoll.getWorkerKey(), &arg);

	KernalPipe* pPipe = m_Epoll.getWorkerPipeByIndex( arg );

	while( !m_quit )
	{		
		int minExpire = m_Timer.getMinTimerExpire(); // 获取最近过期时间的定时器的expire
		fd_set rset;
		FD_ZERO( &rset );
		FD_SET( pPipe->pipe[1], &rset );
		int retval = 0;
		if( -1 == minExpire )
		{
			retval = ::select( pPipe->pipe[1] + 1, &rset, NULL, NULL, NULL );
		}
		else
		{
			struct timeval tm = {0, minExpire * 10000};
			retval = ::select( pPipe->pipe[1] + 1, &rset, NULL, NULL, &tm );
		}
		if( retval > 0 && FD_ISSET(pPipe->pipe[1], &rset)  )
		{
			// 处理网络消息
			KernalMessage pMsg;
			::read(pPipe->pipe[1], &pMsg.type, sizeof(pMsg.type));
			::read(pPipe->pipe[1], &pMsg.netType, sizeof(pMsg.netType));
			::read(pPipe->pipe[1], &pMsg.id, sizeof(pMsg.id));
			::read(pPipe->pipe[1], &pMsg.size, sizeof(pMsg.size));
			pMsg.data = ( char* )malloc( pMsg.size );
			memset( pMsg.data, 0, pMsg.size );
			::read(pPipe->pipe[1], pMsg.data, pMsg.size);

			// 处理消息
			switch( pMsg.type )
			{
				case NETWORK_DATA:
				case NETWORK_CLOSE:
				case NETWORK_CONNECT:
				{
					onMsg(  pMsg.id, pMsg.netType, pMsg.type, (const char *)pMsg.data, pMsg.size );
					break;
				}
				default:
				{
					break;
				}
			}
		}
		
		// 处理定时器消息
		while( true )
		{
			unsigned int timeID = m_Timer.popExpired();
			if( timeID == 0 )
			{
				break;
			}
			onMsg(  timeID, KernalNetWorkType_NO, TIMER_DATA, NULL, 0 );
		}		
	}
}

void KernalServerBase::heartbeatWorker()
{
	
}

void KernalServerBase::quit()
{
	flush();

	m_quit = true;
}

void KernalServerBase::flush()
{

}

void KernalServerBase::pushMsg( KernalMessageType type, KernalNetWorkType netType, void *data, unsigned int size, unsigned int id )
{
	KernalPipe* pPipe = m_Epoll.randWorkerPipe();
	if( pPipe )
	{
		::write( pPipe->pipe[0], &type, sizeof(type) );
		::write( pPipe->pipe[0], &netType, sizeof(netType) );
		::write( pPipe->pipe[0], &id, sizeof(id) );
		::write( pPipe->pipe[0], &size, sizeof(size) );
		::write( pPipe->pipe[0], (char*)data, size );
	}
}

void KernalServerBase::sendMsg( int fd, char *buff, int len )
{
	m_Epoll.send( fd, buff, len );
}

int KernalServerBase::getWorkerThreadKey()
{
	return m_Epoll.getWorkerThreadKey();
}
