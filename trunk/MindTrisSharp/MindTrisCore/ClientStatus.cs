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
            Connected = false;
            Logged_on = false;
            User = "";
            Lobby_id = null;
            Peer_id = null;
            Creator_lobby_id = null;
            Session_id = null;
            Am_playing = false;
        }
        public bool Connected { get; set; }
        public bool Logged_on { set; get; } //or handshaken
        public string User { get; set; }
        public uint? Lobby_id { get; set; }
        public byte? Peer_id { get; set; }
        public uint? Creator_lobby_id { get; set; }
        public ulong? Session_id { get; set; }
        public bool Am_playing { get; set; }
        //Il faudra inclure le client public key
    }
}
