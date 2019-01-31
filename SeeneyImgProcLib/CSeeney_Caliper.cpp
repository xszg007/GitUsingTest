#include "pch.h"
#include "CSeeney_Caliper.h"
#include <algorithm>

using namespace cv;
using namespace std;
CSeeney_Caliper::CSeeney_Caliper()
{
}


CSeeney_Caliper::~CSeeney_Caliper()
{
}


struct convNode 
{
	float data;
	int No;
};

bool compare4bigger(convNode a, convNode b)
{
	return a.data > b.data;
}

bool compare4smaller(convNode a, convNode b)
{
	return a.data < b.data;
}

//����˵����ʽ
/********************************************************
-������:
-��������


-�������:


-�������:


-����ֵ:


-��ע

********************************************************/


//-----------------����������-----------------
int CSeeney_Caliper::CaliperMain()
{
	//0.��ʼ������getCaliperPara ��ȡ��������getInputImg��ȡͼ��


	//----------------------------------
	//1.����roi,������ת�õĴ�ͶӰͼ��
	Rect m_oriROI;						//δ��ת��ԭʼͼ��roi
	//roi��ֵ
	m_oriROI.x = m_enSttInputPara.originX;
	m_oriROI.y = m_enSttInputPara.originY;
	m_oriROI.width = m_enSttInputPara.width;
	m_oriROI.height = m_enSttInputPara.length;

	if (m_enSttInputPara.useCaliperROI == 0) {
		//���ݿ����������ȷ��roi
		//���ݴ��У���ת������ʵ��ͶӰroi
		sny_createOriginROI(m_srcMat, m_oriROI, m_enSttInputPara.rotAngle, m_enSttInputPara.skewAngle, m_mskMat, m_roiMat);
		imshow("m_mskMat", m_mskMat);
	
	}
	else if (m_enSttInputPara.useCaliperROI == 1) {
		//��ԭͼ��Ѱ�ұ�Ե
		m_roiMat = m_srcMat;
		m_mskMat = m_srcMat;
	}

	//----------------------------------
	//2.����ͶӰ����
	int prjSize = m_roiMat.rows;
	m_prjHist = new int[prjSize];
	m_derHist = new int[prjSize];
	memset(m_prjHist, 0, sizeof(int)*prjSize);
	memset(m_derHist, 0, sizeof(int)*prjSize);

	//����ͶӰ
	sny_calcHoriProject(m_roiMat, m_mskMat, m_prjHist);
	if (m_enSttInputPara.displayProjection == 1) {
		//ͶӰ���ͼ������
		sny_showHorProjection(m_prjHist, prjSize, Size(m_roiMat.cols, m_roiMat.rows), m_dspMat);
		imshow("m_dspMat",m_dspMat);
	}

	//��ͶӰ���������
	m_kernelSize = 2;
	sny_convHoriProject(m_prjHist,prjSize,2,m_derHist);
	if (m_enSttInputPara.displayFiltered==1) {
		//����������
		sny_showConProjection(m_derHist, prjSize, Size(m_roiMat.cols, m_roiMat.rows), m_derMat);
		imshow("m_derMat", m_derMat);
	}

	//Ѱ�ұ�Ե
	sny_searchSingleEdge(
		m_derHist, prjSize, Size(m_roiMat.cols, m_roiMat.rows),
		m_enSttInputPara.maxLineNum,
		m_enSttInputPara.contrastTH,
		m_enSttInputPara.edgePlrty,
		m_kernelSize,
		&m_vResEdge);


	//ͶӰͼ���е�λ�ã�����תrotAngle��	
	for (int i = 0; i < m_enSttInputPara.maxLineNum; i++) {
		cv::Point tempP;

		sny_calRotPosition(Point(m_oriROI.x, m_oriROI.y), Point(m_roiMat.cols / 2, m_vResEdge[i].index),
			tempP, m_enSttInputPara.rotAngle);
		circle(m_derMat,Point(0,m_vResEdge[i].index),5,Scalar(100,100,0),-1,8,0);
		circle(m_roiMat,Point(m_roiMat.cols/2,m_vResEdge[i].index),3,Scalar(0,0,255),1,8,0);
		circle(m_srcMat,Point(tempP.x+m_oriROI.x,tempP.y+m_oriROI.y),3,Scalar(0,0,255),1,8,0);
	}
	imshow("maxresults", m_derMat);
	imshow("m_roiMat", m_roiMat);
	imshow("final", m_srcMat);
	//�����Ե����ԭͼ���е�λ��

	waitKey(0);
	
	delete[]m_prjHist;
	delete[]m_derHist;
	return 0;
}

