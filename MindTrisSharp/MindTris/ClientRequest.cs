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
}
