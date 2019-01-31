#pragma once
#include "dataFormat.h"
#include <iostream>
#include<opencv2/core/core.hpp>//OpenCV核心库：定义了图像数据结构
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/opencv.hpp>
using namespace cv;
//#define DllExport   __declspec( dllexport ) 

class  CTheCaliperTool
{
	
public:
	CTheCaliperTool();
	~CTheCaliperTool();

    int getDlgParameters(const dlgParameters src_Info);//函数声明，在.cpp中定义
	
	
	dlgParameters m_enDlgParameters;//实例化结构体，在DataFormat.h中定义

	clpParameters m_enClpParameters;

	

	int getClpParameters(const clpParameters src_Info);

};
