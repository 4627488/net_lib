/********************************************************************
����ʱ��:	2012-4-23  18:42
������:		Ѧ�� email:308821698@qq.com
�ļ�����:	
*********************************************************************/
#pragma once
#include <string>
#include "base_define.h"
#include "socket_service.h"
#include "auto_buffer.h"
#include "authentication_protocol.h"

class per_session_data
{
public:
	typedef void (*destroy_fun_t)(void*);
	per_session_data(void* p_data=NULL,destroy_fun_t fun=NULL,const string& type_name="")
		:m_p_data(p_data),m_destroy(fun),m_type_name(type_name){}
	void* m_p_data;
	destroy_fun_t m_destroy;
	string m_type_name;
};

//xuegang �Ự ��Ҫ���ǵ�����
//����˵�ַ
//��֤��ƾ֤ �û������� ֤���˽Կ
//ͨ������
//���ݼ��ܺ�ǩ������Կ
//�û���ɫ ��Ȩ �û���
class session
{
public:
	session();
	~session();
	bool connect(ip_port addr,const string& account_id,const string& password,uint32_t protocol_id);
	bool reconnect();
	void disconnect(bool b_clear_credential=false);
	void attach_socket(shared_ptr<socket_rw> sp_socket);
	shared_ptr<socket_rw> get_callee_socket();//�������ԶԶ˵ĺ�������
	shared_ptr<sync_socket_rw> get_caller_socket();//����һ�������ĺ�������

	password_credential get_credential(){return m_redential;}
	string get_encrypt_key();//����ͨ����ǩ���ͼ������ݵ���Կ
	authorization get_local_authorization();//����Ȩ��
	authorization get_remote_authorization();//�Զ�Ȩ��
	void set_local_authorization_info(const authorization& info);//����Ȩ��
	void set_remote_authorization_info(const authorization& info);//�Զ�Ȩ��
	bool is_verifyed();
	e_authentication_error get_login_error(){return m_login_error;}
	template<typename T>
	uint32_t add_per_session_data(T* p_object)
	{
		struct delete_wrap
		{
			static void destroy(void* p_object)
			{
				delete (T*)p_object;
			}
		};
		return add_per_session_data(p_object,delete_wrap::destroy,typeid(T).name());
	}
	template<typename T>
	T* get_per_session_data(uint32_t index)
	{
		auto_lock_cs lock(m_cs);
		map<uint32_t,per_session_data>::iterator it=m_per_session_datas.find(index);
		if(it!=m_per_session_datas.end())
		{
			if(it->second.m_type_name==typeid(T).name())
				return (T*)it->second.m_p_data;
		}
		return NULL;
	}
	bool delete_per_session_data(uint32_t index);
	void clear_all_per_session_data();
	void keep_session_longer();
	void updata_last_send_recv_time();
	bool is_half_session();//������
	void post_keep_alive_msg();
private:
	uint32_t add_per_session_data(void* p_data,per_session_data::destroy_fun_t fun,const string& type_name);
	session(session&);
	session& operator=(session&);
	CRITICAL_SECTION m_cs;

	ip_port m_remote_lisen_addr;//Զ�˵ļ�����ַ
	password_credential m_redential;	//�������ӵ��Զ˵�ƾ��
	uint32_t m_protocol_id;
	e_authentication_error m_login_error;

	shared_ptr<socket_rw> m_sp_callee_socket;
	shared_ptr<sync_socket_rw> m_sp_caller_socket;

	string m_encrpyt_key;
	authorization m_local_authorization;
	authorization m_remote_authorization;

	time_t m_last_recv_msg_time;//xuegang ������ʱ��
	map<uint32_t,per_session_data> m_per_session_datas;
};

typedef void (*login_notify_callback_t)(shared_ptr<session> sp_session,bool b_login);
class session_manager
{
public:
	session_manager();
	bool inital();
	shared_ptr<session> get_session(shared_ptr<socket_rw>);
	void delete_session_by_socket(shared_ptr<socket_rw>);
	void close_all_session();
	void clean_half_session();
	void set_login_callback_fun(login_notify_callback_t login_notify_fun){m_login_notify_fun=login_notify_fun;}
	login_notify_callback_t get_login_callback_fun(){return m_login_notify_fun;}
private:
	static time_event_result half_session_check(time_event*);
	session_manager(session_manager&);
	CRITICAL_SECTION m_cs;
	map<socket_rw*,shared_ptr<session> > m_sessions;
	login_notify_callback_t m_login_notify_fun;
	timer_sevice m_timer_srv;
};

session_manager& get_session_manager();

bool remote_user_is_in_group(shared_ptr<session> sp_session,const string& group);
