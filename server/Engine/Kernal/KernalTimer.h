
#ifndef _KERNAL_TIMER_H_
#define _KERNAL_TIMER_H_

#include <time.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <vector>
#include "KernalLock.h"
#include "KernalMap.h"
#include "KernalThread.h"

#define TIMER_SLOT 64 //时间轮槽位

struct KernalTimerNode
{
    unsigned long int     id;
    unsigned int          expire;
    unsigned int          expireTime;
    int                   time;
    KernalTimerNode      *pre;
    KernalTimerNode      *next;

    KernalTimerNode()
        :id( 0 )
        ,expire( 0 )
        ,expireTime( 0 )
        ,time( -1 )
        ,pre( NULL )
        ,next( NULL )
    {

    }
};

struct KernalTimerNodeList
{
	KernalMutexLocker      locker;
    struct KernalTimerNode *head;
	struct KernalTimerNode *tail;

	void lock()
	{
		locker.lock();
	}
	void unlock()
	{
		locker.unlock();
	}
};

class KernalTimer
{
public:
    KernalTimer();
    ~KernalTimer();

	void init();
    unsigned long int addTimer( unsigned int expire, int time/*执行次数：-1 无限*/ );
    void deleteTimer( unsigned long int id );

    unsigned int gettime();

    void update();
    void expired( int slot );
public:
    unsigned int pop();
	
	void initThreadTimer();
    unsigned int popExpired();
	int getMinTimerExpire();  // 获取最近过期时间的定时器的expire
private:   
    void push( unsigned long int id );
    void deleteTimer( struct KernalTimerNode *pTimerNode );	
private:    
    unsigned int        m_StartTime;
	KernalMutexLocker   m_TimerLocker;
#if 0	
    int                 m_CurrentSlot;  // 当前槽位
	KernalMutexLocker   m_ExpriedTimerLocker;

    KernalTimerNodeList m_TimerNodes[ TIMER_SLOT ];
    std::vector< unsigned int > m_expriedTimers;
#endif
	
	std::map< pthread_t, KernalTimerNodeList > m_Timers;
};

#endif
