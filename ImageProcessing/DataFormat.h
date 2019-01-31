#pragma once
#define MAX_PATH_STR_SIZE 256
#define MAX_PIXELS_ARRAY_SIZE 1200000
//对话框参数
typedef struct
{

	int inputMethod;         //图像输入的方式 0:usb camera 1:local
	char strInputFilePath[MAX_PATH_STR_SIZE];     //输入文件路径
	char strSavePath[MAX_PATH_STR_SIZE];		//保存路径
}dlgParameters;
//定义结构体dlgParameters,在interface.cpp中实例化为enDlgPrmtrs，对应parameters.cs中实例化的的enDlgParameters

typedef struct
{
	//-----------------设置部分-----------------
	int edgeModel;			// 0：单个边缘 1：边缘对
	int edgePlrty;			// 0：由暗到明 1：由明到暗 2：任意极性
	int edgeWidth;			// [0,roi宽度] TODO：边缘宽度上限 	

	int contrastTH;			// [0,255] TODO：阈值上限需要调试
	int filtHalfPixel;		// []	   TODO：意义不明？？	
	int maxLineNum;			// [1,??]  TODO：roi中最大线的数量，上限？？？


	//-----------------区域部分-----------------
	int useCaliperRoi;      // 0：使用卡尺ROI。1：使用全图
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
	float skewAngle;		// roi错切角度

							//3点模式使用以下参数
							//TODO坐标顺序
	double X1;				// 第二个顶点X
	double Y1;				// 第二个顶点Y
	double X2;				// 第三个顶点;
	double Y2;				// 第三个顶点

	//-----------------记分部分----------------- 
	int mapScore;			// 0:不启用 1：启用
	int mapX0;				// TODO三个值的范围
	int mapX1;
	int mapXc;

	//-----------------图形部分----------------- 
	//TODO:显示图像的尺寸是否需要可调
	int displayEdge;		//0：不显示。1：显示找到的边缘
	int displayProjection;	//0：不显示。1：显示投影图像
	int displayFiltered;	//0：不显示。1：显示过滤过滤图形 TODO：意义不明
	int dispalyTransEdge;	//0：不显示。1：显示转换图像上的边缘 TODO:意义不明

	//-----------------调试参数-----------------
	int displaySteps;		//0:关闭过程显示 1：显示各个过程的图像
	int writeLog;		    //0:关闭过程显示 1：显示各个过程的图像
}//sttCaliperInput;
clpParameters;
