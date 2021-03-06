// SeeneyImgProcLib.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include <opencv.hpp>
#include "CSeeney_Caliper.h"


using namespace cv;

sttCaliperInput test_enSttInputPara;



int setPara()
{

	//-----------------设置部分-----------------
	test_enSttInputPara.edgeModel = 0;				// 0：单个边缘 1：边缘对
	test_enSttInputPara.edgePlrty = 2;				// 0：由暗到明 1：由明到暗 2：任意极性
	test_enSttInputPara.edgeWidth = 4;				// [0,roi宽度] TODO：边缘宽度上限 	

	test_enSttInputPara.contrastTH = 100;			// [0,255] TODO：阈值上限需要调试
	test_enSttInputPara.filtHalfPixel = 1;		// []	   TODO：意义不明？？	
	test_enSttInputPara.maxLineNum = 1;			// [1,??]  TODO：roi中最大线的数量，上限？？？
	
	//-----------------区域部分-----------------
	test_enSttInputPara.useCaliperROI = 0; 			// 0：使用卡尺区域作为roi。1：全图作为roi
	test_enSttInputPara.imgSpace = 0;				// 0：输入图像空间。1：像素空间。2：根空间。 TODO：意义不明
	test_enSttInputPara.roiMode = 0;				// 0：原点模式。1：中心模式。2：3点模式

	//共同参数
	test_enSttInputPara.originX = 200;					// 原点x坐标
	test_enSttInputPara.originY = 200;					// 原点y坐标 

	//原点模式和中心模式参数相同，使用以下参数
	//TODO坐标是否用float型
	test_enSttInputPara.width = 150;				// 宽度
	test_enSttInputPara.length = 150;				// 长度	
	test_enSttInputPara.rotAngle =  0;				// roi旋转角度
	test_enSttInputPara.skewAngle =0;				// roi错切角度

	//3点模式使用以下参数
	//TODO坐标顺序
	//test_enSttInputPara.X1 = srcPara.X1;							// 第二个顶点X
	//test_enSttInputPara.Y1 = srcPara.Y1;							// 第二个顶点Y
	//test_enSttInputPara.X2 = srcPara.X2;							// 第三个顶点.
	//test_enSttInputPara.Y2 = srcPara.Y2;							// 第三个顶点

	//-----------------记分部分----------------- 
	test_enSttInputPara.mapScore = 1;						// 0:不启用 1：启用
	test_enSttInputPara.mapX0 = 255;						// TODO三个值的范围
	test_enSttInputPara.mapX1 = 0;
	test_enSttInputPara.mapXc = 0;

	//-----------------图形部分----------------- 
	//TODO:显示图像的尺寸是否需要可调
	test_enSttInputPara.displayEdge = 1;		//0：不显示。1：显示找到的边缘
	test_enSttInputPara.displayProjection = 1;	//0：不显示。1：显示投影图像
	test_enSttInputPara.displayFiltered = 1;	//0：不显示。1：显示过滤过滤图形 TODO：需确认，应该是卷积后的投影图像
	test_enSttInputPara.dispalyTransEdge = 1;	//0：不显示。1：显示转换图像上的边缘 TODO:转换后图像意义不明

	return 1;
}
;

int main()
{
	//测试用主程序
	CSeeney_Caliper enCaliper;
	Mat srcMat;
	srcMat=imread("C:/Users/HaoXiaochui/Desktop/彩色报纸.jpg",0);
	//srcMat = imread("G:/test.jpg");
	setPara();
	//----------------------------------


	// Select ROI
	Rect2d r = selectROI(srcMat);

	test_enSttInputPara.originX = r.x;
	test_enSttInputPara.originY = r.y;
	test_enSttInputPara.width = r.width;
	test_enSttInputPara.length = r.height;

	//获得参数
	enCaliper.sny_getCaliperPara(test_enSttInputPara);
	//获得图像
	enCaliper.sny_getInputImg(srcMat);

	//roi选择
	//进入处理主程序
	enCaliper.CaliperMain();					//

	
	
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门提示: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
