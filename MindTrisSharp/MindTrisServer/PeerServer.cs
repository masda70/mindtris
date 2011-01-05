using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using MindTrisCore;
using System.Net;

namespace MindTrisServer
{
    class PeerServer : Peer
    {
        public PeerServer(LobbyServer lobby, User user, string display_name)
        {
            DisplayName = display_name;
            ID = lobby.GeneratePlayerID();
            IpAddress = ((IPEndPoint)user.Socket.RemoteEndPoint).Address;
            Port = user.Port;
            PublicKey = user.PublicKey;
            User = user;
            IsGameLoaded = false;
        }

        public User User { get; set; }
        public bool IsGameLoaded { get; set; }
    }
}
