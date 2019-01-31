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
	delete[]m_prjHist;
	delete[]m_derHist;
	delete[]m_resEdge;
	destroyAllWindows();
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
	int errorCodeHead = EC_CALIPER;
	int resVal;

	//����ͼ��ʧ�ܻ��߶���ͼ��Ϊ�Ҷ�ͼ
	if ((m_srcMat.empty() == 1)||(m_srcMat.type() != CV_8UC1)){
		return errorCodeHead + EC_CLPR_IN_SRCMAT;
	}

	Rect2d m_oriROI;						//δ��ת��ԭʼͼ��roi
	
	//----------------------------------
	//1.����roi,������ת�õĴ�ͶӰͼ��
	if (m_enSttInputPara.useCaliperROI == 0) {
		//���ݿ����������ȷ��roi
		//���ݴ��У���ת������ʵ��ͶӰroi

		if (m_enSttInputPara.roiMode == 0) {
			//ԭ��ģʽ����roi���Ͻ�Ϊ����ת
			//roi��ֵ
			m_oriROI.x = m_enSttInputPara.originX;
			m_oriROI.y = m_enSttInputPara.originY;
			m_oriROI.width = m_enSttInputPara.width;
			m_oriROI.height = m_enSttInputPara.length;

			resVal=sny_createOriginROI(m_srcMat, m_oriROI, 
										   m_enSttInputPara.rotAngle, m_enSttInputPara.skewAngle, 
										   m_mskMat, m_roiMat, m_aAffineMatrix);
			if (resVal != 0)return resVal+errorCodeHead;
		}
		else if (m_enSttInputPara.roiMode==1) {
			//����ģʽ����roi����Ϊ����ת
			//����볤��Ϊδ���м�δ��ת�ĳ���
			//�˴���originX��originYΪ��ת��������
			m_oriROI.x = m_enSttInputPara.originX-m_enSttInputPara.width/2;
			m_oriROI.y = m_enSttInputPara.originY-m_enSttInputPara.length/2;
			m_oriROI.width = m_enSttInputPara.width;
			m_oriROI.height = m_enSttInputPara.length;

			resVal = sny_crtCentModeROI(m_srcMat, m_oriROI,
										m_enSttInputPara.rotAngle, m_enSttInputPara.skewAngle,
										m_mskMat, m_roiMat, m_aAffineMatrix);
			if (resVal != 0)return resVal + errorCodeHead;
		}
		else if (m_enSttInputPara.roiMode==2) {
			//����ģʽ�� ȷ���������roi����
			//����3�������ת�����нǶȣ�Ȼ�����ԭ��ģʽ����
			float rotAngle=0;
			float skewAngle=0;
			resVal = sny_crt3PointsModeROI(m_srcMat,
											cv::Point(m_enSttInputPara.originX,m_enSttInputPara.originY),
											cv::Point(m_enSttInputPara.X1,m_enSttInputPara.Y1),
											cv::Point(m_enSttInputPara.X2,m_enSttInputPara.Y2),
											m_mskMat, m_roiMat, m_aAffineMatrix);
			if (resVal != 0)return resVal + errorCodeHead;

		}


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
	resVal=sny_calcHoriProject(m_roiMat, m_mskMat, m_prjHist);
	if (resVal != 0)return resVal+errorCodeHead;
	if (m_enSttInputPara.displayProjection == 1)
	{
		//ͶӰ���ͼ������
		sny_showHorProjection(m_prjHist, prjSize, Size(m_roiMat.cols, m_roiMat.rows), m_dspMat);
	}

	//��ͶӰ���������
	m_kernelSize = m_enSttInputPara.filtHalfPixel;
	resVal=sny_convHoriProject(m_prjHist,prjSize,2,m_derHist);
	if (resVal != 0)return resVal + errorCodeHead;
	if (m_enSttInputPara.displayFiltered==1)
	{
		//����������
		resVal=sny_showConProjection(m_derHist, prjSize, Size(m_roiMat.cols, m_roiMat.rows), m_derMat);
		if (resVal != 0)return resVal + errorCodeHead;
	}

	m_resEdge = new sttCaliperOutput[m_enSttInputPara.maxLineNum];
	//Ѱ�ұ�Ե
	resVal=sny_searchSingleEdge(
		m_derHist, prjSize, Size(m_roiMat.cols, m_roiMat.rows),
		m_enSttInputPara.maxLineNum,
		m_enSttInputPara.contrastTH,
		m_enSttInputPara.edgePlrty,
		m_kernelSize,
		&m_nEdge,
		m_resEdge);
	
	if (resVal != 0)return resVal+errorCodeHead;

	//����ͶӰͼ���е�λ�ã�������ԭͼ�е�λ��	
	//��opencv�ľ���ת��3*3��ʽ
	cv::Mat transMat;
	sny_convertAffineMat(m_aAffineMatrix[1], transMat);

	for (int i = 0; i < m_enSttInputPara.maxLineNum; i++) {
		//�Աȶȴ�����ֵ
		if (m_resEdge[i].contrast > m_enSttInputPara.contrastTH) {
			cv::Point tempP;
			//2*3�ľ���
			double x = m_roiMat.cols / 2;
			double y = m_resEdge[i].index;
			//[x,y,1]�ľ���
			cv::Mat inputMat = (Mat_<double>(1, 3) << x, y, 1);
			cv::Mat outputMat = inputMat * transMat;
			cv::Point dstPt = cv::Point(outputMat.at<double>(0, 0), outputMat.at<double>(0, 1));
			m_resEdge[i].x = dstPt.x;
			m_resEdge[i].y = dstPt.y;

			if (m_enSttInputPara.displaySteps == 1) {
				//circle(m_derMat,Point(0,m_resEdge[i].index),5,Scalar(255,100,0),1,8,0);
				circle(m_roiMat, Point(m_roiMat.cols / 2, m_resEdge[i].index), 3, Scalar(0, 0, 255), 1, 8, 0);
				circle(m_srcMat, dstPt, 5, Scalar(255, 0, 255), 3, 8, 0);

			}
		}
	}
	

	if (m_enSttInputPara.displaySteps == 1) {
		//imshow("edge_in_prjMat", m_derMat);
		imshow("edge_in_roiMat", m_roiMat);
		imshow("edge_in_srcMat", m_srcMat);
		waitKey(0);
		destroyAllWindows();
	}

	return 0;
}

