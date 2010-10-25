using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net.Sockets;
using System.Threading;

namespace MindTrisServer
{
    internal class User
    {
        Socket _socket;
        bool _locked;

        public User(Socket socket)
        {
            _socket = socket;
        }

        public void Lock()
        {
            _locked = true;
        }
        public void Unlock()
        {
            _locked = false;
        }
        
        public Socket Socket { get { return _socket; } }
        public UserState UserState { get; set; }
        public bool Locked { get { return _locked; } }
    }

    internal enum UserState
    {
        None = 0,
        ConnectionPending,
        Connected
    }
}
