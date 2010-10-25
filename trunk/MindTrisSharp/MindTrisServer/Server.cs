using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using System.IO;
using System.Collections;
using MindTrisCore;

namespace MindTrisServer
{
    public class Server
    {
        string _name;
        IPAddress _address;
        int _port;
        const string _motd = "Welcome to {0}, a MindTris server. ;)";

        //Core
        Socket _listener;

        //Incoming connections handling
        Thread _welcoming;

        //Demands handling
        Thread _listening;

        //Users
        Dictionary<Socket, User> _users;

        //Lag timings
        const int WELCOMING_LAG = 200;
        const int LISTENING_LAG = 50;
        //const int CHECKING_LAG = 1000;

        //Scalability
        int _connection_count;
        const int MAX_CONNECTION_COUNT = 1000;

        //Tricks
        ManualResetEvent _reset;

        public Server(string name, string IP, int port)
        {
            _name = name;
            _address = IPAddress.Parse(IP);
            _port = port;
            _connection_count = 0;
            _listener = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
            _reset = new ManualResetEvent(false);
            _users = new Dictionary<Socket, User>(MAX_CONNECTION_COUNT);
        }

        public Server(string name, string IP)
            : this(name, IP, NetworkProtocol.DEFAULT_PORT) { }

        public void Start()
        {
            //Local endpoint for the socket.
            IPEndPoint localEndPoint = new IPEndPoint(_address, _port);
            //Binding!
            _listener.Bind(localEndPoint);
            Console.WriteLine("Binding...");

            _welcoming = new Thread(new ThreadStart(Welcoming));
            _welcoming.Priority = ThreadPriority.BelowNormal;
            _welcoming.Start();
            _listening = new Thread(new ThreadStart(Listening));
            _listening.Start();
        }

        public void Stop()
        {
            foreach (Socket socket in _users.Keys)
            {
                DisconnectUser(socket);
            }
            _welcoming.Abort();
            _listening.Abort();
        }

        void Welcoming()
        {
            _listener.Listen(MAX_CONNECTION_COUNT);
            Console.WriteLine("Listening...");
            while (true)
            {
                _reset.Reset();
                _listener.BeginAccept(new AsyncCallback(WelcomingSingleClient), _listener);
                _reset.WaitOne();
            }
        }

        void WelcomingSingleClient(IAsyncResult res)
        {
            //Unblocks main welcoming thread
            _reset.Set();
            Socket listener = (Socket)res.AsyncState;
            Socket client = listener.EndAccept(res);

            Console.WriteLine("{0}: Attempting connection...", client.RemoteEndPoint);
            
            Interlocked.Increment(ref _connection_count);
            //Too many clients
            if (_connection_count > MAX_CONNECTION_COUNT)
            {
                //Pour l'instant on kick sans autre forme de procès
                client.Shutdown(SocketShutdown.Both);
                client.Close();
                client = null;
                Interlocked.Decrement(ref _connection_count);
            }
            else
            {
                //Add new user
                User user = new User(client);
                user.UserState = UserState.ConnectionPending;
                lock (_users)
                {
                    _users.Add(client, user);
                }
            }
        }

        void Listening()
        {
            //ArrayList list = new ArrayList(MAX_CONNECTION_COUNT);
            while (true)
            {
                IList list = _users.Keys.ToList<Socket>();
                if (list.Count > 0)
                {
                    Socket.Select(list, null, null, LISTENING_LAG * 1000);
                    Receive(list);
                }
                else Thread.Sleep(LISTENING_LAG);
            }
        }

        void Receive(IList sockets)
        {
            foreach (Socket socket in sockets)
            {
                //Creating minibuffer for headers
                byte[] buffer = new byte[NetworkProtocol.HEADER_LENGTH];
                if (_users[socket].Locked) continue;
                _users[socket].Lock();
                //Asynchronous receive
                try
                {
                    socket.BeginReceive(
                        buffer,
                        0,
                        NetworkProtocol.HEADER_LENGTH,
                        SocketFlags.None,
                        new AsyncCallback(ReceiveFromSingleClient),
                        new Pair<Socket, byte[]>(socket, buffer)
                        );
                }
                catch (SocketException)
                {
                    DisconnectUser(socket);
                }
            }
        }

