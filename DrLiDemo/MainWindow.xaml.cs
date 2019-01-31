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
using System.Runtime.InteropServices;//调用kernel32.dll
using OpenCvSharp;//调用OpenCV

namespace DrLiDemo
{
    /// <summary>
    /// MainWindow.xaml 的交互逻辑
    /// </summary>
    public partial class MainWindow : System.Windows.Window
    {
     
        [DllImport("kernel32")]
        private static extern long WritePrivateProfileString(string section, string key, string value, string filePath);

        // 读操作  
        [DllImport("kernel32")]
        private static extern int GetPrivateProfileString(string section, string key, string def, StringBuilder retVal, int size, string filePath);

        [DllImport("TheCaliperTool.dll")]
        private static extern int interface_getDlgParameter(ref parameters.dlgParameters info);//调用.dll文件中Interface.cpp中的函数定义

        [DllImport("TheCaliperTool.dll")]
        private static extern int interface_getClpParameter(ref parameters.clpParameters info);//调用.dll文件中的函数定义

        [DllImport("TheCaliperTool.dll")]
        private static extern int interface_Main();
      

        [DllImport("TheCaliperTool.dll")]
        private static extern int interface_Caliper();
      
        public MainWindow()
        {
            

            InitializeComponent();

            this.Left = (SystemParameters.PrimaryScreenWidth - this.Width) / 2;
            this.Top = SystemParameters.PrimaryScreenHeight / 8;
            parameters.sAppPath = System.Environment.CurrentDirectory;
            parameters.sIniFilePath = parameters.sAppPath + "\\Config.ini";
            //获得当前目录,创建ini文件路径
            if (!File.Exists(parameters.sIniFilePath))
            {
                //如果不存在
                System.Windows.MessageBox.Show("配置文件不存在，创建配置文件");
                FileStream ConfigFile = new FileStream(@parameters.sIniFilePath, FileMode.OpenOrCreate, FileAccess.ReadWrite);//创建Config.ini
                //TODO:文件保存权限等引起的创建异常
                ConfigFile.Close();
            }
            else
            {
                //如果存在，读取参数并写入对话框
                writeIni2Dlg();
                //读取算法参数，并写入相应结构体
                //writeIni2ParaStrct();           
            }
            //string item = ReadIni(parameters.sIniFilePath, "Dialog_Parameters", "Combo_Items");
            //System.Windows.MessageBox.Show(item);

        }

        private void bttn_MW_InputFile_Click(object sender, RoutedEventArgs e)
        {
            Microsoft.Win32.OpenFileDialog inputFile = new Microsoft.Win32.OpenFileDialog();//实例化OpenFileDialog类，并命名为inputFile
            inputFile.Filter = "All files(*.*)|*.*";//设置过滤器限制可以读取的文件，这里不做限制，可以读取任何 格式的文件。
            Nullable<bool> result = inputFile.ShowDialog();//.ShowDialog显示通用对话框，并将结果赋值给布尔型变量result
            if (result == true)
              {
                // Open document
                string filename = inputFile.FileName;
                //文件名
                txtBx_MW_InputFile.Text = System.IO.Path.GetFullPath(inputFile.FileName);//获得文件完整路径并给textBox控件赋值，即路径显示到textBox控件上。
                //public static string filepath =;
                //getImg();//读取图像内容，获得尺寸及像素值

                //writeImg2Structure(ref parameters.enImgParameters);//图像信息传入结构体
                //imgtest(ref parameters.enImgParameters);//定义3行4列的数组并赋值1-6，传入结构体
                //interface_Img2cppParameter(ref parameters.enImgParameters);//图像结构体信息传入dll
               }

        }

        private void bttn_MW_SavePath_Click(object sender, RoutedEventArgs e)
        {
            FolderBrowserDialog Dialog = new FolderBrowserDialog();
            DialogResult result = Dialog.ShowDialog();

            if (result == System.Windows.Forms.DialogResult.Cancel)
            {
                return;
            }
            string m_Dir = Dialog.SelectedPath.Trim();
            this.txtBx_MW_SavePath.Text = m_Dir;

        }

