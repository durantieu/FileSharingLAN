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
        Credenziali cred;
        Login log;
        IntPtr connessione;
        bool cred_changed = false;
        //Copiare la dll nella cartella di sistema windows\\system32
        [DllImport("FileSharingTest.dll")]
        public static extern IntPtr creaConnessione(string dati);

        public bool cred_change
        {
            get;
            set;
        }

        public Share(Login l)
        {

            InitializeComponent();
            log = l;
            cred = l.get_credentials();
            connessione = creaConnessione(cred.ToString());
            label4.Text = cred.get_nome();
            label5.Text = cred.get_cognome();
            pictureBox2.ImageLocation = cred.get_immagine_profilo();

            System.Drawing.Drawing2D.GraphicsPath gp = new System.Drawing.Drawing2D.GraphicsPath();
            gp.AddEllipse(0, 0, pictureBox2.Width - 3, pictureBox2.Height - 3);
            Region rg = new Region(gp);
            pictureBox2.Region = rg;

        }

        private void button2_Click(object sender, EventArgs e)
        {
            FileDialog fd = new FileDialog(connessione);
            fd.Show();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            Settings set = new Settings(connessione, this);
            set.Show();



        }
        public void update_credentials(Settings form)
        {
            log.fill_credentials();
            cred = log.get_credentials();
            label4.Text = cred.get_nome();
            label5.Text = cred.get_cognome();
            pictureBox2.Image = System.Drawing.Image.FromFile(cred.get_immagine_profilo());
 
            pictureBox2.SizeMode = PictureBoxSizeMode.StretchImage;
            pictureBox2.Refresh();
            pictureBox2.Update();
            
            form.Close();

        }
        private void Share_FormClosing(object sender, FormClosingEventArgs e)
        {
            try
            {
                log.Close();

            }
            catch (NullReferenceException)
            {
                
            }
            
        }

        private void panel2_Paint(object sender, PaintEventArgs e)
        {

        }

        
    }
}
