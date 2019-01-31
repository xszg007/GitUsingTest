#pragma once
#include "snyParameter.h"

//�����ò���
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

	//����������
	sttCaliperInput m_enSttInputPara;
	

	int m_kernelSize;			//Ѱ�߾���ߣ�������ճߴ�Ϊ m_kernelSize*2+1 TODO������ӵ�ǰ�˵����������

	//����ͼ��
	cv::Mat m_srcMat;			//ԭʼͼ��
	cv::Mat m_roiMat;			//��ת���к��roiͼ��
	cv::Mat m_mskMat;			//��m_roiMat��Ӧ��mask
	cv::Mat m_dspMat;			//��ʾͶӰ�����ͼƬ
	cv::Mat m_derMat;			//��ʾ��������ͼƬ
	//cv::Mat m_mskMat;			//��m_roiMat��Ӧ��mask
	//cv::Mat m_mskMat;			//��m_roiMat��Ӧ��mask
	int * m_prjHist = new int;	//ͶӰ������
	int * m_derHist = new int;	//ͶӰ���еľ�����

	std::vector<sttSingleEdge> m_vResEdge;

};

