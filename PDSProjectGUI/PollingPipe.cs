using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PDSProjectGUI
{
    class PollingPipe
    {
        private Task tk;

        PollingPipe()
        {
            //Create here the pipe and hook it with c++s one




            //After pipes are synchronized
            poll_the_pipe();

        }

        private async void poll_the_pipe()
        {
            tk = new Task(polling);
            tk.Start();
            //await tk;
        }

        private void polling()
        {
            //insert here the pipe polling
            while (true)
            {

                //read the pipe (should be blocking)

                //create and launch the receiver form passing it all data about the sender


            }

        }
    }
}
