using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Runtime.InteropServices;
using System.IO;

namespace PDSProjectGUI
{
    static class Program
    {
        /// <summary>
        /// Punto di ingresso principale dell'applicazione.
        /// </summary>
        /// 
        static bool check_credentials()
        {
            bool incomplete_info = false;
            try
            {
                using (StreamReader sr = new StreamReader("Credenziali.txt"))
                {

                    string tmp, first, second;
                    string[] tmp2;
                    while ((tmp = sr.ReadLine()) != null)
                    {
                        tmp2 = tmp.Split(':');
                        first = tmp2[0];
                        second = tmp2[1];


                        if (second == "")
                        {
                            incomplete_info = true;
                            break;
                        }
                    }


                }
            }
            catch (IOException)
            {
                return false;
            }

            if (incomplete_info)
            {
                return false;
            }
            return true;

        }


        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Login l = new Login();
            if (check_credentials())
            {
                Application.Run(new Share(l));
            }
            else
            {
                Application.Run(l);
            }
            
        }
    }
}
