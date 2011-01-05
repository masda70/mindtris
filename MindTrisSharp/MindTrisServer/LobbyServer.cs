using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using MindTrisCore;

namespace MindTrisServer
{
    class LobbyServer : Lobby
    {
        public User UserCreator { get; set; }
        public string Password { get; set; }
        public ulong SessionID { get; set; }
        public RandomGenerator Generator { get; set; }

        public byte GeneratePlayerID()
        {
            var list = Players.Select(x => { return x.ID; }).OrderBy(x => { return x; });
            byte id = 0;
            foreach (byte ids in list)
            {
                if (id != ids) break;
                id++;
            }
            return id;
            throw new Exception("Should not be thrown, use GeneratePlayerID only if there is enough room for another player in the lobby.");
        }

        PeerServer GetPeerServerFromUser(User user)
        {
            PeerServer peer = null;
            foreach (PeerServer peerouze in Players)
            {
                if (Object.ReferenceEquals(peerouze.User, user)) peer = peerouze;
            }
            return peer;
        }

        public LinkedList<PeerServer> Players { get; set; }
        public PeerServer this[User user]
        {
            get
            {
                return GetPeerServerFromUser(user);
            }
        }
    }
}
