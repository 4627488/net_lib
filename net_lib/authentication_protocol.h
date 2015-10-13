/********************************************************************
	����ʱ��:	2012-4-27  11:56
	������:		Ѧ�� email:308821698@qq.com
	�ļ�����:	
*********************************************************************/
#pragma once

//xuegang ��֤���ݰ��ĸ�ʽ
struct authentication_package
{
	uint32_t protocol_id;
	char authentication_data[1];
};

//xuegang ��¼ƾ֤ �����û��������ƾ֤
class password_credential
{
public:
	string m_account_id;
	string m_password;
};

//xuegang Ȩ����Ϣ �û�����ݺ������û���
class authorization
{
public:
	string m_account_id;
	vector<string> m_groups;
};

enum e_authentication_error
{
	e_authentication_success,
	e_authentication_fail,
	e_authentication_invalid_userid_or_pwd,
	e_authentication_protocol_error,
	e_authentication_connect_fail,
};

//xuegang ��֤Э�����
class authentication_protocol
{
public:
	authentication_protocol();
	virtual void initial_credential(const password_credential& credential,auto_buffer& out_package)=0;//xuegang �ڿͻ��˵���
	virtual void process_package(auto_buffer& recved_package,auto_buffer& out_package)=0;//xuegang ����true��ʾҪ��������
	bool need_response(){return m_need_response;}//xuegang ��֤�Ƿ����
	authorization get_local_authorization(){return m_local_authorization;}//xuegang ����Ȩ��
	authorization get_remote_authorization(){return m_remote_authorization;}//xuegang �Զ�Ȩ��
	string get_encrypt_key(){return m_encrypt_key;}
	e_authentication_error get_error(){return m_auth_state;}//���ʧ�ܣ�����ʧ�ܵ�ԭ��
	uint32_t get_protocol_id(){return m_protocol_id;}
	bool is_over(){return m_is_over;}
protected:
	uint32_t m_protocol_id;
	string m_encrypt_key;
	authorization m_local_authorization;
	authorization m_remote_authorization;
	bool m_need_response;
	bool m_is_over;
	e_authentication_error m_auth_state;
};

//xuegang ע��ʹ���Э����֤�������
class authentication_protocol_factory
{
public:
	authentication_protocol_factory(){}
	template<typename CreatorT>
	bool register_protocol(uint32_t protocol_id,CreatorT& creator)
	{
		m_protocols[protocol_id]=shared_ptr<hodler_base>(new hodler<CreatorT>(creator));
		return true;
	}
	auto_ptr<authentication_protocol> create(uint32_t protocol_id);
private:
	authentication_protocol_factory(authentication_protocol_factory&);
	authentication_protocol_factory& operator=(authentication_protocol_factory&);
	struct hodler_base
	{
		virtual authentication_protocol* create()=0;
	};
	template<typename T>
	struct hodler:public hodler_base
	{
		hodler(T& creator):m_creator(creator){}
		virtual authentication_protocol* create()
		{
			return m_creator();
		}
		T m_creator;
	};
	hash_map<uint32_t,shared_ptr<hodler_base> > m_protocols;
};
authentication_protocol_factory& get_authentication_protocol_factory();

class session;
class authentication_sevice
{
public:
	authentication_sevice();
	void initial();//xuegang �ڷ���������ʱ����Ҫ����
	bool process(shared_ptr<session> sp_session,auto_buffer package_in,auto_buffer& package_out);//����true��ʾҪ��������
	void session_colse_callback(shared_ptr<session> sp_session);
private:
	authentication_sevice(authentication_sevice&);
	struct authentication_context
	{
		shared_ptr<session> sp_session;
		shared_ptr<authentication_protocol> sp_protocol;
	};
	CRITICAL_SECTION m_cs;
	map<session*,authentication_context > m_datas;
};
authentication_sevice& get_authentication_sevice();


//////////////////////////////////////////////////////////////////////////
// xuegang ȱʡ����֤Э��ֻ�ܷ������֤�ͻ��ˣ����ܿͻ�����֤�����
//1 �ͻ��˷���һ����֤����
//2 ����˷���һ�������
//3 �ͻ��˷������û��� ����password-md5����Ϊ������ܵ������
//4 ����˶�ȡ����� ��password-md5����Ϊ��������Ǹ������ �����Ϳͻ��˴������ļ������ݱȽ� �������ͬ ����ʾ��֤�ɹ� ��
//   ����һ����password-md5����Ϊ����j���ܵĻỰ��Կ����Ȩ����Ϣ ����֤����
//5 �ͻ����յ�������Կ��Ȩ����Ϣ ��֤����

const uint32_t default_authentication_protocol_id=1;
struct check_password_result
{
	authorization authz;
	string encrypt_key;
	string encrypted_encrtypt_key;
};
typedef check_password_result (*check_password_fun_t)(const string& account_id,uint32_t rand_num,uint32_t encrpyted_rand_num);


class default_authentication_protocol
	:public authentication_protocol
{
public:
	template<typename CheckPasswordFun>
	default_authentication_protocol(CheckPasswordFun checker):m_next_package(1),m_rand_num(0)
	{
		m_ap_checker.reset(new hodler<CheckPasswordFun>(checker));
		m_protocol_id=default_authentication_protocol_id;
	}
	virtual void initial_credential(const password_credential& credential,auto_buffer& out_package);//xuegang �ڿͻ��˵���
	virtual void process_package(auto_buffer& recved_package,auto_buffer& out_package);//xuegang ����true��ʾҪ��������
private:
	void process_protocol_error_helper(auto_buffer& out_package);
	void package1_process(auto_buffer& authentication_package_in,auto_buffer& out_package);//xuegang ����true��ʾҪ��������
	void package2_process(auto_buffer& authentication_package_in,auto_buffer& out_package);
	void package3_process(auto_buffer& authentication_package_in,auto_buffer& out_package);
	void package4_process(auto_buffer& authentication_package_in,auto_buffer& out_package);
	void package5_process(auto_buffer& authentication_package_in,auto_buffer& out_package);
	struct hodler_base
	{
		virtual check_password_result check_password(const string& account_id,uint32_t rand_num,uint32_t encrpyted_rand_num)=0;
	};
	template<typename T>
	struct hodler:public hodler_base
	{
		hodler(T& check_fun):m_check_fun(check_fun){}
		virtual check_password_result check_password(const string& account_id,uint32_t rand_num,uint32_t encrpyted_rand_num)
		{
			return m_check_fun(account_id,rand_num,encrpyted_rand_num);
		}
		T m_check_fun;
	};
private:
	default_authentication_protocol(default_authentication_protocol&);
	default_authentication_protocol& operator=(default_authentication_protocol&);
	int m_next_package;
	auto_ptr<hodler_base> m_ap_checker;
	password_credential m_credential;
	uint32_t m_rand_num;
};

//xuegang ������֤��Ϣ���û�id��ʾ��֤�ɹ�
static check_password_result invalid_checker(const string& account_id,uint32_t rand_num,uint32_t encrpyted_rand_num)
{
	check_password_result result;
	return result;
}

template<typename Checker>
class dap_creator
{
public:
	dap_creator(Checker checker):m_checker(checker){}
	authentication_protocol* operator()(){return new default_authentication_protocol(m_checker);}
private:
	Checker m_checker;
};
template<typename Checker> 
dap_creator<Checker> make_dap_creator(Checker checker)
{
	return dap_creator<Checker>(checker);
}
