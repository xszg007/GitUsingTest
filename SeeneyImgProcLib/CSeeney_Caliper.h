#pragma once
#include "snyParameter.h"

//测试用参数
//#define 

using namespace cv;
class CSeeney_Caliper
{
public:
	CSeeney_Caliper();
	~CSeeney_Caliper();

	int CaliperMain();
	int  sny_getCaliperPara(const sttCaliperInput srcPara);
	int  sny_getInputImg(const cv::Mat src);

	//int createROI(const int mode);
	int sny_createOriginROI(const cv::Mat srcMat, const cv::Rect oriROI, const float rotAngle, const float skewAngle,
		cv::Mat &maskMat, cv::Mat &roiMat);
	int sny_calRotPosition(const cv::Point center,const cv::Point srcPoint,cv::Point &dstPoint,float rotAngle);
	int sny_calcHoriProject(const cv::Mat srcMat,const cv::Mat mskMat, int * dstArry);
	int sny_convHoriProject(const int * srcArry, const int srcSize,const int convSize, int * dstArry);
	int sny_searchSingleEdge(const int * derHist, const int hisSize, const cv::Size roiSize, 
							const int maxNum, 
							const int contstTH,
							const int polarity,
							const int kernelSize,
							std::vector<sttSingleEdge> * dstEdges);
	//int sny_filtHoriProject();
	int sny_showHorProjection(const int * srcArry, const int srcSize, cv::Size disSize, cv::Mat & dstMat);
	int sny_showConProjection(const int * srcArry, const int srcSize, cv::Size disSize, cv::Mat & dstMat);
private:

	//输入计算参数
	sttCaliperInput m_enSttInputPara;
	

	int m_kernelSize;			//寻边卷积尺，卷积最终尺寸为 m_kernelSize*2+1 TODO可以填加到前端的输入参数中

	//输入图像
	cv::Mat m_srcMat;			//原始图像
	cv::Mat m_roiMat;			//旋转错切后的roi图像
	cv::Mat m_mskMat;			//与m_roiMat对应的mask
	cv::Mat m_dspMat;			//显示投影结果的图片
	cv::Mat m_derMat;			//显示卷积结果的图片
	//cv::Mat m_mskMat;			//与m_roiMat对应的mask
	//cv::Mat m_mskMat;			//与m_roiMat对应的mask
	int * m_prjHist = new int;	//投影计算结果
	int * m_derHist = new int;	//投影数列的卷积结果

	std::vector<sttSingleEdge> m_vResEdge;

};