int CSeeney_Caliper::sny_getCaliperPara(const sttCaliperInput srcPara)
{
	
	//-----------------���ò���-----------------
	m_enSttInputPara.edgeModel = srcPara.edgeModel;				// 0��������Ե 1����Ե��
	m_enSttInputPara.edgePlrty = srcPara.edgePlrty;				// 0���ɰ����� 1���������� 2�����⼫��
	m_enSttInputPara.edgeWidth = srcPara.edgeWidth;				// [0,roi���] TODO����Ե������� 	

	m_enSttInputPara.contrastTH = srcPara.contrastTH;			// [0,255] TODO����ֵ������Ҫ����
	m_enSttInputPara.filtHalfPixel = srcPara.filtHalfPixel;		// []	   TODO�����岻������	
	m_enSttInputPara.maxLineNum = srcPara.maxLineNum;			// [1,??]  TODO��roi������ߵ����������ޣ�����


	//-----------------���򲿷�-----------------
	m_enSttInputPara.useCaliperROI=srcPara.useCaliperROI; 		// 0��ʹ�ÿ���������Ϊroi��1��ȫͼ��Ϊroi
	m_enSttInputPara.imgSpace = srcPara.imgSpace;				// 0������ͼ��ռ䡣1�����ؿռ䡣2�����ռ䡣 TODO�����岻��
	m_enSttInputPara.roiMode = srcPara.roiMode;					// 0��ԭ��ģʽ��1������ģʽ��2��3��ģʽ

	//��ͬ����
	m_enSttInputPara.originX = srcPara.originX;					// ԭ��x����
	m_enSttInputPara.originY = srcPara.originY;					// ԭ��y���� 

	//ԭ��ģʽ������ģʽ������ͬ��ʹ�����²���
	//TODO�����Ƿ���float��
	m_enSttInputPara.width = srcPara.width;						// ���
	m_enSttInputPara.length = srcPara.length;					// ����	
	m_enSttInputPara.rotAngle = srcPara.rotAngle;				// roi��ת�Ƕ�
	m_enSttInputPara.skewAngle = srcPara.skewAngle;				// roi���нǶ�

	//3��ģʽʹ�����²���
	//TODO����˳��
	m_enSttInputPara.X1 = srcPara.X1;							// �ڶ�������X
	m_enSttInputPara.Y1 = srcPara.Y1;							// �ڶ�������Y
	m_enSttInputPara.X2 = srcPara.X2;							// ����������.
	m_enSttInputPara.Y2 = srcPara.Y2;							// ����������

	//-----------------�Ƿֲ���----------------- 
	m_enSttInputPara.mapScore = srcPara.mapScore;				// 0:������ 1������
	m_enSttInputPara.mapX0 = srcPara.mapX0;						// TODO����ֵ�ķ�Χ
	m_enSttInputPara.mapX1 = srcPara.mapX1;
	m_enSttInputPara.mapXc = srcPara.mapXc;

	//-----------------ͼ�β���----------------- 
	//TODO:��ʾͼ��ĳߴ��Ƿ���Ҫ�ɵ�
	m_enSttInputPara.displayEdge = srcPara.displayEdge;				//0������ʾ��1����ʾ�ҵ��ı�Ե
	m_enSttInputPara.displayProjection = srcPara.displayProjection;	//0������ʾ��1����ʾͶӰͼ��
	m_enSttInputPara.displayFiltered = srcPara.displayFiltered;		//0������ʾ��1����ʾ���˹���ͼ�� TODO�����岻��
	m_enSttInputPara.dispalyTransEdge = srcPara.dispalyTransEdge;	//0������ʾ��1����ʾת��ͼ���ϵı�Ե TODO:���岻��


	return 0;
}

