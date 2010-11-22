using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net.Sockets;

namespace MindTrisCore
{
    class ServerResponse
    {
        Socket _socket;
        byte[] _packet;
        int _length;

        public ServerResponse(Socket socket, byte[] packet, int length)
        {
            _socket = socket;
            _packet = packet;
            _length = length;
            Offset = 0;
        }

        public Socket Socket { get { return _socket; } }
        public byte[] Packet { get { return _packet; } }
        public int Length { get { return _length; } }
        public int Offset { get; set; }
    }
}
