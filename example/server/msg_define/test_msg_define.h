/********************************************************************
����ʱ��:	2012-4-23  18:42
������:		Ѧ�� email:308821698@qq.com
�ļ�����:	
*********************************************************************/
#pragma once
#include "../../../net_lib/net_lib.h"

//������Ϣ
const uint32_t msg_int_id=11;
const uint32_t msg_fix_string_id=12;
const uint32_t msg_var_string_id=13;

struct msg_int
{
	int32_t count;
};

struct msg_fix_string
{
	char data[1024];
};

struct msg_var_string
{
	string data;
};

//������Ҫ����msg_load msg_saveʱ��RegisterMsg1
RegisterMsg1(msg_int_id,msg_int);
RegisterMsg1(msg_fix_string_id,msg_fix_string);
//����Ҫ����msg_load msg_saveʱ��RegisterMsg2
RegisterMsg2(msg_var_string_id,msg_var_string);
