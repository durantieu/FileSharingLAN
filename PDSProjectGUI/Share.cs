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
        Login l;
        IntPtr connessione;
        [DllImport("C:\\Users\\Mattia\\Desktop\\FileSharingTest.dll")]
        public static extern IntPtr creaConnessione(string dati);

        public Share(Login l)
        {

            InitializeComponent();
            cred = l.get_credentials();
            connessione = creaConnessione(cred.ToString());
            
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

        private void Share_FormClosing(object sender, FormClosingEventArgs e)
        {
            try
            {
                l.Close();

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
