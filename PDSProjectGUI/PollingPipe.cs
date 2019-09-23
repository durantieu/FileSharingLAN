using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO.Pipes;
using System.Threading;
using System.Windows.Forms;

namespace PDSProjectGUI
{
   

    public class PollingPipe
    {
        private Task tk;
        int tipo_pipe; // 1 = Listener (main pipe che sta in ascolto); 2 = pipe comunicazione GUI-Server; 3= pipe comunicazione GUI e Client
        ProgressBarDialog pbd;
        ProgressBarDialogRic pbdr;
        NamedPipeClientStream namedPipe;
        ProgressBarDialog.delegateUpdateProgressBar barraDel;
        ProgressBarDialogRic.delegateUpdateProgressBar barraDelRic;
        IncomingConnection incCon;
        Share mainForm;
        Mutex mtx;
        bool accepted;
        ConnectionRejected cr;

        public PollingPipe(ProgressBarDialog.delegateUpdateProgressBar barra,
            ProgressBarDialogRic.delegateUpdateProgressBar barraRic,
            ProgressBarDialog calling_form,
            ProgressBarDialogRic calling_form_ric,
            NamedPipeClientStream pipe,
            Share shareForm,
            Mutex mt,
            int type,
            bool acc)
        {

            namedPipe = pipe;
            accepted = acc;
            pbd = calling_form;
            pbdr = calling_form_ric;
            barraDel = barra;
            barraDelRic = barraRic;
            mtx = mt;
            
            tipo_pipe = type;
            mainForm = shareForm;

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
                    pbdr.Close();
                    break;

                case 3:
                    tk = new Task(polling_client_pipe);
                    tk.Start();
                    await tk;
                    pbd.Close();
                    break;
            }

        }




        /*
         * 
         * Pipe con il TCP_Server 
         * 
         */
        //----------------------------------------------------------
        private void polling_main_pipe()
        {
            namedPipe.Connect();

            byte[] buffer = new byte[1024];
            string buff_string;
            string tmp;
            string[] substrings;
            string MAC = "", nomeUtente = "", nPipe = "";

            while (true)
            {
                
                namedPipe.Read(buffer, 0, 1024);
                string visibility = mainForm.cred.get_visibility();
                string accetta = mainForm.cred.get_accettaAutomaticamente();
           
                    byte[] buff = Encoding.UTF8.GetBytes("OK\0");
                    namedPipe.Write(buff, 0, buff.Length);

                    buff_string = System.Text.Encoding.UTF8.GetString(buffer);

                    tmp = buff_string.Replace("\0", string.Empty);
                    substrings = tmp.Split(new string[] { "|" }, StringSplitOptions.None);
                    int index = 0;
                    foreach (string i in substrings)
                    {
                        if (i != "")
                        {
                            switch (index)
                            {
                                case 0:
                                    MAC = i;
                                    break;
                                case 1:
                                    nomeUtente = i;
                                    break;
                                case 2:
                                    nPipe = i;
                                    break;
                            }
                            index++;
                        }
                    }
                

                    mainForm.BeginInvoke((Action)delegate
                    {
                        if (visibility.CompareTo("true") == 0)
                        {
                            if(accetta.CompareTo("true") == 0)
                            {
                                incCon = new IncomingConnection(MAC, nomeUtente, nPipe, true, false); 
                            }
                            else
                            {
                                incCon = new IncomingConnection(MAC, nomeUtente, nPipe, false, false);
                                incCon.Show();
                            }   
                        }
                        else
                        {
                            incCon = new IncomingConnection(MAC, nomeUtente, nPipe, false, true);
                        }   
                    });
                
            
                          
            }
        }




