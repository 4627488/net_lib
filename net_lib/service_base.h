/********************************************************************
	����ʱ��:	2012-2-22  14:57
	������:		Ѧ�� email:308821698@qq.com
	�ļ�����:	��ʱ�� , �򵥵���Ϣ�����ķ���
*********************************************************************/
#pragma once
#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <string>
#include <memory>
#include <fstream>
#include <strstream>
#include <list>
#include <map>
#include <set>
#include <vector>
#include <process.h>
#include <algorithm>
using namespace std;
//using namespace std::tr1;
#include "auto_lock_cs.h"
//[2012/2/22 author:xuegang email:308821698@qq.com] 
//��һԪȫ�ֺ�����װΪ�� ����ptr_fun
template<class Arg,class Result>
class unary_function_x: public unary_function<Arg, Result>
{
public:
	explicit unary_function_x(Result (* fun)(Arg)):m_fun(fun){}
	Result operator()(Arg _Left) const{ return (m_fun(_Left));}
private:
	Result (* m_fun)(Arg);
};

template<class Arg,class Result>
unary_function_x<Arg,Result> make_unary_function(Result (* fun)(Arg))
{
	return unary_function_x<Arg,Result>(fun);
}

//xuegang ����һ�������ĳ�Ա������װ�ɺ�������
template<typename T,typename Arg,typename Result>
class mem_fun_1arg
{
public:
	mem_fun_1arg(T* p_object,Result (T::*p_memfun)(Arg)):m_p_object(p_object),m_p_memfun(p_memfun){}
	inline Result operator()(Arg arg)
	{
		return (m_p_object->*m_p_memfun)(arg);
	}
	T* m_p_object;
	Result (T::*m_p_memfun)(Arg);
};

template<typename Result,typename Arg,typename T>
mem_fun_1arg<T,Arg,Result> make_mem_fun_1arg(T* p_object,Result (T::*p_memfun)(Arg))
{
	return mem_fun_1arg<T,Arg,Result>(p_object,p_memfun);
}

//[2012/2/22 author:xuegang email:308821698@qq.com] 
//��ʱ���¼�
class time_event
{
public:
	//begin_time_sec ���룩����������¼��Ŀ�ʼʱ�� , end_time_sec���룩�¼�����ʱ�䣬per_sec���룩 �೤ʱ�䴥��һ��
	//��end_time_sec=-1ʱ ��ʾ��Զ�ȴ� ��end_time_sec=0ʱ �ȴ�һ��
	time_event(int  begin_time_sec,int end_time_sec=0,int per_sec=0);
	//begin_time_t ��time_t��ʾ���¼��Ŀ�ʼʱ��
	time_event(time_t begin_time_t,int end_time_sec=0,int per_sec=0);
	~time_event(){}
	time_t plus_begin_time(){return m_begin_time+=m_per_sec;}
	bool is_over();
	time_t get_begin_time(){return m_begin_time;}
private:
	time_t m_begin_time;
	time_t m_end_time;
	int m_per_sec;
};

enum time_event_result
{
	time_e_loop,//����ѭ�������¼�
	time_e_delete,//���غ��ɾ���¼�
};

//[2012/2/22 author:xuegang email:308821698@qq.com] 
//��ʱ���ص������ķ�װ�����㴫�ݺ����ͷº���
class time_event_fun_wrap_base
{
public:
	time_event_fun_wrap_base(const time_event& timer):m_timer(timer){};
	virtual time_event_result fire()=0;
	time_event m_timer;
};

template<typename Functor>
class time_event_fun_wrap:public time_event_fun_wrap_base
{
public:
	time_event_fun_wrap(const time_event& timer,const Functor& functor)
		:time_event_fun_wrap_base(timer),m_functor(functor){};
	time_event_result fire(){return m_functor(&m_timer);}
private:
	Functor m_functor;
};

//[2012/2/22 author:xuegang email:308821698@qq.com] 
//��ʱ���¼��Ĺ�����
class time_event_mgr
{
public:
	time_event_mgr();
	template<typename Functor>
	bool add_event(const time_event& timer,Functor& functor)
	{
		shared_ptr<time_event_fun_wrap_base> sp_event_fun(new time_event_fun_wrap<Functor>(timer,functor));
		auto_lock_cs lock(m_cs);
		m_events.insert(make_pair(sp_event_fun->m_timer.get_begin_time(),sp_event_fun));
		return true;
	}
	bool add_event(const time_event& timer,time_event_result(* functor)(time_event*));
	void fire_event();
private:
	CRITICAL_SECTION m_cs;
	multimap<time_t,shared_ptr<time_event_fun_wrap_base> > m_events;
};

class msg_base
{
public:
	msg_base(int msg_id):m_msg_id(msg_id){}
	int m_msg_id;
	//shared_ptr<session> m_sp_session;
};

//[2012/2/22 author:xuegang email:308821698@qq.com] 
//������Ϣ�����ķ���
class msg_driven_service
{
public:
	~msg_driven_service();
	bool start_sevice();
	bool stop_sevice();
	template <typename T> 
	bool add_msg(auto_ptr<T> ap_msg)
	{
		auto_lock_cs lock(m_cs);
		m_msg_list.push_back(shared_ptr<msg_base>(ap_msg.release()));
		ReleaseSemaphore(m_semaphore,1,NULL);
		return true;
	}
	template<typename Functor>//time_event_result time_event_fun_t(time_event*);
	bool add_time_event(const time_event& timer,Functor& functor)
	{
		return m_ap_event_mgr->add_event(timer,functor);
	}

protected:
	msg_driven_service();
	virtual bool initial(){return true;};
	virtual void msg_dispatch(msg_base* p_msg){};
private:
	void fire_timer_events();
	msg_driven_service(const msg_driven_service&);
	msg_driven_service& operator=(const msg_driven_service&);
	static DWORD WINAPI thread_proc(void* vThis);
private:
	CRITICAL_SECTION m_cs;
	HANDLE m_hthread;
	HANDLE m_semaphore;
	list<shared_ptr<msg_base> > m_msg_list;
	auto_ptr<time_event_mgr> m_ap_event_mgr;
	HANDLE m_timer;
	bool m_b_notify_exit;
};

class timer_sevice:public msg_driven_service
{
};

/****************************************************
//test 
class test_service:public msg_driven_service
{
};

time_event_result my_time_event_fun(time_event*)
{
	SYSTEMTIME time;
	GetLocalTime(&time);
	printf("my_time_event %d:%d:%d \n",time.wHour,time.wMinute,time.wSecond); 
	return time_e_loop;
}

int main(int argc, _TCHAR* argv[])
{
	test_service srv;
	srv.start_sevice();
	time_event timer(0,-1,5);
	//ÿ��5���ӡһ�µ�ǰʱ��
	srv.add_time_event(timer,my_time_event_fun);
	Sleep(100000);
	srv.stop_sevice();
	return 0;
}
************************************************************///