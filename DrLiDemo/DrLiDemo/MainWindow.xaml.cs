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
    /// <summary>
    /// MainWindow.xaml 的交互逻辑
    /// </summary>
    public partial class MainWindow : Window
    {
        //写操作
        [DllImport("kernel32")]
        private static extern long WritePrivateProfileString(string section, string key, string value, string filePath);
        // 读操作  
        [DllImport("kernel32")]
        private static extern int GetPrivateProfileString(string section, string key, string def, StringBuilder retVal, int size, string filePath);

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
                FileStream ConfigFile = new FileStream(@parameters.sIniFilePath, FileMode.OpenOrCreate, FileAccess.ReadWrite);
                //TODO:文件保存权限等引起的创建异常
                ConfigFile.Close();
            }
           else
            {
                writeIni2Dlg();
            }


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
            }

        }

        private void button_Click(object sender, RoutedEventArgs e)
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
            writeDlg2Ini();
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

        }
        private void writeDlg2Ini()
        {
            WritePrivateProfileString("Dialog_Parameters", "Input_Path", txtBx_MW_InputFile.Text, parameters.sIniFilePath);
            WritePrivateProfileString("Dialog_Parameters", "Save_Path", txtBx_MW_SavePath.Text, parameters.sIniFilePath);

        }
        // 自定义读ini文件  
        public static string ReadIni(string Path, string section, string key)
        {
            StringBuilder temp = new StringBuilder(255);
            int i = GetPrivateProfileString(section, key, "", temp, 255, Path);
            return temp.ToString();
        }

    }
}
