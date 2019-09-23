using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace PDSProjectGUI
{
    public partial class FileDialog : Form
    {
       
       
        string path = "";
        bool checkBox;
        IntPtr Conn;
        Share dad;
        public FileDialog(IntPtr conn, Share padre)
        {
            dad = padre;
            Conn = conn;
            InitializeComponent();
        }
        private void FileDialog_Load(object sender, EventArgs e)
        {

        }

        private void button1_Click(object sender, EventArgs e)
        {

            if(checkBox == true)
            {
                FolderBrowserDialog fbd = new FolderBrowserDialog();
                if (fbd.ShowDialog() == DialogResult.OK && !string.IsNullOrWhiteSpace(fbd.SelectedPath))
                {
                    path = fbd.SelectedPath;
                    textBox1.Text = fbd.SelectedPath;
                }
            }
            else
            {
                OpenFileDialog ofd = new OpenFileDialog();
                if (ofd.ShowDialog() == DialogResult.OK)
                {
                    path = ofd.FileName;
                    textBox1.Text = path;
                    textBox2.Text = ofd.SafeFileName;
                }
            }
            

        }

        private void button2_Click(object sender, EventArgs e) // "Ok" Button
        {
            if (path.CompareTo("") != 0) {
                this.Hide();
               

                SendFiles sf = new SendFiles(path, Conn, textBox1.Text, dad, false);
                sf.Show();
            }
            
            
            //passare il valore 
        }

       public string get_path()
        {
            return path;
        }

        private void checkBox1_CheckedChanged(object sender, EventArgs e)
        {
            if(checkBox1.Checked == true)
            {
                checkBox = true;
            }
            else
            {
                checkBox = false;
            }
        }

        private void button3_Click(object sender, EventArgs e)
        {
            this.Hide();
        }
    }
}
