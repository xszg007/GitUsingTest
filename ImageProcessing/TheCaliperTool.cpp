#include "TheCaliperTool.h"
#include "Math.h"
#include "iostream"


int CTheCaliperTool::getDlgParameters(const dlgParameters src_Info)//º¯Êý¶¨Òå
{
	m_enDlgParameters.inputMethod = src_Info.inputMethod;
	memcpy(m_enDlgParameters.strInputFilePath, src_Info.strInputFilePath, MAX_PATH_STR_SIZE);
	memcpy(m_enDlgParameters.strSavePath, src_Info.strSavePath, MAX_PATH_STR_SIZE);
	/*if (m_enDlgParameters.strInputFilePath == NULL)
	{
		return 0;
	}
	else
		return 1;*/
	return 0;
}

int CTheCaliperTool::getClpParameters(const clpParameters src_Info)
{
	m_enClpParameters.edgeModel = src_Info.edgeModel;
	m_enClpParameters.edgePlrty = src_Info.edgePlrty;
	m_enClpParameters.edgeWidth = src_Info.edgeWidth;
	m_enClpParameters.contrastTH = src_Info.contrastTH;
	m_enClpParameters.filtHalfPixel = src_Info.filtHalfPixel;
	m_enClpParameters.maxLineNum = src_Info.maxLineNum;
	m_enClpParameters.useCaliperRoi = src_Info.useCaliperRoi;
	m_enClpParameters.imgSpace = src_Info.imgSpace;
	m_enClpParameters.roiMode = src_Info.roiMode;
	m_enClpParameters.originX = src_Info.originX;
	m_enClpParameters.originY = src_Info.originY;
	m_enClpParameters.width = src_Info.width;
	m_enClpParameters.length = src_Info.length;
	m_enClpParameters.rotAngle = src_Info.rotAngle;
	m_enClpParameters.skewAngle = src_Info.skewAngle;
	m_enClpParameters.X1 = src_Info.X1;
	m_enClpParameters.Y1 = src_Info.Y1;
	m_enClpParameters.X2 = src_Info.X2;
	m_enClpParameters.Y2 = src_Info.Y2;
	m_enClpParameters.mapScore = src_Info.mapScore;
	m_enClpParameters.mapX0 = src_Info.mapX0;
	m_enClpParameters.mapX1 = src_Info.mapX1;
	m_enClpParameters.mapXc = src_Info.mapXc;
	m_enClpParameters.displayEdge = src_Info.displayEdge;
	m_enClpParameters.displayProjection = src_Info.displayProjection;
	m_enClpParameters.displayFiltered = src_Info.displayFiltered;
	m_enClpParameters.dispalyTransEdge = src_Info.dispalyTransEdge;
	
	return 0;
}


CTheCaliperTool::CTheCaliperTool()
{
}


CTheCaliperTool::~CTheCaliperTool()
{
}