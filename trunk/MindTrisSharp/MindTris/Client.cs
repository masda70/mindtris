using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net;
using System.Net.Sockets;
using MindTrisCore;
using MindTrisCore.DGMTEncoding;

namespace MindTris
{
    class Client
    {
        string _login;
        //Server address
        IPAddress _address;
        int _port;

        //Core
        Socket _socket;

        public Client(string login, string IP, int port)
        {
            _login = login;
            _address = IPAddress.Parse(IP);
            _port = port;
            _socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
        }

        public Client(string login, string IP)
            : this(login, IP, NetworkProtocol.DEFAULT_PORT) { }

        public void HelloFromClient()
        {
            try
            {
                _socket.Connect(_address, _port);
            }
            catch (SocketException e)
            {
                Console.WriteLine(e.ToString());
                return;
            }
            Console.WriteLine("Connecting...");

            int size = NetworkProtocol.PROTOCOL_ID_LENGTH +
                NetworkProtocol.PACKET_LENGTH_LENGTH +
                NetworkProtocol.PACKET_ID_LENGTH +
                NetworkProtocol.PROTOCOL_VERSION_LENGTH;
            byte[] packet = new byte[size];
            int i = 0;
            Encoding.ASCII.GetBytes(NetworkProtocol.DGMT).CopyTo(packet, i);
            ushort length = (ushort)size;
            i += NetworkProtocol.PROTOCOL_ID_LENGTH;
            BitConverter.GetBytes(length).CopyTo(packet, i);
            BigE.E(packet, i, NetworkProtocol.PACKET_LENGTH_LENGTH);
            i += NetworkProtocol.PACKET_LENGTH_LENGTH;
            packet[i] = (byte)NetworkProtocol.PacketID.HelloFromClient;
            i++;
            BitConverter.GetBytes(NetworkProtocol.VERSION).CopyTo(packet, i);
            BigE.E(packet, i, NetworkProtocol.PROTOCOL_VERSION_LENGTH);
            //Sending
            Console.WriteLine("Sending...");
            _socket.Send(packet, 0, packet.Length, SocketFlags.None);
            Console.WriteLine("Sent.");

            //Receving
            ReceiveServerResponse();
        }

        void ReceiveServerResponse()
        {
            byte[] buffer = new byte[NetworkProtocol.HEADER_LENGTH];
            int qte = 0;
            while (qte < NetworkProtocol.HEADER_LENGTH)
            {
                qte += _socket.Receive(buffer, qte, NetworkProtocol.HEADER_LENGTH - qte, SocketFlags.None);
            }
            //Here, the header is complete
            //Checks
            //...
            //Read la length
            BigE.E(buffer, NetworkProtocol.PROTOCOL_ID_LENGTH, NetworkProtocol.PACKET_LENGTH_LENGTH);
            ushort content_length = (ushort)(BitConverter.ToUInt16(buffer, NetworkProtocol.PROTOCOL_ID_LENGTH) - NetworkProtocol.HEADER_LENGTH);
            //Continue receiving
            buffer = new byte[content_length];
            qte = 0;
            while (qte < content_length)
            {
                qte += _socket.Receive(buffer, qte, content_length - qte, SocketFlags.None);
            }
            //End
            switch (buffer[0])
            {
                case 0x00:
                    Console.WriteLine("Connection success.");
                    break;
                case 0x01:
                    Console.WriteLine("Connection failed. Wrong protocol version number.");
                    break;
                case 0x02:
                    Console.WriteLine("Connection failed. Unknown error.");
                    break;
                default:
                    Console.WriteLine("This was not supposed to happen, lol.");
                    break;
            }
            if (buffer[0] == 0x00)
            {
                string motd = Encoding.UTF8.GetString(buffer, 1, buffer.Length - 1);
                Console.WriteLine("Connected:");
                Console.WriteLine("Receiving MOTD...");
                Console.WriteLine(motd);
            }
        }
    }
}
