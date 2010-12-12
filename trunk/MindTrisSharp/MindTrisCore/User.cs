using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net.Sockets;
using System.Security.Cryptography;

namespace MindTrisCore
{
    class User
    {
        Socket _socket;
        BufferWindow _buffer;
        ClientStatus _status;
        LinkedList<ServerResponse> _responses_to_send;

        public User(Socket socket)
        {
            _socket = socket;
            _buffer = new BufferWindow(Dgmt.BUFFER_MAX_LENGTH);
            _responses_to_send = new LinkedList<ServerResponse>();
            _status = new ClientStatus();
            LastTimeSeen = DateTime.Now;
        }

        public Socket Socket { get { return _socket; } }
        public BufferWindow Buffer { get { return _buffer; } }
        public LinkedList<ServerResponse> SendingPending { get { return _responses_to_send; } }
        public ClientStatus UserStatus { get { return _status; } }
        public DateTime LastTimeSeen { get; set; }

        public ushort Port { get; set; }
        public string PublicKey { get; set; }
        public RSACryptoServiceProvider RSACrypto { get; set; }
    }
}
