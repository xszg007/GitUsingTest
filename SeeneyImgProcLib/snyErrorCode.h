#pragma once
typedef enum ErrorCode
{
	CALLFUNCTIONSUCCESS = 0, //函数调用成功
	PARMISEMPTY = -1,		 //传入参数为空
	CALLFUNCTIONFAIL = -2,	     //函数调用失败
	OTHERERROR = -3			//其他未知错误
	/*······*/
};
