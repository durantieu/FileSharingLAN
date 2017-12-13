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
            string[] lines = { "Nome: ", "Cognome: ", "Path_immagine_profilo: ", "Path_file_transfer: ", "Visible: " };
            string[] tmp = { nome, cognome, path_immagine_profilo, path_file_transfer, visibility};
            StreamWriter sw = new StreamWriter("Credenziali.txt");
            int j = 0;
            foreach(string i in lines){
                sw.WriteLine(i + tmp[j]);
                j++;
            }
            sw.Close();

        }




    }
}