int CSeeney_Caliper::sny_getInputImg(const cv::Mat src)
{
	//���ⲿ����ͼ��
	//����Ƿ�Ϊ8λ�Ҷ�ͼ
	if (src.type() == CV_8UC1)
	{
		src.copyTo(m_srcMat);
		return 0;
	}
	else {
		return 1;
	}
	
}










/********************************************************
-������:createOriginROI

-����������
1.����ԭ�����꣬roi�����Ƚ�������roi��Ȼ��ͨ��skew�ǶȽ��д��д���
���ڴ���skew���д�����Ҫ����mask Mat������mask���ݴ���ͶӰ����
2.����roi�Ƕ���ת��˳ʱ��Ϊ������ʱ��Ϊ������opencv�෴
3.�Ƕ���ת�󣬵õ�roi����������

���Ͼ���roi���������һ������ͼ�����

4.����Ѱ�ߵķ���Ϊ��ֱx�����£��ʽ�roi�����ͼ����ת������
�ں����ͶӰ������ֻ����ˮƽͶӰ��

TODOͶӰʵ����Ϊ�ۼ�����ֵ������0����Ӱ��ͶӰ�����ʵ�ʲ���Ҫmask��

-�������:
ԭʼͼ��const cv::Mat srcMat, 
ͼ��ԭʼδ��תδ���е�roi��const cv::Rect oriROI, 
��ת�Ƕȣ�const float rotAngle, 
���нǶȣ�const float skewAngle

-�������:
��ת���roi������ͼ���Լ���ȫ��ͬ����mask Mat

-����ֵ:
ִ�гɹ���0
������룺

-��ע
���Ƚ��������Σ������Ͻ�Ϊԭ��[x,y]
  [x,y]--------
    /|\        |
   /a|b\       | 
      ---------
˳ʱ��aΪ���Ƕȣ���ʱ��bΪ���Ƕ�

��ת����˳ʱ��Ϊ��

********************************************************/
int CSeeney_Caliper::sny_createOriginROI(const cv::Mat srcMat, const cv::Rect oriROI, const float rotAngle, const float skewAngle,
									 cv::Mat &maskMat,cv::Mat &roiMat)
{
	//���нǶȴ��󣬲���Ϊ���½Ƕ�
	if (skewAngle == 90 || skewAngle == -90 || skewAngle > 180 || skewAngle < -180) {
		return 1;
	}

	cv::Mat displayMat;	    //��ʾ��
	srcMat.copyTo(displayMat);
	Size skewedRoiSize;

	//��ԭͼͬ���ߴ��rect���ж��Ƿ������
	cv::Rect srcMatRect;
	srcMatRect.x = 0;
	srcMatRect.y = 0;
	srcMatRect.width  = srcMat.cols;
	srcMatRect.height = srcMat.rows;
 
	//���к���ı��ζ������꣬ԭͼ����ϵ
	vector<vector<Point>> contour;
	vector<Point> pts;
	//���к���ı��ζ������꣬roi����ϵ
	vector<vector<Point>> roi_contour;
	vector<Point> roi_pts;

	//��ת����ı��ζ������꣬ԭͼ����ϵ
	vector<vector<Point>> rot_contour;
	vector<Point> rpts;
	//����������roi����ϵ
	vector<vector<Point>> dst_contour;
	vector<Point> dst_pts;
	//dx,����ƽ�ƾ��룬py������������ϵ��

	//----------------1.���д���------------------ 
	//����skewAngle����roi���к�Ķ���	
	//���нǶ�Ӧ��(-90,90)
	//���� = �Ƕ� * Pi / 180;
	float skewRadian = skewAngle * CV_PI / 180;
	float rotaRadian = rotAngle * CV_PI / 180;
	float dx;
	float py;
	//������к��ƽ�ƾ���
	if (skewAngle > -90 && skewAngle < 90) {
		dx = - oriROI.height * tan(skewRadian);
		py = 1;
	}
	else if ((skewAngle > 90 && skewAngle <= 180) || (skewAngle < -90 && skewAngle >= -180) ){
		dx = -oriROI.height * tan(CV_PI - skewRadian);
		py = -1;
	}
	//���ɴ��к���ĸ�����,��ԭ����ϵ������	
	pts.push_back(Point(oriROI.x, oriROI.y));
	pts.push_back(Point(oriROI.x+oriROI.width, oriROI.y));
	pts.push_back(Point(oriROI.x+oriROI.width+dx, oriROI.y+py*oriROI.height));
	pts.push_back(Point(oriROI.x+dx, oriROI.y+py*oriROI.height));
	contour.push_back(pts);

	//TODOcheck���к��Ƿ񳬳�ͼ��
	//ʵ�ʲ�Ӧ�ó��磬�������ڣ�ǰ�˴��صĲ���������ת�����̿��ܴ���������check
	//�Ľ����η�ʽ��Ӧ�ÿ���ȥ��

	for (int n = 0; n < 4;n++) {
		if (srcMatRect.contains(pts[n])!=1) {
			//return 1;
		}
	}
	//�������ͶӰͼ��ľ��γߴ�׼��roi
	cv::Rect rotbackROI= cv::boundingRect(cv::Mat(pts).reshape(2));

	//----------------2.��ת����------------------ 
	//�����ĸ�������ת֮���������
	for (int i = 0; i < 4; i++) {
		cv::Point temp1;
		//������ת�����꺯������ԭͼ����ϵ
		//����˳ʱ��Ϊ������������Ӹ���
		sny_calRotPosition(pts[0], pts[i], temp1, (-1*rotAngle));
		//TODO�ж��Ƿ����
		
		//Ϊ����ͶӰͼ�����4���������꣬������ͶӰͼ������ϵ
		cv::Point temp2;
		temp2.x = pts[i].x - rotbackROI.x;
		temp2.y = pts[i].y - rotbackROI.y;

		roi_pts.push_back(temp2);
		rpts.push_back(temp1);

	}
	rot_contour.push_back(rpts);
	roi_contour.push_back(roi_pts);

	//----------------3.����ͼ��------------------ 
	//���м���ת��ɣ����ɵ�roi,��roi����Ϊԭͼ������ϵ�е�����
	cv::Rect dstRoiRect = cv::boundingRect(cv::Mat(rpts).reshape(2));
	
	//����maskͼ��
	//�޸�mask���꣬����roi����ϵ�µ�����
	for (int i = 0; i < 4; i++) {
		//TODO�ж��Ƿ����
		cv::Point temp;
		temp.x = rpts[i].x - dstRoiRect.x;
		temp.y = rpts[i].y - dstRoiRect.y;
		dst_pts.push_back(temp);
	}
	dst_contour.push_back(dst_pts);

	//�������յ�maskMat����ͼ������ϵ
	maskMat = Mat::zeros(dstRoiRect.size(),CV_8UC1);
	drawContours(maskMat, dst_contour, 0, Scalar::all(255), -1);

	//�������յ�roiͼ����ͼ������ϵ
	cv::Mat temp = srcMat(dstRoiRect);
	temp.copyTo(roiMat,maskMat);

	//----------------4.ͼ�����------------------ 
	//Ϊ��һ��ͶӰ���������ɷ���ͶӰ���������ͼ��
	//����roiMat��maskMat������תrotAngle
	// �任ǰ����
	const cv::Point2f src_pt[] = { cv::Point2f(dst_pts[0].x, dst_pts[0].y),
								   cv::Point2f(dst_pts[1].x, dst_pts[1].y),
								   cv::Point2f(dst_pts[2].x, dst_pts[2].y)};
	// ��������
	const cv::Point2f dst_pt[] = { cv::Point2f(roi_pts[0].x, roi_pts[0].y),
								   cv::Point2f(roi_pts[1].x, roi_pts[1].y),
								   cv::Point2f(roi_pts[2].x, roi_pts[2].y) };
	const cv::Mat affine_matrix = cv::getAffineTransform(src_pt, dst_pt);
	cv::warpAffine(roiMat, roiMat, affine_matrix, rotbackROI.size());
	cv::warpAffine(maskMat, maskMat, affine_matrix, rotbackROI.size());
	imshow("roiMat",roiMat);

	//�軭���
	//���к����״
	drawContours(displayMat, contour, 0, Scalar::all(255),1);
	//��ת�����״
	drawContours(displayMat, rot_contour, 0, cvScalar(0, 255, 255), 2);
	//ԭ��
	circle(displayMat, Point(oriROI.x, oriROI.y), 3, cvScalar(0, 0, 255), 8, 0);
	//����roi
	rectangle(displayMat, dstRoiRect, cvScalar(255, 0, 255), 1, 8, 0);
	//ԭʼroi
	rectangle(displayMat, oriROI, cvScalar(255, 255, 255), 1, 8, 0);
	
	imshow("display", displayMat);
	imshow("srcMat", srcMat);
	imshow("dstMask", maskMat);
	//waitKey(0);
	return 0;
}

