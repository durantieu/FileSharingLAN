using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO.Pipes;

namespace PDSProjectGUI
{
    public class PollingPipe
    {
        private Task tk;
        int tipo_pipe; // 1 = Listener (main pipe che sta in ascolto); 2 = pipe comunicazione GUI-Server; 3= pipe comunicazione GUI e Client
        ProgressBarDialog pbd;
        string pipeID;
        NamedPipeClientStream pipe;
        ProgressBarDialog.delegateUpdateProgressBar barraDel;

        public PollingPipe(ProgressBarDialog.delegateUpdateProgressBar barra, ProgressBarDialog calling_form, string nome_pipe, int type)
        {
            pbd = calling_form;
            barraDel = barra;
            //Create here the pipe and hook it with c++s one
            pipeID = nome_pipe;
            pipe = new NamedPipeClientStream(pipeID);
            pipe.Connect();

            tipo_pipe = type;
            //After pipes are synchronized
            poll_the_pipe();

        }

        private async void poll_the_pipe()
        {
            switch (tipo_pipe)
            {
                case 1:
                    tk = new Task(polling_main_pipe);
                    tk.Start();
                    await tk;
                    break;
                case 2:
                    tk = new Task(polling_server_pipe);
                    tk.Start();
                    await tk;
                    pbd.Close();
                    break;

                case 3:
                    tk = new Task(polling_client_pipe);
                    tk.Start();
                    await tk;
                    pbd.Close();
                    break;
            }

        }

        private void polling_main_pipe()
        {
            //insert here the pipe polling
            while (true)
            {

                //read the pipe (should be blocking)

                //create and launch the receiver form passing it all data about the sender


            }

        }

        private void polling_server_pipe()
        {
            //insert here the pipe polling
            while (true)
            {

                //read the pipe 

                //leggere dalla pipe lo stato di avanzamento del trasferimento


            }
           


        }
        private void polling_client_pipe()
        {
            byte[] buffer = new byte[1024];
            string buff_string;
            string tmp;
            string[] substrings;
            int max = 0;
            string maxStr = "";

            //using (System.IO.StreamWriter file =
            //new System.IO.StreamWriter(@"C:\Users\Mattia\Desktop\logCS.txt", true))

            while (true)
            {
                pipe.Read(buffer, 0, 1024);
                buff_string = System.Text.Encoding.UTF8.GetString(buffer);

                tmp = buff_string.Replace("\0", string.Empty);
                substrings = tmp.Split(new string[] { "|" }, StringSplitOptions.None);
                foreach(string i in substrings){
                    if(i != "")
                    {
                        int x = Int32.Parse(i);
                        if(x > max)
                        {
                            max = x;
                            maxStr = i;
                        }
                    }
                    
                }
                
                
                //file.WriteLine("BuffString: " + buff_string);
                //file.WriteLine("max: " + max);

                //leggere dalla pipe lo stato di avanzamento del trasferimento

                 pbd.Invoke(barraDel, new object[] { maxStr });
        
                if (maxStr == "100")
                {
                    
                    break;
                    
                }
            }

            pipe.Close();

        }
    }
}
