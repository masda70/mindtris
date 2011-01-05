using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using MindTrisCore.DGMTEncoding;

namespace MindTrisCore
{
    public class Dgmt
    {
        //Protocol ID
        public const string DGMT_STRING = "DGMT";
        public static readonly byte[] DGMT_BYTES = Encoding.ASCII.GetBytes(DGMT_STRING);
        public const string DGMTP2P_STRING = "DGMTP2P";
        public static readonly byte[] DGMTP2P_BYTES = Encoding.ASCII.GetBytes(DGMTP2P_STRING);
        //Protocol version
        public static readonly byte[] VERSION = { 1, 2, 0, 3 };
        //Default port for early versions of the protocol
        public const int DEFAULT_PORT = 1337 + 42;

        public const int PACKET_MAX_LENGTH = 1024;
        public const int BUFFER_MAX_LENGTH = 100 * PACKET_MAX_LENGTH;

        //Lengths of fields, in bytes
        public const int PROTOCOL_ID_LENGTH = 4;
        public const int PACKET_LENGTH_LENGTH = 2;
        public const int PACKET_ID_LENGTH = 1;
        public const int HEADER_LENGTH = PROTOCOL_ID_LENGTH + PACKET_LENGTH_LENGTH;

        public const int PROTOCOL_P2P_ID_LENGTH = 7;
        public const int HEADER_P2P_LENGTH = PROTOCOL_P2P_ID_LENGTH + PACKET_LENGTH_LENGTH;

        public const int PROTOCOL_VERSION_LENGTH = 4;

        //Regex
        public const string REGEX_LOGIN = @"[:alpha:][:alnum:_.-]*";
        public const string REGEX_PASSWORD = @"^.*(?=.{6,})(?=.*[:alnum:])(?=.*[:punct:]).*$";
        public const string REGEX_EMAIL = @"^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,4}$";

        public static bool DGMTCheck(byte[] buffer, int offset)
        {
            if (buffer.Length < offset + PROTOCOL_ID_LENGTH) return false;
            return Encoding.ASCII.GetString(buffer, offset, PROTOCOL_ID_LENGTH) == DGMT_STRING;
        }

        public static bool DGMTP2PCheck(byte[] buffer, int offset)
        {
            if (buffer.Length < offset + PROTOCOL_P2P_ID_LENGTH) return false;
            return Encoding.ASCII.GetString(buffer, offset, PROTOCOL_P2P_ID_LENGTH) == DGMTP2P_STRING;
        }

        public static byte[] ForgeNewPacket()
        {
            byte[] res = new byte[Dgmt.BUFFER_MAX_LENGTH];
            return res;
        }

        public static int FinalizePacket(byte[] packet, int content_length)
        {
            int size = Dgmt.PROTOCOL_ID_LENGTH +
                Dgmt.PACKET_LENGTH_LENGTH +
                content_length
                ;
            DGMT_BYTES.CopyTo(packet, 0);
            ushort length = (ushort)size;
            BitConverter.GetBytes(length).CopyTo(packet, Dgmt.PROTOCOL_ID_LENGTH);
            //On assure le BigEndian
            BigE.E(packet, Dgmt.PROTOCOL_ID_LENGTH, Dgmt.PACKET_LENGTH_LENGTH);
            //return the full length of the packet
            return size;
        }

        public static int FinalizePacketP2P(byte[] packet, int content_length)
        {
            int size = Dgmt.PROTOCOL_P2P_ID_LENGTH +
                Dgmt.PACKET_LENGTH_LENGTH +
                content_length
                ;
            DGMTP2P_BYTES.CopyTo(packet, 0);
            ushort length = (ushort)size;
            BitConverter.GetBytes(length).CopyTo(packet, Dgmt.PROTOCOL_P2P_ID_LENGTH);
            //On assure le BigEndian
            BigE.E(packet, Dgmt.PROTOCOL_P2P_ID_LENGTH, Dgmt.PACKET_LENGTH_LENGTH);
            //return the full length of the packet
            return size;
        }

        public enum PacketID
        {
            HelloFromClient = (byte)0x00,
            CreateUser = 0x01,
            Login = 0x02,
            CreateLobby = 0x03,
            GetLobbyList = 0x04,
            JoinLobby = 0x05,
            LeaveLobby = 0x06,
            StartGame = 0x10,
            LoadedGame = 0x11,
            GiveNewPieces = 0x13,

            HelloFromServer = 0x80,
            UserCreation = 0x81,
            LoginReply = 0x82,
            LobbyCreation = 0x83,
            LobbyList = 0x84,
            JoinedLobby = 0x85,
            

            UpdateClientStatus = 0x88,

            GameStarting = 0x90,
            LoadGame = 0x91,
            BeginGame = 0x92,
            NewPieces = 0x93
            //...
        }

        public enum PacketP2PID
        {
            ConnectionRequest = (byte)0x00,
            ConnectionAccepted = 0x01,
            ConnectionAcknowledged = 0x02,
            ChatSend = 0x10,
            Round = 0x11
        }
    }

    public class DgmtProtocolException : Exception
    {
        public DgmtProtocolException(string message) : base(message) { }
    }

    public class DgmtWithoutRequestHackzorzException : Exception
    {
        byte[] _contents;

        //Advised by masda70
        public DgmtWithoutRequestHackzorzException(string message, byte[] contents) { _contents = contents; }
        public byte[] Contents { get { return _contents; } }
    }
}
