using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
namespace PDSProjectGUI
{
    class Credenziali
    {
        
        private string visibility;
        private string nome;
        private string cognome;
        private string path_immagine_profilo;
        private string path_file_transfer;
        

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
            StreamWriter sw = new StreamWriter("Credenziali.txt");
            int j = 0;
            foreach(string i in lines){
                sw.WriteLine(i + tmp[j]);
                j++;
            }
            sw.Close();

        }
        public void change_nome(string newNome)
        {
            using (StreamReader sr = new StreamReader("Credenziali.txt"))
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




    }
}
