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
        [DllImport("C:\\Users\\duran\\Source\\Repos\\FileSharingTest2\\x64\\Debug\\FileSharingTest.dll")]
        public static extern void modPrivata(IntPtr conn);

        [DllImport("C:\\Users\\duran\\Source\\Repos\\FileSharingTest2\\x64\\Debug\\FileSharingTest.dll")]
        public static extern void cambiaFilePath(IntPtr conn, string path);

        [DllImport("C:\\Users\\duran\\Source\\Repos\\FileSharingTest2\\x64\\Debug\\FileSharingTest.dll")]
        public static extern void cambiaImmagine(IntPtr conn, string path);

        string path_fileTransfer;
        string path_immagineProfilo;
        bool visible = true;

        FolderBrowserDialog browse_fileTransfer = new FolderBrowserDialog();
        OpenFileDialog browse_immagineProfilo = new OpenFileDialog();
        IntPtr connessione;
        
        public Settings(IntPtr Connection)
        {
            InitializeComponent();
            //prelevare e inizializzare il valore di Visible dal file credenziali

            connessione = Connection;
            set_private(visible);
        }
        void set_private(bool visible)
        {
            checkBox1.Checked = visible;


        }
        private void button1_Click(object sender, EventArgs e)
        {

        }

        private void button2_Click(object sender, EventArgs e) //scegli cartella dove salvare i file
        {
            if (browse_fileTransfer.ShowDialog() == DialogResult.OK && !string.IsNullOrWhiteSpace(browse_fileTransfer.SelectedPath))
            {
                path_fileTransfer = browse_fileTransfer.SelectedPath;
            }
        }

        private void button3_Click(object sender, EventArgs e)
        {
            if (browse_immagineProfilo.ShowDialog() == DialogResult.OK)
            {
                path_immagineProfilo = browse_immagineProfilo.FileName;
               
            }
        }

       
        
        private void checkBox1_CheckedChanged(object sender, EventArgs e)
        {
            if (checkBox1.Checked)
            {
                visible = false;
                
            }
            else
            {
                visible = true;
            }
        }

        private void button5_Click(object sender, EventArgs e)
        {
            //operazioni per scrivere sul file Credenziali tutti i cambiamenti

            this.Close();
        }

        private void button4_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void panel1_Paint(object sender, PaintEventArgs e)
        {

        }
    }
}
