/********************************************************************
����ʱ��:	2012-4-23  18:42
������:		Ѧ�� email:308821698@qq.com
�ļ�����:	
*********************************************************************/
#pragma once
#include "base_define.h"

//xuegang ����������
//��copy �������Ϊ����auto_ptr
//���ڴ���malloc��free�ͷ�
//һ�������ɻ�����������Ȩ���������ú��� �����ú����Ĳ�������Ϊauto_bufferʱһ���Ϻõ�ѡ��

class auto_buffer
{
public:
	auto_buffer(uint32_t buf_size=0);
	auto_buffer(auto_buffer& other);
	auto_buffer& operator=(auto_buffer&);
	~auto_buffer();
	bool write(void* p_data,uint32_t size);
	inline uint8_t* get(){return m_buf;}
	uint8_t* release();
	inline uint32_t size(){return m_data_size;}
	bool set_size(uint32_t data_size);
	void reserve(uint32_t size);
	//�ڴ��������malloc����� ����reset��ȡ��p_data��ָ�ڴ������Ȩ
	void reset(uint8_t* p_data=NULL,uint32_t data_size=0);
	bool good(){return m_read_point<=m_data_size;}
private:
	uint8_t* alloc_space(uint32_t size);

	uint32_t m_buf_size;
	uint32_t m_data_size;
	uint32_t m_read_point;
	uint8_t* m_buf;

	template<typename T> friend auto_buffer& operator<<(auto_buffer&,const T);
	template<typename T> friend auto_buffer& operator<<(auto_buffer&,const vector<T>&);
	friend auto_buffer& operator<<(auto_buffer&,const string&);
	template<typename T> friend auto_buffer& operator>>(auto_buffer&,T&);
	template<typename T> friend auto_buffer& operator>>(auto_buffer&,vector<T>&);
	friend auto_buffer& operator>>(auto_buffer&,string&);

};

//!!! ֧�� int8_t int16_t int32_t uint8_t uint16_t uint32_t 
//!!! ֧�� string   vector  

template<typename T>
struct is_auto_buffer_suport_type
{
	void check()=0;//��һ�䱨�����auto_buffer& operator<<(auto_buffer& ,const T)��֧������T��ʵ����
}; 
#define Auto_Buffer_T_Support_Type(T) \
template<> struct is_auto_buffer_suport_type<T>\
{\
	static void check(){}\
};

Auto_Buffer_T_Support_Type(bool);
Auto_Buffer_T_Support_Type(int8_t);
Auto_Buffer_T_Support_Type(int16_t);
Auto_Buffer_T_Support_Type(int32_t);
Auto_Buffer_T_Support_Type(int64_t);
Auto_Buffer_T_Support_Type(uint8_t);
Auto_Buffer_T_Support_Type(uint16_t);
Auto_Buffer_T_Support_Type(uint32_t);
Auto_Buffer_T_Support_Type(uint64_t);

template<typename T>
inline auto_buffer& operator<<(auto_buffer& out,const T data)
{
	is_auto_buffer_suport_type<T>::check();
	uint8_t* p_write_point=out.alloc_space(sizeof(T));
	*(T*)p_write_point=data;
	return out;
}

auto_buffer& operator<<(auto_buffer& out,const string& data);

template<typename T>
inline auto_buffer& operator<<(auto_buffer& out,const vector<T>& data)
{
	uint8_t* p_write_point=out.alloc_space(sizeof(uint32_t));
	*(uint32_t*)p_write_point=data.size();
	for(vector<T>::const_iterator it=data.begin();it!=data.end();++it)
	{
		out<<*it;
	}
	return out;
}



template<typename T>
auto_buffer& operator>>(auto_buffer& in,T& data)
{
	is_auto_buffer_suport_type<T>::check();
	uint8_t* p_read_point=in.m_buf+in.m_read_point;
	in.m_read_point+=sizeof(T);
	if(in.m_read_point>in.m_data_size)
	{
		return in;//error
	}
	data=*(T*)p_read_point;
	return in;
}

auto_buffer& operator>>(auto_buffer& in,string& data);

template<typename T>
auto_buffer& operator>>(auto_buffer& in,vector<T>& data)
{
	uint8_t* p_read_point=in.m_buf+in.m_read_point;
	in.m_read_point+=4;
	if(in.m_read_point>in.m_data_size)
	{
		return in;//error
	}
	uint32_t vec_size=*(uint32_t*)p_read_point;

	for(uint32_t count_i=0;count_i<vec_size;++count_i)
	{
		T temp;
		in>>temp;
		data.push_back(temp);
	}
	return in;
}



