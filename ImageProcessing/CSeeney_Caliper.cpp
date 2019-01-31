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

//函数说明格式
/********************************************************
-函数名:
-功能描述


-输入参数:


-输出参数:


-返回值:


-备注

********************************************************/


//-----------------卡尺主函数-----------------
int CSeeney_Caliper::CaliperMain()
{ 
	//0.初始化，由getCaliperPara 获取参数，由getInputImg获取图像
	int errorCodeHead = EC_CALIPER;
	int resVal;

	//读入图像失败或者读入图像不为灰度图
	if ((m_srcMat.empty() == 1)||(m_srcMat.type() != CV_8UC1)){
		return errorCodeHead + EC_CLPR_IN_SRCMAT;
	}

	Rect2d m_oriROI;						//未旋转的原始图像roi
	
	//----------------------------------
	//1.建立roi,生成旋转好的待投影图像
	if (m_enSttInputPara.useCaliperROI == 0) {
		//根据卡尺输入参数确定roi
		//根据错切，旋转，生成实际投影roi

		if (m_enSttInputPara.roiMode == 0) {
			//原点模式，以roi左上角为轴旋转
			//roi赋值
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
			//中心模式，以roi中心为轴旋转
			//宽度与长度为未错切及未旋转的长宽
			//此处的originX及originY为旋转中心坐标
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
			//三点模式， 确定三点决定roi区域
			//根据3点计算旋转及错切角度，然后调用原点模式函数
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
		//在原图中寻找边缘
		m_roiMat = m_srcMat;
		m_mskMat = m_srcMat;
	}

	//----------------------------------
	//2.进行投影处理
	int prjSize = m_roiMat.rows;
	m_prjHist = new int[prjSize];
	m_derHist = new int[prjSize];
	memset(m_prjHist, 0, sizeof(int)*prjSize);
	memset(m_derHist, 0, sizeof(int)*prjSize);

	//计算投影
	resVal=sny_calcHoriProject(m_roiMat, m_mskMat, m_prjHist);
	if (resVal != 0)return resVal+errorCodeHead;
	if (m_enSttInputPara.displayProjection == 1)
	{
		//投影结果图像生成
		sny_showHorProjection(m_prjHist, prjSize, Size(m_roiMat.cols, m_roiMat.rows), m_dspMat);
	}

	//对投影结果计算卷积
	m_kernelSize = m_enSttInputPara.filtHalfPixel;
	resVal=sny_convHoriProject(m_prjHist,prjSize,2,m_derHist);
	if (resVal != 0)return resVal + errorCodeHead;
	if (m_enSttInputPara.displayFiltered==1)
	{
		//卷积结果生成
		resVal=sny_showConProjection(m_derHist, prjSize, Size(m_roiMat.cols, m_roiMat.rows), m_derMat);
		if (resVal != 0)return resVal + errorCodeHead;
	}

	m_resEdge = new sttCaliperOutput[m_enSttInputPara.maxLineNum];
	//寻找边缘
	resVal=sny_searchSingleEdge(
		m_derHist, prjSize, Size(m_roiMat.cols, m_roiMat.rows),
		m_enSttInputPara.maxLineNum,
		m_enSttInputPara.contrastTH,
		m_enSttInputPara.edgePlrty,
		m_kernelSize,
		&m_nEdge,
		m_resEdge);
	
	if (resVal != 0)return resVal+errorCodeHead;

	//根据投影图像中的位置，计算在原图中的位置	
	//把opencv的矩阵转换3*3格式
	cv::Mat transMat;
	sny_convertAffineMat(m_aAffineMatrix[1], transMat);

	for (int i = 0; i < m_enSttInputPara.maxLineNum; i++) {
		//对比度大于阈值
		if (m_resEdge[i].contrast > m_enSttInputPara.contrastTH) {
			cv::Point tempP;
			//2*3的矩阵
			double x = m_roiMat.cols / 2;
			double y = m_resEdge[i].index;
			//[x,y,1]的矩阵
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
	
	//-----------------设置部分-----------------
	m_enSttInputPara.edgeModel = srcPara.edgeModel;				// 0：单个边缘 1：边缘对
	m_enSttInputPara.edgePlrty = srcPara.edgePlrty;				// 0：由暗到明 1：由明到暗 2：任意极性
	m_enSttInputPara.edgeWidth = srcPara.edgeWidth;				// [0,roi宽度] TODO：边缘宽度上限 	

	m_enSttInputPara.contrastTH = srcPara.contrastTH;			// [0,255] TODO：阈值上限需要调试
	m_enSttInputPara.filtHalfPixel = srcPara.filtHalfPixel;		// 卷积尺寸的一半，尺寸=filtHalfPixel*2+1	   	
	m_enSttInputPara.maxLineNum = srcPara.maxLineNum;			// 检测边缘的最大个数


	//-----------------区域部分-----------------
	m_enSttInputPara.useCaliperROI=srcPara.useCaliperROI; 		// 0：使用卡尺区域作为roi。1：全图作为roi
	m_enSttInputPara.imgSpace = srcPara.imgSpace;				// 0：输入图像空间。1：像素空间。2：根空间。 TODO：意义不明
	m_enSttInputPara.roiMode = srcPara.roiMode;					// 0：原点模式。1：中心模式。2：3点模式

	//共同参数
	m_enSttInputPara.originX = srcPara.originX;					// 原点x坐标
	m_enSttInputPara.originY = srcPara.originY;					// 原点y坐标 

	//原点模式和中心模式参数相同，使用以下参数
	//TODO坐标是否用float型
	m_enSttInputPara.width = srcPara.width;						// 宽度
	m_enSttInputPara.length = srcPara.length;					// 长度	
	m_enSttInputPara.rotAngle = srcPara.rotAngle;				// roi旋转角度
	m_enSttInputPara.skewAngle = srcPara.skewAngle;				// roi错切角度

	//3点模式使用以下参数
	//TODO坐标顺序
	m_enSttInputPara.X1 = srcPara.X1;							// 第二个顶点X
	m_enSttInputPara.Y1 = srcPara.Y1;							// 第二个顶点Y
	m_enSttInputPara.X2 = srcPara.X2;							// 第三个顶点.
	m_enSttInputPara.Y2 = srcPara.Y2;							// 第三个顶点

	//-----------------记分部分----------------- 
	m_enSttInputPara.mapScore = srcPara.mapScore;				// 0:不启用 1：启用
	m_enSttInputPara.mapX0 = srcPara.mapX0;						// TODO三个值的范围
	m_enSttInputPara.mapX1 = srcPara.mapX1;
	m_enSttInputPara.mapXc = srcPara.mapXc;

	//-----------------图形部分----------------- 
	//TODO:显示图像的尺寸是否需要可调
	m_enSttInputPara.displayEdge = srcPara.displayEdge;				//0：不显示。1：显示找到的边缘
	m_enSttInputPara.displayProjection = srcPara.displayProjection;	//0：不显示。1：显示投影图像
	m_enSttInputPara.displayFiltered = srcPara.displayFiltered;		//0：不显示。1：显示过滤过滤图形 TODO：意义不明
	m_enSttInputPara.dispalyTransEdge = srcPara.dispalyTransEdge;	//0：不显示。1：显示转换图像上的边缘 TODO:意义不明

	//-----------------调试参数-----------------
	m_enSttInputPara.displaySteps=srcPara.displaySteps;								//0:关闭过程显示 1：显示各个过程的图像
	m_enSttInputPara.writeLog=srcPara.writeLog;								//0:关闭过程显示 1：显示各个过程的图像

	return 0;
}

int CSeeney_Caliper::sny_getInputImg(const cv::Mat src)
{
	//从外部复制图像
	//检查是否为8位灰度图
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
-函数名:createOriginROI

-功能描述：
1.根据原点坐标，roi长宽先建立矩形roi，然后通过skew角度进行错切处理。
由于存在skew错切处理，需要建立mask Mat，并将mask数据传给投影函数
2.进行roi角度旋转，顺时针为正，逆时针为负，与opencv相反
3.角度旋转后，得到roi的最终区域。

以上均是roi操作，最后一步进行图像操作

4.由于寻边的方向为垂直x轴向下，故将roi区域的图像旋转回来。
在后面的投影函数中只进行水平投影。

TODO投影实际上为累加像素值，所以0不会影响投影结果，实际不需要mask，

-输入参数:
原始图像：const cv::Mat srcMat, 
图像原始未旋转未错切的roi：const cv::Rect oriROI, 
旋转角度：const float rotAngle, 
错切角度：const float skewAngle

-输出参数:
旋转后的roi包括的图像，以及完全相同规格的mask Mat
affine_matrix：返回affine变换的矩阵

-返回值:
执行成功：0
错误代码：

	EC_CLPR_CR_SKEWANGLE 错切角度错误,不能为±90，且不能超过±180,
	EC_CLPR_CR_ROIOUT	 ROI原始位置出界,
	EC_CLPR_CR_SKEWOUT	 错切后范围出界,
	EC_CLPR_CR_ROTOUT	旋转后范围出界

-备注
首先建立长方形，以左上角为原点[x,y]
  [x,y]--------
    /|\        |
   /a|b\       | 
      ---------
顺时针a为正角度，逆时针b为负角度

旋转方向顺时针为正，水平向右为0度

********************************************************/
int CSeeney_Caliper::sny_createOriginROI(const cv::Mat srcMat, const cv::Rect2d oriROI, const float rotAngle, const float skewAngle,
									 cv::Mat &maskMat,cv::Mat &roiMat, cv::Mat * affine_matrix)
{
	//错切角度错误，不能为以下角度
	if (skewAngle == 90 || skewAngle == -90 || skewAngle > 180 || skewAngle < -180) {
		return EC_CLPR_CR_SKEWANGLE;
	}

	
	//和原图同样尺寸的rect，判断是否出界用
	cv::Rect srcMatRect;
	srcMatRect.x = 0;
	srcMatRect.y = 0;
	srcMatRect.width  = srcMat.cols;
	srcMatRect.height = srcMat.rows;

	cv::Point tmpOutSide;
	tmpOutSide.x = oriROI.x;
	tmpOutSide.y = oriROI.y;

	//ROI原始位置出界
	if (srcMatRect.contains(tmpOutSide) != 1) {
		return EC_CLPR_CR_ROIOUT;
	}
	tmpOutSide.x += oriROI.width;
	tmpOutSide.y += oriROI.height;
	if (srcMatRect.contains(tmpOutSide) != 1) {
		return EC_CLPR_CR_ROIOUT;
	}

	cv::Mat displayMat;	    //显示用
	srcMat.copyTo(displayMat);
	Size skewedRoiSize;
 
	//错切后的四边形顶点坐标，原图坐标系
	vector<vector<Point>> contour;
	vector<Point> pts;
	//错切后的四边形顶点坐标，roi坐标系
	vector<vector<Point>> roi_contour;
	vector<Point> roi_pts;

	//旋转后的四边形顶点坐标，原图坐标系
	vector<vector<Point>> rot_contour;
	vector<Point> rpts;
	//最终轮廓，roi坐标系
	vector<vector<Point>> dst_contour;
	vector<Point> dst_pts;

	//----------------1.错切处理------------------ 
	//根据skewAngle计算roi错切后的顶点	
	//弧度 = 角度 * Pi / 180;
	float skewRadian = skewAngle * CV_PI / 180;
	float rotaRadian = rotAngle * CV_PI / 180;
	float dx;
	float py;

	//计算错切后的平移距离
	//dx,错切平移距离，py错切象限正负系数
	//角度在（90,190），（-180，-90），四边形需要做水平镜像，故py=-1作为y坐标系数
	if (skewAngle > -90 && skewAngle < 90) {
		dx = - oriROI.height * tan(skewRadian);
		py = 1;
	}
	else if ((skewAngle > 90 && skewAngle <= 180) || (skewAngle < -90 && skewAngle >= -180) ){
		dx = -oriROI.height * tan(CV_PI - skewRadian);
		py = -1;
	}
	//生成错切后的四个顶点,在原坐标系的坐标	
	pts.push_back(Point(oriROI.x, oriROI.y));
	pts.push_back(Point(oriROI.x+oriROI.width-1, oriROI.y));
	pts.push_back(Point(oriROI.x+oriROI.width-1+dx, oriROI.y+py*oriROI.height-1));
	pts.push_back(Point(oriROI.x+dx, oriROI.y+py*oriROI.height-1));
	contour.push_back(pts);

	//判断是否出界
	for (int n = 0; n < 4;n++) {
		if (srcMatRect.contains(pts[n])!=1) {
			return EC_CLPR_CR_SKEWOUT;
		}
	}

	//获得最终投影图像的矩形尺寸准备roi
	cv::Rect rotbackROI= cv::boundingRect(cv::Mat(pts).reshape(2));

	//----------------2.旋转处理------------------ 
	//计算四个顶点旋转之后的新坐标
	for (int i = 0; i < 4; i++) {
		cv::Point2d temp1;
		//计算旋转后坐标函数，在原图坐标系
		//由于顺时针为正，所以这里加负号
		sny_calRotPosition(pts[0], pts[i], temp1, (-1*rotAngle));
		rpts.push_back(temp1);
		
		//为最终投影图像计算4个顶点坐标，在最终投影图像坐标系
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

	//----------------3.复制图像------------------ 
	//错切及旋转完成，生成的roi,该roi坐标为原图像坐标系中的坐标
	cv::Rect dstRoiRect = cv::boundingRect(cv::Mat(rpts).reshape(2));
	
	//生成mask图像
	//修改mask坐标，生成roi坐标系下的坐标
	for (int i = 0; i < 4; i++) {
		//TODO判断是否出界
		cv::Point temp;
		temp.x = rpts[i].x - dstRoiRect.x;
		temp.y = rpts[i].y - dstRoiRect.y;
		dst_pts.push_back(temp);
	}
	dst_contour.push_back(dst_pts);

	//绘制最终的maskMat，在图像坐标系
	maskMat = Mat::zeros(dstRoiRect.size(),CV_8UC1);
	drawContours(maskMat, dst_contour, 0, Scalar::all(255), -1);

	//拷贝最终的roi图像，在图像坐标系
	cv::Mat temp = srcMat(dstRoiRect);
	temp.copyTo(roiMat,maskMat);

	//----------------4.图像回旋------------------ 
	//为下一个投影函数，生成符合投影方向的最终图像
	//即将roiMat和maskMat往回旋转rotAngle
	// 变换前坐标
	const cv::Point2f src_pt[] = { cv::Point2f(dst_pts[0].x, dst_pts[0].y),
								   cv::Point2f(dst_pts[1].x, dst_pts[1].y),
								   cv::Point2f(dst_pts[2].x, dst_pts[2].y)};
	// 変后后坐标
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
	// 変后后坐标
	const cv::Point2f dst_pt2[] = { cv::Point2f(rpts[0].x, rpts[0].y),
								   cv::Point2f(rpts[1].x, rpts[1].y),
								   cv::Point2f(rpts[2].x, rpts[2].y) };
	affine_matrix[CA_PRJ2ORI]= cv::getAffineTransform(src_pt2, dst_pt2);


	
	if (m_enSttInputPara.displaySteps == 1) {
		//描画结果
		//错切后的形状
		drawContours(displayMat, contour, 0, Scalar::all(255),1);
		//旋转后的形状
		drawContours(displayMat, rot_contour, 0, cvScalar(0, 255, 255), 2);
			//----------------------------------//原点
		circle(displayMat, Point(oriROI.x, oriROI.y), 3, cvScalar(0, 0, 255), 8, 0);
		//最终roi
		rectangle(displayMat, dstRoiRect, cvScalar(255, 0, 255), 1, 8, 0);
		//原始roi
		rectangle(displayMat, oriROI, cvScalar(255, 255, 255), 1, 8, 0);
		imshow("display", displayMat);
		imshow("roiMat", roiMat);
		imshow("dstMask", maskMat);
	}

	return 0;
}

//函数说明格式
/********************************************************
-函数名:sny_crtCentModeROI
-功能描述
以原点模式创建roi，
第一步错切和左上角顶点模式一样
第二步旋转模式以中心

与原点模式不同的是
原roi的中心点，在错切和旋转中都固定。

错切角度在(90,180]及(-180,-90)的区间时，是以原点做镜像



-输入参数:


-输出参数:


-返回值:
执行成功：0
错误代码：

	EC_CLPR_CR_SKEWANGLE 错切角度错误,不能为±90，且不能超过±180,
	EC_CLPR_CR_ROIOUT	 ROI原始位置出界,
	EC_CLPR_CR_SKEWOUT	 错切后范围出界,
	EC_CLPR_CR_ROTOUT	旋转后范围出界


-备注
首先建立长方形，以左上角为原点[x,y]
  [x,y]--------
	/|\        |
   /a|b\       |
	  ---------
顺时针a为正角度，逆时针b为负角度

旋转方向顺时针为正，水平向右为0度

oriROI的x，y坐标为未旋转前坐标，应在函数外部修改好！！！！

********************************************************/
int CSeeney_Caliper::sny_crtCentModeROI(const cv::Mat srcMat, const cv::Rect2d oriROI, const float rotAngle, const float skewAngle, cv::Mat & maskMat, cv::Mat & roiMat, cv::Mat * affine_matrix)
{
	//错切角度错误，不能为以下角度
	if (skewAngle == 90 || skewAngle == -90 || skewAngle > 180 || skewAngle < -180) {
		return EC_CLPR_CR_SKEWANGLE;
	}


	//和原图同样尺寸的rect，判断是否出界用
	cv::Rect srcMatRect;
	srcMatRect.x = 0;
	srcMatRect.y = 0;
	srcMatRect.width = srcMat.cols;
	srcMatRect.height = srcMat.rows;

	//roi中心，旋转用
	cv::Point centerPoint;
	cv::Point tmpOutSide;
	tmpOutSide.x = oriROI.x;
	tmpOutSide.y = oriROI.y;

	//ROI原始位置出界
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

	cv::Mat displayMat;	    //显示用
	srcMat.copyTo(displayMat);
	Size skewedRoiSize;

	//错切后的四边形顶点坐标，原图坐标系
	vector<vector<Point>> contour;
	vector<Point> pts;
	//错切后的四边形顶点坐标，roi坐标系
	vector<vector<Point>> roi_contour;
	vector<Point> roi_pts;

	//旋转后的四边形顶点坐标，原图坐标系
	vector<vector<Point>> rot_contour;
	vector<Point> rpts;
	//最终轮廓，roi坐标系
	vector<vector<Point>> dst_contour;
	vector<Point> dst_pts;

	//----------------1.错切处理------------------ 
	//根据skewAngle计算roi错切后的顶点	
	//错切角度应该(-90,90)
	//弧度 = 角度 * Pi / 180;
	float skewRadian = skewAngle * CV_PI / 180;
	float rotaRadian = rotAngle * CV_PI / 180;
	float dx;
	float px;
	//计算错切后的平移距离
	//以原点错切，平移距离为
	if (skewAngle > -90 && skewAngle < 90) {
		dx = oriROI.height * tan(skewRadian);
		px = 1;
	}
	else if ((skewAngle > 90 && skewAngle <= 180) || (skewAngle < -90 && skewAngle >= -180)) {
		dx = oriROI.height * tan(CV_PI - skewRadian);
		px = -1;
	}

	//修改4个顶点坐标
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
	//获得最终投影图像的矩形尺寸准备roi
	cv::Rect rotbackROI = cv::boundingRect(cv::Mat(pts).reshape(2));

	//----------------2.旋转处理------------------ 
	//计算四个顶点旋转之后的新坐标
	for (int i = 0; i < 4; i++) {
		cv::Point2d temp1;
		//计算旋转后坐标函数，在原图坐标系
		//
		//由于顺时针为正，所以这里加负号
		sny_calRotPosition(centerPoint, pts[i], temp1, (-1 * rotAngle));
		rpts.push_back(temp1);

		//为最终投影图像计算4个顶点坐标，在最终投影图像坐标系
		cv::Point temp2;
		temp2.x = pts[i].x - rotbackROI.x;
                        		temp2.y = pts[i].y - rotbackROI.y;
		roi_pts.push_back(temp2);
	}
	rot_contour.push_back(rpts);
	roi_contour.push_back(roi_pts);

	//判断是否出界
	for (int n = 0; n < 4; n++) {
		if (srcMatRect.contains(rpts[n]) != 1) {
			return EC_CLPR_CR_ROTOUT;
		}
	}

	//----------------3.复制图像------------------ 
	//错切及旋转完成，生成的roi,该roi坐标为原图像坐标系中的坐标
	cv::Rect dstRoiRect = cv::boundingRect(cv::Mat(rpts).reshape(2));
	//生成mask图像
	//修改mask坐标，生成roi坐标系下的坐标
	for (int i = 0; i < 4; i++) {
		//TODO判断是否出界
		cv::Point temp;
		temp.x = rpts[i].x - dstRoiRect.x;
		temp.y = rpts[i].y - dstRoiRect.y;
		dst_pts.push_back(temp);
	}
	dst_contour.push_back(dst_pts);

	//绘制最终的maskMat，在图像坐标系
	maskMat = Mat::zeros(dstRoiRect.size(), CV_8UC1);
	drawContours(maskMat, dst_contour, 0, Scalar::all(255), -1);

	//拷贝最终的roi图像，在图像坐标系
	cv::Mat temp = srcMat(dstRoiRect);
	temp.copyTo(roiMat, maskMat);

	//----------------4.图像回旋------------------ 
	//为下一个投影函数，生成符合投影方向的最终图像
	//即将roiMat和maskMat往回旋转rotAngle
	// 变换前坐标
	const cv::Point2f src_pt[] = { cv::Point2f(dst_pts[0].x, dst_pts[0].y),
								   cv::Point2f(dst_pts[1].x, dst_pts[1].y),
								   cv::Point2f(dst_pts[2].x, dst_pts[2].y) };
	// 変后后坐标
	const cv::Point2f dst_pt[] = { cv::Point2f(roi_pts[0].x, roi_pts[0].y),
								   cv::Point2f(roi_pts[1].x, roi_pts[1].y),
								   cv::Point2f(roi_pts[2].x, roi_pts[2].y) };
	affine_matrix[CA_ORI2PRJ] = cv::getAffineTransform(src_pt, dst_pt);
	cv::warpAffine(roiMat, roiMat, affine_matrix[0], rotbackROI.size());
	cv::warpAffine(maskMat, maskMat, affine_matrix[0], rotbackROI.size());


	const cv::Point2f src_pt2[] = { cv::Point2f(roi_pts[0].x, roi_pts[0].y),
							   cv::Point2f(roi_pts[1].x, roi_pts[1].y),
							   cv::Point2f(roi_pts[2].x, roi_pts[2].y) };
	// 変后后坐标
	const cv::Point2f dst_pt2[] = { cv::Point2f(rpts[0].x, rpts[0].y),
								   cv::Point2f(rpts[1].x, rpts[1].y),
								   cv::Point2f(rpts[2].x, rpts[2].y) };
	affine_matrix[CA_PRJ2ORI] = cv::getAffineTransform(src_pt2, dst_pt2);

	if (m_enSttInputPara.displaySteps == 1) {
		//描画结果

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
-函数名sny_crt3PointsModeROI
-功能描述
3点模式，创建roi

-输入参数:


-输出参数:
const cv::Mat srcMat：输入图像

前端给出的3个点
const cv::Point srcP1, 
const cv::Point srcP2, 
const cv::Point srcP3,


-返回值:
同原点及中心模式
 cv::Mat & maskMat, cv::Mat & roiMat, cv::Mat * affine_matrix

-备注
错误码
3个点，有重叠
EC_CLPR_CR_SAMEPOSITION = 9,

********************************************************/
int CSeeney_Caliper::sny_crt3PointsModeROI(const cv::Mat srcMat,
										   const cv::Point2d pt1, const cv::Point2d pt2, const cv::Point2d pt3, 
										   cv::Mat & maskMat, cv::Mat & roiMat, cv::Mat * affine_matrix)
{
	cv::Point2d pt4;
	
	//和原图同样尺寸的rect，判断是否出界用
	cv::Rect srcMatRect;
	srcMatRect.x = 0;
	srcMatRect.y = 0;
	srcMatRect.width = srcMat.cols;
	srcMatRect.height = srcMat.rows;	

	//显示用
	cv::Mat displayMat;
	m_srcMat.copyTo(displayMat);

	//由3点生成的原图中的roi，原图坐标系
	vector<vector<Point>> roiImg_contour;
	vector<Point> roiImg_pts;

	//由3点生成的原图中的roi，roi坐标系，拷贝用
	vector<vector<Point>> roiRectImg_contour;
	vector<Point> roiRectImg_pts;

	//投影图像中的轮廓，投影坐标系
	vector<vector<Point>> prjImg_contour;
	vector<Point> prjImg_pts;

	//首先判断 3个点是否有重叠
	if ((sny_samePoints2d(pt1, pt2) == 1) ||
		(sny_samePoints2d(pt1, pt3) == 1) ||
		(sny_samePoints2d(pt2, pt3) == 1)) {
		return EC_CLPR_CR_SAMEPOSITION;
	}

	//根据3个点计算四边形，及旋转回x轴的坐标
	sny_3ptsCreateCountors(pt1,pt2,pt3,roiImg_pts,prjImg_pts);
	roiImg_contour.push_back(roiImg_pts);
	//判断是否出界
	for (int n = 0; n < 4; n++) {
		if (srcMatRect.contains(roiImg_pts[n]) != 1) {
			return EC_CLPR_CR_ROTOUT;
		}
	}
	//绘制最终的maskMat，在图像坐标系
	cv::Rect roiImgRect = cv::boundingRect(cv::Mat(roiImg_pts).reshape(2));

	//获得最终投影图像的矩形尺寸准备roi
	cv::Rect prjImgRect = cv::boundingRect(cv::Mat(prjImg_pts).reshape(2));
	//原图roi被复制的rect区域

	//将prjImgRect的坐标修改至prjImg坐标系
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

	//拷贝最终的roi图像，在图像坐标系
	cv::Mat temp = srcMat(roiImgRect);
	temp.copyTo(roiMat, maskMat);
	
	//----------------4.图像回旋------------------ 
	//为下一个投影函数，生成符合投影方向的最终图像
	//即将roiMat和maskMat往回旋转rotAngle
	// 变换前坐标
	const cv::Point2f src_pt[] = { cv::Point2f(roiRectImg_pts[0].x, roiRectImg_pts[0].y),
								   cv::Point2f(roiRectImg_pts[1].x, roiRectImg_pts[1].y),
								   cv::Point2f(roiRectImg_pts[2].x, roiRectImg_pts[2].y) };
	// 変后后坐标
	const cv::Point2f dst_pt[] = { cv::Point2f(prjImg_pts[0].x, prjImg_pts[0].y),
								   cv::Point2f(prjImg_pts[1].x, prjImg_pts[1].y),
								   cv::Point2f(prjImg_pts[2].x, prjImg_pts[2].y) };
	affine_matrix[CA_ORI2PRJ] = cv::getAffineTransform(src_pt, dst_pt);
	cv::warpAffine(roiMat, roiMat, affine_matrix[0], prjImgRect.size());
	cv::warpAffine(maskMat, maskMat, affine_matrix[0], prjImgRect.size());

	
	const cv::Point2f src_pt2[] = { cv::Point2f(prjImg_pts[0].x, prjImg_pts[0].y),
									cv::Point2f(prjImg_pts[1].x, prjImg_pts[1].y),
									cv::Point2f(prjImg_pts[2].x, prjImg_pts[2].y) };
	// 変后后坐标
	const cv::Point2f dst_pt2[] = { cv::Point2f(roiImg_pts[0].x, roiImg_pts[0].y),
								   cv::Point2f(roiImg_pts[1].x, roiImg_pts[1].y),
								   cv::Point2f(roiImg_pts[2].x, roiImg_pts[2].y) };
	affine_matrix[CA_PRJ2ORI] = cv::getAffineTransform(src_pt2, dst_pt2);

	if (m_enSttInputPara.displaySteps == 1) {
		//描画结果
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
-函数名: sny_calcRotSkew
-功能描述
3点模式调用此函数，根据给出的3点，计算错切的角度和旋转的角度

-输入参数:
srcP1,srcP2,srcP3
三个点坐标，srcP1相当于原点模式中的原点

-输出参数:
rotAngle：旋转角度
skewAngle：错切角度
正负与原点模式及中心模式一样
以第一个点为左上角原点[x,y]
  [x,y]--------
	/|\        |
   /a|b\       |
	  ---------
顺时针a为正角度，逆时针b为负角度

旋转方向顺时针为正,水平为0度
-返回值:
执行成功：0

-备注
不判断是否出界，在下一个步骤会调用



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
	//第一个点和第二个点组成直线，与水平的顺时针角度。
	float dst12;
	dst12 = (srcP1.x - srcP2.x)*(srcP1.x - srcP2.x) + (srcP1.y-srcP2.y)*(srcP1.y - srcP2.y);
	
	if (dst12 == 0) {
		//距离为0，即点重复
		return EC_CLPR_CR_SAMEPOSITION;
	}

	//构造x轴上的点
	cv::Point2f P2rot2X	;
	P2rot2X.y = srcP1.y;
	P2rot2X.x = srcP1.x + sqrt(dst12);

	circle(displayMat, P2rot2X, 5, Scalar(180	, 100, 0), -1, 8, 0);
	line(displayMat, srcP1, P2rot2X, Scalar(255, 0, 0), 1, 8, 0);
	
	float dst2temp=(srcP2.x-P2rot2X.x)*(srcP2.x - P2rot2X.x) + (srcP2.y-P2rot2X.y)*(srcP2.y - P2rot2X.y);


	if (srcP1.y<=srcP2.y) {
		//0-180度
		//*rotAngle = acos((dst12 + dst12 - dst2temp) / (2 * dst12));
		*rotAngle = (acos((dst12 + dst12 - dst2temp) / (2 * dst12)))*180/CV_PI;
	}
	else if (srcP1.y > srcP2.y){
		*rotAngle = -(acos((dst12 + dst12 - dst2temp) / (2 * dst12)) * 180 / CV_PI);
		//*rotAngle = -acos((dst12 + dst12 - dst2temp) / (2 * dst12));
	}

	//计算错切角度
	//P1以P2为原点，旋转-90度，旋转后的坐标计为skP,计算skP和P3的夹角
	//P2，P3逆时针转rotAngle度
	cv::Point2d skP;
	sny_calRotPosition(srcP1, srcP3, skP, *rotAngle);
	//构造y轴上的点,x=P2rot2X.x,y=skP.y
	cv::Point2f tempY;
	tempY.x = P2rot2X.x;
	tempY.y = skP.y;

	float dst_a = (skP.x - tempY.x)*(skP.x - tempY.x) + (skP.y - tempY.y)*(skP.y - tempY.y);
	float dst_b = ((P2rot2X.x-skP.x)*(P2rot2X.x-skP.x))+((P2rot2X.y-skP.y)*(P2rot2X.y-skP.y));
	float dst_c = ((P2rot2X.x-tempY.x)*(P2rot2X.x-tempY.x))+((P2rot2X.y-tempY.y)*(P2rot2X.y-tempY.y));
	*skewAngle = acos((dst_b + dst_c - dst_a) / (2 * sqrt(dst_b)*sqrt(dst_c))) * 180 / CV_PI;

	//计算以
	line(displayMat, P2rot2X, tempY, Scalar(255, 0, 0), 1, 8, 0);
	circle(displayMat, tempY, 5, Scalar(180, 100, 0), -1, 8, 0);
	line(displayMat,srcP1,skP,Scalar(150,0,0),1,8,0);
	circle(displayMat, skP, 5, Scalar(180, 100, 0), -1, 8, 0);


	imshow("111",displayMat);
	waitKey(0);
	return 0;
}

/********************************************************
-函数名:sny_calRotPosition


-功能描述：
以center的坐标为圆心，计算原坐标srcPoint旋转rotAngle度后的坐标。
通过3×3的affine矩阵实现
-输入参数:
const cv::Point center：旋转原点：
const cv::Point srcPoint：输入坐标：
float roAngle：旋转角度，角度值，非弧度

-输出参数:
cv::Point & dstPoint：旋转后的坐标

-返回值:

-备注
rotAngle为角度值，在函数内部转换为弧度，拟时针为正值，x轴为0度

********************************************************/
int CSeeney_Caliper::sny_calRotPosition(const cv::Point2d center, const cv::Point2d srcPoint, cv::Point2d & dstPoint, float rotAngle)
{
	float rotaRadian = rotAngle * CV_PI / 180;

	//计算4个点旋转后的坐标，通过旋转矩阵	
	float ocx = center.x;
	float ocy = center.y;
	cv::Point2f rotCenter(ocx, ocy);

	//平移矩阵
	float offset[3][3] =
	{
		{1,     0,   0},
		{0,    -1,   0},
		{-ocx, ocy,  1}
	};
	cv::Mat offsetMat(3, 3, CV_32FC1, offset);

	//旋转矩阵
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
-函数名:sny_calcHoriProject
-功能描述
水平方向的投影
累加所有灰度值
 
-输入参数:
const cv::Mat srcMat：输入图像
const cv::Mat mskMat：mask，目前没有使用。累加灰度值，黑色不累加。故

-输出参数:
int * dstArry：投影结果，1维数组，dstArry[0]

-返回值:
返回0，创建成功
EC_CLPR_CP_WIDTHOVER = 输入尺寸溢出危险

-备注
dstArry数组为int型，故投影像素总数最大值为 2147483647 / 255 = 8,421,504
图像宽度不能超过此上限
正常来说尺寸不会达到这个数值

********************************************************/
int CSeeney_Caliper::sny_calcHoriProject(const cv::Mat srcMat, const cv::Mat mskMat, int * dstArry)
{

	if (srcMat.cols>=8421504) {
		return 	EC_CLPR_CALP_WIDTHOVER;
	}

	//逐行累加
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
-函数名:sny_horizontalProjection
-功能描述
获取hist的梯度值

-输入参数:
const int * srcArry：输入hist的数组
const int convSize：卷积尺寸conSize*2+1
const int srcSize：输入hist数组的尺寸

-输出参数:
int * dstArry；输出结果的数组

-返回值:
返回0，创建成功

-备注
卷积：size=5
[-1,-1,0,+1,+1]

两侧无法卷积部分设为0，no padding
********************************************************/
int CSeeney_Caliper::sny_convHoriProject(const int * srcArry, const int srcSize,const int convSize, int * dstArry)
{
	int fsize = convSize * 2 + 1;
	int * kernel= new int[fsize];	

	if ((convSize <= 0) || (fsize > srcSize)) {
		delete[]kernel;
	return  EC_CLPR_CONP_KERNEL;
	}

	//生成卷积核
	for (int n = 0; n < convSize;n++) {
		kernel[n] = -1;
		kernel[fsize - n-1] = 1;
	}
	kernel[convSize] = 0;

	//卷积处理
	//TODO卷积优化，或调opencv
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
-函数名:sny_showHorProjection
-功能描述
绘制水平投影的结果直方图
直方图为竖方向，如下

5        -----|
7      -------|
2           --|       
3          ---|   

-输入参数:
const int * srcArry：输入hist的数组
const int srcSize：hist数组的size
cv::Size disSize：输出图像的尺寸，目前是和卡尺roi所选取的区域尺寸相同，便于对照观察。TODO可添加缩放系数

-输出参数:
cv::Mat & dstMat：描画结果的图像

-返回值:
返回0，创建成功

-备注
输出图像目前是和卡尺roi所选取的区域尺寸相同，便于对照观察。TODO可添加缩放系数
以数组中最大的累加值做归一化，以图像右上角为原点
如最大值为 maxVal,输出图像宽度为width
某一个bin数值为x
则该bin在图像中的长度 bin_length为：
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
-函数名:sny_showConProjection 
-功能描述
将投影图像卷积后的结果图像化

-输入参数:
const int * srcArry：输入卷积结果的数组
const int srcSize：  卷积结果数组尺寸
const int srcSize：  生成目标图像的尺寸，建议设置为被计算投影图像的两倍

-输出参数:
int * dstMat；创建的图像

-返回值:
返回0，创建成功

-备注
显示图像height与被计算投影的图像相同
由于卷积结果有正有负，显示图像width设置为被计算投影图像的两倍
左侧显示正值，右侧显示负值
如：
5        -----|
7      -------|
2           --|
3          ---|
-4            |----
-7            |-------
-1            |-
3          ---|

采用绝对值最大的值进行归一化

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
			//如果大于0，则在图像左侧描画
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
-函数名: sny_samePoints
-功能描述
判断2个输入点是否为相同点

-输入参数:
cv::Point pt1
cv::Point pt2

-输出参数:

-返回值:
相同点
return 1 
两个点不相同
return 0 

-备注

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
-函数名:sny_Parallelogram
-功能描述
根据给出的3个点，计算平行四边形第4个点的坐标

-输入参数:
cv::Point pt1，cv::Point pt2，cv::Point pt3

-输出参数:
dstPt；第4个点坐标

-返回值:
计算成功 return 0

-备注
pt1和pt2，组成第一条边A
pt2和pt3，组成第二条边B
pt3和pt4组成的边平行于A
pt1和pt4组成的边平行于B

********************************************************/
int CSeeney_Caliper::sny_3ptsCreateCountors(const cv::Point2d pt1, const cv::Point2d pt2, const cv::Point2d pt3,
		vector<Point> &roiImgPts, vector<Point> &roiPrjPts)
{
	float rotAngle;
	cv::Point2d dstPt;
	cv::Mat displayMat;
	m_srcMat.copyTo(displayMat);
	
	//第一个点和第二个点组成直线，与水平的顺时针角度。
	double dst12;
	dst12 = (pt1.x - pt2.x)*(pt1.x - pt2.x) + (pt1.y - pt2.y)*(pt1.y - pt2.y);

	//构造x轴上的点
	cv::Point2d pt2_to_X;
	pt2_to_X.y = pt1.y;
	pt2_to_X.x = pt1.x + sqrt(dst12);
	double dst_pt2_toX= (pt2.x - pt2_to_X.x)*(pt2.x - pt2_to_X.x) + (pt2.y - pt2_to_X.y)*(pt2.y - pt2_to_X.y);

	if (pt1.y <= pt2.y) {
		//0-180度
		rotAngle =	 acos((float)((dst12 + dst12 - dst_pt2_toX) / (2 * dst12))) * 180 / CV_PI;
	}
	else if (pt1.y > pt2.y) {
		rotAngle =	 acos((float)((dst12 + dst12 - dst_pt2_toX) / (2 * dst12))) * 180 / CV_PI;
		rotAngle =	-rotAngle;
	}

	//pt3，旋转rotAngle°
	cv::Point2d rotPt3;
	sny_calRotPosition(pt1, pt3, rotPt3, rotAngle);

	//计算rotPt3和pt2_to_X坐标差值
	double dx = pt2_to_X.x - rotPt3.x;
	double dy = pt2_to_X.y - rotPt3.y;
	
	//根据坐标差值计算4边型第4个点，该点为旋转后的点
	cv::Point2d tempPt4;
	tempPt4.x = pt1.x - dx;
	tempPt4.y = pt1.y - dy;

	//旋转回实际的位置，即旋转-rotAngle°
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
-函数名:
-功能描述


-输入参数:
const int * derHist：输入的导数数组
const int hisSize：输入导数数组的尺寸。
const int maxNum：寻找边缘的个数，即寻找derHist中最大的前maxNum个点
const int contstTH：对比度阈值，边缘的对比度应该大于此数值。如要修改对比度定义，该函数内也应该一同修改
const int polarity：0：由暗到明 1：由明到暗 2：任意极性

-输出参数:
std::vector<sttSingleEdge>* dstEdges：检测结果数未知，故通过向量输出结果，向量内容为自定义结构体
结构体内容如下
typedef struct {
	//每个检测到的点对应一个结构体
	int position;			//投影的位置
	float x;				//图像中的位置，存在旋转时，需要旋转回图像坐标系。
	float y;				//图像中的位置，同上
	float score;			//边缘得分
	float contrast;			//对比度

}sttCaliperOutput;

-返回值:
返回检测到的边缘个数
EC_CLPR_SE_MAXNUM 设置的寻边最大个数出错

-备注
关于极性：
极性方向定义为从上往下寻找
投影数组，卷积数组，均以上方为原点
卷积核定义
	-1
	-1
	 0
	 1
	 1
故从暗到明为 正值
  从明到暗为 负值

float contrast
对比度计算方法
卷积结果值/kernelSize * roiSize.width

score
得分计算方法：
对一列边缘来说最大可能差分值为 255*kernelSize  kernelSize*2+1为最终卷积值
如：kernelSize==2
图像  卷积
0     -1
0	  -1
0      0
255    1
255    1

被投影的roi共有 m_roiMat.width 列
∴满分值   = 255 * kernelSize * m_roiMat.width
∴边缘分值 = contrast / 满分值
TODO：溢出检查 INT_MAX == 2147483647 
最大允许列数为，INT_MAX / (255 * kernelSize)
kernelSize为2时，最大列数为4210752，正常不会溢出

边缘在实际图像中的坐标在下一个步骤中计算

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

	//构造结构体向量，排序用
	std::vector<convNode> vDerHist;
	for (int n = 0; n < hisSize;n++) {
		convNode temp;
		if (polarity==2) {
			//不计极性时，取绝对值
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
		//寻找正值中的最大的前maxNum个
		//max_element
		partial_sort(vDerHist.begin(),vDerHist.begin()+maxNum,vDerHist.end(),compare4bigger);

	}
	else if (polarity == 1) {
		//寻找负值中的最大的前maxNum个
		partial_sort(vDerHist.begin(),vDerHist.begin()+maxNum,vDerHist.end(),compare4smaller);

	}
	else if (polarity==2) {
		//寻找绝对值最大的前maxNum个
		partial_sort(vDerHist.begin(),vDerHist.begin()+maxNum,vDerHist.end(),compare4bigger);
	}

	//计算边缘得分
	float maxSore = 255 * kernelSize*roiSize.width;
	float norm = kernelSize * roiSize.width;

	//保存结果
	
	int cnt = 0;
	for (int i = 0; i < maxNum; i++) {
		int tempcontrast = (int)(vDerHist[i].data / norm);
		if (tempcontrast >= contstTH) {
			dstEdges[i].index = vDerHist[i].No;
			dstEdges[i].contrast = vDerHist[i].data / norm;				//除以kernelSize，归一化到0-255之间
			dstEdges[i].score = (vDerHist[i].data / maxSore);
			dstEdges[i].x = 0;											//初始化为0，具体坐标下一个步骤计算
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
-函数名:
-功能描述
将opencv中的affine变换矩阵转换为3*3的标准affine矩阵

-输入参数:
opencvAffineMat:opencv的get matrix等函数获得的affine矩阵，矩阵为2*3尺寸

-输出参数:
dstMat，标准3*3矩阵

-返回值:
dstMat

-备注

********************************************************/
int CSeeney_Caliper::sny_convertAffineMat(const cv::Mat opencvAffineMat, cv::Mat & dstMat)
{
	//3*3的矩阵
	dstMat = Mat::eye(3, 3, CV_64FC1);

	dstMat.at<double>(0, 0) = opencvAffineMat.at<double>(0, 0);
	dstMat.at<double>(0, 1) = opencvAffineMat.at<double>(1, 0);
	dstMat.at<double>(1, 0) = opencvAffineMat.at<double>(0, 1);
	dstMat.at<double>(1, 1) = opencvAffineMat.at<double>(1, 1);
	dstMat.at<double>(2, 0) = opencvAffineMat.at<double>(0, 2);
	dstMat.at<double>(2, 1) = opencvAffineMat.at<double>(1, 2);

	return 0;
}



