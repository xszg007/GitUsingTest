using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Forms;//手动添加dll文件
using System.IO;
using System.Runtime.InteropServices;


namespace DrLiDemo
{
    
    public partial class parameters
    {
        public struct sttCaliperOutput
        {
            public int index;           //投影的位置
            public float x;                //图像中的位置，存在旋转时，需要旋转回图像坐标系。
            public float y;                //图像中的位置，同上
            public float score;            //边缘得分，TODO不知道确切的计算方法，暂时以对比度为得分。
            public float contrast;			//对比度，TODO暂定为卷积值，卷积值即表示左右的差值。
        }
        public static sttCaliperOutput[] enClpOutputs;//实例化输出结构体

        //------与Interface.cpp对应，接收输出结果-------//
        [DllImport("TheCaliperTool.dll",EntryPoint = "getResult",CallingConvention =CallingConvention.StdCall)]
        public static extern void getResult(int num, IntPtr  resEdge);


        public const int MAX_PATH_STR_SIZE = 256;//定义保存文件路径的数组的最长size.
        public static string sAppPath;
        public static string sIniFilePath;


        [StructLayout(LayoutKind.Sequential)]//控制结构体成员的内存布局为顺序布局
      //定义结构体dlgParameters
        public struct dlgParameters
        {
            //所定义的内容都必须赋值，不然会报错
            //变量定义
            public int inputMethod;         //图像输入的方式 0:usb camera 1:local files

            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = MAX_PATH_STR_SIZE)]
            public string strInputFilePath;     //输入文件路径
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = MAX_PATH_STR_SIZE)]
            public string strSavePath;      //保存文件路径
            //以上两个数组用来向C++的dll中传递字符串
        }

        public static dlgParameters enDlgParameters;//上述定义的结构体实例化

//定义结构体卡尺工具
        public struct clpParameters
        {
            //定义.h文件中对应的变量，所有变量赋值。
            public int edgeModel;          // 0：单个边缘 1：边缘对
            public int edgePlrty;          // 0：由暗到明 1：由明到暗 2：任意极性
            public int edgeWidth;          // [0,roi宽度] TODO：边缘宽度上限 	

            public int contrastTH;         // [0,255] TODO：阈值上限需要调试
            public int filtHalfPixel;      // []	   TODO：意义不明？？	
            public int maxLineNum;          // [1,??]  TODO：roi中最大线的数量，上限？？？
                                            //-----------------区域部分-----------------
            public int useCaliperRoi;     // 0：使用卡尺ROI。1：使用全图

            public int imgSpace;           // 0：输入图像空间。1：像素空间。2：根空间。 TODO：意义不明
            public int roiMode;            // 0：原点模式。1：中心模式。2：3点模式

            //共同参数
            public double originX;         // 原点x坐标
            public double originY;         // 原点y坐标 

            //原点模式和中心模式参数相同，使用以下参数
            //TODO坐标是否用float型
            public double width;           // 宽度
            public double length;          // 长度	
            public float rotAngle;         // roi旋转角度
            public float skewAngle;        // roi错切角度

            //3点模式使用以下参数
            //TODO坐标顺序
            public double X1;              // 第二个顶点X
            public double Y1;              // 第二个顶点Y
            public double X2;              // 第三个顶点;
            public double Y2;              // 第三个顶点

            //-----------------记分部分----------------- 
            public int mapScore;           // 0:不启用 1：启用
            public int mapX0;              // TODO三个值的范围
            public int mapX1;
            public int mapXc;

            //-----------------图形部分----------------- 
            //TODO:显示图像的尺寸是否需要可调
            public int displayEdge;        //0：不显示。1：显示找到的边缘
            public int displayProjection;  //0：不显示。1：显示投影图像
            public int displayFiltered;    //0：不显示。1：显示过滤过滤图形 TODO：意义不明
            public int dispalyTransEdge;	//0：不显示。1：显示转换图像上的边缘 TODO:意义不明

            public int displaySteps;       //0:关闭过程显示 1：显示各个过程的图像
            public int writeLog;           //0:关闭输出日志文件 1：输出日志文件 
        }

        public static clpParameters enClpParameters;//实例化上述卡尺工具结构体

        //----------------将内存数据读到数组----------------//
        //输入值为前端输入的最大结果数
        //返回值为结构体数组


      // public static IntPtr pt = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(sttCaliperOutput)) * 50);//分配内存空间TODO是否与报错有关
        public static sttCaliperOutput[] Result2Array(int num)
        {
            IntPtr pt = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(sttCaliperOutput)) * (num +1));
            sttCaliperOutput[] a = new sttCaliperOutput[num];
           
            getResult(num, pt);
            //var b = Marshal.SizeOf(typeof(sttCaliperOutput));
            for (int i = 0; i < num; i++)
            {
                IntPtr ptr = (IntPtr)((UInt64)pt + (UInt64)(i * Marshal.SizeOf(typeof(sttCaliperOutput))));
                a[i] = (sttCaliperOutput)Marshal.PtrToStructure(ptr, typeof(sttCaliperOutput));

            }
            Marshal.FreeCoTaskMem(pt);
            return a;
        }
    }
}