        void ReceiveFromSingleClient(IAsyncResult res)
        {
            var state = (Pair<Socket, byte[]>)res.AsyncState;
            Socket socket = state.Fst;
            byte[] buffer = state.Snd;
            User user = _users[socket];
            //End receive
            int qte = socket.EndReceive(res);
            //Continue receiving until a complete header is read
            while (qte < NetworkProtocol.HEADER_LENGTH)
            {
                qte += socket.Receive(buffer, qte, NetworkProtocol.HEADER_LENGTH - qte, SocketFlags.None);
            }
            //Header check
            if (NetworkProtocol.DGMTCheck(buffer, 0))
            {
                Console.WriteLine("{0}: DGMT passed.", socket.RemoteEndPoint);
                //Attention, dépend du protocole : sizeof(ushort) == PACKET_LENGTH_LENGTH
                ushort content_length = (ushort)(BitConverter.ToUInt16(buffer, NetworkProtocol.PROTOCOL_ID_LENGTH) - NetworkProtocol.HEADER_LENGTH);
                byte id = buffer[NetworkProtocol.PROTOCOL_ID_LENGTH + NetworkProtocol.PACKET_LENGTH_LENGTH];
                //Pour l'instant on fait qu'un test kikoo pour valider la connection
                //On check le HELLO FROM CLIENT
                if (user.UserState == UserState.ConnectionPending &&
                    (id == (byte)NetworkProtocol.PacketID.HelloFromClient) &&
                    content_length == NetworkProtocol.PROTOCOL_VERSION_LENGTH)
                {
                    Console.WriteLine("{0}: HelloFromClient passed.", socket.RemoteEndPoint);
                    //On vérifie la version du protocole
                    buffer = new byte[NetworkProtocol.PROTOCOL_VERSION_LENGTH];
                    //Read all
                    qte = 0;
                    while (qte < content_length)
                    {
                        qte += socket.Receive(buffer, qte, content_length - qte, SocketFlags.None);
                    }
                    Console.WriteLine("{0}: Version number #{1}", socket.RemoteEndPoint, BitConverter.ToUInt32(buffer, 0));
                    if (BitConverter.ToUInt32(buffer, 0) == NetworkProtocol.VERSION)
                    {
                        //Connection success
                        HelloFromServer(user, 0x00);
                    }
                    else
                    {
                        HelloFromServer(user, 0x01);
                    }
                }
                else HelloFromServer(user, 0x02);
            }
            else HelloFromServer(user, 0x02);
            user.Unlock();
        }

        void HelloFromServer(User user, byte response)
        {
            user.UserState = UserState.Connected;
            string motd = String.Format(_motd, _name);
            int size = NetworkProtocol.PROTOCOL_ID_LENGTH +
                NetworkProtocol.PACKET_LENGTH_LENGTH +
                NetworkProtocol.PACKET_ID_LENGTH +
                1 + Encoding.UTF8.GetByteCount(motd);
            byte[] packet = new byte[size];
            int i = 0;
            Encoding.ASCII.GetBytes(NetworkProtocol.DGMT).CopyTo(packet, i);
            ushort length = (ushort)size;
            i += NetworkProtocol.PROTOCOL_ID_LENGTH;
            BitConverter.GetBytes(length).CopyTo(packet, i);
            i += NetworkProtocol.PACKET_LENGTH_LENGTH;
            packet[i] = (byte)NetworkProtocol.PacketID.HelloFromServer;
            i++;
            packet[i] = response;
            i++;
            if (response == 0x00) Encoding.UTF8.GetBytes(motd).CopyTo(packet, i);

            //Sending
            Console.WriteLine("{0}: Sending MOTD to client...", user.Socket.RemoteEndPoint);
            user.Socket.Send(packet, 0, packet.Length, SocketFlags.None);
        }

        void DisconnectUser(Socket socket)
        {
            _users.Remove(socket);
            Console.WriteLine("{0}: Disconnected.", socket.RemoteEndPoint);
            try
            {
                socket.Shutdown(SocketShutdown.Both);
            }
            finally
            {
                socket.Close();
            }
        }
    }
}
