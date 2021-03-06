
#include "Client.h"

Client *Client::ms_pClient = NULL;

#if 0
void *ClientWriteWorker( void *arg )
{
	Client *pClient = (Client*)arg;
	pClient->clientWorker();
	return ((void *)0);
}
#endif

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
	m_Log.init( (char*)"client_test.log" );

	m_threadNum = 8;

#if 0
	for( int i = 0; i < m_threadNum; ++i )
	{
		KernalThread *pThread = new KernalThread();
		pThread->init(ClientWriteWorker, this);
		pThread->detach();
		m_ClientWorkThreads.push( pThread );
	}
#endif

	unsigned int tid = m_Timer.addTimer(100, 1);
	
}

void Client::handleTimerMsg( unsigned int id )
{
	
}

void Client::onMsg( unsigned int id, KernalNetWorkType netType, KernalMessageType type, const char *data, unsigned int size )
{
	printf("Client::onMsg 000 %d %d %d %d\n\r",type,TIMER_DATA,NETWORK_DATA,NETWORK_CONNECT);
	if( TIMER_DATA == type )
	{
		handleTimerMsg( id );
	}
	else if( NETWORK_DATA == type )
	{
		
		int cmd = 0;
		NReadInt32(data,&cmd);
		//printf("client onMsg cmd=%d\r\n",cmd);
		int err = 0;
		NReadInt32(data,&err);
		//printf("client onMsg err=%d\r\n",err);
		int len = 0;
		NReadInt32(data,&len);
		//printf("client onMsg len=%d\r\n",len);
		
		platformprotocol::SVerifyToken verifyToken;
		ParseFromArrayToObj( verifyToken, data, len );
		if( verifyToken.has_playerinfo() )
		{
			printf("client onMsg cmd=%d err=%d len=%d uid=%d\r\n",cmd,err,len,verifyToken.playerinfo().uid());
		}
	}
	else if( NETWORK_CONNECT == type )
	{		
		for( int i = 0; i < 10; ++i )
		{
			char token[128] = {0};
			memset(token, 0, sizeof(token));
			sprintf(token,"Token Test %d", i);
			platformprotocol::CVerifyToken verifyToken;
			verifyToken.set_token(token); 
			//printf("CVerifyToken len %d\r\n",verifyToken.ByteSize());
			ProtobufMsgSend(m_Epoll, id, platformprotocol::PLATFORM_VERIFY_TOKEN, 0, verifyToken);
		}
	}
}

void Client::clientConnectGateWay()
{	
	printf("Client::clientConnectGateWay 000 \n\r");
	const char *serverIP   = getConfig()->getAttributeStr("config/gateway/listen", "ip");
	int         serverPort = getConfig()->getAttributeInt("config/gateway/listen", "port");

	int serverID = 0;
	int sfd = 0;
	serverID = connect(serverIP, serverPort, sfd);
	printf("Client::clientConnectGateWay 111 \n\r");
	if( -1 == serverID )
	{
		printf("client clientConnectGateWay err:%d \r\n", errno);
	}
	printf("Client::clientConnectGateWay 222 sid:%d\n\r", serverID);
}

#if 0
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
	int sfd = 0;

	do{
		gettimeofday(&now, NULL);
		delay.tv_sec = now.tv_sec + 2;
		delay.tv_nsec = now.tv_usec * 1000;
		pthread_cond_timedwait(&cond, &mutex, &delay);

		serverID = connect(serverIP, serverPort, sfd);
		
		if( serverID <= 0 )
		{
			continue;
		}
		
		//struct KernalNetWork *pNet = m_Epoll.getNetWork(serverID);
		
		struct timeval tm = {2, 0};
		fd_set wset, rset;
		FD_ZERO( &wset );
		FD_ZERO( &rset );
		FD_SET( sfd , &wset );
		FD_SET( sfd, &rset );
		int res = select( sfd + 1, &rset, &wset, NULL, &tm );
		if( res > 0 && FD_ISSET(sfd, &wset)  )
		{
				gettimeofday(&now, NULL);
				delay.tv_sec = now.tv_sec + 5;
				delay.tv_nsec = now.tv_usec * 1000;
				pthread_cond_timedwait(&cond, &mutex, &delay);

				int error, code;
				socklen_t len;
				len = sizeof(error);
				code = getsockopt(sfd, SOL_SOCKET, SO_ERROR, &error, &len);
				if (code < 0 || error)
				{
					::close( sfd );
				}
				else
				{
					break;
				}
		}
	}
	while( true );
	
	/*int size = 0;
	char _buf[BUFF_SIZE] = {0};
	char* dataBuf = _buf;
	NWriteInt32(dataBuf, &serverID);
	NWriteInt32(dataBuf, &socket_connect);
	NWriteInt32(dataBuf, &size);
	dataBuf = _buf;
	m_Epoll.sendToPipe( dataBuf, size + 12 );*/

	gettimeofday(&now, NULL);
	delay.tv_sec = now.tv_sec + 5;
	delay.tv_nsec = now.tv_usec * 1000;
	pthread_cond_timedwait(&cond, &mutex, &delay);

	for( int i = 0; i < 10; ++i )
	{
#if 0		
		char msg[128];
		memset(msg,0,sizeof(msg));
		sprintf(msg, "client test threadid=%ld,i=%d,serverID=%d===",pthread_self(),i,serverID);
		//printf("client send msg=%s\n", msg);
		int len = strlen(msg);
		//m_Epoll.send( serverID, &len, 4 );
				
		m_Epoll.send( serverID, msg, len );
#endif
#if 1
        char token[128] = {0};
        memset(token, 0, sizeof(token));
        sprintf(token,"Token Test %d", i);
		platformprotocol::CVerifyToken verifyToken;
		verifyToken.set_token(token); 
		//printf("CVerifyToken len %d\r\n",verifyToken.ByteSize());
		ProtobufMsgSend(m_Epoll, serverID, platformprotocol::PLATFORM_VERIFY_TOKEN, 0, verifyToken);
		
#endif		
#if 0
		char _buf[BUFF_SIZE] = {0}; 
		char* data = _buf;
		int len = 0; 
		NWriteInt32(data,len);
		int err = 0;
		NWriteInt32(data,&err);

		len = data - _buf;
		data = _buf; 
		m_Epoll.send( serverID, msg, len );
#endif
		//usleep(20);
	}
	//m_Epoll.closeSocket(serverID);
}
#endif

void Client::onProcess()
{
	
}

void Client::onRun()
{
	printf("Client::onRun \n\r");
	for( int i = 0; i < 1; ++i )
	{
		clientConnectGateWay();
	}	
}

void Client::onExit()
{	
	
}
