#pragma once
#include "snyParameter.h"

//�����ò���
//#define 

using namespace cv;
using namespace std;
class CSeeney_Caliper
{
public:
	CSeeney_Caliper();
	~CSeeney_Caliper();

	int CaliperMain();
	int sny_getCaliperPara(const sttCaliperInput srcPara);
	int sny_getInputImg(const cv::Mat src);
	int	sny_setResult(const int num, sttCaliperOutput * resEdge);
	//int createROI(const int mode);
	int sny_createOriginROI(const cv::Mat srcMat, const cv::Rect2d oriROI, 
							const float rotAngle, const float skewAngle,
							cv::Mat &maskMat, cv::Mat &roiMat, cv::Mat * affine_matrix);
	int sny_crtCentModeROI(const cv::Mat srcMat, const cv::Rect2d oriROI, 
						   const float rotAngle, const float skewAngle,
						   cv::Mat &maskMat, cv::Mat &roiMat, cv::Mat * affine_matrix);
	int sny_crt3PointsModeROI(const cv::Mat srcMat,
							  const cv::Point2d pt1,
							  const cv::Point2d pt2,
							  const cv::Point2d pt3,
							  cv::Mat &maskMat, cv::Mat &roiMat, cv::Mat * affine_matrix);

	int sny_calcRotSkew(const cv::Point srcP1, const cv::Point srcP2, const cv::Point srcP3,
		cv::Rect &oriROI, float *rotAngle, float *skewAngle);

	int sny_calRotPosition(const cv::Point2d center,const cv::Point2d srcPoint,cv::Point2d &dstPoint,float rotAngle);
	int sny_calcHoriProject(const cv::Mat srcMat,const cv::Mat mskMat, int * dstArry);
	int sny_convHoriProject(const int * srcArry, const int srcSize,const int convSize, int * dstArry);
	int sny_searchSingleEdge(const int * derHist, const int hisSize, const cv::Size roiSize, 
							const int maxNum, 
							const int contstTH,
							const int polarity,
							const int kernelSize,
							int * nEdge,sttCaliperOutput * dstEdges);
	//int sny_calcOriginPos();
	int sny_convertAffineMat(const cv::Mat opencvAffineMat,cv::Mat & dstMat);
	//int sny_filtHoriProject();
	int sny_showHorProjection(const int * srcArry, const int srcSize, cv::Size disSize, cv::Mat & dstMat);
	int sny_showConProjection(const int * srcArry, const int srcSize, cv::Size disSize, cv::Mat & dstMat);
	

	//float sny_Distance();
	int sny_samePoints2d(const cv::Point2d pt1,const cv::Point2d pt2);
	int sny_3ptsCreateCountors(const cv::Point2d pt1, const cv::Point2d pt2, const cv::Point2d pt3,
								vector<Point> &roiImgPts, vector<Point> &roiPrjPts);

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

	cv::Mat m_aAffineMatrix[CA_TOTALNUM];	//0����ͶӰͼ����4�����㵽ԭͼ���е�ӳ���ϵ��
											//1��ԭͼ�񵽱�ͶӰͼ���ӳ���ϵ 
	
	sttCaliperOutput * m_resEdge;
	int m_nEdge;
	
};

