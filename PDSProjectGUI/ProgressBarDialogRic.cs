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
        }

        private void button1_Click(object sender, EventArgs e)
        {
            mut_pipe.WaitOne();
            is_pipe_closed = true;
            mut_pipe.ReleaseMutex();
        }

    }
}
