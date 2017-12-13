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
    public partial class Login : Form
    {
        string nome;
        string cognome;
        string path_file_trans;
        string path_imm_profilo;
        bool visibility;
        FolderBrowserDialog fbd = new FolderBrowserDialog();

        public Login()
        {
            InitializeComponent();
        }

        private void textBox1_TextChanged(object sender, EventArgs e)
        {

        }

        private void button3_Click(object sender, EventArgs e)
        {
            nome = textBox1.Text;
            cognome = textBox2.Text;
            path_file_trans = textBox3.Text;
            path_imm_profilo = textBox4.Text;
            visibility = checkBox1.Checked;

            Credenziali cred = new Credenziali(nome, cognome, path_imm_profilo, path_file_trans, visibility);

            Share f1 = new Share(this);
            f1.Show();
            this.Hide();
        }

        private void button4_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            
            if (fbd.ShowDialog() == DialogResult.OK && !string.IsNullOrWhiteSpace(fbd.SelectedPath))
            {
                textBox3.Text = fbd.SelectedPath;
            }
        }

        private void button2_Click(object sender, EventArgs e)
        {
            OpenFileDialog ofd = new OpenFileDialog();
            if (ofd.ShowDialog() == DialogResult.OK)
            {
                textBox4.Text = ofd.FileName;

            }
        }
    }
}
