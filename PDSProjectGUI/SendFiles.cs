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
using Microsoft.Win32.SafeHandles;

namespace PDSProjectGUI{
    public struct utente
    {
        public string MAC;
        public string Nome;
        public string Cognome;
        public string ha_la_foto;
        public string bloccato;
    }


    public partial class SendFiles : Form
    {
       
        utente[] utenti_online;
        public string path;
        int numero_utenti_in_rete=0;
        int counter_button = 0;
        int counter_picBox = 0;
        int counter_textBox = 0;
        string home_dir;
        Timer timer;

       [DllImport("FileSharingTest.dll")]
        public static extern void inviaFile(IntPtr conn, string file, string MAC);

        [DllImport("FileSharingTest.dll", CharSet = CharSet.Ansi)]
        public static extern void firstGetHomeDir(StringBuilder str);

        unsafe char** utenti_info;
        unsafe int size;
        public IntPtr conn;
        public string nomeFile;
        Share dad;
        bool closeEverything;
        public unsafe SendFiles(string pt, IntPtr connection, string nome_file, Share padre, bool isCallByDxButton)
        {
            dad = padre;
            nomeFile = nome_file;
            string[] tmp;
            StringBuilder str = new StringBuilder();
            firstGetHomeDir(str);
            home_dir = str.ToString();
            path = pt;
            InitializeComponent();
            conn = connection;
            MarshalVectorWrapper(conn, out utenti_info, out size);
            closeEverything = isCallByDxButton;
            this.FormClosing += new FormClosingEventHandler(SendFiles_FormClosing);
            if (size != 0)
            {
                utenti_online = new utente[size];
                tmp = new string[size];
               

                //Parse the info utenti got from DLL and save them into data structures

                for (int i = 0; i < size; i++)
                {
                    tmp[i] = Marshal.PtrToStringAnsi((IntPtr)utenti_info[i]);

                    string[] tmp2 = tmp[i].Split('|');

                    utenti_online[i].MAC = tmp2[0];
                    utenti_online[i].Nome = tmp2[1];
                    utenti_online[i].Cognome = tmp2[2];
                    utenti_online[i].ha_la_foto = tmp2[3];
                    utenti_online[i].bloccato = tmp2[4];

                }
            }
            else
            {
               // label1.Text = "Non sono presenti utenti in rete";
            }
            //acchiappo il numero di utenti in rete  e lo assegno a numero_utenti_in_rete
            /*System.IO.StreamWriter myStream = new System.IO.StreamWriter("C:\\Users\\duran\\Desktop\\Miolog.txt");
            myStream.Write("ok1");
            myStream.Close();*/

        }


        private void NewButton_Click(object sender, EventArgs e)
        {
            Button btn = (Button)sender;

            for (int i = 0; i < 6; i++)
            {
                if (btn.Name == ("Butt" + i))
                {
                    //qui scrivo le funzioni che devono essere richiamate quando scelgo un utente a cui inviare ()
                    //dovrò chiamare il lancio di un client per l'invio di un file verso l'utente identificato da questo bottone
                    
                    ProgressBarDialog pbd = new ProgressBarDialog(conn, utenti_online[i], path, this, dad);

                    break;
                }
            }
        }


        private Button CreateButton_Click(int i, int j, utente usr_corrente)
        { 
            Button button = new Button();
            
            
            // Posiziono il bottone
            button.Name = "Butt" + counter_button;
            button.BackgroundImage = System.Drawing.Image.FromFile(home_dir + "Invia50x40.png");
            button.ForeColor = Color.Black;
            button.BackColor = Color.White;
            button.BackgroundImageLayout = ImageLayout.Center;
            button.FlatStyle = FlatStyle.Flat;
            button.FlatAppearance.BorderSize = 1;

            
            button.Location = new Point(i+80, j+170);
            button.Size = new Size(100, 40);
           
           
            // incremento l'identificatore.
            counter_button++;
           
           //aggiungo l'evento Click al nuovo bottone
            button.Click += new EventHandler(NewButton_Click);

            return button;
        }
        private PictureBox create_picBox(int riga, int colonna, utente usr_corrente)
        {
           

            PictureBox newPicBox = new PictureBox();
            newPicBox.Name = "PicBox"+counter_picBox;
            newPicBox.Location = new Point(riga+70, colonna+70);
            newPicBox.Size = new Size(120, 100);

            
           if(utenti_online[counter_picBox].ha_la_foto != "2") {
                Image im = Image.FromFile(home_dir + "\\immagine_vuota.jpg");
                newPicBox.ImageLocation = home_dir + "\\immagine_vuota.jpg";
            }
            else
            {
                String extUpperCase = ".JPG";
                String extLowerCase = ".jpg";
                try
                {
                   // Image im = Image.FromFile(home_dir + "Immagini_utenti\\" + utenti_online[counter_picBox].MAC + extLowerCase );
                    

                    Image img;
                    using (var bmpTemp = new Bitmap(home_dir + "Immagini_utenti\\" + utenti_online[counter_picBox].MAC + extLowerCase))
                    {
                        img = new Bitmap(bmpTemp);
                        newPicBox.ImageLocation = home_dir + "Immagini_utenti\\" + utenti_online[counter_picBox].MAC + extLowerCase;
                    }
                }
                catch(Exception e)
                {
                    try
                    {
                        // Image im = Image.FromFile(home_dir + "Immagini_utenti\\" + utenti_online[counter_picBox].MAC + extUpperCase );
                        Image img;
                        using (var bmpTemp = new Bitmap(home_dir + "Immagini_utenti\\" + utenti_online[counter_picBox].MAC + extUpperCase))
                        {
                            img = new Bitmap(bmpTemp);
                            newPicBox.ImageLocation = home_dir + "Immagini_utenti\\" + utenti_online[counter_picBox].MAC + extUpperCase;
                        }
                        
                    }
                    catch (Exception exception)
                    {
                        this.Close();
                    }
                }
                
            }
            


            newPicBox.BorderStyle = BorderStyle.FixedSingle;
            //newPicBox.InitialImage = im;
            //newPicBox.Image = im;
            newPicBox.SizeMode = PictureBoxSizeMode.StretchImage;

            // newPicBox.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Stretch;
            System.Drawing.Drawing2D.GraphicsPath gp = new System.Drawing.Drawing2D.GraphicsPath();
            gp.AddEllipse(0, 0, newPicBox.Width - 3, newPicBox.Height - 3);
            Region rg = new Region(gp);
            newPicBox.Region = rg;

            counter_picBox++;
            //newPicBox.Show();

            return newPicBox;
        }
        private Label create_Label(int riga, int colonna, utente usr_corrente)
        {
            Label newTextBox = new Label();
            newTextBox.Name = "TextBox" + counter_textBox;
            newTextBox.Location = new Point(riga+80 ,colonna+210);
            newTextBox.ForeColor = Color.FromArgb(0, 53, 118);
            newTextBox.Text =  usr_corrente.Nome +" "+ usr_corrente.Cognome;

            newTextBox.Font = new Font("Segoe UI", 9, FontStyle.Regular);
            newTextBox.TextAlign = ContentAlignment.TopCenter;
            counter_textBox++;

            return newTextBox;
        }

