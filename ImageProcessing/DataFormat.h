#pragma once
#define MAX_PATH_STR_SIZE 256
#define MAX_PIXELS_ARRAY_SIZE 1200000
//�Ի������
typedef struct
{

	int inputMethod;         //ͼ������ķ�ʽ 0:usb camera 1:local
	char strInputFilePath[MAX_PATH_STR_SIZE];     //�����ļ�·��
	char strSavePath[MAX_PATH_STR_SIZE];		//����·��
}dlgParameters;
//����ṹ��dlgParameters,��interface.cpp��ʵ����ΪenDlgPrmtrs����Ӧparameters.cs��ʵ�����ĵ�enDlgParameters

typedef struct
{
	//-----------------���ò���-----------------
	int edgeModel;			// 0��������Ե 1����Ե��
	int edgePlrty;			// 0���ɰ����� 1���������� 2�����⼫��
	int edgeWidth;			// [0,roi���] TODO����Ե������� 	

	int contrastTH;			// [0,255] TODO����ֵ������Ҫ����
	int filtHalfPixel;		// []	   TODO�����岻������	
	int maxLineNum;			// [1,??]  TODO��roi������ߵ����������ޣ�����


	//-----------------���򲿷�-----------------
	int useCaliperRoi;      // 0��ʹ�ÿ���ROI��1��ʹ��ȫͼ
	int imgSpace;			// 0������ͼ��ռ䡣1�����ؿռ䡣2�����ռ䡣 TODO�����岻��
	int roiMode;			// 0��ԭ��ģʽ��1������ģʽ��2��3��ģʽ

							//��ͬ����
	double originX;			// ԭ��x����
	double originY;			// ԭ��y���� 

							//ԭ��ģʽ������ģʽ������ͬ��ʹ�����²���
							//TODO�����Ƿ���float��
	double width;			// ���
	double length;			// ����	
	float rotAngle;			// roi��ת�Ƕ�
	float skewAngle;		// roi���нǶ�

							//3��ģʽʹ�����²���
							//TODO����˳��
	double X1;				// �ڶ�������X
	double Y1;				// �ڶ�������Y
	double X2;				// ����������;
	double Y2;				// ����������

	//-----------------�Ƿֲ���----------------- 
	int mapScore;			// 0:������ 1������
	int mapX0;				// TODO����ֵ�ķ�Χ
	int mapX1;
	int mapXc;

	//-----------------ͼ�β���----------------- 
	//TODO:��ʾͼ��ĳߴ��Ƿ���Ҫ�ɵ�
	int displayEdge;		//0������ʾ��1����ʾ�ҵ��ı�Ե
	int displayProjection;	//0������ʾ��1����ʾͶӰͼ��
	int displayFiltered;	//0������ʾ��1����ʾ���˹���ͼ�� TODO�����岻��
	int dispalyTransEdge;	//0������ʾ��1����ʾת��ͼ���ϵı�Ե TODO:���岻��

	//-----------------���Բ���-----------------
	int displaySteps;		//0:�رչ�����ʾ 1����ʾ�������̵�ͼ��
	int writeLog;		    //0:�رչ�����ʾ 1����ʾ�������̵�ͼ��
}//sttCaliperInput;
clpParameters;
