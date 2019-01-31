#include "pch.h"

//卡尺工具前段输入参数结构体
typedef struct{

	//-----------------设置部分-----------------
	int edgeModel;			// 0：单个边缘 1：边缘对
	int edgePlrty;			// 0：由暗到明 1：由明到暗 2：任意极性
	int edgeWidth;			// 当 edgeModel==1 时，[0,roi宽度] TODO：边缘宽度上限 	

	int contrastTH;			// [0,255] TODO：阈值上限需要调试
	int filtHalfPixel;		// []	   TODO：意义不明？？	
	int maxLineNum;			// [1,??]  TODO：roi中最大线的数量，上限？？？

	//-----------------区域部分-----------------
	int useCaliperROI;		// 0：使用卡尺区域作为roi。1：全图作为roi
	int imgSpace;			// 0：输入图像空间。1：像素空间。2：根空间。 TODO：意义不明
	int roiMode;			// 0：原点模式。1：中心模式。2：3点模式
	
	//共同参数
	double originX;			// 原点x坐标
	double originY;			// 原点y坐标 

	//原点模式和中心模式参数相同，使用以下参数
	//TODO坐标是否用float型
	double width;			// 宽度
	double length;			// 长度	
	float rotAngle;			// roi旋转角度
	float skewAngle;		// roi错切角度，原点垂直向下，逆时针为负角度，顺时针为正角度。

	//3点模式使用以下参数
	//TODO坐标顺序
	double X1;				// 第二个顶点X
	double Y1;				// 第二个顶点Y
	double X2;				// 第三个顶点;
	double Y2;				// 第三个顶点

	//-----------------记分部分----------------- 
	int mapScore;			// 0:不启用 1：启用
	float mapX0;			// TODO三个值的范围,应该是[0,1]
	float mapX1;
	float mapXc;

	//-----------------图形部分----------------- 
	//TODO:显示图像的尺寸是否需要可调
	int displayEdge;		//0：不显示。1：显示找到的边缘
	int displayProjection;	//0：不显示。1：显示投影图像
	int displayFiltered;	//0：不显示。1：显示过滤过滤图形 TODO：意义不明
	int dispalyTransEdge;	//0：不显示。1：显示转换图像上的边缘 TODO:意义不明

}sttCaliperInput;

//检测到的边缘的相关数据
typedef struct {
	//每个检测到的点对应一个结构体
	int index;			//投影的位置
	float x;				//图像中的位置，存在旋转时，需要旋转回图像坐标系。
	float y;				//图像中的位置，同上
	float score;			//边缘得分，TODO不知道确切的计算方法，暂时以对比度为得分。
	float contrast;			//对比度，TODO暂定为卷积值，卷积值即表示左右的差值。	

}sttSingleEdge;

typedef struct {




}sttCaliperResult;

