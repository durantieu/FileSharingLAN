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



        public IncomingConnection(string mac, string nomeUtente, string nPipe, Boolean acceptedDefault, Boolean rejectedDefault)
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
            String extLowerCase = ".jpg";
            String extUpperCase = ".JPG";
            try
            {
               // pictureBox1.Image = Image.FromFile(path_immagini + "Immagini_utenti\\" + mac + extLowerCase);
                Image img;
                using (var bmpTemp = new Bitmap(path_immagini + "Immagini_utenti\\" + mac + extLowerCase))
                {
                    img = new Bitmap(bmpTemp);
                    pictureBox1.Image = img;
                }
            }
            catch(Exception e)
            {
                try
                {
                   // pictureBox1.Image = Image.FromFile(path_immagini + "Immagini_utenti\\" + mac + extLowerCase + extUpperCase );
                    Image img;
                    using (var bmpTemp = new Bitmap(path_immagini + "Immagini_utenti\\" + mac + extUpperCase))
                    {
                        img = new Bitmap(bmpTemp);
                        pictureBox1.Image = img;
                    }

                }
                catch(Exception exception)
                {
                    this.Close();
                }
               
            }
           
            pictureBox1.SizeMode = PictureBoxSizeMode.StretchImage;
            label4.Text = nomeUtente;
            string pipeStr = "pipe" + pipeID;

            if (rejectedDefault)
            {
                pbdRic = new ProgressBarDialogRic(utente, pipeID, false);
                this.Close();
            }
            if (acceptedDefault)
            {
                pbdRic = new ProgressBarDialogRic(utente, pipeID, true);
                this.Close();

            }
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