        /*
         * 
         * Pipe con il TCP_Server 
         * 
         */
        //----------------------------------------------------------
        private void polling_server_pipe()
        {
            namedPipe.Connect();

            byte[] buffer = new byte[1024];
            string buff_string;
            string tmp;
            string[] substrings;
            int max = 0;
            string maxStr = "0";
            string maxStrPrec = "0";
            int ind = 0;

            //-----------------------------------------------------
            //HELLO Da TCP_Server e OK
            //-----------------------------------------------------
            namedPipe.Read(buffer, 0, 6);
            //-------------------
            if (accepted)
            {
                buff_string = "OK\0";
                buffer = Encoding.UTF8.GetBytes(buff_string);
                namedPipe.Write(buffer, 0, buffer.Length);
            }
            else
            {              
                buff_string = "X\0";
                buffer = Encoding.UTF8.GetBytes(buff_string);
                namedPipe.Write(buffer, 0, buffer.Length);
                pbdr.Close();
                namedPipe.Close();
                return;
            }
            
            //-----------------------------------------------------
            //-----------------------------------------------------

           

            while (true)
            {
                ind= 0;

                //-----------------------------------------------------
                // ricezione % di trasferimento e ACK-%
                //-----------------------------------------------------
                bool found = false;

                byte[] dataBuffer = new byte[5];

                while (!found)
                {
                    namedPipe.Read(dataBuffer, 0, 5);
                    buff_string = System.Text.Encoding.UTF8.GetString(dataBuffer);
                    if (buff_string.Contains("|"))
                        found = true;
                    ind++;
                }
                
                tmp = buff_string.Replace("\0", string.Empty);
                substrings = tmp.Split(new string[] { "|" }, StringSplitOptions.None);
                foreach (string i in substrings)
                {
                    if (i != "")
                    {
                        int x = Int32.Parse(i);
                        if (x > max)
                        {
                            max = x;
                            maxStr = i;
                        }
                    }
                }
                //Scrittura sulla progress bar l'avanzamento del trasferimento
                if (maxStr != maxStrPrec)
                {
                    maxStrPrec = maxStr;
                    pbdr.Invoke(barraDelRic, new object[] { maxStr });
                }
                if(maxStr == "100")
                {
                    break;
                }

                buff_string = "OK\0";
                buffer = Encoding.UTF8.GetBytes(buff_string);
                namedPipe.Write(buffer, 0, buffer.Length);
                //-----------------------------------------------------
                //-----------------------------------------------------


                //-----------------------------------------------------
                //ACK-A da TCP_Client e ACK-B a TCP_client
                //-----------------------------------------------------

                byte[] ackABuffer = new byte[2];
                namedPipe.Read(ackABuffer, 0, 2);
                buff_string = System.Text.Encoding.UTF8.GetString(ackABuffer);
                if (buff_string.Contains("X"))
                {
                    //Visual messaggio di comm killata
                    pbdr.BeginInvoke((Action)delegate
                    {
                        cr = new ConnectionRejected();
                        cr.Show();
                    });
                    break;
                }
                //-------------------

                //-------------------
                mtx.WaitOne();
                if (pbdr.is_pipe_closed)
                {
                    buffer = Encoding.UTF8.GetBytes("X\0");
                    try
                    {
                        namedPipe.Write(buffer, 0, buffer.Length);
                    }
                    catch (System.IO.IOException)
                    {
                        namedPipe.Connect();
                        namedPipe.Write(buffer, 0, buffer.Length);
                    }
                    namedPipe.Close();
                    pbdr.is_pipe_closed = false;
                    break;
                }
                else
                {
                    buffer = Encoding.UTF8.GetBytes("K\0");
                    try
                    {
                        namedPipe.Write(buffer, 0, buffer.Length);
                    }
                    catch (System.IO.IOException)
                    {
                        namedPipe.Connect();
                        namedPipe.Write(buffer, 0, buffer.Length);
                    }
                }
                mtx.ReleaseMutex();
            }
            namedPipe.Close();
            return;
        }




