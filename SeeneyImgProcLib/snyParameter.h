#include "pch.h"

//���߹���ǰ����������ṹ��
typedef struct{

	//-----------------���ò���-----------------
	int edgeModel;			// 0��������Ե 1����Ե��
	int edgePlrty;			// 0���ɰ����� 1���������� 2�����⼫��
	int edgeWidth;			// �� edgeModel==1 ʱ��[0,roi���] TODO����Ե������� 	

	int contrastTH;			// [0,255] TODO����ֵ������Ҫ����
	int filtHalfPixel;		// []	   TODO�����岻������	
	int maxLineNum;			// [1,??]  TODO��roi������ߵ����������ޣ�����

	//-----------------���򲿷�-----------------
	int useCaliperROI;		// 0��ʹ�ÿ���������Ϊroi��1��ȫͼ��Ϊroi
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
	float skewAngle;		// roi���нǶȣ�ԭ�㴹ֱ���£���ʱ��Ϊ���Ƕȣ�˳ʱ��Ϊ���Ƕȡ�

	//3��ģʽʹ�����²���
	//TODO����˳��
	double X1;				// �ڶ�������X
	double Y1;				// �ڶ�������Y
	double X2;				// ����������;
	double Y2;				// ����������

	//-----------------�Ƿֲ���----------------- 
	int mapScore;			// 0:������ 1������
	float mapX0;			// TODO����ֵ�ķ�Χ,Ӧ����[0,1]
	float mapX1;
	float mapXc;

	//-----------------ͼ�β���----------------- 
	//TODO:��ʾͼ��ĳߴ��Ƿ���Ҫ�ɵ�
	int displayEdge;		//0������ʾ��1����ʾ�ҵ��ı�Ե
	int displayProjection;	//0������ʾ��1����ʾͶӰͼ��
	int displayFiltered;	//0������ʾ��1����ʾ���˹���ͼ�� TODO�����岻��
	int dispalyTransEdge;	//0������ʾ��1����ʾת��ͼ���ϵı�Ե TODO:���岻��

}sttCaliperInput;

//��⵽�ı�Ե���������
typedef struct {
	//ÿ����⵽�ĵ��Ӧһ���ṹ��
	int index;			//ͶӰ��λ��
	float x;				//ͼ���е�λ�ã�������תʱ����Ҫ��ת��ͼ������ϵ��
	float y;				//ͼ���е�λ�ã�ͬ��
	float score;			//��Ե�÷֣�TODO��֪��ȷ�еļ��㷽������ʱ�ԶԱȶ�Ϊ�÷֡�
	float contrast;			//�Աȶȣ�TODO�ݶ�Ϊ���ֵ�����ֵ����ʾ���ҵĲ�ֵ��	

}sttSingleEdge;

typedef struct {




}sttCaliperResult;