/********************************************************
-������:sny_calRotPosition


-����������
��center������ΪԲ�ģ�����ԭ����srcPoint��תrotAngle�Ⱥ�����ꡣ
ͨ��3��3��affine����ʵ��
-�������:
const cv::Point center����תԭ�㣺
const cv::Point srcPoint���������꣺
float roAngle����ת�Ƕȣ��Ƕ�ֵ���ǻ���

-�������:
cv::Point & dstPoint����ת�������

-����ֵ:

-��ע
rotAngleΪ�Ƕ�ֵ���ں����ڲ�ת��Ϊ���ȣ���ʱ��Ϊ��ֵ��x��Ϊ0��

********************************************************/
int CSeeney_Caliper::sny_calRotPosition(const cv::Point center, const cv::Point srcPoint, cv::Point & dstPoint, float rotAngle)
{
	float rotaRadian = rotAngle * CV_PI / 180;

	//����4������ת������꣬ͨ����ת����	
	float ocx = center.x;
	float ocy = center.y;
	cv::Point2f rotCenter(ocx, ocy);

	//ƽ�ƾ���
	float offset[3][3] =
	{
		{1,     0,   0},
		{0,    -1,   0},
		{-ocx, ocy,  1}
	};
	cv::Mat offsetMat(3, 3, CV_32FC1, offset);

	//��ת����
	float rot[3][3] =
	{
		{    std::cos(rotaRadian),   std::sin(rotaRadian),     0},
		{ -1*std::sin(rotaRadian),   std::cos(rotaRadian),     0 },
		{                       0,                      0,     1 }
	};

	cv::Mat rotMat(3, 3, CV_32FC1, rot);
	
	cv::Mat dstPosMat = cv::Mat::ones(1, 3, CV_32FC1);
	dstPosMat.at<float>(0, 0) = srcPoint.x;
	dstPosMat.at<float>(0, 1) = srcPoint.y;
	cv::Mat tmp = offsetMat * rotMat;
	cv::Mat topLeftDst = dstPosMat * tmp;

	dstPoint.x= topLeftDst.at<float>(0, 0) + ocx;
	dstPoint.y= (-1*topLeftDst.at<float>(0, 1)) + ocy;

	return 0;
}