        private void bttn_MW_Start_Click(object sender, RoutedEventArgs e)
        {
          
            writeDlg2Structure(ref parameters.enDlgParameters);//对话框信息传入结构体（parameters.cs中实例化的enDlgParameters）
            interface_getDlgParameter(ref parameters.enDlgParameters);//结构体信息传入dll
            //interface_getClpParameter(ref parameters.enClpParameters);//
            writeDlg2Ini();
            interface_Main();
            //getDlgParameters(ref parameters.enDlgParameters);
        }
      
        unsafe private void bttn_MW_Caliper_Click(object sender, RoutedEventArgs e)
        {
            
            writeDlg2Structure(ref parameters.enDlgParameters);//对话框信息传入结构体（parameters.cs中实例化的enDlgParameters）
            writeClp2Structure(ref parameters.enClpParameters);
            int maxEdgeNum = 0;
            int.TryParse(textBox_MW_maxLineNum.Text, out maxEdgeNum);
            interface_getDlgParameter(ref parameters.enDlgParameters);//结构体信息传入dll
            interface_getClpParameter(ref parameters.enClpParameters);
            writeDlg2Ini();
            interface_Main();
            parameters.sttCaliperOutput[] a= new parameters.sttCaliperOutput[maxEdgeNum];
            try
            {
                a = parameters.Result2Array(int.Parse(textBox_MW_maxLineNum.Text));//输出结构体数组
            }
            catch (Exception ex)
            {

                Console.WriteLine(ex.Message);
            }
           
            

        }
        private void bttn_MW_Close_Click(object sender, RoutedEventArgs e)
        {
            writeDlg2Ini();
            System.Windows.Application.Current.Shutdown();//关闭程序功能。
        }
        private void writeIni2Dlg()
        {
            //配置文件信息写入对话框
            txtBx_MW_InputFile.Text = ReadIni(parameters.sIniFilePath, "Dialog_Parameters", "Input_Path");
            txtBx_MW_SavePath.Text = ReadIni(parameters.sIniFilePath, "Dialog_Parameters", "Save_Path");
            textBox_MW_edgeModel.Text = ReadIni(parameters.sIniFilePath, "Dialog_Parameters", "edgeModel");
            textBox_MW_edgePlrty.Text = ReadIni(parameters.sIniFilePath, "Dialog_Parameters", "edgePlrty");
            textBox_MW_edgeWidth.Text = ReadIni(parameters.sIniFilePath, "Dialog_Parameters", "edgeWidth");
            textBox_MW_contrastTH.Text = ReadIni(parameters.sIniFilePath, "Dialog_Parameters", "contrastTH");
            textBox_MW_filtHalfPixel.Text = ReadIni(parameters.sIniFilePath, "Dialog_Parameters", "filtHalfPixel");
            textBox_MW_maxLineNum.Text = ReadIni(parameters.sIniFilePath, "Dialog_Parameters", "maxLineNum");
            textBox_MW_useCaliperRoi.Text = ReadIni(parameters.sIniFilePath, "Dialog_Parameters", "useCaliperRoi");
            textBox_MW_imgSpace.Text = ReadIni(parameters.sIniFilePath, "Dialog_Parameters", "imgSpace");
            textBox_MW_roiMode.Text = ReadIni(parameters.sIniFilePath, "Dialog_Parameters", "roiMode");
            textBox_MW_originX.Text = ReadIni(parameters.sIniFilePath, "Dialog_Parameters", "originX");
            textBox_MW_originY.Text = ReadIni(parameters.sIniFilePath, "Dialog_Parameters", "originY");
            textBox_MW_length.Text = ReadIni(parameters.sIniFilePath, "Dialog_Parameters", "length");
            textBox_MW_width.Text = ReadIni(parameters.sIniFilePath, "Dialog_Parameters", "width");
            textBox_MW_rotAngle.Text = ReadIni(parameters.sIniFilePath, "Dialog_Parameters", "rotAngle");
            textBox_MW_skewAngle.Text = ReadIni(parameters.sIniFilePath, "Dialog_Parameters", "skewAngle");
            textBox_MW_X1.Text = ReadIni(parameters.sIniFilePath, "Dialog_Parameters", "X1");
            textBox_MW_Y1.Text = ReadIni(parameters.sIniFilePath, "Dialog_Parameters", "Y1");
            textBox_MW_X2.Text = ReadIni(parameters.sIniFilePath, "Dialog_Parameters", "X2");
            textBox_MW_Y2.Text = ReadIni(parameters.sIniFilePath, "Dialog_Parameters", "Y2");
            textBox_MW_mapX0.Text = ReadIni(parameters.sIniFilePath, "Dialog_Parameters", "mapX0");
            textBox_MW_mapX1.Text = ReadIni(parameters.sIniFilePath, "Dialog_Parameters", "mapX1");
            textBox_MW_mapXc.Text = ReadIni(parameters.sIniFilePath, "Dialog_Parameters", "mapXc");
            textBox_MW_mapScore.Text = ReadIni(parameters.sIniFilePath, "Dialog_Parameters", "mapScore");
            textBox_MW_displayEdge.Text = ReadIni(parameters.sIniFilePath, "Dialog_Parameters", "displayEdge");
            textBox_MW_displayProjection.Text = ReadIni(parameters.sIniFilePath, "Dialog_Parameters", "displayProjection");
            textBox_MW_displayFiltered.Text = ReadIni(parameters.sIniFilePath, "Dialog_Parameters", "displayFiltered");
            textBox_MW_dispalyTransEdge.Text = ReadIni(parameters.sIniFilePath, "Dialog_Parameters", "dispalyTransEdge");

            string item= ReadIni(parameters.sIniFilePath, "Dialog_Parameters", "Combo_Items");
            //string titem = item.Replace("System.Windows.Controls.ComboBoxItem:","");
            string titem = "Camera";
            if (item.Contains(titem))
                cmbBx_MW_InputMethod.SelectedIndex = 0;
            else
                cmbBx_MW_InputMethod.SelectedIndex = 1;
            //System.Windows.MessageBox.Show(titem);
            //cmbBx_MW_InputMethod = ReadIni(parameters.sIniFilePath, "Dialog_Parameters", "InputMethod");
        }
        private void writeDlg2Ini()
        {
            //foreach (var item in MainWindow.)
           
            WritePrivateProfileString("Dialog_Parameters", "Input_Path", txtBx_MW_InputFile.Text, parameters.sIniFilePath);
            WritePrivateProfileString("Dialog_Parameters", "Save_Path", txtBx_MW_SavePath.Text, parameters.sIniFilePath);

            WritePrivateProfileString("Dialog_Parameters", "edgeModel", textBox_MW_edgeModel.Text, parameters.sIniFilePath);
            WritePrivateProfileString("Dialog_Parameters", "edgePlrty", textBox_MW_edgePlrty.Text, parameters.sIniFilePath);
            WritePrivateProfileString("Dialog_Parameters", "edgeWidth", textBox_MW_edgeWidth.Text, parameters.sIniFilePath);
            WritePrivateProfileString("Dialog_Parameters", "contrastTH", textBox_MW_contrastTH.Text, parameters.sIniFilePath);
            WritePrivateProfileString("Dialog_Parameters", "filtHalfPixel", textBox_MW_filtHalfPixel.Text, parameters.sIniFilePath);
            WritePrivateProfileString("Dialog_Parameters", "maxLineNum", textBox_MW_maxLineNum.Text, parameters.sIniFilePath);
            WritePrivateProfileString("Dialog_Parameters", "useCaliperRoi", textBox_MW_useCaliperRoi.Text, parameters.sIniFilePath);
            WritePrivateProfileString("Dialog_Parameters", "imgSpace", textBox_MW_imgSpace.Text, parameters.sIniFilePath);
            WritePrivateProfileString("Dialog_Parameters", "roiMode", textBox_MW_roiMode.Text, parameters.sIniFilePath);
            WritePrivateProfileString("Dialog_Parameters", "originX", textBox_MW_originX.Text, parameters.sIniFilePath);
            WritePrivateProfileString("Dialog_Parameters", "originY", textBox_MW_originY.Text, parameters.sIniFilePath);
            WritePrivateProfileString("Dialog_Parameters", "length", textBox_MW_length.Text, parameters.sIniFilePath);
            WritePrivateProfileString("Dialog_Parameters", "width", textBox_MW_width.Text, parameters.sIniFilePath);
            WritePrivateProfileString("Dialog_Parameters", "rotAngle", textBox_MW_rotAngle.Text, parameters.sIniFilePath);
            WritePrivateProfileString("Dialog_Parameters", "skewAngle", textBox_MW_skewAngle.Text, parameters.sIniFilePath);
            WritePrivateProfileString("Dialog_Parameters", "X1", textBox_MW_X1.Text, parameters.sIniFilePath);
            WritePrivateProfileString("Dialog_Parameters", "Y1", textBox_MW_Y1.Text, parameters.sIniFilePath);
            WritePrivateProfileString("Dialog_Parameters", "X2", textBox_MW_X2.Text, parameters.sIniFilePath);
            WritePrivateProfileString("Dialog_Parameters", "Y2", textBox_MW_Y2.Text, parameters.sIniFilePath);
            WritePrivateProfileString("Dialog_Parameters", "mapX0", textBox_MW_mapX0.Text, parameters.sIniFilePath);
            WritePrivateProfileString("Dialog_Parameters", "mapX1", textBox_MW_mapX1.Text, parameters.sIniFilePath);
            WritePrivateProfileString("Dialog_Parameters", "mapXc", textBox_MW_mapXc.Text, parameters.sIniFilePath);
            WritePrivateProfileString("Dialog_Parameters", "mapScore", textBox_MW_mapScore.Text, parameters.sIniFilePath);
            WritePrivateProfileString("Dialog_Parameters", "displayEdge", textBox_MW_displayEdge.Text, parameters.sIniFilePath);
            WritePrivateProfileString("Dialog_Parameters", "displayProjection", textBox_MW_displayProjection.Text, parameters.sIniFilePath);
            WritePrivateProfileString("Dialog_Parameters", "displayFiltered", textBox_MW_displayFiltered.Text, parameters.sIniFilePath);
            WritePrivateProfileString("Dialog_Parameters", "dispalyTransEdge", textBox_MW_dispalyTransEdge.Text, parameters.sIniFilePath);
           
            WritePrivateProfileString("Dialog_Parameters", "Combo_Items", cmbBx_MW_InputMethod.SelectedItem.ToString(), parameters.sIniFilePath);
        }
        public static string ReadIni(string Path, string section, string key)//自定义函数：读取ini文件
        {
            StringBuilder temp = new StringBuilder(255);
            int i = GetPrivateProfileString(section, key, "", temp, 255, Path);
            return temp.ToString();
        }
        private void writeDlg2Structure(ref parameters.dlgParameters dst)
        {
            dst.inputMethod = cmbBx_MW_InputMethod.SelectedIndex;
            dst.strInputFilePath = txtBx_MW_InputFile.Text;
            dst.strSavePath = txtBx_MW_SavePath.Text;
        }

