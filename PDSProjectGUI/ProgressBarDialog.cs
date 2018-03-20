using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Runtime.InteropServices;
using System.IO.Pipes;

namespace PDSProjectGUI
{

    public partial class ProgressBarDialog : Form
    {
        [DllImport("FileSharingTest.dll", CharSet = CharSet.Ansi)]
        public static extern void inviaFile(IntPtr connessione, string file, string MAC, StringBuilder str);
        private string nome_file;
        IntPtr connessione;

        public delegate void delegateUpdateProgressBar(string value);


        private delegateUpdateProgressBar updateProgressBar;
        PollingPipe p;

        public ProgressBarDialog(IntPtr connection, utente usr, string path)
        {
            InitializeComponent();

            updateProgressBar = new delegateUpdateProgressBar(modify_progress_bar);

            StringBuilder str = new StringBuilder();
            connessione = connection;
            inviaFile(connessione, path, usr.MAC, str);

            progressBar1.Maximum = 100;
            progressBar1.Minimum = 0;
            progressBar1.MarqueeAnimationSpeed = 50;

            p = new PollingPipe(updateProgressBar, this, str.ToString(), 3); // 3 = pipe comunicazione GUI-Client
            
            nome_file = path;
            label3.Text = nome_file;
            label4.Text = usr.Nome + " " + usr.Cognome;
            

        }
        
        public void modify_progress_bar(string value)
        {
            progressBar1.Value = Int32.Parse(value);
        }

        private void button1_Click(object sender, EventArgs e)
        {
            //Terminate the transfer by calling DLL function

            this.Close();
        }
      
    }
}