/********************************************************
-������:sny_calcHoriProject
-��������
ˮƽ�����ͶӰ
�ۼ����лҶ�ֵ
 
-�������:
const cv::Mat srcMat������ͼ��
const cv::Mat mskMat��mask��Ŀǰû��ʹ�á��ۼӻҶ�ֵ����ɫ���ۼӡ���

-�������:
int * dstArry��ͶӰ�����1ά���飬dstArry[0]

-����ֵ:
����0�������ɹ�
����1�����ͼ��ǻҶ�ͼ
����2������ߴ����

-��ע
dstArry����Ϊint�ͣ���ͶӰ�����������ֵΪ 2147483647 / 255 = 8,421,504
ͼ���Ȳ��ܳ���������
������˵�ߴ粻��ﵽ�����ֵ

********************************************************/
int CSeeney_Caliper::sny_calcHoriProject(const cv::Mat srcMat, const cv::Mat mskMat, int * dstArry)
{
	if (srcMat.type() != CV_8UC1) {
		return 1;
	}
	
	if (srcMat.cols>=8421504) {
		return 2;
	}
	//�����ۼ�
	int height = srcMat.rows;
	int width  = srcMat.cols;
	for (int i = 0; i < height;i++) {
		int intentSum = 0;
		for (int j = 0; j < width;j++) {

			intentSum+= srcMat.at<uchar>(i,j);
		}
		dstArry[i] = intentSum;
	}
	return 0;
}

