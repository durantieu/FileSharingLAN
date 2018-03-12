using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Runtime.InteropServices;

namespace PDSProjectGUI
{
    public class Credenziali
    {
        
        private string visibility;
        private string nome;
        private string cognome;
        private string path_immagine_profilo;
        private string path_file_transfer;
        private string home_dir;

        [DllImport("FileSharingTest.Dll")]
        public static extern string firstGetHomeDir();

        public Credenziali(string name, string surname, string path_imm_profilo, string path_f_trans, bool visibile)
        {
            
            nome = name;
            cognome = surname;
            path_immagine_profilo = path_imm_profilo;
            path_file_transfer = path_f_trans;
            
            if (visibile)
            {
                visibility = "true";
            }
            else
            {
                visibility = "false";
            }
            string[] lines = { "Nome|", "Cognome|", "Path_immagine_profilo|", "Path_file_transfer|", "Visible|" };
            string[] tmp = { nome, cognome, path_immagine_profilo, path_file_transfer, visibility};
            StreamWriter sw = new StreamWriter(home_dir + "Credenziali.txt");
            int j = 0;
            foreach(string i in lines){
                sw.WriteLine(i + tmp[j]);
                j++;
            }
            sw.Close();

        }
        public void change_nome(string newNome)
        {
            nome = newNome;
            using (StreamReader sr = new StreamReader(home_dir + "Credenziali.txt"))
            {
                string temp;
                string[] temp2;
                StreamWriter sw = new StreamWriter("temp.txt");
                while ( (temp = sr.ReadLine()) != null)
                {
                   
                    temp2 = temp.Split('|');
                    if (temp2[0] != "Nome")
                    {
                        sw.WriteLine(temp);
                    }
                    else
                    {
                        sw.Write(temp2[0]);
                        sw.Write('|');
                        sw.Write(newNome);
                    }
                }
                sw.Close();
                
            }
            System.Diagnostics.Process process = new System.Diagnostics.Process();
            System.Diagnostics.ProcessStartInfo startInfo = new System.Diagnostics.ProcessStartInfo();
            startInfo.WindowStyle = System.Diagnostics.ProcessWindowStyle.Hidden;
            startInfo.FileName = "cmd.exe";
            startInfo.Arguments = "rename temp.txt Credenziali.txt";
            process.StartInfo = startInfo;
            process.Start();
        }

        public void change_cognome(string newCognome)
        {
            cognome = newCognome;
            using (StreamReader sr = new StreamReader("Credenziali.txt"))
            {
                string temp;
                string[] temp2;
                StreamWriter sw = new StreamWriter("temp.txt");
                while ((temp = sr.ReadLine()) != null)
                {

                    temp2 = temp.Split('|');
                    if (temp2[0] != "Cognome")
                    {
                        sw.WriteLine(temp);
                    }
                    else
                    {
                        sw.Write(temp2[0]);
                        sw.Write('|');
                        sw.Write(newCognome);
                    }
                }
                sw.Close();

            }
            System.Diagnostics.Process process = new System.Diagnostics.Process();
            System.Diagnostics.ProcessStartInfo startInfo = new System.Diagnostics.ProcessStartInfo();
            startInfo.WindowStyle = System.Diagnostics.ProcessWindowStyle.Hidden;
            startInfo.FileName = "cmd.exe";
            startInfo.Arguments = "rename temp.txt Credenziali.txt";
            process.StartInfo = startInfo;
            process.Start();
        }


        public void change_path_file_transfer(string newPathFileTransfer)
        {
            path_file_transfer = newPathFileTransfer;

            using (StreamReader sr = new StreamReader("Credenziali.txt"))
            {
                string temp;
                string[] temp2;
                StreamWriter sw = new StreamWriter("temp.txt");
                while ((temp = sr.ReadLine()) != null)
                {

                    temp2 = temp.Split('|');
                    if (temp2[0] != "Path_file_transfer")
                    {
                        sw.WriteLine(temp);
                    }
                    else
                    {
                        sw.Write(temp2[0]);
                        sw.Write('|');
                        sw.Write(newPathFileTransfer);
                    }
                }
                sw.Close();

            }
            System.Diagnostics.Process process = new System.Diagnostics.Process();
            System.Diagnostics.ProcessStartInfo startInfo = new System.Diagnostics.ProcessStartInfo();
            startInfo.WindowStyle = System.Diagnostics.ProcessWindowStyle.Hidden;
            startInfo.FileName = "cmd.exe";
            startInfo.Arguments = "rename temp.txt Credenziali.txt";
            process.StartInfo = startInfo;
            process.Start();

        }


        public void change_path_immagine_profilo(string newPathImmagineProfilo)
        {
            path_immagine_profilo = newPathImmagineProfilo;
            using (StreamReader sr = new StreamReader("Credenziali.txt"))
            {
                string temp;
                string[] temp2;
                StreamWriter sw = new StreamWriter("temp.txt");
                while ((temp = sr.ReadLine()) != null)
                {

                    temp2 = temp.Split('|');
                    if (temp2[0] != "Path_immagine_profilo")
                    {
                        sw.WriteLine(temp);
                    }
                    else
                    {
                        sw.Write(temp2[0]);
                        sw.Write('|');
                        sw.Write(newPathImmagineProfilo);
                    }
                }
                sw.Close();

            }
            System.Diagnostics.Process process = new System.Diagnostics.Process();
            System.Diagnostics.ProcessStartInfo startInfo = new System.Diagnostics.ProcessStartInfo();
            startInfo.WindowStyle = System.Diagnostics.ProcessWindowStyle.Hidden;
            startInfo.FileName = "cmd.exe";
            startInfo.Arguments = "rename temp.txt Credenziali.txt";
            process.StartInfo = startInfo;
            process.Start();
        }


        public void change_visibility(string newVisibility)
        {
            visibility = newVisibility;
            using (StreamReader sr = new StreamReader("Credenziali.txt"))
            {
                string temp;
                string[] temp2;
                StreamWriter sw = new StreamWriter("temp.txt");
                while ((temp = sr.ReadLine()) != null)
                {

                    temp2 = temp.Split('|');
                    if (temp2[0] != "Visible")
                    {
                        sw.WriteLine(temp);
                    }
                    else
                    {
                        sw.Write(temp2[0]);
                        sw.Write('|');
                        sw.Write(newVisibility);
                    }
                }
                sw.Close();

            }
            System.Diagnostics.Process process = new System.Diagnostics.Process();
            System.Diagnostics.ProcessStartInfo startInfo = new System.Diagnostics.ProcessStartInfo();
            startInfo.WindowStyle = System.Diagnostics.ProcessWindowStyle.Hidden;
            startInfo.FileName = "cmd.exe";
            startInfo.Arguments = "rename temp.txt Credenziali.txt";
            process.StartInfo = startInfo;
            process.Start();


        }

        public override string ToString()
        {
            //System.Console.WriteLine(nome + '|' + cognome + '|' + path_immagine_profilo + '|' + path_file_transfer + '|' + visibility + '|');
            return nome+'|'+cognome+'|'+path_immagine_profilo+'|'+path_file_transfer+'|'+ visibility+'|';

        }
        public string get_nome()
        {
            return nome;
        }
        public string get_cognome()
        {
            return cognome;
        }
        public string get_immagine_profilo()
        {
            return path_immagine_profilo;
        }

    }
}
