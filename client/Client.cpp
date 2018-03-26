
#include "Client.h"

Client *Client::ms_pClient = NULL;

void *ClientWriteWorker( void *arg )
{
	Client *pClient = (Client*)arg;
	pClient->clientWorker();
	return ((void *)0);
}

Client::Client()
{

}

Client::~Client()
{

}

void Client::createInstance()
{
	ms_pClient = new Client();
}

void Client::destroyInstance()
{
	ms_pClient->uninit();
	delete ms_pClient;
	ms_pClient = NULL;
}

Client *Client::getInstance()
{
	return ms_pClient;
}

void Client::oninit()
{
	m_threadNum = 1;

	for( int i = 0; i < m_threadNum; ++i )
	{
		KernalThread *pThread = new KernalThread();
		pThread->init(ClientWriteWorker, this);
		pThread->detach();
		m_ClientWorkThreads.push( pThread );
	}

	unsigned int tid = m_Timer.addTimer(100, 1);
	
}

void Client::onuninit()
{
	
}

void Client::handleTimerMsg( unsigned int id )
{
	
}

void Client::onMsg( unsigned int id, KernalMessageType type, const char *data, unsigned int size )
{
	if( TIMER_DATA == type )
	{
		handleTimerMsg( id );
	}
	else
	{
		
	}
}

void Client::clientWorker()
{
	const char *serverIP   = getConfig()->getAttributeStr("config/gateway/listen", "ip");
	int         serverPort = getConfig()->getAttributeInt("config/gateway/listen", "port");

	pthread_cond_t  cond;
	pthread_mutex_t mutex;
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cond, NULL);
	pthread_mutex_lock(&mutex);

	struct timespec delay;
	struct timeval now;

	int serverID = 0;//connect(serverIP, serverPort);
	do{
		gettimeofday(&now, NULL);
		delay.tv_sec = now.tv_sec + 2;
		delay.tv_nsec = now.tv_usec * 1000;
		pthread_cond_timedwait(&cond, &mutex, &delay);

		serverID = connect(serverIP, serverPort);
		
		struct timeval tm = {2, 0};
		fd_set wset, rset;
		FD_ZERO( &wset );
		FD_ZERO( &rset );
		FD_SET( serverID, &wset );
		FD_SET( serverID, &rset );
		int res = select( serverID + 1, &rset, &wset, NULL, &tm );
		if( res > 0 && FD_ISSET(serverID, &wset)  )
		{
				int error, code;
				socklen_t len;
				len = sizeof(error);
				code = getsockopt(serverID, SOL_SOCKET, SO_ERROR, &error, &len);
				if (code < 0 || error)
				{
					::close( serverID );
				}
				else
				{
					break;
				}
		}
	}
	while( true );
	
	int size = 0;
	char _buf[BUFF_SIZE] = {0};
	char* dataBuf = _buf;
	WriteInt32(dataBuf, &serverID);
	WriteInt32(dataBuf, &socket_connect);
	WriteInt32(dataBuf, &size);
	dataBuf = _buf;
	m_Epoll.sendToPipe( dataBuf, size + 12 );

	gettimeofday(&now, NULL);
	delay.tv_sec = now.tv_sec + 2;
	delay.tv_nsec = now.tv_usec * 1000;
	pthread_cond_timedwait(&cond, &mutex, &delay);
	
	for( int i = 0; i < 50000; ++i )
	{
		char msg[128];
		memset(msg,0,sizeof(msg));
		sprintf(msg, "client test threadid=%ld,i=%d,serverID=%d===",pthread_self(),i,serverID);
		//printf("client send msg=%s\n", msg);
		int len = strlen(msg);
		//m_Epoll.send( serverID, &len, 4 );
				
		m_Epoll.send( serverID, msg, len );
#if 0
		char _buf[BUFF_SIZE] = {0}; 
		char* data = _buf;
		int len = 0; 
		WriteInt32(data,len);
		int err = 0;
		WriteInt32(data,&err);

		len = data - _buf;
		data = _buf; 
		m_Epoll.send( serverID, msg, len );
#endif
		//usleep(20);
	}
	//m_Epoll.closeSocket(serverID);
}

void Client::onProcess()
{
	
}

void Client::onRun()
{
	
}

void Client::onExit()
{	
	
}
