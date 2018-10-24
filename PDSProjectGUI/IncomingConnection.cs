using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;
using System.Windows.Forms;
using System.IO.Pipes;

namespace PDSProjectGUI
{
    public partial class IncomingConnection : Form
    {

        [DllImport("FileSharingTest.dll", CharSet = CharSet.Ansi)]
        public static extern void firstGetHomeDir(StringBuilder str);

        NamedPipeClientStream pipeServer;
        PollingPipe pp;
        string MAC, utente, pipeID;
        bool accettato;
        ProgressBarDialogRic pbdRic;
        String path_immagini;



        public IncomingConnection(string mac, string nomeUtente, string nPipe)
        {
            MAC = mac;
            utente = nomeUtente;
            pipeID = nPipe;
            StringBuilder sb = new StringBuilder();
            firstGetHomeDir(sb);

            path_immagini = sb.ToString();


            InitializeComponent();
            System.Drawing.Drawing2D.GraphicsPath gp = new System.Drawing.Drawing2D.GraphicsPath();
            gp.AddEllipse(0, 0, pictureBox1.Width - 3, pictureBox1.Height - 3);
            Region rg = new Region(gp);
            pictureBox1.Region = rg;

            pictureBox1.Image = Image.FromFile(path_immagini + "Immagini_utenti\\"+ mac+".jpg");
            pictureBox1.SizeMode = PictureBoxSizeMode.StretchImage;
            label4.Text = nomeUtente;
            string pipeStr = "pipe" + pipeID;
        }

        private void AcceptButton_Click(object sender, EventArgs e)
        {           
            pbdRic = new ProgressBarDialogRic(utente, pipeID, true);

            this.Close();
        }

        private void DenyButton_Click(object sender, EventArgs e)
        {
            pbdRic = new ProgressBarDialogRic(utente, pipeID, false);

            this.Close();
        }
    }
}
