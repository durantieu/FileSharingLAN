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

namespace PDSProjectGUI
{
    public partial class Settings : Form
    {
        //la funzione C++ modPrivata non accetta nessun parametro, 
        //si occupa di invertire il valore booleano in struttura dat
        [DllImport("FileSharingTest.dll")]
        public static extern void modPrivata(IntPtr conn);

        [DllImport("FileSharingTest.dll")]
        public static extern void cambiaFilePath(IntPtr conn, string path);

        //[DllImport("C:\\Users\\duran\\Source\\Repos\\FileSharingTest2\\x64\\Debug\\FileSharingTest.dll")]
        [DllImport("FileSharingTest.dll")]
        public static extern void cambiaImmagine(IntPtr conn, string path);


        [DllImport("FileSharingTest.dll", CharSet = CharSet.Ansi)]
        public static extern void cambiaNome(IntPtr conn, string nome);

        [DllImport("FileSharingTest.dll", CharSet = CharSet.Ansi)]
        public static extern void cambiaCognome(IntPtr conn, string cognome);

        string path_fileTransfer;
        string path_immagineProfilo;
        bool visible = false;
        bool button2_changed = false;
        bool button3_changed = false;
        bool vis_changed = false;
        FolderBrowserDialog browse_fileTransfer = new FolderBrowserDialog();
        OpenFileDialog browse_immagineProfilo = new OpenFileDialog();
        IntPtr connessione;
        Credenziali credentials;
        private Share s = null;
        public Settings(IntPtr Connection, Share callingForm, Credenziali cred)
        {
            //prelevare e inizializzare il valore di Visible dal file credenziali
            s = callingForm;
            connessione = Connection;
            credentials = cred;
            if(cred.get_visibility() == "true")
            {
                visible = true;
            }
            else
            {
                visible = false;
            }

            InitializeComponent();
            set_private(visible);

            
        }

        void set_private(bool visible)
        {
            checkBox1.Checked = !visible;
            


        }
        private void button1_Click(object sender, EventArgs e)
        {

        }

        private void button2_Click(object sender, EventArgs e) //scegli cartella dove salvare i file
        {
            if (browse_fileTransfer.ShowDialog() == DialogResult.OK && !string.IsNullOrWhiteSpace(browse_fileTransfer.SelectedPath))
            {
                path_fileTransfer = browse_fileTransfer.SelectedPath;
                button2_changed = true;
            }
        }

        private void button3_Click(object sender, EventArgs e)
        {
            browse_immagineProfilo.Filter = "Image Files(*.BMP;*.JPG;*.GIF)|*.BMP;*.JPG;*.GIF";
            if (browse_immagineProfilo.ShowDialog() == DialogResult.OK)
            {
                path_immagineProfilo = browse_immagineProfilo.FileName;
                button3_changed = true;
               
            }
        }

        private void checkBox1_CheckedChanged(object sender, EventArgs e)
        {
            if (checkBox1.Checked)
            {
                visible = false;
                vis_changed = true;
            }
            else
            {
                visible = true;
                vis_changed = true;
            }
        }

        private void button5_Click(object sender, EventArgs e)
        {
            //tasto annulla... esci senza salvare

            this.Close();
        }

        private void button4_Click(object sender, EventArgs e)
        {
            // salvare i dati di impostazioni scrivendo nel file credenziali con i metodi della DLL
           /* if (button2_changed) {
                cambiaFilePath(connessione, path_fileTransfer);
                button2_changed = false;
            }
            if (button3_changed)
            {
                cambiaImmagine(connessione, path_immagineProfilo);
                button3_changed = false;
            }

            //cambiaNome();
            //cambiaCognome();


            s.update_credentials();

            this.Close();*/
        }

        private void panel1_Paint(object sender, PaintEventArgs e)
        {

        }

       

        public void credential_changed()
        {
            

        }

        private void button5_Click_1(object sender, EventArgs e)
        {
            this.Close();
        }

        private void button4_Click_1(object sender, EventArgs e)
        {
            // salvare i dati di impostazioni scrivendo nel file credenziali con i metodi della DLL
            if (button2_changed)
            {
                cambiaFilePath(connessione, path_fileTransfer);
                button2_changed = false;
            }
            if (button3_changed)
            {

                cambiaImmagine(connessione, path_immagineProfilo);
                button3_changed = false;
            }
            if (vis_changed)
            {

                modPrivata(connessione);
                vis_changed = false;
                
            }
            if(textBox1.Text != "")
            {
                cambiaNome(connessione, textBox1.Text);
            }
            if(textBox2.Text != "")
            {
                cambiaCognome(connessione, textBox2.Text);
            }



            s.update_credentials(this);

            
        }
    }
}