        private void writeClp2Structure(ref parameters.clpParameters dst)
        {
            int edgeModel = 0;
            int.TryParse(textBox_MW_edgeModel.Text,out edgeModel);
            dst.edgeModel = edgeModel;

            int edgePlrty = 0;
            int.TryParse(textBox_MW_edgePlrty.Text, out edgePlrty);
            dst.edgePlrty = edgePlrty;

            int edgeWidth = 0;
            int.TryParse(textBox_MW_edgeWidth.Text, out edgeWidth);
            dst.edgeWidth = edgeWidth;

            int contrastTH = 0;
            int.TryParse(textBox_MW_contrastTH.Text, out contrastTH);
            dst.contrastTH = contrastTH;

            int filtHalfPixel = 0;
            int.TryParse(textBox_MW_filtHalfPixel.Text, out filtHalfPixel);
            dst.filtHalfPixel = filtHalfPixel;

            int maxLineNum = 0;
            int.TryParse(textBox_MW_maxLineNum.Text, out maxLineNum);
            dst.maxLineNum = maxLineNum;

            int useCaliperRoi = 0;
            int.TryParse(textBox_MW_useCaliperRoi.Text, out useCaliperRoi);
            dst.useCaliperRoi = useCaliperRoi;

            int roiMode = 0;
            int.TryParse(textBox_MW_roiMode.Text, out roiMode);
            dst.roiMode = roiMode;

            double originX = 0;
            double.TryParse(textBox_MW_originX.Text, out originX);
            dst.originX = originX;

            double originY = 0;
            double.TryParse(textBox_MW_originY.Text, out originY);
            dst.originY = originY;

            double width = 0;
            double.TryParse(textBox_MW_width.Text, out width);
            dst.width = width;

            double length = 0;
            double.TryParse(textBox_MW_length.Text, out length);
            dst.length = length;

            float rotAngle = 0;
            float.TryParse(textBox_MW_rotAngle.Text, out rotAngle);
            dst.rotAngle = rotAngle;

            float skewAngle = 0;
            float.TryParse(textBox_MW_skewAngle.Text, out skewAngle);
            dst.skewAngle = skewAngle;

            double X1 = 0;
            double.TryParse(textBox_MW_X1.Text, out X1);
            dst.X1 = X1;

            double Y1 = 0;
            double.TryParse(textBox_MW_Y1.Text, out Y1);
            dst.Y1 = Y1;

            double X2 = 0;
            double.TryParse(textBox_MW_X2.Text, out X2);
            dst.X2 = X2;

            double Y2 = 0;
            double.TryParse(textBox_MW_Y2.Text, out Y2);
            dst.Y2 = Y2;

            int mapScore = 0;
            int.TryParse(textBox_MW_mapScore.Text, out mapScore);
            dst.mapScore = mapScore;

            int mapX0 = 0;
            int.TryParse(textBox_MW_mapX0.Text, out mapX0);
            dst.mapX0 = mapX0;

            int mapX1 = 0;
            int.TryParse(textBox_MW_mapX1.Text, out mapX1);
            dst.mapX1 = mapX1;

            int mapXc = 0;
            int.TryParse(textBox_MW_mapXc.Text, out mapXc);
            dst.mapXc = mapXc;

            int displayEdge = 0;
            int.TryParse(textBox_MW_displayEdge.Text, out displayEdge);
            dst.displayEdge = displayEdge;

            int displayProjection = 0;
            int.TryParse(textBox_MW_displayProjection.Text, out displayProjection);
            dst.displayProjection = displayProjection;

            int displayFiltered = 0;
            int.TryParse(textBox_MW_displayFiltered.Text, out displayFiltered);
            dst.displayFiltered = displayFiltered;

            int dispalyTransEdge = 0;
            int.TryParse(textBox_MW_dispalyTransEdge.Text, out dispalyTransEdge);
            dst.dispalyTransEdge = dispalyTransEdge;

            dst.displaySteps = 1;//调试参数，0:关闭过程显示 1：显示各个过程的图像
            dst.writeLog = 1;//0:关闭输出日志文件 1：输出日志文件
        }

        private void getImg()
        {
            string filepth = ReadIni(parameters.sIniFilePath, "Dialog_Parameters", "Input_Path");
            Mat src = new Mat(filepth, ImreadModes.Grayscale);
            Cv2.ImShow("src image", src);//图像显示在弹窗
            int row = src.Rows;//行数row
            int col = src.Cols;//列数col
            int[,] pixels = new int[row, col];//定义数组存放像素
            for (int i = 0; i < row; i++)//遍历每个像素
            {
                for (int j = 0; j < col; j++)
                {
                    pixels[i, j] = src.At<int>(i, j);
                }
            }
        }
    }
}
