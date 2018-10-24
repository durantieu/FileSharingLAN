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
using System.IO.Pipes;

namespace PDSProjectGUI
{
    public partial class Share : Form
    {
        Credenziali cred;
        Login log;
        IntPtr connessione;
        bool cred_changed = false;
        bool closing;
        NamedPipeClientStream mainPipe;
        PollingPipe p;
        //Copiare la dll nella cartella di sistema windows\\system32
        [DllImport("FileSharingTest.dll")]
        public static extern IntPtr creaConnessione(string dati);

        [DllImport("FileSharingTest.dll")]
        public static extern void modPrivata(IntPtr connessione);

        private System.Windows.Forms.ContextMenu contextMenu1;
        private System.Windows.Forms.MenuItem menuItem1;
        private System.Windows.Forms.MenuItem menuItem2;

        private System.ComponentModel.IContainer components1;
        private SendFiles sf;
        public bool cred_change
        {
            get;
            set;
        }
        public Share(Login l)
        {
            closing = false;
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

            mainPipe = new NamedPipeClientStream(@"mainPipe");

            p = new PollingPipe(null, null, null, null, mainPipe, this, null, 1, true);


            //-------------------- BackgroundMode code
            this.menuItem1 = new System.Windows.Forms.MenuItem();
            this.menuItem2 = new System.Windows.Forms.MenuItem();

            this.components1 = new System.ComponentModel.Container();

            this.contextMenu1 = new System.Windows.Forms.ContextMenu();

            this.contextMenu1.MenuItems.AddRange(
                    new System.Windows.Forms.MenuItem[] { this.menuItem1, this.menuItem2 });



            this.menuItem1.Index = 0;
            this.menuItem1.Text = "Exit";
            this.menuItem1.Click += new System.EventHandler(this.menuItem1_Click);

            this.menuItem2.Index = 1;
            this.menuItem2.Text = "Modalità Privata";
            this.menuItem2.Click += new System.EventHandler(this.menuItem2_Click);

            if (cred.get_visibility() == "false")
            {
                menuItem2.Checked = true;
            }
            else
            {
                menuItem2.Checked = false;
            }


            notifyIcon1.ContextMenu = this.contextMenu1;



        }
        private void button2_Click(object sender, EventArgs e)
        {
            FileDialog fd = new FileDialog(connessione);
            fd.Show();
        }
        private void button1_Click(object sender, EventArgs e)
        {
            Settings set = new Settings(connessione, this, cred);
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
            if (e.CloseReason == CloseReason.WindowsShutDown || e.CloseReason == CloseReason.TaskManagerClosing)
            {
                try
                {
                    log.Close();

                }
                catch (NullReferenceException)
                {

                }
            }
            else
            {
                if (e.CloseReason == CloseReason.UserClosing)
                {

                    if (!closing)
                    {
                        this.Hide();
                        e.Cancel = true;
                        notifyIcon1.Visible = true;
                        notifyIcon1.Text = "Inizia a condividere!";
                        notifyIcon1.Icon = new Icon("C:\\Users\\duran\\FileSharing\\app-share-icon-1.ico");
                        notifyIcon1.DoubleClick += new System.EventHandler(this.notifyIcon1_DoubleClick);
                    }



                }
            }


        }
        private void panel2_Paint(object sender, PaintEventArgs e)
        {

        }




        private void notifyIcon1_DoubleClick(object sender, EventArgs e)
        {
            if (this.WindowState == FormWindowState.Minimized)
                this.WindowState = FormWindowState.Normal;


            this.Activate();
            this.Show();

            this.notifyIcon1.Visible = false;
        }

        private void menuItem1_Click(object sender, EventArgs e)
        {
            notifyIcon1.Visible = false;
            closing = true;
            this.Close();
        }
        private void menuItem2_Click(object sender, EventArgs e)
        {

            if (Convert.ToBoolean(cred.get_visibility()))
            {
                cred.set_visibility(false);
                modPrivata(connessione);
                menuItem2.Checked = true;

            }
            else
            {
                cred.set_visibility(true);
                modPrivata(connessione);
                menuItem2.Checked = false;
            }
        }


    }
}
