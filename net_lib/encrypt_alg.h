/********************************************************************
	����ʱ��:	2012-5-2  12:42
	������:		Ѧ�� email:308821698@qq.com
	�ļ�����:	
*********************************************************************/
#pragma once
#include <string>
using namespace std;

string calulate_md5(const string&);
 bool encrypt_data(uint8_t* p_data,uint32_t size,const string& password); 
 bool decrypt_data(uint8_t* p_data,uint32_t size,const string& password);