int CSeeney_Caliper::sny_getCaliperPara(const sttCaliperInput srcPara)
{
	
	//-----------------���ò���-----------------
	m_enSttInputPara.edgeModel = srcPara.edgeModel;				// 0��������Ե 1����Ե��
	m_enSttInputPara.edgePlrty = srcPara.edgePlrty;				// 0���ɰ����� 1���������� 2�����⼫��
	m_enSttInputPara.edgeWidth = srcPara.edgeWidth;				// [0,roi���] TODO����Ե������� 	

	m_enSttInputPara.contrastTH = srcPara.contrastTH;			// [0,255] TODO����ֵ������Ҫ����
	m_enSttInputPara.filtHalfPixel = srcPara.filtHalfPixel;		// ����ߴ��һ�룬�ߴ�=filtHalfPixel*2+1	   	
	m_enSttInputPara.maxLineNum = srcPara.maxLineNum;			// ����Ե��������


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

	//-----------------���Բ���-----------------
	m_enSttInputPara.displaySteps=srcPara.displaySteps;								//0:�رչ�����ʾ 1����ʾ�������̵�ͼ��
	m_enSttInputPara.writeLog=srcPara.writeLog;								//0:�رչ�����ʾ 1����ʾ�������̵�ͼ��

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

int CSeeney_Caliper::sny_setResult(const int num,sttCaliperOutput * resEdge)
{
	for (int i = 0; i < num;i++) {
		resEdge[i].contrast = m_resEdge[i].contrast;
		resEdge[i].index = m_resEdge[i].index;
		resEdge[i].score = m_resEdge[i].score;
		resEdge[i].x = m_resEdge[i].x;
		resEdge[i].y = m_resEdge[i].y;
	}
	return 0;
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
affine_matrix������affine�任�ľ���

-����ֵ:
ִ�гɹ���0
������룺

	EC_CLPR_CR_SKEWANGLE ���нǶȴ���,����Ϊ��90���Ҳ��ܳ�����180,
	EC_CLPR_CR_ROIOUT	 ROIԭʼλ�ó���,
	EC_CLPR_CR_SKEWOUT	 ���к�Χ����,
	EC_CLPR_CR_ROTOUT	��ת��Χ����

-��ע
���Ƚ��������Σ������Ͻ�Ϊԭ��[x,y]
  [x,y]--------
    /|\        |
   /a|b\       | 
      ---------
˳ʱ��aΪ���Ƕȣ���ʱ��bΪ���Ƕ�

��ת����˳ʱ��Ϊ����ˮƽ����Ϊ0��

********************************************************/
int CSeeney_Caliper::sny_createOriginROI(const cv::Mat srcMat, const cv::Rect2d oriROI, const float rotAngle, const float skewAngle,
									 cv::Mat &maskMat,cv::Mat &roiMat, cv::Mat * affine_matrix)
{
	//���нǶȴ��󣬲���Ϊ���½Ƕ�
	if (skewAngle == 90 || skewAngle == -90 || skewAngle > 180 || skewAngle < -180) {
		return EC_CLPR_CR_SKEWANGLE;
	}

	
	//��ԭͼͬ���ߴ��rect���ж��Ƿ������
	cv::Rect srcMatRect;
	srcMatRect.x = 0;
	srcMatRect.y = 0;
	srcMatRect.width  = srcMat.cols;
	srcMatRect.height = srcMat.rows;

	cv::Point tmpOutSide;
	tmpOutSide.x = oriROI.x;
	tmpOutSide.y = oriROI.y;

	//ROIԭʼλ�ó���
	if (srcMatRect.contains(tmpOutSide) != 1) {
		return EC_CLPR_CR_ROIOUT;
	}
	tmpOutSide.x += oriROI.width;
	tmpOutSide.y += oriROI.height;
	if (srcMatRect.contains(tmpOutSide) != 1) {
		return EC_CLPR_CR_ROIOUT;
	}

	cv::Mat displayMat;	    //��ʾ��
	srcMat.copyTo(displayMat);
	Size skewedRoiSize;
 
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

	//----------------1.���д���------------------ 
	//����skewAngle����roi���к�Ķ���	
	//���� = �Ƕ� * Pi / 180;
	float skewRadian = skewAngle * CV_PI / 180;
	float rotaRadian = rotAngle * CV_PI / 180;
	float dx;
	float py;

	//������к��ƽ�ƾ���
	//dx,����ƽ�ƾ��룬py������������ϵ��
	//�Ƕ��ڣ�90,190������-180��-90�����ı�����Ҫ��ˮƽ���񣬹�py=-1��Ϊy����ϵ��
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
	pts.push_back(Point(oriROI.x+oriROI.width-1, oriROI.y));
	pts.push_back(Point(oriROI.x+oriROI.width-1+dx, oriROI.y+py*oriROI.height-1));
	pts.push_back(Point(oriROI.x+dx, oriROI.y+py*oriROI.height-1));
	contour.push_back(pts);

	//�ж��Ƿ����
	for (int n = 0; n < 4;n++) {
		if (srcMatRect.contains(pts[n])!=1) {
			return EC_CLPR_CR_SKEWOUT;
		}
	}

	//�������ͶӰͼ��ľ��γߴ�׼��roi
	cv::Rect rotbackROI= cv::boundingRect(cv::Mat(pts).reshape(2));

	//----------------2.��ת����------------------ 
	//�����ĸ�������ת֮���������
	for (int i = 0; i < 4; i++) {
		cv::Point2d temp1;
		//������ת�����꺯������ԭͼ����ϵ
		//����˳ʱ��Ϊ������������Ӹ���
		sny_calRotPosition(pts[0], pts[i], temp1, (-1*rotAngle));
		rpts.push_back(temp1);
		
		//Ϊ����ͶӰͼ�����4���������꣬������ͶӰͼ������ϵ
		cv::Point temp2;
		temp2.x = pts[i].x - rotbackROI.x;
		temp2.y = pts[i].y - rotbackROI.y;
		roi_pts.push_back(temp2);
	}
	rot_contour.push_back(rpts);
	roi_contour.push_back(roi_pts);

	for (int n = 0; n < 4; n++) {
		if (srcMatRect.contains(rpts[n]) != 1) {
			return EC_CLPR_CR_ROTOUT;
		}
	}

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
	affine_matrix[CA_ORI2PRJ]= cv::getAffineTransform(src_pt, dst_pt);
	//const cv::Mat affine_matrix = cv::getAffineTransform(src_pt, dst_pt);
	cv::warpAffine(roiMat, roiMat, affine_matrix[0], rotbackROI.size());
	cv::warpAffine(maskMat, maskMat, affine_matrix[0], rotbackROI.size());


	const cv::Point2f src_pt2[] = { cv::Point2f(roi_pts[0].x, roi_pts[0].y),
							   cv::Point2f(roi_pts[1].x, roi_pts[1].y),
							   cv::Point2f(roi_pts[2].x, roi_pts[2].y) };
	// ��������
	const cv::Point2f dst_pt2[] = { cv::Point2f(rpts[0].x, rpts[0].y),
								   cv::Point2f(rpts[1].x, rpts[1].y),
								   cv::Point2f(rpts[2].x, rpts[2].y) };
	affine_matrix[CA_PRJ2ORI]= cv::getAffineTransform(src_pt2, dst_pt2);


	
	if (m_enSttInputPara.displaySteps == 1) {
		//�軭���
		//���к����״
		drawContours(displayMat, contour, 0, Scalar::all(255),1);
		//��ת�����״
		drawContours(displayMat, rot_contour, 0, cvScalar(0, 255, 255), 2);
			//----------------------------------//ԭ��
		circle(displayMat, Point(oriROI.x, oriROI.y), 3, cvScalar(0, 0, 255), 8, 0);
		//����roi
		rectangle(displayMat, dstRoiRect, cvScalar(255, 0, 255), 1, 8, 0);
		//ԭʼroi
		rectangle(displayMat, oriROI, cvScalar(255, 255, 255), 1, 8, 0);
		imshow("display", displayMat);
		imshow("roiMat", roiMat);
		imshow("dstMask", maskMat);
	}

	return 0;
}

//����˵����ʽ
/********************************************************
-������:sny_crtCentModeROI
-��������
��ԭ��ģʽ����roi��
��һ�����к����ϽǶ���ģʽһ��
�ڶ�����תģʽ������

��ԭ��ģʽ��ͬ����
ԭroi�����ĵ㣬�ڴ��к���ת�ж��̶���

���нǶ���(90,180]��(-180,-90)������ʱ������ԭ��������



-�������:


-�������:


-����ֵ:
ִ�гɹ���0
������룺

	EC_CLPR_CR_SKEWANGLE ���нǶȴ���,����Ϊ��90���Ҳ��ܳ�����180,
	EC_CLPR_CR_ROIOUT	 ROIԭʼλ�ó���,
	EC_CLPR_CR_SKEWOUT	 ���к�Χ����,
	EC_CLPR_CR_ROTOUT	��ת��Χ����


-��ע
���Ƚ��������Σ������Ͻ�Ϊԭ��[x,y]
  [x,y]--------
	/|\        |
   /a|b\       |
	  ---------
˳ʱ��aΪ���Ƕȣ���ʱ��bΪ���Ƕ�

��ת����˳ʱ��Ϊ����ˮƽ����Ϊ0��

oriROI��x��y����Ϊδ��תǰ���꣬Ӧ�ں����ⲿ�޸ĺã�������

********************************************************/
int CSeeney_Caliper::sny_crtCentModeROI(const cv::Mat srcMat, const cv::Rect2d oriROI, const float rotAngle, const float skewAngle, cv::Mat & maskMat, cv::Mat & roiMat, cv::Mat * affine_matrix)
{
	//���нǶȴ��󣬲���Ϊ���½Ƕ�
	if (skewAngle == 90 || skewAngle == -90 || skewAngle > 180 || skewAngle < -180) {
		return EC_CLPR_CR_SKEWANGLE;
	}


	//��ԭͼͬ���ߴ��rect���ж��Ƿ������
	cv::Rect srcMatRect;
	srcMatRect.x = 0;
	srcMatRect.y = 0;
	srcMatRect.width = srcMat.cols;
	srcMatRect.height = srcMat.rows;

	//roi���ģ���ת��
	cv::Point centerPoint;
	cv::Point tmpOutSide;
	tmpOutSide.x = oriROI.x;
	tmpOutSide.y = oriROI.y;

	//ROIԭʼλ�ó���
	if (srcMatRect.contains(tmpOutSide) != 1) {
		return EC_CLPR_CR_ROIOUT;
	}
	tmpOutSide.x += oriROI.width;
	tmpOutSide.y += oriROI.height;
	if (srcMatRect.contains(tmpOutSide) != 1) {
		return EC_CLPR_CR_ROIOUT;
	}
	centerPoint.x = oriROI.x + oriROI.width / 2;
	centerPoint.y = oriROI.y + oriROI.height / 2;

	cv::Mat displayMat;	    //��ʾ��
	srcMat.copyTo(displayMat);
	Size skewedRoiSize;

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

	//----------------1.���д���------------------ 
	//����skewAngle����roi���к�Ķ���	
	//���нǶ�Ӧ��(-90,90)
	//���� = �Ƕ� * Pi / 180;
	float skewRadian = skewAngle * CV_PI / 180;
	float rotaRadian = rotAngle * CV_PI / 180;
	float dx;
	float px;
	//������к��ƽ�ƾ���
	//��ԭ����У�ƽ�ƾ���Ϊ
	if (skewAngle > -90 && skewAngle < 90) {
		dx = oriROI.height * tan(skewRadian);
		px = 1;
	}
	else if ((skewAngle > 90 && skewAngle <= 180) || (skewAngle < -90 && skewAngle >= -180)) {
		dx = oriROI.height * tan(CV_PI - skewRadian);
		px = -1;
	}

	//�޸�4����������
	pts.push_back(Point(oriROI.x + px*(dx/2)				  , oriROI.y));
	pts.push_back(Point(oriROI.x + px*(dx/2) + oriROI.width-1 , oriROI.y));
	pts.push_back(Point(oriROI.x - px*(dx/2) + oriROI.width-1 , oriROI.y + oriROI.height - 1));
	pts.push_back(Point(oriROI.x - px*(dx/2)				  , oriROI.y + oriROI.height - 1));
	contour.push_back(pts);

	for (int n = 0; n < 4; n++) {
		if (srcMatRect.contains(pts[n]) != 1) {
			return EC_CLPR_CR_SKEWOUT;
		}
	}
	//�������ͶӰͼ��ľ��γߴ�׼��roi
	cv::Rect rotbackROI = cv::boundingRect(cv::Mat(pts).reshape(2));

	//----------------2.��ת����------------------ 
	//�����ĸ�������ת֮���������
	for (int i = 0; i < 4; i++) {
		cv::Point2d temp1;
		//������ת�����꺯������ԭͼ����ϵ
		//
		//����˳ʱ��Ϊ������������Ӹ���
		sny_calRotPosition(centerPoint, pts[i], temp1, (-1 * rotAngle));
		rpts.push_back(temp1);

		//Ϊ����ͶӰͼ�����4���������꣬������ͶӰͼ������ϵ
		cv::Point temp2;
		temp2.x = pts[i].x - rotbackROI.x;
                        		temp2.y = pts[i].y - rotbackROI.y;
		roi_pts.push_back(temp2);
	}
	rot_contour.push_back(rpts);
	roi_contour.push_back(roi_pts);

	//�ж��Ƿ����
	for (int n = 0; n < 4; n++) {
		if (srcMatRect.contains(rpts[n]) != 1) {
			return EC_CLPR_CR_ROTOUT;
		}
	}

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
	maskMat = Mat::zeros(dstRoiRect.size(), CV_8UC1);
	drawContours(maskMat, dst_contour, 0, Scalar::all(255), -1);

	//�������յ�roiͼ����ͼ������ϵ
	cv::Mat temp = srcMat(dstRoiRect);
	temp.copyTo(roiMat, maskMat);

	//----------------4.ͼ�����------------------ 
	//Ϊ��һ��ͶӰ���������ɷ���ͶӰ���������ͼ��
	//����roiMat��maskMat������תrotAngle
	// �任ǰ����
	const cv::Point2f src_pt[] = { cv::Point2f(dst_pts[0].x, dst_pts[0].y),
								   cv::Point2f(dst_pts[1].x, dst_pts[1].y),
								   cv::Point2f(dst_pts[2].x, dst_pts[2].y) };
	// ��������
	const cv::Point2f dst_pt[] = { cv::Point2f(roi_pts[0].x, roi_pts[0].y),
								   cv::Point2f(roi_pts[1].x, roi_pts[1].y),
								   cv::Point2f(roi_pts[2].x, roi_pts[2].y) };
	affine_matrix[CA_ORI2PRJ] = cv::getAffineTransform(src_pt, dst_pt);
	cv::warpAffine(roiMat, roiMat, affine_matrix[0], rotbackROI.size());
	cv::warpAffine(maskMat, maskMat, affine_matrix[0], rotbackROI.size());


	const cv::Point2f src_pt2[] = { cv::Point2f(roi_pts[0].x, roi_pts[0].y),
							   cv::Point2f(roi_pts[1].x, roi_pts[1].y),
							   cv::Point2f(roi_pts[2].x, roi_pts[2].y) };
	// ��������
	const cv::Point2f dst_pt2[] = { cv::Point2f(rpts[0].x, rpts[0].y),
								   cv::Point2f(rpts[1].x, rpts[1].y),
								   cv::Point2f(rpts[2].x, rpts[2].y) };
	affine_matrix[CA_PRJ2ORI] = cv::getAffineTransform(src_pt2, dst_pt2);

	if (m_enSttInputPara.displaySteps == 1) {
		//�軭���

		rectangle(displayMat, dstRoiRect, cvScalar(255, 0, 255), 1, 8, 0);
		rectangle(displayMat, oriROI, cvScalar(255, 0, 255), 1, 8, 0);
		drawContours(displayMat, contour , 0, cvScalar(255, 255, 255), 1);
		drawContours(displayMat, rot_contour, 0, cvScalar(255, 255, 255), 1);
		drawContours(displayMat, roi_contour, 0, cvScalar(255, 255, 255), 1);
		circle(displayMat, centerPoint, 3, cvScalar(255, 0, 255), 8, 0);

		imshow("display", displayMat);
		imshow("roiMat", roiMat);
		imshow("dstMask", maskMat);
	}

	return 0;
}

/********************************************************
-������sny_crt3PointsModeROI
-��������
3��ģʽ������roi

-�������:


-�������:
const cv::Mat srcMat������ͼ��

ǰ�˸�����3����
const cv::Point srcP1, 
const cv::Point srcP2, 
const cv::Point srcP3,


-����ֵ:
ͬԭ�㼰����ģʽ
 cv::Mat & maskMat, cv::Mat & roiMat, cv::Mat * affine_matrix

-��ע
������
3���㣬���ص�
EC_CLPR_CR_SAMEPOSITION = 9,

********************************************************/
int CSeeney_Caliper::sny_crt3PointsModeROI(const cv::Mat srcMat,
										   const cv::Point2d pt1, const cv::Point2d pt2, const cv::Point2d pt3, 
										   cv::Mat & maskMat, cv::Mat & roiMat, cv::Mat * affine_matrix)
{
	cv::Point2d pt4;
	
	//��ԭͼͬ���ߴ��rect���ж��Ƿ������
	cv::Rect srcMatRect;
	srcMatRect.x = 0;
	srcMatRect.y = 0;
	srcMatRect.width = srcMat.cols;
	srcMatRect.height = srcMat.rows;	

	//��ʾ��
	cv::Mat displayMat;
	m_srcMat.copyTo(displayMat);

	//��3�����ɵ�ԭͼ�е�roi��ԭͼ����ϵ
	vector<vector<Point>> roiImg_contour;
	vector<Point> roiImg_pts;

	//��3�����ɵ�ԭͼ�е�roi��roi����ϵ��������
	vector<vector<Point>> roiRectImg_contour;
	vector<Point> roiRectImg_pts;

	//ͶӰͼ���е�������ͶӰ����ϵ
	vector<vector<Point>> prjImg_contour;
	vector<Point> prjImg_pts;

	//�����ж� 3�����Ƿ����ص�
	if ((sny_samePoints2d(pt1, pt2) == 1) ||
		(sny_samePoints2d(pt1, pt3) == 1) ||
		(sny_samePoints2d(pt2, pt3) == 1)) {
		return EC_CLPR_CR_SAMEPOSITION;
	}

	//����3��������ı��Σ�����ת��x�������
	sny_3ptsCreateCountors(pt1,pt2,pt3,roiImg_pts,prjImg_pts);
	roiImg_contour.push_back(roiImg_pts);
	//�ж��Ƿ����
	for (int n = 0; n < 4; n++) {
		if (srcMatRect.contains(roiImg_pts[n]) != 1) {
			return EC_CLPR_CR_ROTOUT;
		}
	}
	//�������յ�maskMat����ͼ������ϵ
	cv::Rect roiImgRect = cv::boundingRect(cv::Mat(roiImg_pts).reshape(2));

	//�������ͶӰͼ��ľ��γߴ�׼��roi
	cv::Rect prjImgRect = cv::boundingRect(cv::Mat(prjImg_pts).reshape(2));
	//ԭͼroi�����Ƶ�rect����

	//��prjImgRect�������޸���prjImg����ϵ
	//
	for (int n = 0; n < 4; n++) {
		prjImg_pts[n].x -= prjImgRect.x;
		prjImg_pts[n].y -= prjImgRect.y;
		cv::Point temp;
		temp.x = roiImg_pts[n].x-roiImgRect.x;
		temp.y = roiImg_pts[n].y-roiImgRect.y;
		roiRectImg_pts.push_back(temp);
	}
	roiRectImg_contour.push_back(roiRectImg_pts);
	prjImg_contour.push_back(prjImg_pts);

	maskMat = Mat::zeros(roiImgRect.size(), CV_8UC1);
	drawContours(maskMat, roiRectImg_contour, 0, Scalar::all(255), -1);

	//�������յ�roiͼ����ͼ������ϵ
	cv::Mat temp = srcMat(roiImgRect);
	temp.copyTo(roiMat, maskMat);
	
	//----------------4.ͼ�����------------------ 
	//Ϊ��һ��ͶӰ���������ɷ���ͶӰ���������ͼ��
	//����roiMat��maskMat������תrotAngle
	// �任ǰ����
	const cv::Point2f src_pt[] = { cv::Point2f(roiRectImg_pts[0].x, roiRectImg_pts[0].y),
								   cv::Point2f(roiRectImg_pts[1].x, roiRectImg_pts[1].y),
								   cv::Point2f(roiRectImg_pts[2].x, roiRectImg_pts[2].y) };
	// ��������
	const cv::Point2f dst_pt[] = { cv::Point2f(prjImg_pts[0].x, prjImg_pts[0].y),
								   cv::Point2f(prjImg_pts[1].x, prjImg_pts[1].y),
								   cv::Point2f(prjImg_pts[2].x, prjImg_pts[2].y) };
	affine_matrix[CA_ORI2PRJ] = cv::getAffineTransform(src_pt, dst_pt);
	cv::warpAffine(roiMat, roiMat, affine_matrix[0], prjImgRect.size());
	cv::warpAffine(maskMat, maskMat, affine_matrix[0], prjImgRect.size());

	
	const cv::Point2f src_pt2[] = { cv::Point2f(prjImg_pts[0].x, prjImg_pts[0].y),
									cv::Point2f(prjImg_pts[1].x, prjImg_pts[1].y),
									cv::Point2f(prjImg_pts[2].x, prjImg_pts[2].y) };
	// ��������
	const cv::Point2f dst_pt2[] = { cv::Point2f(roiImg_pts[0].x, roiImg_pts[0].y),
								   cv::Point2f(roiImg_pts[1].x, roiImg_pts[1].y),
								   cv::Point2f(roiImg_pts[2].x, roiImg_pts[2].y) };
	affine_matrix[CA_PRJ2ORI] = cv::getAffineTransform(src_pt2, dst_pt2);

	if (m_enSttInputPara.displaySteps == 1) {
		//�軭���
		rectangle(displayMat, prjImgRect, cvScalar(255, 0, 255), 1, 8, 0);
		rectangle(displayMat, roiImgRect, cvScalar(255, 0, 255), 1, 8, 0);
		drawContours(displayMat, prjImg_contour, 0, cvScalar(255, 255, 255), 1);
		drawContours(displayMat, roiImg_contour, 0, cvScalar(255, 255, 255), 1);
		circle(displayMat, pt1, 3, cvScalar(255, 0, 255), 8, 0);
		imshow("temp", temp);
		imshow("maskMat", maskMat);
		imshow("roiMat", roiMat);
		imshow("display", displayMat);
	}

	//waitKey(0);
	return 0;
}



/********************************************************
-������: sny_calcRotSkew
-��������
3��ģʽ���ô˺��������ݸ�����3�㣬������еĽǶȺ���ת�ĽǶ�

-�������:
srcP1,srcP2,srcP3
���������꣬srcP1�൱��ԭ��ģʽ�е�ԭ��

-�������:
rotAngle����ת�Ƕ�
skewAngle�����нǶ�
������ԭ��ģʽ������ģʽһ��
�Ե�һ����Ϊ���Ͻ�ԭ��[x,y]
  [x,y]--------
	/|\        |
   /a|b\       |
	  ---------
˳ʱ��aΪ���Ƕȣ���ʱ��bΪ���Ƕ�

��ת����˳ʱ��Ϊ��,ˮƽΪ0��
-����ֵ:
ִ�гɹ���0

-��ע
���ж��Ƿ���磬����һ����������



!!!!!NG!!!!!

********************************************************/

int CSeeney_Caliper::sny_calcRotSkew(const cv::Point srcP1, const cv::Point srcP2, const cv::Point srcP3, 
									 cv::Rect &oriROI,float *rotAngle, float *skewAngle)
{

	cv::Mat displayMat;
	m_srcMat.copyTo(displayMat);
	circle(displayMat, srcP1, 5, Scalar(255, 100, 0), -1, 8, 0);
	circle(displayMat, srcP2, 5, Scalar(255, 100, 0), -1, 8, 0);
	circle(displayMat, srcP3, 5, Scalar(255, 100, 0), -1, 8, 0);
	//��һ����͵ڶ��������ֱ�ߣ���ˮƽ��˳ʱ��Ƕȡ�
	float dst12;
	dst12 = (srcP1.x - srcP2.x)*(srcP1.x - srcP2.x) + (srcP1.y-srcP2.y)*(srcP1.y - srcP2.y);
	
	if (dst12 == 0) {
		//����Ϊ0�������ظ�
		return EC_CLPR_CR_SAMEPOSITION;
	}

	//����x���ϵĵ�
	cv::Point2f P2rot2X	;
	P2rot2X.y = srcP1.y;
	P2rot2X.x = srcP1.x + sqrt(dst12);

	circle(displayMat, P2rot2X, 5, Scalar(180	, 100, 0), -1, 8, 0);
	line(displayMat, srcP1, P2rot2X, Scalar(255, 0, 0), 1, 8, 0);
	
	float dst2temp=(srcP2.x-P2rot2X.x)*(srcP2.x - P2rot2X.x) + (srcP2.y-P2rot2X.y)*(srcP2.y - P2rot2X.y);


	if (srcP1.y<=srcP2.y) {
		//0-180��
		//*rotAngle = acos((dst12 + dst12 - dst2temp) / (2 * dst12));
		*rotAngle = (acos((dst12 + dst12 - dst2temp) / (2 * dst12)))*180/CV_PI;
	}
	else if (srcP1.y > srcP2.y){
		*rotAngle = -(acos((dst12 + dst12 - dst2temp) / (2 * dst12)) * 180 / CV_PI);
		//*rotAngle = -acos((dst12 + dst12 - dst2temp) / (2 * dst12));
	}

	//������нǶ�
	//P1��P2Ϊԭ�㣬��ת-90�ȣ���ת��������ΪskP,����skP��P3�ļн�
	//P2��P3��ʱ��תrotAngle��
	cv::Point2d skP;
	sny_calRotPosition(srcP1, srcP3, skP, *rotAngle);
	//����y���ϵĵ�,x=P2rot2X.x,y=skP.y
	cv::Point2f tempY;
	tempY.x = P2rot2X.x;
	tempY.y = skP.y;

	float dst_a = (skP.x - tempY.x)*(skP.x - tempY.x) + (skP.y - tempY.y)*(skP.y - tempY.y);
	float dst_b = ((P2rot2X.x-skP.x)*(P2rot2X.x-skP.x))+((P2rot2X.y-skP.y)*(P2rot2X.y-skP.y));
	float dst_c = ((P2rot2X.x-tempY.x)*(P2rot2X.x-tempY.x))+((P2rot2X.y-tempY.y)*(P2rot2X.y-tempY.y));
	*skewAngle = acos((dst_b + dst_c - dst_a) / (2 * sqrt(dst_b)*sqrt(dst_c))) * 180 / CV_PI;

	//������
	line(displayMat, P2rot2X, tempY, Scalar(255, 0, 0), 1, 8, 0);
	circle(displayMat, tempY, 5, Scalar(180, 100, 0), -1, 8, 0);
	line(displayMat,srcP1,skP,Scalar(150,0,0),1,8,0);
	circle(displayMat, skP, 5, Scalar(180, 100, 0), -1, 8, 0);


	imshow("111",displayMat);
	waitKey(0);
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
int CSeeney_Caliper::sny_calRotPosition(const cv::Point2d center, const cv::Point2d srcPoint, cv::Point2d & dstPoint, float rotAngle)
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
		{ -1*std::sin(rotaRadian),   std::cos(rotaRadian),     0},
		{                       0,                      0,     1}
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
EC_CLPR_CP_WIDTHOVER = ����ߴ����Σ��

-��ע
dstArry����Ϊint�ͣ���ͶӰ�����������ֵΪ 2147483647 / 255 = 8,421,504
ͼ���Ȳ��ܳ���������
������˵�ߴ粻��ﵽ�����ֵ

********************************************************/
int CSeeney_Caliper::sny_calcHoriProject(const cv::Mat srcMat, const cv::Mat mskMat, int * dstArry)
{

	if (srcMat.cols>=8421504) {
		return 	EC_CLPR_CALP_WIDTHOVER;
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

	if ((convSize <= 0) || (fsize > srcSize)) {
		delete[]kernel;
	return  EC_CLPR_CONP_KERNEL;
	}

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

	if (m_enSttInputPara.displaySteps == 1) {
		imshow("projectionMat", dstMat);
	}


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
	
	if (m_enSttInputPara.displaySteps == 1) {
		imshow(" derivativeMat", dstMat);
	}

	return 0;
}

/********************************************************
-������: sny_samePoints
-��������
�ж�2��������Ƿ�Ϊ��ͬ��

-�������:
cv::Point pt1
cv::Point pt2

-�������:

-����ֵ:
��ͬ��
return 1 
�����㲻��ͬ
return 0 

-��ע

********************************************************/
int CSeeney_Caliper::sny_samePoints2d(const cv::Point2d pt1, const cv::Point2d pt2)
{
	if ((pt1.x == pt2.x) && (pt1.y == pt2.y)) {
		return 1;
	}
	else {
		return 0;
	}
}

/********************************************************
-������:sny_Parallelogram
-��������
���ݸ�����3���㣬����ƽ���ı��ε�4���������

-�������:
cv::Point pt1��cv::Point pt2��cv::Point pt3

-�������:
dstPt����4��������

-����ֵ:
����ɹ� return 0

-��ע
pt1��pt2����ɵ�һ����A
pt2��pt3����ɵڶ�����B
pt3��pt4��ɵı�ƽ����A
pt1��pt4��ɵı�ƽ����B

********************************************************/
int CSeeney_Caliper::sny_3ptsCreateCountors(const cv::Point2d pt1, const cv::Point2d pt2, const cv::Point2d pt3,
		vector<Point> &roiImgPts, vector<Point> &roiPrjPts)
{
	float rotAngle;
	cv::Point2d dstPt;
	cv::Mat displayMat;
	m_srcMat.copyTo(displayMat);
	
	//��һ����͵ڶ��������ֱ�ߣ���ˮƽ��˳ʱ��Ƕȡ�
	double dst12;
	dst12 = (pt1.x - pt2.x)*(pt1.x - pt2.x) + (pt1.y - pt2.y)*(pt1.y - pt2.y);

	//����x���ϵĵ�
	cv::Point2d pt2_to_X;
	pt2_to_X.y = pt1.y;
	pt2_to_X.x = pt1.x + sqrt(dst12);
	double dst_pt2_toX= (pt2.x - pt2_to_X.x)*(pt2.x - pt2_to_X.x) + (pt2.y - pt2_to_X.y)*(pt2.y - pt2_to_X.y);

	if (pt1.y <= pt2.y) {
		//0-180��
		rotAngle =	 acos((float)((dst12 + dst12 - dst_pt2_toX) / (2 * dst12))) * 180 / CV_PI;
	}
	else if (pt1.y > pt2.y) {
		rotAngle =	 acos((float)((dst12 + dst12 - dst_pt2_toX) / (2 * dst12))) * 180 / CV_PI;
		rotAngle =	-rotAngle;
	}

	//pt3����תrotAngle��
	cv::Point2d rotPt3;
	sny_calRotPosition(pt1, pt3, rotPt3, rotAngle);

	//����rotPt3��pt2_to_X�����ֵ
	double dx = pt2_to_X.x - rotPt3.x;
	double dy = pt2_to_X.y - rotPt3.y;
	
	//���������ֵ����4���͵�4���㣬�õ�Ϊ��ת��ĵ�
	cv::Point2d tempPt4;
	tempPt4.x = pt1.x - dx;
	tempPt4.y = pt1.y - dy;

	//��ת��ʵ�ʵ�λ�ã�����ת-rotAngle��
	sny_calRotPosition(pt1, tempPt4, dstPt, -rotAngle);
	
	roiImgPts.push_back(pt1);
	roiImgPts.push_back(pt2);
	roiImgPts.push_back(pt3);
	roiImgPts.push_back(dstPt);

	roiPrjPts.push_back(pt1);
	roiPrjPts.push_back(pt2_to_X);
	roiPrjPts.push_back(rotPt3);
	roiPrjPts.push_back(tempPt4);

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
	float score;			//��Ե�÷�
	float contrast;			//�Աȶ�

}sttCaliperOutput;

-����ֵ:
���ؼ�⵽�ı�Ե����
EC_CLPR_SE_MAXNUM ���õ�Ѱ������������

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

float contrast
�Աȶȼ��㷽��
������ֵ/kernelSize * roiSize.width

score
�÷ּ��㷽����
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

��Ե��ʵ��ͼ���е���������һ�������м���

********************************************************/
int CSeeney_Caliper::sny_searchSingleEdge(const int * derHist, const int hisSize, const cv::Size roiSize,
										  const int maxNum, 
										  const int contstTH, 
										  const int polarity, 
										  const int kernelSize,
										  int * nEdge,
										  sttCaliperOutput * dstEdges)
{
	if (maxNum>hisSize) {
		return EC_CLPR_SE_MAXNUM;
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
	float norm = kernelSize * roiSize.width;

	//������
	
	int cnt = 0;
	for (int i = 0; i < maxNum; i++) {
		int tempcontrast = (int)(vDerHist[i].data / norm);
		if (tempcontrast >= contstTH) {
			dstEdges[i].index = vDerHist[i].No;
			dstEdges[i].contrast = vDerHist[i].data / norm;				//����kernelSize����һ����0-255֮��
			dstEdges[i].score = (vDerHist[i].data / maxSore);
			dstEdges[i].x = 0;											//��ʼ��Ϊ0������������һ���������
			dstEdges[i].y = 0;
			cnt++;
		}
		else {
			dstEdges[i].index = 0;
			dstEdges[i].contrast = 0;
			dstEdges[i].score = 0;
			dstEdges[i].x = 0;
			dstEdges[i].y = 0;
		}
	}
	*nEdge = cnt;
	return 0;
}

/********************************************************
-������:
-��������
��opencv�е�affine�任����ת��Ϊ3*3�ı�׼affine����

-�������:
opencvAffineMat:opencv��get matrix�Ⱥ�����õ�affine���󣬾���Ϊ2*3�ߴ�

-�������:
dstMat����׼3*3����

-����ֵ:
dstMat

-��ע

********************************************************/
int CSeeney_Caliper::sny_convertAffineMat(const cv::Mat opencvAffineMat, cv::Mat & dstMat)
{
	//3*3�ľ���
	dstMat = Mat::eye(3, 3, CV_64FC1);

	dstMat.at<double>(0, 0) = opencvAffineMat.at<double>(0, 0);
	dstMat.at<double>(0, 1) = opencvAffineMat.at<double>(1, 0);
	dstMat.at<double>(1, 0) = opencvAffineMat.at<double>(0, 1);
	dstMat.at<double>(1, 1) = opencvAffineMat.at<double>(1, 1);
	dstMat.at<double>(2, 0) = opencvAffineMat.at<double>(0, 2);
	dstMat.at<double>(2, 1) = opencvAffineMat.at<double>(1, 2);

	return 0;
}



