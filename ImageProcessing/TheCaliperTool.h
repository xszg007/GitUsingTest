#pragma once
#include "dataFormat.h"
#include <iostream>
#include<opencv2/core/core.hpp>//OpenCV���Ŀ⣺������ͼ�����ݽṹ
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/opencv.hpp>
using namespace cv;
//#define DllExport   __declspec( dllexport ) 

class  CTheCaliperTool
{
	
public:
	CTheCaliperTool();
	~CTheCaliperTool();

    int getDlgParameters(const dlgParameters src_Info);//������������.cpp�ж���
	
	
	dlgParameters m_enDlgParameters;//ʵ�����ṹ�壬��DataFormat.h�ж���

	clpParameters m_enClpParameters;

	

	int getClpParameters(const clpParameters src_Info);

};
