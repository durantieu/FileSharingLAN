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

        IntPtr connessione;
        string pipeID;
        NamedPipeClientStream pipe;

        public ProgressBarDialog(IntPtr connection, utente usr, string path)
        {
            
            StringBuilder str = new StringBuilder();
            connessione = connection;
            inviaFile(connessione, path, usr.MAC, str);
            pipeID = str.ToString();
            pipe = new NamedPipeClientStream(pipeID);
            pipe.Connect();

            InitializeComponent();

        }

        private void button1_Click(object sender, EventArgs e)
        {
            //Terminate the transfer by calling DLL function

            this.Close();
        }
      
    }
}
