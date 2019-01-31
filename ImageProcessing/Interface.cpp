#include "dataFormat.h"//引用头文件，展开贴入结构体定义。
#include <iostream>
#include "TheCaliperTool.h"
#include "CSeeney_Caliper.h"


dlgParameters enDlgPrmtrs;//实例化结构体（已经在DataFormat.h中定义）
CTheCaliperTool ImagePro;
sttCaliperInput enClpPrmtrs;//实例化结构体（在snyParameter.h中定义）
sttCaliperOutput enClpRes;
CSeeney_Caliper enSeeneyCaliper;



extern "C" _declspec(dllexport) void _stdcall interface_getClpParameter(clpParameters *info);
void _stdcall interface_getClpParameter(clpParameters *info)
{
	//-----------------设置部分-----------------
	enClpPrmtrs.edgeModel = info->edgeModel;
	enClpPrmtrs.edgePlrty = info->edgePlrty;
	enClpPrmtrs.edgeWidth = info->edgeWidth;
	enClpPrmtrs.contrastTH = info->contrastTH;
	enClpPrmtrs.filtHalfPixel = info->filtHalfPixel;
	enClpPrmtrs.maxLineNum = info->maxLineNum;

	//-----------------区域部分-----------------
	enClpPrmtrs.useCaliperROI = info->useCaliperRoi;
	enClpPrmtrs.imgSpace = info->imgSpace;
	enClpPrmtrs.roiMode = info->roiMode;
	enClpPrmtrs.originX = info->originX;
	enClpPrmtrs.originY = info->originY;
	enClpPrmtrs.width = info->width;
	enClpPrmtrs.length = info->length;
	enClpPrmtrs.rotAngle = info->rotAngle;
	enClpPrmtrs.skewAngle = info->skewAngle;
	enClpPrmtrs.X1 = info->X1;
	enClpPrmtrs.Y1 = info->Y1;
	enClpPrmtrs.X2 = info->X2;
	enClpPrmtrs.Y2 = info->Y2;
	
	//-----------------记分部分----------------- 
	enClpPrmtrs.mapScore = info->mapScore;
	enClpPrmtrs.mapX0 = info->mapX0;
	enClpPrmtrs.mapX1 = info->mapX1;
	enClpPrmtrs.mapXc = info->mapXc;

	//-----------------图形部分----------------- 
	enClpPrmtrs.displayEdge = info->displayEdge;
	enClpPrmtrs.displayProjection = info->displayProjection;
	enClpPrmtrs.displayFiltered = info->displayFiltered;
	enClpPrmtrs.dispalyTransEdge = info->dispalyTransEdge;

	//-----------------调试参数-----------------
	enClpPrmtrs.displaySteps=info->displaySteps;								
	enClpPrmtrs.writeLog=info->writeLog;								
}

//定义函数“interface_getDlgParameter”
extern "C" _declspec(dllexport) void _stdcall interface_getDlgParameter(dlgParameters *info);
void _stdcall interface_getDlgParameter(dlgParameters *info)//C#中DllImport的调用一定要和它完全一样！！！
{
	enDlgPrmtrs.inputMethod = info->inputMethod;//int型变量，可以直接赋值
	memcpy(enDlgPrmtrs.strInputFilePath, info->strInputFilePath, MAX_PATH_STR_SIZE);
	memcpy(enDlgPrmtrs.strSavePath, info->strSavePath, MAX_PATH_STR_SIZE);

}


extern "C" _declspec(dllexport) void _stdcall interface_Main();
void _stdcall interface_Main()
{
ImagePro.getDlgParameters(enDlgPrmtrs);
	//输入参数
	int f=1;//卡尺参数传递完成则f返回值为0
	f=enSeeneyCaliper.sny_getCaliperPara(enClpPrmtrs);
	//enSeeneyCaliper.sny_getInputImg();
	Mat src = imread(enDlgPrmtrs.strInputFilePath, 0);
	enSeeneyCaliper.sny_getInputImg(src);
	int resVal=enSeeneyCaliper.CaliperMain();
	
	//获得结果
	sttCaliperOutput * resEdge = new sttCaliperOutput[enClpPrmtrs.maxLineNum];
	if (resVal == 0) {
		enSeeneyCaliper.sny_setResult(enClpPrmtrs.maxLineNum, resEdge);//sny_setResult已经存储了各点的输出值
		// 此处应将sny_setResult对应位置的值赋给接口的变量

	}

	int xxx = 0;

	delete[]resEdge;
}

extern "C" _declspec(dllexport) void _stdcall getResult(int num, sttCaliperOutput * resEdge);
void _stdcall getResult(int num, sttCaliperOutput * resEdge)
{
	enSeeneyCaliper.sny_setResult(num, resEdge);//与parameter.c对应

}
