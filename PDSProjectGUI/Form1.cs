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
    public partial class Form1 : Form
    {
        private IntPtr connessione;

        [DllImport("C:\\Users\\Mattia\\Source\\Repos\\FileSharingTest2\\x64\\Debug\\FileSharingTest.dll")]
        public static extern IntPtr creaConnessione(string dati);

        public Form1()
        {
            //in questa stringa di dati devono esserci i seguenti dati
            //(presi dal file credenziali):
            //nome-cognome-fotopath-filepath-visibility
            string dati = "mattia-lavacca-C:\\Users\\Mattia\\Desktop\\a.jpg-C:\\Users\\Mattia\\Desktop-true";

            InitializeComponent();
            connessione = creaConnessione(dati);
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            
           
        }

       

        private void roundButton1_Click(object sender, EventArgs e)
        {
            //vado ad aprire l'esplora risorse per scegliere un path
            FileDialog fd = new FileDialog();
            fd.Show();
            
        }

        private void button1_Click(object sender, EventArgs e)
        {
            Settings set = new Settings(connessione);
            set.Show();
        }

        private void toolTip1_Popup(object sender, PopupEventArgs e)
        {
           
        }

       
    }
}
