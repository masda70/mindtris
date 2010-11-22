using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace MindTrisCore
{
    public class ClientStatus
    {
        public ClientStatus()
        {
            Connected = 0;
            Logged_on = 0;
            User = "";
            Lobby = 0;
            Am_playing = 0;
        }
        public int Connected { get; set; }
        public int Logged_on { set; get; } //or handshaken
        public string User { get; set; }
        public uint Lobby { get; set; }
        public int Am_playing { get; set; }
        //Il faudra inclure le client public key
    }
}
