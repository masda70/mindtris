using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace MindTrisCore
{
    public class NetworkProtocol
    {
        //Protocol ID
        public const string DGMT = "DGMT";
        //Protocol version
        public const uint VERSION = 0x0000001;
        //Default port for early versions of the protocol
        public const int DEFAULT_PORT = 1337 + 42;

        //Lengths of fields, in bytes
        public const int PROTOCOL_ID_LENGTH = 4;
        public const int PACKET_LENGTH_LENGTH = 2;
        public const int PACKET_ID_LENGTH = 1;
        public const int HEADER_LENGTH = PROTOCOL_ID_LENGTH + PACKET_LENGTH_LENGTH + PACKET_ID_LENGTH;

        public const int PROTOCOL_VERSION_LENGTH = 4;

        public static bool DGMTCheck(byte[] buffer, int offset)
        {
            if (buffer.Length < offset + PROTOCOL_ID_LENGTH) return false;
            return Encoding.ASCII.GetString(buffer, offset, PROTOCOL_ID_LENGTH) == DGMT;
        }

        public enum PacketID
        {
            HelloFromClient = (byte) 0x00,
            CreateUser = 0x01,
            Login = 0x02,
            CreateLobby = 0x03, //CreateGame
            GetLobbyList = 0x04, //CreateGameList
            JoinLobby = 0x05,
            SendChatMessage = 0x06,

            HelloFromServer = 0x80,
            LoginSuccess = 0x81,
            LobbyCreated = 0x82,
            //...
        }
    }
}
