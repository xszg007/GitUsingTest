#include "dataFormat.h"//����ͷ�ļ���չ������ṹ�嶨�塣
#include <iostream>
#include "TheCaliperTool.h"
#include "CSeeney_Caliper.h"


dlgParameters enDlgPrmtrs;//ʵ�����ṹ�壨�Ѿ���DataFormat.h�ж��壩
CTheCaliperTool ImagePro;
sttCaliperInput enClpPrmtrs;//ʵ�����ṹ�壨��snyParameter.h�ж��壩
sttCaliperOutput enClpRes;
CSeeney_Caliper enSeeneyCaliper;



extern "C" _declspec(dllexport) void _stdcall interface_getClpParameter(clpParameters *info);
void _stdcall interface_getClpParameter(clpParameters *info)
{
	//-----------------���ò���-----------------
	enClpPrmtrs.edgeModel = info->edgeModel;
	enClpPrmtrs.edgePlrty = info->edgePlrty;
	enClpPrmtrs.edgeWidth = info->edgeWidth;
	enClpPrmtrs.contrastTH = info->contrastTH;
	enClpPrmtrs.filtHalfPixel = info->filtHalfPixel;
	enClpPrmtrs.maxLineNum = info->maxLineNum;

	//-----------------���򲿷�-----------------
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
	
	//-----------------�Ƿֲ���----------------- 
	enClpPrmtrs.mapScore = info->mapScore;
	enClpPrmtrs.mapX0 = info->mapX0;
	enClpPrmtrs.mapX1 = info->mapX1;
	enClpPrmtrs.mapXc = info->mapXc;

	//-----------------ͼ�β���----------------- 
	enClpPrmtrs.displayEdge = info->displayEdge;
	enClpPrmtrs.displayProjection = info->displayProjection;
	enClpPrmtrs.displayFiltered = info->displayFiltered;
	enClpPrmtrs.dispalyTransEdge = info->dispalyTransEdge;

	//-----------------���Բ���-----------------
	enClpPrmtrs.displaySteps=info->displaySteps;								
	enClpPrmtrs.writeLog=info->writeLog;								
}

//���庯����interface_getDlgParameter��
extern "C" _declspec(dllexport) void _stdcall interface_getDlgParameter(dlgParameters *info);
void _stdcall interface_getDlgParameter(dlgParameters *info)//C#��DllImport�ĵ���һ��Ҫ������ȫһ��������
{
	enDlgPrmtrs.inputMethod = info->inputMethod;//int�ͱ���������ֱ�Ӹ�ֵ
	memcpy(enDlgPrmtrs.strInputFilePath, info->strInputFilePath, MAX_PATH_STR_SIZE);
	memcpy(enDlgPrmtrs.strSavePath, info->strSavePath, MAX_PATH_STR_SIZE);

}


extern "C" _declspec(dllexport) void _stdcall interface_Main();
void _stdcall interface_Main()
{
ImagePro.getDlgParameters(enDlgPrmtrs);
	//�������
	int f=1;//���߲������������f����ֵΪ0
	f=enSeeneyCaliper.sny_getCaliperPara(enClpPrmtrs);
	//enSeeneyCaliper.sny_getInputImg();
	Mat src = imread(enDlgPrmtrs.strInputFilePath, 0);
	enSeeneyCaliper.sny_getInputImg(src);
	int resVal=enSeeneyCaliper.CaliperMain();
	
	//��ý��
	sttCaliperOutput * resEdge = new sttCaliperOutput[enClpPrmtrs.maxLineNum];
	if (resVal == 0) {
		enSeeneyCaliper.sny_setResult(enClpPrmtrs.maxLineNum, resEdge);//sny_setResult�Ѿ��洢�˸�������ֵ
		// �˴�Ӧ��sny_setResult��Ӧλ�õ�ֵ�����ӿڵı���

	}

	int xxx = 0;

	delete[]resEdge;
}

extern "C" _declspec(dllexport) void _stdcall getResult(int num, sttCaliperOutput * resEdge);
void _stdcall getResult(int num, sttCaliperOutput * resEdge)
{
	enSeeneyCaliper.sny_setResult(num, resEdge);//��parameter.c��Ӧ

}