/********************************************************
-������:sny_horizontalProjection
-��������
��ȡhist���ݶ�ֵ

-�������:
const int * srcArry������hist������
const int convSize������ߴ�conSize*2+1
const int srcSize������hist����ĳߴ�

-�������:
int * dstArry��������������

-����ֵ:
����0�������ɹ�

-��ע
�����size=5
[-1,-1,0,+1,+1]

�����޷����������Ϊ0��no padding
********************************************************/
int CSeeney_Caliper::sny_convHoriProject(const int * srcArry, const int srcSize,const int convSize, int * dstArry)
{
	int fsize = convSize * 2 + 1;
	int * kernel= new int[fsize];	

	//���ɾ����
	for (int n = 0; n < convSize;n++) {
		kernel[n] = -1;
		kernel[fsize - n-1] = 1;
	}
	kernel[convSize] = 0;

	//�������
	//TODO����Ż������opencv
	memset(dstArry,0,sizeof(int)*srcSize);
	int ulimit = srcSize - convSize;
	for (int i = convSize; i < ulimit;i++) {
		int sum = 0;
		for (int j = 0; j <= convSize;j++) {
			sum += srcArry[i - j]*kernel[convSize-j] + srcArry[i+j]*kernel[convSize+j];
		}
		dstArry[i] = sum;
	}
	
	delete[]kernel;
	return 0;
}



/********************************************************
-������:sny_showHorProjection
-��������
����ˮƽͶӰ�Ľ��ֱ��ͼ
ֱ��ͼΪ����������

5        -----|
7      -------|
2           --|       
3          ---|   

-�������:
const int * srcArry������hist������
const int srcSize��hist�����size
cv::Size disSize�����ͼ��ĳߴ磬Ŀǰ�ǺͿ���roi��ѡȡ������ߴ���ͬ�����ڶ��չ۲졣TODO���������ϵ��

-�������:
cv::Mat & dstMat���軭�����ͼ��

-����ֵ:
����0�������ɹ�

-��ע
���ͼ��Ŀǰ�ǺͿ���roi��ѡȡ������ߴ���ͬ�����ڶ��չ۲졣TODO���������ϵ��
�������������ۼ�ֵ����һ������ͼ�����Ͻ�Ϊԭ��
�����ֵΪ maxVal,���ͼ����Ϊwidth
ĳһ��bin��ֵΪx
���bin��ͼ���еĳ��� bin_lengthΪ��
x / maxVal = bin_length / width  
bin_length = (width * x) / maxVal

********************************************************/
int CSeeney_Caliper::sny_showHorProjection(const int * srcArry, const int srcSize, cv::Size disSize, cv::Mat & dstMat)
{
	//dstMat = Mat::zeros(disSize,CV_8UC1);
	cv::Mat red_img(disSize, CV_8UC3, cv::Scalar::all(255));

	int maxVal=*std::max_element(srcArry,srcArry+srcSize);

	for (int i = 0; i < srcSize;i++) {
		int bin_length=(disSize.width*srcArry[i])/maxVal;
		line(red_img, cv::Point(disSize.width-bin_length, i),cv::Point(disSize.width,i),Scalar(0,0,255), 1, 8, 0);
	}
	
	red_img.copyTo(dstMat);

	return 0;
}



