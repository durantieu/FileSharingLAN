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

namespace PDSProjectGUI
{
    public partial class SendFiles : Form
    {
       [DllImport("FileSharingTest.dll")]
       public static extern IntPtr getUtentiConnessi(IntPtr conn);

        string path;
        int numero_utenti_in_rete=0;
        int counter_button = 0;
        int counter_picBox = 0;
        IntPtr utenti_attivi;

        public SendFiles(string pt, IntPtr connection)
        {
            path = pt;
            InitializeComponent();
            //utenti_attivi = getUtentiConnessi(connection);
            
            //acchiappo il numero di utenti in rete  e lo assegno a numero_utenti_in_rete

        }
        private void NewButton_Click(object sender, EventArgs e)
        {
            Button btn = (Button)sender;

            for (int i = 0; i < 8; i++)
            {
                if (btn.Name == ("Butt" + i))
                {
                    //qui scrivo le funzioni che devono essere richiamate quando scelgo un utente a cui inviare ()
                    //dovrò chiamare il lancio di un client per l'invio di un file verso l'utente identificato da questo bottone
                    break;
                }
            }
        }
        private Button CreateButton_Click(int i, int j)
        { 
            Button button = new Button();
            
            
            // Posiziono il bottone
            button.Name = "Butt" + counter_button;
            // button.Text = "Invia a "; //aggiungere nome e cognome utente
            button.BackgroundImage = System.Drawing.Image.FromFile("C:\\Users\\duran\\Desktop\\Immagini interfaccia progetto PDS\\Invia50x40.png");
            button.ForeColor = Color.Black;
            button.BackColor = Color.White;
            button.BackgroundImageLayout = ImageLayout.Center;
            button.FlatStyle = FlatStyle.Flat;
            button.FlatAppearance.BorderSize = 1;

            
            button.Location = new Point(i+80, j+170);
            button.Size = new Size(100, 40);
           
           
            // incremento l'identificatore.
            counter_button++;
            button.Show();
           //aggiungo l'evento Click al nuovo bottone
            button.Click += new EventHandler(NewButton_Click);
            return button;
        }
        private PictureBox create_picBox(int riga, int colonna)
        {
           

            PictureBox newPicBox = new PictureBox();
            newPicBox.Name = "PicBox"+counter_picBox;
            newPicBox.Location = new Point(riga+70, colonna+70);
            newPicBox.Size = new Size(120, 100);

            Image im = Image.FromFile("C:\\Users\\duran\\Documents\\Immagini_utenti\\immagine_vuota.jpg");
            newPicBox.ImageLocation = "C:\\Users\\duran\\Documents\\Immagini_utenti\\immagine_vuota.jpg";
            newPicBox.BorderStyle = BorderStyle.FixedSingle;
            
            newPicBox.InitialImage = im;
            newPicBox.Image = im;
            newPicBox.SizeMode = PictureBoxSizeMode.StretchImage;
            // newPicBox.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Stretch;
            System.Drawing.Drawing2D.GraphicsPath gp = new System.Drawing.Drawing2D.GraphicsPath();
            gp.AddEllipse(0, 0, newPicBox.Width - 3, newPicBox.Height - 3);
            Region rg = new Region(gp);
            newPicBox.Region = rg;



            counter_picBox++;
            newPicBox.Show();


            return newPicBox;

        }


        private void disegna_utenti() //qui metto un ciclo dove, in base al valore numero utenti in rete provo a creare dei box dinamicamente e posizionarli all'interno del form.l
        {
            int pos_riga=0, pos_colonna=0;
            int pos_riga_picBox=0, pos_colonna_picBox = 0;
            for (int i = 0; i < 10; i++)
            {
                if(pos_riga >= 500)
                {
                    pos_riga = 0;
                    pos_colonna = 180;
                    pos_riga_picBox = 0;
                    pos_colonna_picBox = 180;
                }

                Button newButton =CreateButton_Click(pos_riga, pos_colonna);
                PictureBox newPic =  create_picBox(pos_riga_picBox, pos_colonna_picBox);

                this.Controls.Add(newButton);
                this.Controls.Add(newPic);

                pos_riga_picBox += 140;
                pos_riga += 140;

            }

        }

        private void SendFiles_Load(object sender, EventArgs e)
        {
            disegna_utenti();
        }



    }
}