        /*
         * 
         * Pipe con il TCP_Client
         * 
         */
        //----------------------------------------------------------
        private void polling_client_pipe()
        {
            namedPipe.Connect();

            byte[] buffer = new byte[1024];
            string buff_string;
            string tmp;
            string[] substrings;
            int max = 0;
            string maxStr = "0";
            string maxStrPrec = "0";

            //-----------------------------------------------------
            //OK da TCP_Listener e Hello
            //-----------------------------------------------------
            namedPipe.Read(buffer, 0, 3);
            //-------------------
            Array.Clear(buffer, 0, buffer.Length);
            //-------------------
            buff_string = "Hello\0";
            buffer = Encoding.UTF8.GetBytes(buff_string);
            namedPipe.Write(buffer, 0, buffer.Length);
            //-----------------------------------------------------
            //-----------------------------------------------------

            Array.Clear(buffer, 0, buffer.Length);
            buff_string = "";

            //-----------------------------------------------------
            //OK da TCP_Server e ACK-OK
            //-----------------------------------------------------
            namedPipe.Read(buffer, 0, 3);
            buff_string = System.Text.Encoding.UTF8.GetString(buffer);
            if (buff_string.Contains("X"))
            {
                //Connessione rifiutata, far apparire il popup
                try
                {
                    pbd.BeginInvoke((Action)delegate
                    {

                        cr = new ConnectionRejected();
                        cr.Show();


                    });
                }catch(InvalidOperationException e)
                {
                   
                }
              
                
                namedPipe.Close();
                return;
            }
            else
            {
                /*pbd.BeginInvoke((Action)delegate
               {
                    pbd.Show();
               });*/

            }
            //-------------------
            Array.Clear(buffer, 0, buffer.Length);
            //-------------------
            buff_string = "OK\0";
            buffer = Encoding.UTF8.GetBytes(buff_string);
            namedPipe.Write(buffer, 0, buffer.Length);
            //-----------------------------------------------------
            //-----------------------------------------------------

            while (true)
            {


                //-----------------------------------------------------
                // ricezione % di trasferimento e ACK-A
                //-----------------------------------------------------
                byte[] dataBuffer = new byte[5];

                namedPipe.Read(dataBuffer, 0, 5);
                buff_string = System.Text.Encoding.UTF8.GetString(dataBuffer);
                tmp = buff_string.Replace("\0", string.Empty);
                substrings = tmp.Split(new string[] { "|" }, StringSplitOptions.None);
                foreach (string i in substrings)
                {
                    if (i != "")
                    {
                        int x = Int32.Parse(i);
                        if (x > max)
                        {
                            max = x;
                            maxStr = i;
                        }
                    }
                }
                //Scrittura sulla progress bar l'avanzamento del trasferimento
                if (maxStr != maxStrPrec)
                {
                    maxStrPrec = maxStr;
                    try
                    {
                        pbd.Invoke(barraDel, new object[] { maxStr });
                    }catch(Exception generic)
                    {

                    }
                   
                }
                //-------------------

                //Array.Clear(buffer, 0, buffer.Length);
                //buff_string = "";

                //-------------------
                mtx.WaitOne();

                if (pbd.is_pipe_closed)
                {
                    buffer = Encoding.UTF8.GetBytes("X\0");
                    try
                    {
                        namedPipe.Write(buffer, 0, buffer.Length);
                    }
                    catch (System.IO.IOException)
                    {
                        namedPipe.Connect();
                        namedPipe.Write(buffer, 0, buffer.Length);
                    }
                    namedPipe.Close();
                    pbd.is_pipe_closed = false;
                    break;
                }
                else
                {
                    buffer = Encoding.UTF8.GetBytes("K\0");
                    try
                    {
                        namedPipe.Write(buffer, 0, buffer.Length);
                    }
                    catch (System.IO.IOException)
                    {
                        namedPipe.Connect();
                        namedPipe.Write(buffer, 0, buffer.Length);
                    }
                }
                mtx.ReleaseMutex();
                //-----------------------------------------------------
                //-----------------------------------------------------

                Array.Clear(buffer, 0, buffer.Length);
                buff_string = "";

                //-----------------------------------------------------
                //ACK-B da TCP_Server e ACK-ACK-B
                //-----------------------------------------------------
                namedPipe.Read(buffer, 0, buffer.Length);
                buff_string = System.Text.Encoding.UTF8.GetString(buffer);
                if (buff_string.Contains("X"))
                {
                    //visual messaggio di comm killata
                    pbd.BeginInvoke((Action)delegate
                    {
                        cr = new ConnectionRejected();
                        cr.Show();
                    });

                    break;
                }

                //-------------------
                buff_string = "OK\0";
                buffer = Encoding.UTF8.GetBytes(buff_string);
                namedPipe.Write(buffer, 0, buffer.Length);
                //-----------------------------------------------------
                //-----------------------------------------------------


                //Condizione di terminazione di trasferimento del file
                if (maxStr == "100")
                {
                    break;
                }
            }

            namedPipe.Close();
            return;
        }

    }

}

    