        public void disegna_utenti() //qui metto un ciclo dove, in base al valore numero utenti in rete provo a creare dei box dinamicamente e posizionarli all'interno del form.l
        {
            utente utente_corrente;
            int pos_riga=0, pos_colonna=0;
            int pos_riga_picBox=0, pos_colonna_picBox = 0;
            int pos_riga_textBox = 0, pos_colonna_TextBox = 0;


            for (int i = 0; i < size; i++)
            {
                utente_corrente = utenti_online[i];

                if(pos_riga >= 500)
                {
                    pos_riga = 0;
                    pos_colonna = 200;
                    pos_riga_picBox = 0;
                    pos_colonna_picBox = 200;
                    pos_riga_textBox = 0;
                    pos_colonna_TextBox = 200;
                }


                Button newButton =CreateButton_Click(pos_riga, pos_colonna, utente_corrente);
                PictureBox newPic=create_picBox(pos_riga_picBox, pos_colonna_picBox, utente_corrente);
                Label newTextBox = create_Label(pos_riga_textBox, pos_colonna_TextBox, utente_corrente);

               
                this.Controls.Add(newButton);
                this.Controls.Add(newPic);
                this.Controls.Add(newTextBox);


                pos_riga_picBox += 180;
                pos_riga += 180;
                pos_riga_textBox += 180;

            }

        }

        private void SendFiles_Load(object sender, EventArgs e)
        {
            timer = new Timer();
            timer.Interval = (3 * 1000); // 10 secs
            timer.Tick += new EventHandler(timer_Tick);
            timer.Start();

            disegna_utenti();
        }

        private void timer_Tick(object sender, EventArgs e)
        {
            numero_utenti_in_rete = 0;
            counter_button = 0;
            counter_picBox = 0;
            counter_textBox = 0;
            refresh();
        }

        public unsafe void refresh()
        {
            MarshalVectorWrapper(conn, out utenti_info, out size);
            label1.Text = "";
            if (size != 0)
            {
                utenti_online = new utente[size];
                string[] tmp = new string[size];


                //Parse the info utenti got from DLL and save them into data structures

                for (int i = 0; i < size; i++)
                {
                    tmp[i] = Marshal.PtrToStringAnsi((IntPtr)utenti_info[i]);

                    string[] tmp2 = tmp[i].Split('|');

                    utenti_online[i].MAC = tmp2[0];
                    utenti_online[i].Nome = tmp2[1];
                    utenti_online[i].Cognome = tmp2[2];
                    utenti_online[i].ha_la_foto = tmp2[3];
                    utenti_online[i].bloccato = tmp2[4];

                }
            }
            else
            {
                label1.Text = "Non sono presenti utenti in rete";
            }

            disegna_utenti();
        }

        private void SendFiles_FormClosing(object sender, FormClosingEventArgs e)
        {
            
            if (closeEverything)
            {
                
                dad.Close();
            }
        }

        //---------------------------------------------------------------------------------------//

        //---------------------------------------------------------------//

        //------------------------------------------------------------------//

        #region wrapper
        [DllImport("FileSharingTest.dll")]
        public static unsafe extern bool MarshalVector(IntPtr conn, out  ItemsSafeHandle hItems, out char** ItemsData, out int ItemsCounter);

        [DllImport("FileSharingTest.dll")]
        public static extern bool deleteVector(IntPtr item);

        public class ItemsSafeHandle : SafeHandleZeroOrMinusOneIsInvalid
        {
                public ItemsSafeHandle() : base(true)
                {

                }

                protected override bool ReleaseHandle()
                {
                    return deleteVector(handle);
                }


        }
        static unsafe ItemsSafeHandle MarshalVectorWrapper( IntPtr conn, out  char** items, out  int itemsCount){

                    ItemsSafeHandle itemsHandle;

                    if (!MarshalVector(conn, out itemsHandle, out  items, out  itemsCount))
                    {
                        throw new InvalidOperationException();
                    }

                    return itemsHandle;

        }


#endregion 





    }


}



