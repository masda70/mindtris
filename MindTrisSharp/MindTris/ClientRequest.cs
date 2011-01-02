using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace MindTris
{
    class ClientRequest
    {

    }
    //*/

    class ClientRequestJoinLobby : ClientRequest
    {
        uint _lobbyID;
        string _pass;

        public ClientRequestJoinLobby(uint lobbyID, string pass)
        {
            _lobbyID = lobbyID;
            _pass = pass;
        }

        public uint LobbyID { get { return _lobbyID; } }
        public string Pass { get { return _pass; } }
    }

    class ClientRequestConnectionRequest : ClientRequest
    {
        uint _lobbyID;
        byte _peerID;
        byte _listeningPeerID;
        ulong _challengeNumber;

        public ClientRequestConnectionRequest(uint lobbyID, byte peerID, byte listeningPeerID, ulong challengeNumber)
        {
            _lobbyID = lobbyID;
            _peerID = peerID;
            _listeningPeerID = listeningPeerID;
            _challengeNumber = challengeNumber;
        }

        public uint LobbyID { get { return _lobbyID; } }
        public byte PeerID { get { return _peerID; } }
        public byte ListeningPeerID { get { return _listeningPeerID; } }
        public ulong ChallengeNumber { get { return _challengeNumber; } }
    }

    class ClientRequestConnectionAccepted : ClientRequestConnectionRequest
    {
        ulong _listeningChallengeNumber;

        public ClientRequestConnectionAccepted(uint lobbyID, byte peerID, byte listeningPeerID, ulong challengeNumber, ulong listeningChallengeNumber) :
            base(lobbyID, peerID, listeningPeerID, challengeNumber)
        {
            _listeningChallengeNumber = listeningChallengeNumber;
        }

        public ulong ListeningChallengeNumber { get { return _listeningChallengeNumber; } }
    }
}
