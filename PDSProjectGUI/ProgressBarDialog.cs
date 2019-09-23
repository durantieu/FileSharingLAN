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
using System.Threading;

namespace PDSProjectGUI
{

    public partial class ProgressBarDialog : Form
    {
        [DllImport("FileSharingTest.dll", CharSet = CharSet.Ansi)]
        public static extern void inviaFile(IntPtr connessione, string file, string MAC, StringBuilder str);
        private string nome_file;
        IntPtr connessione;

        public delegate void delegateUpdateProgressBar(string value);
        private DateTime prima;
        public bool is_pipe_closed;
        Mutex mut_pipe;
        private delegateUpdateProgressBar updateProgressBar;
        PollingPipe p;
        NamedPipeClientStream pipe;
        Share grandDad;
        public ProgressBarDialog(IntPtr connection, utente usr, string path, SendFiles padre, Share nonno)
        {
            grandDad = nonno;
            InitializeComponent();
            prima = DateTime.Now;
            mut_pipe = new Mutex();
            is_pipe_closed = false;

            updateProgressBar = new delegateUpdateProgressBar(modify_progress_bar);

            StringBuilder str = new StringBuilder();
            connessione = connection;



            inviaFile(connessione, path, usr.MAC, str);
            progressBar1.Maximum = 100;
            progressBar1.Minimum = 0;
            progressBar1.MarqueeAnimationSpeed = 0;

            string tmpID = str.ToString();
            if(tmpID == "-1")
            {
                SendFiles padre2 = new SendFiles(padre.path, padre.conn, padre.nomeFile, nonno, false);
                padre.Close();
                padre2.Show();
                this.Close();

            }
            else
            {
                this.Show();
                pipe = new NamedPipeClientStream(tmpID);

                p = new PollingPipe(updateProgressBar, null, this, null, pipe, null, mut_pipe, 3, true);
                nome_file = path;
                label3.Text = nome_file;
                label4.Text = usr.Nome + " " + usr.Cognome;
            }
            
        }
        
        public void modify_progress_bar(string value)
        {
            progressBar1.Value = Int32.Parse(value);
            DateTime ora = DateTime.Now;
            TimeSpan offsetTemp = ora - prima;

            double diffOre = offsetTemp.TotalHours;
            double diffMin = offsetTemp.TotalMinutes;
            double diffSec = offsetTemp.TotalSeconds;

            double valueDouble = Convert.ToDouble(value);
            
            string secondiStimati = Convert.ToString((int)((diffSec * (100 - valueDouble)) ) % 60);
            string minutiStimati = Convert.ToString((int)((diffMin * (100 - valueDouble)) ) % 60);
            string oreStimate = Convert.ToString((int) (diffOre * (100 - valueDouble)));



            string ValoreVisualizzato = value + "% -- ";

            if (oreStimate != "0")
            {
                ValoreVisualizzato += oreStimate + " h, ";
            }
            if (minutiStimati != "0")
            {
                ValoreVisualizzato += minutiStimati + " m, ";
            }
            ValoreVisualizzato += secondiStimati + " s rimanenti";

            label5.Text = ValoreVisualizzato;

            prima = ora;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            mut_pipe.WaitOne();
            is_pipe_closed = true;
            mut_pipe.ReleaseMutex();

            this.Close();
        }

        private void progressBar1_Click(object sender, EventArgs e)
        {

        }

        private void label5_Click(object sender, EventArgs e)
        {

        }

        private void label4_Click(object sender, EventArgs e)
        {

        }
    }
}