/********************************************************
-������:sny_showConProjection 
-��������
��ͶӰͼ������Ľ��ͼ��

-�������:
const int * srcArry�����������������
const int srcSize��  ����������ߴ�
const int srcSize��  ����Ŀ��ͼ��ĳߴ磬��������Ϊ������ͶӰͼ�������

-�������:
int * dstMat��������ͼ��

-����ֵ:
����0�������ɹ�

-��ע
��ʾͼ��height�뱻����ͶӰ��ͼ����ͬ
���ھ����������и�����ʾͼ��width����Ϊ������ͶӰͼ�������
�����ʾ��ֵ���Ҳ���ʾ��ֵ
�磺
5        -----|
7      -------|
2           --|
3          ---|
-4            |----
-7            |-------
-1            |-
3          ---|

���þ���ֵ����ֵ���й�һ��

********************************************************/
int CSeeney_Caliper::sny_showConProjection(const int * srcArry, const int srcSize, cv::Size disSize, cv::Mat & dstMat)
{

	//dstMat = Mat::zeros(disSize,CV_8UC1);
	cv::Mat red_img(cv::Size(disSize.width*2,disSize.height), CV_8UC3, cv::Scalar::all(255));

	int maxVal = *std::max_element(srcArry, srcArry + srcSize);
	int minVal = *std::min_element(srcArry, srcArry + srcSize);
	int nomVal;
	
	if (maxVal >= abs(minVal)) {
		nomVal = maxVal;
	}
	else {
		nomVal = abs(minVal);
	}

	int centerX = disSize.width ;

	for (int i = 0; i < srcSize; i++) {
		if (srcArry[i]>0) {
			//�������0������ͼ������軭
			int bin_length = (centerX*srcArry[i]) / nomVal;
			line(red_img, cv::Point(centerX - bin_length, i), cv::Point(centerX, i), Scalar(0, 0, 255), 1, 8, 0);

		}
		else if (srcArry[i]<0) {
			int bin_length = (centerX*srcArry[i]) / nomVal;
			line(red_img, cv::Point(centerX - bin_length, i), cv::Point(centerX, i), Scalar(255, 0, 0), 1, 8, 0);
		}
		else if (srcArry[i]==0) {

		}
	}

	red_img.copyTo(dstMat);

	return 0;
}

