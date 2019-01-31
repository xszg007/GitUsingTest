#pragma once
typedef enum CaliperAffineCode {
	CA_ORI2PRJ = 0,							//0：被投影图像中4个顶点到原图像中的映射关系。
	CA_PRJ2ORI = 1,							//1：原图像到被投影图像的映射关系 
	CA_TOTALNUM = 2							// 参数总数，声明数组用
};


//错误编码

//第部分：
//3位，表示处理库的功能
//第一部分
//3位，表示该步骤中的具体错误
//错误代码= function×100 + 第三部分
//错误码第二部分具体定义

typedef enum ErrorCodeFunction
{
	EC_IMAGE_CONVERT		=  3000,
	EC_HISTOGRAM			=  4000,
	EC_AFFINE_TRANSFORM		=  5000,
	EC_BLOB					=  6000,
	EC_IPONEIMAGE			=  7000,
	EC_COPY_REGION			=  8000,
	EC_FIXTURE				=  9000,
	EC_CALIPER				= 10000,
	EC_FIND_CIRCLE			= 11000,
	EC_FIND_LINE			= 12000,
	EC_INTERSECT_LL			= 13000,
	EC_PMALIGN				= 14000,
	EC_SEARCH_MAX			= 15000,
	EC_CREATE_SEGMENT		= 16000,
	EC_DISTANCE_PL			= 17000,
	EC_ANGLE_LL				= 18000,
	EC_SOBEL_EDGE			= 19000,
	EC_DISTANCE_PC			= 20000,
	EC_DISTANCE_PP			= 21000,
};


//----------------------------------------------------------
//----------------------------------------------------------
//-----------------以下为卡尺寻边功能错误码-----------------
typedef enum ErrorCodeCaliper
{
	EC_CLPR_IN_SRCMAT = 1,
	EC_CLPR_CR_SKEWANGLE = 2,
	EC_CLPR_CR_ROIOUT = 3,
	EC_CLPR_CR_SKEWOUT = 4,
	EC_CLPR_CR_ROTOUT = 5,
	EC_CLPR_CALP_WIDTHOVER = 6,
	EC_CLPR_CONP_KERNEL = 7,
	EC_CLPR_SE_MAXNUM = 8,
	EC_CLPR_CR_SAMEPOSITION = 9,
};
