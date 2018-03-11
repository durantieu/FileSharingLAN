using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace PDSProjectGUI
{
    public partial class ProgressBarDialog : Form
    {
        public ProgressBarDialog()
        {
            InitializeComponent();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            //Terminate the transfer by calling DLL function

            this.Close();
        }

        
    }
}