/********************************************************
-������:
-��������


-�������:
const int * derHist������ĵ�������
const int hisSize�����뵼������ĳߴ硣
const int maxNum��Ѱ�ұ�Ե�ĸ�������Ѱ��derHist������ǰmaxNum����
const int contstTH���Աȶ���ֵ����Ե�ĶԱȶ�Ӧ�ô��ڴ���ֵ����Ҫ�޸ĶԱȶȶ��壬�ú�����ҲӦ��һͬ�޸�
const int polarity��0���ɰ����� 1���������� 2�����⼫��

-�������:
std::vector<sttSingleEdge>* dstEdges���������δ֪����ͨ����������������������Ϊ�Զ���ṹ��
�ṹ����������
typedef struct {
	//ÿ����⵽�ĵ��Ӧһ���ṹ��
	int position;			//ͶӰ��λ��
	float x;				//ͼ���е�λ�ã�������תʱ����Ҫ��ת��ͼ������ϵ��
	float y;				//ͼ���е�λ�ã�ͬ��
	float score;			//��Ե�÷֣�TODO��֪��ȷ�еļ��㷽������ʱ�ԶԱȶ�Ϊ����ֵ�÷֡�
	float contrast;			//�Աȶȣ�TODO�ݶ�Ϊ���ֵ�����ֵ����ʾ���ҵĲ�ֵ��

}sttSingleEdge;

-����ֵ:
���ؼ�⵽�ı�Ե����
������-1

-��ע
���ڼ��ԣ�
���Է�����Ϊ��������Ѱ��
ͶӰ���飬������飬�����Ϸ�Ϊԭ��
����˶���
	-1
	-1
	 0
	 1
	 1
�ʴӰ�����Ϊ ��ֵ
  ��������Ϊ ��ֵ

��������
float contrast��Ϊ������ֵ�����ݾ������������
���㷽����
��һ�б�Ե��˵�����ܲ��ֵΪ 255*kernelSize  kernelSize*2+1Ϊ���վ��ֵ
�磺kernelSize==2
ͼ��  ���
0     -1
0	  -1
0      0
255    1
255    1

��ͶӰ��roi���� m_roiMat.width ��
������ֵ   = 255 * kernelSize * m_roiMat.width
���Ե��ֵ = contrast / ����ֵ
TODO�������� INT_MAX == 2147483647 
�����������Ϊ��INT_MAX / (255 * kernelSize)
kernelSizeΪ2ʱ���������Ϊ4210752�������������

********************************************************/
int CSeeney_Caliper::sny_searchSingleEdge(const int * derHist, const int hisSize, const cv::Size roiSize,
										  const int maxNum, 
										  const int contstTH, 
										  const int polarity, 
										  const int kernelSize,
										  std::vector<sttSingleEdge> * dstEdges)
{
	if (maxNum>hisSize) {
		return -1;
	}
	
	//����ṹ��������������
	std::vector<convNode> vDerHist;
	for (int n = 0; n < hisSize;n++) {
		convNode temp;
		if (polarity==2) {
			//���Ƽ���ʱ��ȡ����ֵ
			if (derHist[n] >= 0) {
				temp.data = derHist[n];
			}
			else {
				temp.data = -derHist[n];
			}
		}
		else {
			temp.data = derHist[n];
		}
		temp.No	  = n;
		vDerHist.push_back(temp);
	}

	if (polarity==0) {
		//Ѱ����ֵ�е�����ǰmaxNum��
		//max_element
		partial_sort(vDerHist.begin(),vDerHist.begin()+maxNum,vDerHist.end(),compare4bigger);

	}
	else if (polarity == 1) {
		//Ѱ�Ҹ�ֵ�е�����ǰmaxNum��
		partial_sort(vDerHist.begin(),vDerHist.begin()+maxNum,vDerHist.end(),compare4smaller);

	}
	else if (polarity==2) {
		//Ѱ�Ҿ���ֵ����ǰmaxNum��
		partial_sort(vDerHist.begin(),vDerHist.begin()+maxNum,vDerHist.end(),compare4bigger);
	}

	//�����Ե�÷�
	float maxSore = 255 * kernelSize*roiSize.width;

	//������
	for (int i = 0; i < maxNum;i++) {
		sttSingleEdge tempEdge;	
		tempEdge.index = vDerHist[i].No;
		tempEdge.contrast = vDerHist[i].data;
		tempEdge.score = ( vDerHist[i].data / maxSore);
		tempEdge.x = 0;
		tempEdge.y = 0;
		dstEdges->push_back(tempEdge);
	}
	return 0;
}



