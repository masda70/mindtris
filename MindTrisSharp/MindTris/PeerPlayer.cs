using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using MindTrisCore;

namespace MindTris
{
    class PeerPlayer : Peer
    {
        public PeerPlayer(Peer peer, User user)
        {
            ID = peer.ID;
            DisplayName = peer.DisplayName;
            IpAddress = peer.IpAddress;
            Port = peer.Port;
            PublicKey = peer.PublicKey;
            User = user;
        }

        public PeerPlayer(byte peerID, User user)
        {
            ID = peerID;
            User = user;
        }

        public User User { get; set; }
        public ulong InitiatingChallengeCode { get; set; }
        public ulong ListeningChallengeCode { get; set; }
        public byte[] RoundDataHash { get; set; }
    }
}
