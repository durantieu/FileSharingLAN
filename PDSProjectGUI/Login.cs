﻿using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO;
using System.Runtime.InteropServices;

namespace PDSProjectGUI
{
    public partial class Login : Form
    {
        [DllImport("FileSharingTest.dll", CharSet = CharSet.Ansi)]
        public static extern void firstGetHomeDir(StringBuilder str);

        string nome;
        string cognome;
        string path_file_trans;
        string path_imm_profilo;
        string home_dir;
        bool visibility;
        bool accettaAutomaticamente;
        FolderBrowserDialog fbd = new FolderBrowserDialog();
        private Credenziali cred;

        public Login()
        {
            StringBuilder str = new StringBuilder();
            firstGetHomeDir(str);
            home_dir = str.ToString();

            InitializeComponent();
        }
        private void textBox1_TextChanged(object sender, EventArgs e)
        {

        }
        public void fill_credentials()
        {
            string temp;
            string[] temp2;
            StreamReader sr = new StreamReader(home_dir + "Credenziali.txt");
            while ((temp = sr.ReadLine() )!= null)
            {
                temp2 = temp.Split('|');
                if(temp2[0] == "Nome")
                {
                    nome = temp2[1];
                }
                if(temp2[0] == "Cognome")
                {
                    cognome = temp2[1];
                }
                if (temp2[0] == "Path_immagine_profilo")
                {
                    path_imm_profilo = temp2[1];
                }
                if (temp2[0] == "Path_file_transfer")
                {
                    path_file_trans = temp2[1];
                }
                if(temp2[0] == "accettaAutomaticamente")
                {
                    if(temp2[1] == "true")
                    {
                        accettaAutomaticamente = true;
                    }
                    else
                    {
                        accettaAutomaticamente = false;
                    }
                }
                if(temp2[0] == "Visible")
                {
                    if(temp2[1] == "true")
                    {
                        visibility = true;
                    }
                    else
                    {
                        visibility = false;
                    }
                    
                }
            }
            sr.Close();
            cred = new Credenziali(nome, cognome, path_imm_profilo, path_file_trans, accettaAutomaticamente, visibility);
            

        }
        private void button3_Click(object sender, EventArgs e)
        {

            if (textBox1.Text != "" && textBox2.Text != "" && textBox3.Text != "" && textBox4.Text != "")
            {
                nome = textBox1.Text;
                cognome = textBox2.Text;
                path_file_trans = textBox3.Text;
                path_imm_profilo = textBox4.Text;
                visibility = !checkBox1.Checked;

                cred = new Credenziali(nome, cognome, path_imm_profilo, path_file_trans, accettaAutomaticamente, visibility);

                Share f1 = new Share(this, false, "");
                f1.Show();
                this.Hide();
            }
        }
        public Credenziali get_credentials()
        {
            return cred;
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
            ofd.Filter = "Image Files(*.BMP;*.JPG;*.GIF)|*.BMP;*.JPG;*.GIF";
            if (ofd.ShowDialog() == DialogResult.OK)
            {
                textBox4.Text = ofd.FileName;

            }
        }
        private void Login_Load(object sender, EventArgs e)
        {

        }
        public override string ToString()
        {
            return nome + '|' + cognome + '|' + path_imm_profilo + '|' + path_file_trans + '|' + visibility + '|';
        }

       
    }
}
