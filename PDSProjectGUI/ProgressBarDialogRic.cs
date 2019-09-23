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

    public partial class ProgressBarDialogRic : Form
    {
        public delegate void delegateUpdateProgressBar(string value);

        public bool is_pipe_closed;
        Mutex mut_pipe;
        private delegateUpdateProgressBar updateProgressBar;
        PollingPipe pp;
        NamedPipeClientStream serverPipe;
        bool acc;
        DateTime prima;
        public ProgressBarDialogRic(string nomeUtente, string nPipe, bool accepted)
        {
            acc = accepted;
            InitializeComponent();
            if (acc)
            {
                this.Show();
            }
            else
            {
                this.Hide();
            }
            
            label4.Text = nomeUtente;
            prima = DateTime.Now;
            mut_pipe = new Mutex();
            is_pipe_closed = false;

            updateProgressBar = new delegateUpdateProgressBar(modify_progress_bar);

            progressBar1.Maximum = 100;
            progressBar1.Minimum = 0;
            progressBar1.MarqueeAnimationSpeed = 0;

            string pipeName = "pipe" + nPipe;
            serverPipe = new NamedPipeClientStream(pipeName);

            pp = new PollingPipe(null, updateProgressBar, null, this, serverPipe, null, mut_pipe, 2, acc);

            
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

            string oreStimate = Convert.ToString((int)((diffOre * (100 - valueDouble))));
            string minutiStimati = Convert.ToString((int)((diffMin * (100 - valueDouble))) % 60);
            string secondiStimati = Convert.ToString((int)(diffSec * (100 - valueDouble)) % 60);

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

            label1.Text = ValoreVisualizzato;

            prima = ora;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            mut_pipe.WaitOne();
            is_pipe_closed = true;
            mut_pipe.ReleaseMutex();
        }

    }
}
