using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using WindowsFormsApplication1;

namespace PDSProjectGUI
{
    public partial class Share : Form
    {

        IntPtr connessione;
        [DllImport("C:\\Users\\duran\\Source\\Repos\\FileSharingTest2\\x64\\Debug\\FileSharingTest.dll")]
        public static extern void creaConnessione();
        public Share()
        {

            InitializeComponent();
            
        }

        private void button2_Click(object sender, EventArgs e)
        {
            FileDialog fd = new FileDialog();
            fd.Show();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            Settings set = new Settings(connessione);
            set.Show();

        }
    }
}
