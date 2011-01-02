using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net;
using System.Net.Sockets;
using MindTrisCore;
using MindTrisCore.DGMTEncoding;
using System.Threading;
using System.Security.Cryptography;
using System.Collections;
using System.Text.RegularExpressions;
using System.Diagnostics;

namespace MindTris
{
    class Client
    {
        string _login;
        string _email;
        string _pass;
        //Server address
        IPAddress _server_address;
        int _server_port;

        ClientStatus _status;



        //Core

        //Socket connected to the server
        Socket _socket_server;
        string _server_public_key;
        RSACryptoServiceProvider _rsa_server;


        //Socket listening to peer connections
        Socket _listener;
        IPAddress _address;
        ushort _port;
        DSACryptoServiceProvider _dsa_client;
        DSACryptoServiceProvider _dsa_peer;
        Random _rand;

        //Incoming connections handling
        Thread _main;
        LinkedList<ClientRequest> _requests_to_grant;

        //Users
        Dictionary<Socket, User> _users;
        User _user_server;

        //Lobby
        Lobby _lobby;
        Dictionary<byte, Peer> _remaining_unseen_peers;
        Dictionary<User, PeerPlayer> _players;

        //Lag timings
        const int WELCOMING_LAG = 200;
        const int LISTENING_LAG = 50;
        const int CHECKING_ALIVE_LAG = 90000;

        //Server side scalability
        int _connection_count;
        const int MAX_CONNECTION_COUNT = 1000;

        public Client(string serverIP, int serverPort)
        {
            _server_address = IPAddress.Parse(serverIP);
            _server_port = serverPort;
            _socket_server = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
            _requests_to_grant = new LinkedList<ClientRequest>();
            _listener = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
            _listener.Blocking = false;
            _users = new Dictionary<Socket, User>(20);
            _players = new Dictionary<User, PeerPlayer>(20);
            _remaining_unseen_peers = new Dictionary<byte, Peer>();
            _rsa_server = new RSACryptoServiceProvider();
            _dsa_client = new DSACryptoServiceProvider();
            string lol = _dsa_client.ToXmlString(false);
            lol.ToString();
            _status = new ClientStatus();
        }

        public Client(string IP)
            : this(IP, Dgmt.DEFAULT_PORT) { }

        public void Connect()
        {
            try
            {
                _socket_server.BeginConnect(_server_address, _server_port, new AsyncCallback(CallbackConnect), null);
                _user_server = new User(_socket_server);
            }
            catch (SocketException e)
            {
                Console.WriteLine(e.ToString());
                return;
            }
            Console.WriteLine("Connecting...");
        }
        void CallbackConnect(IAsyncResult res)
        {
            //Start the engine
            _main = new Thread(new ThreadStart(Listening));
            _main.Name = "Main Client Thread";
            _main.Start();
            Send_HelloFromClient();
        }

        public void CreateUser(string login, string pass, string email)
        {
            _login = login;
            _pass = pass;
            _email = email;
            if (_status.Connected && !_status.Logged_on)
            {
                Send_CreateUser();
            }
        }

        public void Login(string login, string pass)
        {
            _login = login;
            _pass = pass;
            if (_status.Connected && !_status.Logged_on)
            {
                Send_Login();
            }
        }

        public void CreateLobby(string name, byte max_players, bool hasPassword, string pass)
        {
            if (_status.Logged_on)
            {
                Send_CreateLobby(name, max_players, hasPassword, pass);
            }
        }

        public void LobbyListRetrieval()
        {
            if (_status.Logged_on)
            {
                Send_RetrieveLobbies();
            }
        }

        public void JoinLobby(uint lobbyID, string pass)
        {
            if (_status.Logged_on)
            {
                Send_JoinLobby(lobbyID, pass);
            }
        }

        public void LeaveLobby()
        {
            if (_status.Logged_on && _status.Lobby_id != 0)
            {
                Send_LeaveLobby();
            }
        }

        public delegate void VoidFunction();
        public delegate void ConnectedFunction(string motd);
        public event ConnectedFunction Connected;
        public delegate void ConfirmationFunction(byte response);
        public event ConfirmationFunction UserCreated;
        public delegate void LoggedOnFunction(byte response, string displayed_username);
        public event LoggedOnFunction LoggedOn;
        public delegate void LobbyCreatedFunction(byte response, uint? lobbyID, byte? peerID, ulong? sessionID);
        public event LobbyCreatedFunction LobbyCreated;
        public delegate void RetrieveLobbiesFunction(Lobby[] lobbies);
        public event RetrieveLobbiesFunction LobbiesRetrieved;
        public delegate void JoinedLobbyFunction(byte response, byte? clientID, ulong? sessionID, Peer[] peers);
        public event JoinedLobbyFunction LobbyJoined;
        public event VoidFunction LobbyLeft;
        public delegate void UpdateFunction(byte update, Peer peer);
        public event UpdateFunction StatusUpdated;

        public delegate void MessageReceivedFunction(Peer peer, string message);
        public event MessageReceivedFunction MessageReceived;
        public delegate void PeerHandshakenFunction(Peer peer, ulong challengeNumber);
        public event PeerHandshakenFunction PeerHandshaken;
        public delegate void ConnectionAcceptedFunction(Peer peer, ulong challengeNumber, ulong listeningChallengeNumber);
        public event ConnectionAcceptedFunction ConnectionAccepted;
        public event ConnectionAcceptedFunction ConnectionAcknowledged;



        void Send_CreateUser()
        {
            //[TODO] Gérer les dépassement de tailles d'entier
            byte[] login = Encoding.UTF8.GetBytes(_login);
            byte[] email = Encoding.ASCII.GetBytes(_email);
            byte[] pass = Encoding.ASCII.GetBytes(_pass);

            //Copy all infos
            byte[] packet = Dgmt.ForgeNewPacket();
            int i = Dgmt.HEADER_LENGTH;
            packet[i++] = (byte)Dgmt.PacketID.CreateUser;
            BigE.WriteSizePrefixed(packet, ref i, 1, login);
            BigE.WriteSizePrefixed(packet, ref i, 1, login);
            BigE.WriteSizePrefixed(packet, ref i, 2, email);
            //Encrypt
            byte[] encrypted_user_info = Encrypt(pass);
            string lol2 = System.Convert.ToBase64String(encrypted_user_info);
            string public_key = _rsa_server.ToXmlString(false);
            BigE.WriteSizePrefixed(packet, ref i, 2, encrypted_user_info);

            int length = Dgmt.FinalizePacket(packet, i - Dgmt.HEADER_LENGTH);

            //Send
            ServerResponse response = new ServerResponse(_socket_server, packet, length);
            QueueResponse(response);
        }

        void Send_Login()
        {
            byte[] packet = Dgmt.ForgeNewPacket();
            int i = Dgmt.HEADER_LENGTH;
            BigE.WritePacketID(packet, ref i, Dgmt.PacketID.Login);
            BigE.WriteSizePrefixedUTF8(packet, ref i, 1, _login);
            BigE.WriteSizePrefixed(packet, ref i, 2, Encrypt(Encoding.ASCII.GetBytes(_pass)));

            int length = Dgmt.FinalizePacket(packet, i - Dgmt.HEADER_LENGTH);

            //Send
            ServerResponse response = new ServerResponse(_socket_server, packet, length);
            QueueResponse(response);
        }

        void Send_CreateLobby(string name, byte max_players, bool hasPass, string pass)
        {
            byte[] packet = Dgmt.ForgeNewPacket();
            int i = Dgmt.HEADER_LENGTH;
            BigE.WritePacketID(packet, ref i, Dgmt.PacketID.CreateLobby);
            BigE.WriteSizePrefixedUTF8(packet, ref i, 1, name);
            BigE.WriteByte(packet, ref i, max_players);
            BigE.WriteBool(packet, ref i, hasPass);
            if (hasPass)
            {
                byte[] pass_crypted = Encrypt(Encoding.ASCII.GetBytes(pass));
                BigE.WriteSizePrefixed(packet, ref i, 2, pass_crypted);
            }
            BigE.WriteInt32(packet, ref i, _port);
            BigE.WriteDSAPublicKey(packet, ref i, _dsa_client.ToXmlString(false));

            _lobby = new Lobby()
            {
                Creator = _status.User,
                ID = 0,
                Name = name,
                PasswordProtected = hasPass,
                PlayerCount = max_players,
            };

            //Send
            int length = Dgmt.FinalizePacket(packet, i - Dgmt.HEADER_LENGTH);
            ServerResponse response = new ServerResponse(_socket_server, packet, length);
            QueueResponse(response);
        }

        void Send_RetrieveLobbies()
        {
            byte[] packet = Dgmt.ForgeNewPacket();
            packet[Dgmt.HEADER_LENGTH] = (byte)Dgmt.PacketID.GetLobbyList;
            int length = Dgmt.FinalizePacket(packet, 1);
            //Send
            ServerResponse response = new ServerResponse(_socket_server, packet, length);
            QueueResponse(response);
        }

        void Send_JoinLobby(uint lobbyID, string pass)
        {
            byte[] packet = Dgmt.ForgeNewPacket();
            int i = Dgmt.HEADER_LENGTH;
            BigE.WritePacketID(packet, ref i, Dgmt.PacketID.JoinLobby);
            BigE.WriteInt32(packet, ref i, lobbyID);
            BigE.WriteSizePrefixedASCII(packet, ref i, 1, pass);
            BigE.WriteInt16(packet, ref i, _port);
            BigE.WriteDSAPublicKey(packet, ref i, _dsa_client.ToXmlString(false));

            int length = Dgmt.FinalizePacket(packet, i - Dgmt.HEADER_LENGTH);

            //Add to requests to grant
            ClientRequestJoinLobby request = new ClientRequestJoinLobby(lobbyID, pass);
            RegisterRequest(request);
            //Send to server
            ServerResponse response = new ServerResponse(_socket_server, packet, length);
            QueueResponse(response);
        }

        void Send_LeaveLobby()
        {
            byte[] packet = Dgmt.ForgeNewPacket();
            packet[Dgmt.HEADER_LENGTH] = (byte)Dgmt.PacketID.LeaveLobby;
            int length = Dgmt.FinalizePacket(packet, 1);

            ServerResponse response = new ServerResponse(_socket_server, packet, length);
            QueueResponse(response);
        }

        void Send_HelloFromClient()
        {
            byte[] packet = Dgmt.ForgeNewPacket();
            int i = Dgmt.HEADER_LENGTH;
            BigE.WritePacketID(packet, ref i, Dgmt.PacketID.HelloFromClient);
            BigE.WriteRawBytes(packet, ref i, Dgmt.VERSION);
            int length = Dgmt.FinalizePacket(packet, i - Dgmt.HEADER_LENGTH);

            //Sending
            Console.WriteLine("Sending...");
            ServerResponse response = new ServerResponse(_socket_server, packet, length);
            QueueResponse(response);
            Console.WriteLine("Sent.");
        }

        byte[] Encrypt(byte[] rgb)
        {
            return _rsa_server.Encrypt(rgb, true);
        }

        //Client side
        public void ConnectPeer(Peer peer)
        {
            Thread t = new Thread(() =>
                {
                    Socket client = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
                    client.Connect(peer.IpAddress, peer.Port);
                    // This is how you can determine whether a socket is still connected.
                    bool blockingState = client.Blocking;
                    try
                    {
                        byte[] tmp = new byte[1];
                        client.Blocking = false;
                        client.Send(tmp, 0, 0);
                        Console.WriteLine("{0}: Connected to this peer.", client.RemoteEndPoint);
                    }
                    catch (SocketException e)
                    {
                        // 10035 == WSAEWOULDBLOCK
                        if (e.NativeErrorCode.Equals(10035))
                            Console.WriteLine("Still Connected, but the Send would block");
                        else
                        {
                            Console.WriteLine("Disconnected: error code {0}!", e.NativeErrorCode);
                        }
                    }
                    finally
                    {
                        client.Blocking = blockingState;
                    }

                    if (client.Connected)
                    {
                        User user = new User(client);
                        user.Port = peer.Port;
                        user.PublicKey = peer.PublicKey;
                        PeerPlayer player = new PeerPlayer(peer, user);
                        lock (_users) { _users.Add(client, user); }
                        lock (_players) { _players.Add(user, player); }
                        ulong challengeNumber;
                        //On envoie l'HelloFromPeer
                        Send_ConnectionRequest(user);
                        user.UserStatus.Logged_on = true;
                    }
                    else
                    {
                        //[TODO] You're screwed
                    }
                }
            );
            t.Start();
        }

        public void SendMessage(string message)
        {
            Thread t = new Thread(() =>
                {
                    IEnumerable<User> list = null;
                    lock (_users)
                    {
                        list = _users.Values.Where(x => { return true; });
                    }
                    foreach (User user in list)
                    {
                        Send_ChatSend(user, message);
                    }
                }
            );
            t.Start();
        }

        //Server side
        public void StartListening(IPAddress address, ushort port)
        {
            _address = address;
            _port = port;

            //Local endpoint for the socket.
            IPEndPoint localEndPoint = new IPEndPoint(_address, _port);
            //Binding!
            _listener.Bind(localEndPoint);
            Console.WriteLine("Binding...");
            _listener.Listen(MAX_CONNECTION_COUNT);
            Console.WriteLine("Listening...");
        }

        public void Stop()
        {
            foreach (Socket socket in _users.Keys)
            {
                DisconnectUser(socket);
            }
            _main.Abort();
        }


        void Welcoming()
        {
            while (true)
            {
                Socket client;
                try
                {
                    client = _listener.Accept();
                }
                catch (SocketException)
                {
                    break;
                }
                Console.WriteLine("{0}: Attempting connection...", client.RemoteEndPoint);
                client.Blocking = false;

                _connection_count++;
                //Too many clients
                if (_connection_count > MAX_CONNECTION_COUNT)
                {
                    //[TOCHECK]
                    //Pour l'instant on kick sans autre forme de procès
                    client.Shutdown(SocketShutdown.Both);
                    client.Close();
                    client = null;
                    _connection_count--;
                }
                else
                {
                    //Add new user
                    User user = new User(client);
                    _users.Add(client, user);
                }
            }
        }

        void Listening()
        {
            while (true)
            {
                if (_listener.IsBound) Welcoming();
                IList list = _users.Keys.ToList<Socket>();
                //Add server socket to the list
                list.Add(_user_server.Socket);
                if (list.Count > 0)
                {
                    //Select
                    Socket.Select(list, null, null, LISTENING_LAG * 1000);
                    //Write socket data in local buffers
                    Receive(list);
                    //Process buffers
                    ProcessPackets();
                    //Send responses
                    SendResponses();
                    //[TODO] Check alive clients
                    //CheckAliveUsers();
                }
                else Thread.Sleep(LISTENING_LAG);
            }
        }

        void Receive(IList sockets)
        {
            foreach (Socket socket in sockets)
            {
                User user = GetUserFromSocket(socket);
                /*
                if (user.Locked) continue;
                user.Lock();
                //*/
                //Synchronous receive, supposed to be instantaneous
                try
                {
                    int qte = socket.Receive(user.Buffer.BufferRaw,
                        user.Buffer.BufferPosition,
                        Math.Max(socket.Available, Math.Max(
                            user.Buffer.WindowStart - user.Buffer.BufferPosition,
                            user.Buffer.Length - user.Buffer.BufferPosition)),
                        SocketFlags.None
                        );
                    user.Buffer.WindowLength += qte;
                    //If extra window space is available, write additional data to fill the window
                    if (socket.Available > 0 && user.Buffer.BufferPosition < user.Buffer.WindowStart)
                    {
                        qte = socket.Receive(user.Buffer.BufferRaw,
                            user.Buffer.BufferPosition,
                            Math.Max(socket.Available, user.Buffer.WindowStart - user.Buffer.BufferPosition),
                            SocketFlags.None
                            );
                        user.Buffer.WindowLength += qte;
                    }
                }
                catch (SocketException)
                {
                    DisconnectUser(socket);
                }
            }
        }

        void SendResponses()
        {
            foreach (User user in _users.Values.Concat<User>(new List<User>(){ _user_server }))
            {
                if (user.SendingPending.Count == 0) continue;
                ServerResponse response = user.SendingPending.First.Value;
                SocketError err;
                int qte = response.Socket.Send(response.Packet, response.Offset, response.Length - response.Offset, SocketFlags.None, out err);
                switch (err)
                {
                    case SocketError.WouldBlock:
                        continue;
                }
                response.Offset += qte;
                //If completed, remove the packet from pending responses
                if (response.Offset >= response.Length)
                    user.SendingPending.RemoveFirst();
            }
        }

        void ProcessPackets()
        {
            //Server
            ProcessServerPacket(_user_server);
            //Peers
            foreach (User user in _users.Values)
            {
                ProcessUserPacket(user);
            }
        }

        void ProcessUserPacket(User user)
        {
            //If the data do not even contain a full header, skip
            if (user.Buffer.WindowLength < Dgmt.HEADER_LENGTH) return;
            //Header check
            byte[] minibuffer = user.Buffer.GetSubbufferCopy(0, Dgmt.HEADER_LENGTH);
            if (Dgmt.DGMTCheck(minibuffer, 0))
            {
                int i = Dgmt.PROTOCOL_ID_LENGTH;
                ushort packet_length = BigE.ReadInt16(minibuffer, ref i);
                //If the whole packet has not been read yet, skip
                if (user.Buffer.WindowLength < packet_length) return;
                ushort content_length = (ushort)(packet_length - Dgmt.HEADER_LENGTH);
                if (content_length <= 0)
                {
                    //It's a keep-alive, we update
                    user.LastTimeSeen = DateTime.Now;
                }
                else
                {
                    //Grab the message type
                    byte id = user.Buffer[Dgmt.PROTOCOL_ID_LENGTH + Dgmt.PACKET_LENGTH_LENGTH];
                    //Process accordingly
                    switch ((Dgmt.PacketP2PID)id)
                    {
                        case Dgmt.PacketP2PID.ConnectionRequest:
                            Process_ConnectionRequest(user, content_length);
                            break;
                        case Dgmt.PacketP2PID.ConnectionAccepted:
                            Process_ConnectionAccepted(user, content_length);
                            break;
                        case Dgmt.PacketP2PID.ConnectionAcknowledged:
                            Process_ConnectionAcknowledged(user, content_length);
                            break;
                        case Dgmt.PacketP2PID.ChatSend:
                            Process_ChatSend(user, content_length);
                            break;
                        default:
                            throw new NotImplementedException("Unknown message type, or the feature is not implemented yet.");
                    }
                }
                //Update the window
                user.Buffer.WindowStart += packet_length;
                user.Buffer.WindowLength -= packet_length;
            }
            else
            {
                Console.WriteLine("{0}: DGMT failed, user will be disconnected.", user.Socket.RemoteEndPoint);
                //Solution temporaire
                DisconnectUser(user.Socket);
            }
        }

        void ProcessServerPacket(User user)
        {
            //If the data do not even contain a full header, skip
            if (user.Buffer.WindowLength < Dgmt.HEADER_LENGTH) return;
            //Header check
            byte[] minibuffer = user.Buffer.GetSubbufferCopy(0, Dgmt.HEADER_LENGTH);
            if (Dgmt.DGMTCheck(minibuffer, 0))
            {
                int i = Dgmt.PROTOCOL_ID_LENGTH;
                ushort packet_length = BigE.ReadInt16(minibuffer, ref i);
                //If the whole packet has not been read yet, skip
                if (user.Buffer.WindowLength < packet_length) return;
                ushort content_length = (ushort)(packet_length - Dgmt.HEADER_LENGTH);
                if (content_length <= 0)
                {
                    //It's a keep-alive, we update
                    user.LastTimeSeen = DateTime.Now;
                }
                else
                {
                    //Grab the message type
                    byte id = user.Buffer[Dgmt.PROTOCOL_ID_LENGTH + Dgmt.PACKET_LENGTH_LENGTH];
                    //Process accordingly
                    switch ((Dgmt.PacketID)id)
                    {
                        case Dgmt.PacketID.HelloFromServer:
                            Process_HelloFromServer(user, content_length);
                            break;
                        case Dgmt.PacketID.UserCreation:
                            Process_CreateUser(user, content_length);
                            break;
                        case Dgmt.PacketID.LoginReply:
                            Process_Login(user, content_length);
                            break;
                        case Dgmt.PacketID.LobbyCreation:
                            Process_CreateLobby(user, content_length);
                            break;
                        case Dgmt.PacketID.LobbyList:
                            Process_RetrieveLobbyList(user, content_length);
                            break;
                        case Dgmt.PacketID.JoinedLobby:
                            Process_JoinLobby(user, content_length);
                            break;
                        case Dgmt.PacketID.UpdateClientStatus:
                            Process_UpdateClientStatus(user, content_length);
                            break;
                        default:
                            throw new NotImplementedException("Unknown message type, or the feature is not implemented yet.");
                    }
                }
                //Update the window
                user.Buffer.WindowStart += packet_length;
                user.Buffer.WindowLength -= packet_length;
            }
            else
            {
                Console.WriteLine("{0}: DGMT failed, user will be disconnected.", user.Socket.RemoteEndPoint);
                //Solution temporaire
                DisconnectUser(user.Socket);
            }
        }

        void CheckAliveUsers()
        {
            LinkedList<Socket> to_del = new LinkedList<Socket>();
            //Check
            foreach (User user in _users.Values)
            {
                TimeSpan interval = DateTime.Now.Subtract(user.LastTimeSeen);
                if (interval.TotalMilliseconds > CHECKING_ALIVE_LAG) to_del.AddLast(user.Socket);
            }
            //Disconnect
            foreach (Socket socket in to_del)
            {
                DisconnectUser(socket);
            }
            to_del.Clear();
        }

        void DisconnectUser(Socket socket)
        {
            User user = _users[socket];
            _users.Remove(socket);
            if (_players.ContainsKey(user)) _players.Remove(user);
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

        User GetUserFromSocket(Socket socket)
        {
            if (Object.ReferenceEquals(_socket_server, socket))
            {
                //Server
                return _user_server;
            }
                //Peer
            else return _users[socket];
        }

        void QueueResponse(ServerResponse response)
        {
            User user = GetUserFromSocket(response.Socket);
            lock (user.SendingPending)
            {
                user.SendingPending.AddLast(response);
            }
        }

        void RegisterRequest(ClientRequest request)
        {
            lock (_requests_to_grant)
            {
                _requests_to_grant.AddLast(request);
            }
        }


        //Process Packets from peers
        void Process_ConnectionRequest(User user, int content_length)
        {
            if (!user.UserStatus.Logged_on)
            {
                Console.WriteLine("{0}: Handshaking.", user.Socket.RemoteEndPoint);
                int i = Dgmt.HEADER_LENGTH;
                byte[] packet = user.Buffer.GetSubbufferCopy(i, content_length);
                //Skip the type
                i = 1;
                uint lobbyID = BigE.ReadInt32(packet, ref i);
                byte peerID = BigE.ReadByte(packet, ref i);
                byte listeningPeerID = BigE.ReadByte(packet, ref i);
                ulong challengeNumber = BigE.ReadInt64(packet, ref i);
                //Check lobby
                if (_lobby.ID != lobbyID) return;
                //Check destination
                if (_status.Peer_id != listeningPeerID) return;
                if (!_remaining_unseen_peers.ContainsKey(peerID)) return;
                Peer matching_peer = _remaining_unseen_peers[peerID];
                user.Port = matching_peer.Port;
                user.PublicKey = matching_peer.PublicKey;
                PeerPlayer player = new PeerPlayer(matching_peer, user);
                lock (_players) { _players.Add(user, player); }
                lock (_remaining_unseen_peers) { _remaining_unseen_peers.Remove(peerID); }
                user.UserStatus.Logged_on = true;

                if (PeerHandshaken != null) PeerHandshaken(player, challengeNumber);
            }
        }

        void Process_ConnectionAccepted(User user, int content_length)
        {
            if (user.UserStatus.Logged_on)
            {
                Console.WriteLine("{0}: is accepting the connection.", user.Socket.RemoteEndPoint);
                int i = Dgmt.HEADER_LENGTH;
                byte[] packet = user.Buffer.GetSubbufferCopy(i, content_length);
                //Skip the type
                i = 1;
                //Find the matching request in the registered requests
                ClientRequestConnectionRequest request = null;
                foreach (ClientRequest request2 in _requests_to_grant)
                {
                    if (request2 is ClientRequestConnectionRequest)
                    {
                        request = (ClientRequestConnectionRequest)request2;
                        break;
                    }
                }
                if (request == null)
                {
                    //If a JoinLobby request has never been sent, just skip this
                    return;
                }
                else _requests_to_grant.Remove(request);
                int j = i;
                
                uint lobbyID = BigE.ReadInt32(packet, ref i);
                byte peerID = BigE.ReadByte(packet, ref i);
                byte listeningPeerID = BigE.ReadByte(packet, ref i);
                ulong challengeNumber = BigE.ReadInt64(packet, ref i);
                ulong listeningChallengeNumber = BigE.ReadInt64(packet, ref i);
                //On copie pour vérifier la signature
                byte[] to_sign = user.Buffer.GetSubbufferCopy(j, i - j);
                byte[] signatureDSA = BigE.ReadSizePrefixedRawBytes(packet, ref i, 2);

                //Là faire des tests
                if (lobbyID == request.LobbyID &&
                    peerID == request.PeerID &&
                    listeningPeerID == request.ListeningPeerID &&
                    challengeNumber == request.ChallengeNumber)
                {
                    _dsa_peer.FromXmlString(user.PublicKey);
                    if (_dsa_peer.VerifyData(to_sign, signatureDSA))
                    {
                        //Tout est bon, dans le cochon
                        Peer peer = _players[user];
                        if (ConnectionAccepted != null) ConnectionAccepted(peer, challengeNumber, listeningChallengeNumber);
                        return;
                    }
                    Console.WriteLine("{0}: bad DSA signature.", user.Socket.RemoteEndPoint);
                }
                DisconnectUser(user.Socket);
            }
        }

        void Process_ConnectionAcknowledged(User user, int content_length)
        {
            if (user.UserStatus.Logged_on)
            {
                Console.WriteLine("{0}: is acknowledging your accepting the connection.", user.Socket.RemoteEndPoint);
                int i = Dgmt.HEADER_LENGTH;
                byte[] packet = user.Buffer.GetSubbufferCopy(i, content_length);
                //Skip the type
                i = 1;
                //Find the matching request in the registered requests
                ClientRequestConnectionAccepted request = null;
                foreach (ClientRequest request2 in _requests_to_grant)
                {
                    if (request2 is ClientRequestConnectionAccepted)
                    {
                        request = (ClientRequestConnectionAccepted)request2;
                        break;
                    }
                }
                if (request == null)
                {
                    //If a JoinLobby request has never been sent, just skip this
                    return;
                }
                else _requests_to_grant.Remove(request);
                int j = i;

                uint lobbyID = BigE.ReadInt32(packet, ref i);
                byte peerID = BigE.ReadByte(packet, ref i);
                byte listeningPeerID = BigE.ReadByte(packet, ref i);
                ulong challengeNumber = BigE.ReadInt64(packet, ref i);
                ulong listeningChallengeNumber = BigE.ReadInt64(packet, ref i);
                //On copie pour vérifier la signature
                byte[] to_sign = user.Buffer.GetSubbufferCopy(j, i - j);
                byte[] signatureDSA = BigE.ReadSizePrefixedRawBytes(packet, ref i, 2);

                //Là faire des tests
                if (lobbyID == request.LobbyID &&
                    peerID == request.PeerID &&
                    listeningPeerID == request.ListeningPeerID &&
                    challengeNumber == request.ChallengeNumber)
                {
                    _dsa_peer.FromXmlString(user.PublicKey);
                    if (_dsa_peer.VerifyData(to_sign, signatureDSA))
                    {
                        //Tout est bon, dans le cochon
                        Peer peer = _players[user];
                        if (ConnectionAcknowledged != null) ConnectionAcknowledged(peer, challengeNumber, listeningChallengeNumber);
                        return;
                    }
                    Console.WriteLine("{0}: bad DSA signature.", user.Socket.RemoteEndPoint);
                }
                DisconnectUser(user.Socket);
            }
        }

        void Process_ChatSend(User user, int content_length)
        {
            if (user.UserStatus.Logged_on)
            {
                int i = Dgmt.HEADER_LENGTH;
                byte[] packet = user.Buffer.GetSubbufferCopy(i, content_length);
                //Skip the type
                i = 1;
                byte[] signature = BigE.ReadSizePrefixedRawBytes(packet, ref i, 2);
                int j = i;
                ulong sessionID = BigE.ReadInt64(packet, ref i);
                string message = BigE.ReadSizePrefixedUTF8(packet, ref i, 2);
                byte[] to_sign = user.Buffer.GetSubbufferCopy(j, i - j);

                _dsa_peer.FromXmlString(user.PublicKey);
                if (!_dsa_peer.VerifyData(to_sign, signature))
                {
                    Console.WriteLine("{0}: Bad signature on sent message.", user.Socket.RemoteEndPoint);
                }

                Console.WriteLine("{0} says: {1}", user.Socket.RemoteEndPoint, message);

                if (MessageReceived != null) MessageReceived(_players[user], message);
            }
        }

        //Process Packets from server

        void Process_HelloFromServer(User user, int content_length)
        {
            int i = Dgmt.HEADER_LENGTH;
            byte[] response = user.Buffer.GetSubbufferCopy(i, content_length);
            i = 1;
            //End
            switch (response[i])
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
            string motd;
            if (response[i] == 0x00)
            {
                //Récupérer la public key du serveur
                i++;
                _server_public_key = BigE.ReadRSAPublicKey(response, ref i);
                _rsa_server.FromXmlString(_server_public_key);
                motd = BigE.ReadSizePrefixedUTF8(response, ref i, 2);
                //update status
                _status.Connected = true;
                Console.WriteLine("Connected:");
                Console.WriteLine("Receiving MOTD...");
                Console.WriteLine(motd);
            }
            else motd = "";
            
            if (Connected != null) Connected(motd);
        }

        void Process_CreateUser(User user, int content_length)
        {
            int i = Dgmt.HEADER_LENGTH;
            byte[] response = user.Buffer.GetSubbufferCopy(i, content_length);
            byte res = response[1];
            if (UserCreated != null) UserCreated(res);
        }

        void Process_Login(User user, int content_length)
        {
            string displayed_username = "";
            int i = Dgmt.HEADER_LENGTH;
            byte[] response = user.Buffer.GetSubbufferCopy(i, content_length);
            if (response[1] == 0x00)
            {
                i = 2;
                displayed_username = BigE.ReadSizePrefixedUTF8(response, ref i, 1);
                //update le status
                _status.Logged_on = true;
                _status.User = _login;
            }
            if (LoggedOn != null) LoggedOn(response[1], displayed_username);
        }

        void Process_CreateLobby(User user, int content_length)
        {
            int i = Dgmt.HEADER_LENGTH;
            byte[] response = user.Buffer.GetSubbufferCopy(i, content_length);
            i = 1;
            uint lobbyID;
            byte peerID;
            ulong sessionID;
            byte answer = BigE.ReadByte(response, ref i);
            if (answer == 0x00)
            {
                lobbyID = BigE.ReadInt32(response, ref i);
                peerID = BigE.ReadByte(response, ref i);
                sessionID = BigE.ReadInt64(response, ref i);
            }
            else
            {
                lobbyID = 0;
                peerID = 0;
                sessionID = 0;
            }
            //Creator is by default joining his lobby
            _status.Peer_id = peerID;
            _status.Creator_lobby_id = lobbyID;
            _status.Lobby_id = lobbyID;
            _status.Session_id = sessionID;
            _lobby.ID = lobbyID;
            _lobby.PlayerCount = 1;

            if (LobbyCreated != null) LobbyCreated(answer, lobbyID, peerID, sessionID);
        }

        void Process_RetrieveLobbyList(User user, int content_length)
        {
            int i = Dgmt.HEADER_LENGTH;
            byte[] response = user.Buffer.GetSubbufferCopy(i, content_length);
            i = 1;
            int count = BigE.ReadByte(response, ref i);
            Lobby[] result = new Lobby[count];
            for (int k = 0; k < count; k++)
            {
                result[k] = new Lobby();
                result[k].ID = BigE.ReadInt32(response, ref i);
                result[k].Name = BigE.ReadSizePrefixedUTF8(response, ref i, 1);
                result[k].PlayerCount = BigE.ReadByte(response, ref i);
                result[k].PlayerMaxCount = BigE.ReadByte(response, ref i);
                result[k].PasswordProtected = BigE.ReadBool(response, ref i);
                result[k].Creator = BigE.ReadSizePrefixedUTF8(response, ref i, 1);
            }
            if (LobbiesRetrieved != null) LobbiesRetrieved(result);
        }

        void Process_JoinLobby(User user, int content_length)
        {
            int i = Dgmt.HEADER_LENGTH;
            byte[] response = user.Buffer.GetSubbufferCopy(i, content_length);
            i = 1;
            uint lobbyID = BigE.ReadInt32(response, ref i);
            //Find the matching request in the registered requests
            ClientRequestJoinLobby request = null;
            foreach (ClientRequest request2 in _requests_to_grant)
            {
                if (request2 is ClientRequestJoinLobby)
                {
                    request = (ClientRequestJoinLobby)request2;
                    break;
                }
            }
            if (request == null)
            {
                //If a JoinLobby request has never been sent, just skip this
                return;
            }
            else _requests_to_grant.Remove(request);
            if (request.LobbyID != lobbyID)
            {
                //[TODO]Comment on fait si on avait demandé un autre lobby ? Wtf?
            }
            byte answer = BigE.ReadByte(response, ref i);
            Peer[] peers;
            if (answer == 0x00)
            {
                _lobby = new Lobby();
                _lobby.Name = BigE.ReadSizePrefixedUTF8(response, ref i, 1);
                _lobby.PlayerMaxCount = BigE.ReadByte(response, ref i);
                _lobby.CreatorPeerID = BigE.ReadByte(response, ref i);
                _status.Peer_id = BigE.ReadByte(response, ref i);
                _status.Session_id = BigE.ReadInt64(response, ref i);
                byte count = BigE.ReadByte(response, ref i);
                peers = new Peer[count];
                for (byte k = 0; k < count; k++)
                {
                    peers[k] = new Peer();
                    peers[k].ID = BigE.ReadByte(response, ref i);
                    peers[k].DisplayName = BigE.ReadSizePrefixedUTF8(response, ref i, 1);
                    peers[k].IpAddress = BigE.ReadIPAddress(response, ref i);
                    peers[k].Port = BigE.ReadInt16(response, ref i);
                    peers[k].PublicKey = BigE.ReadDSAPublicKey(response, ref i);
                }
                _status.Lobby_id = lobbyID;
                //Add to the list of peers who will connect later on
                foreach (Peer peer in peers)
                {
                    _remaining_unseen_peers.Add(peer.ID, peer);
                }
            }
            else
            {
                _status.Peer_id = 0;
                _status.Session_id = 0;
                peers = null;
            }
            if (LobbyJoined != null) LobbyJoined(answer, _status.Peer_id, _status.Session_id, peers);
        }

        void Process_UpdateClientStatus(User user, int content_length)
        {
            int i = Dgmt.HEADER_LENGTH;
            byte[] response = user.Buffer.GetSubbufferCopy(i, content_length);
            i = 1;
            Peer peer = new Peer();
            uint lobbyID = BigE.ReadInt32(response, ref i);
            peer.ID = BigE.ReadByte(response, ref i);
            byte answer = BigE.ReadByte(response, ref i);
            if (answer == 0x00)
            {
                peer.DisplayName = BigE.ReadSizePrefixedUTF8(response, ref i, 1);
                peer.IpAddress = BigE.ReadIPAddress(response, ref i);
                peer.Port = BigE.ReadInt16(response, ref i);
                peer.PublicKey = BigE.ReadDSAPublicKey(response, ref i);
            }
            if (StatusUpdated != null) StatusUpdated(answer, peer);
            if (peer.ID == _status.Peer_id)
            {
                //[TOCHECK] On fire les 2 events pour l'instant
                if (LobbyLeft != null) LobbyLeft();
            }
        }

        //Send Packets to peers
        void Send_ConnectionRequest(User user)
        {
            byte[] packet = Dgmt.ForgeNewPacket();
            int i = Dgmt.HEADER_LENGTH;
            BigE.WritePacketIDP2P(packet, ref i, Dgmt.PacketP2PID.ConnectionRequest);
            BigE.WriteInt32(packet, ref i, _lobby.ID);
            Debug.Assert(_status.Peer_id != null);
            BigE.WriteByte(packet, ref i, (byte)_status.Peer_id);
            Debug.Assert(_status.Peer_id != null);
            BigE.WriteByte(packet, ref i, (byte)user.UserStatus.Peer_id);
            //Initiation du challenge
            byte[] bytes = new byte[8];
            _rand.NextBytes(bytes);
            ulong challengeNumber = BitConverter.ToUInt64(bytes, 0);
            BigE.WriteInt64(packet, ref i, challengeNumber);
            int length = Dgmt.FinalizePacket(packet, i - Dgmt.HEADER_LENGTH);

            //Add to requests to grant
            ClientRequestConnectionRequest request = new ClientRequestConnectionRequest(_lobby.ID, (byte)_status.Peer_id, (byte)user.UserStatus.Peer_id, challengeNumber);
            RegisterRequest(request);
            //Send
            ServerResponse response = new ServerResponse(user.Socket, packet, length);
            QueueResponse(response);
        }

        void Send_ConnectionAccepted(User user, ulong challengeNumber)
        {
            byte[] packet = Dgmt.ForgeNewPacket();
            int i = Dgmt.HEADER_LENGTH;
            BigE.WritePacketIDP2P(packet, ref i, Dgmt.PacketP2PID.ConnectionAccepted);
            int j = i;
            BigE.WriteInt32(packet, ref i, _lobby.ID);
            BigE.WriteByte(packet, ref i, (byte)user.UserStatus.Peer_id);
            BigE.WriteByte(packet, ref i, (byte)_status.Peer_id);
            BigE.WriteInt64(packet, ref i, challengeNumber);
            //Initiation du listening challenge
            byte[] bytes = new byte[8];
            _rand.NextBytes(bytes);
            ulong listeningChallengeNumber = BitConverter.ToUInt64(bytes, 0);
            BigE.WriteInt64(packet, ref i, listeningChallengeNumber);
            byte[] signed = _dsa_client.SignData(packet, j, i - j);
            BigE.WriteSizePrefixed(packet, ref i, 2, signed);
            int length = Dgmt.FinalizePacket(packet, i - Dgmt.HEADER_LENGTH);

            ClientRequestConnectionAccepted request = new ClientRequestConnectionAccepted(_lobby.ID, (byte)_status.Peer_id, (byte)user.UserStatus.Peer_id, challengeNumber, listeningChallengeNumber);
            RegisterRequest(request);
            //Send
            ServerResponse response = new ServerResponse(user.Socket, packet, length);
            QueueResponse(response);
        }

        void Send_ConnectionAcknowledged(User user, ulong challengeNumber, ulong listeningChallengeNumber)
        {
            byte[] packet = Dgmt.ForgeNewPacket();
            int i = Dgmt.HEADER_LENGTH;
            BigE.WritePacketIDP2P(packet, ref i, Dgmt.PacketP2PID.ConnectionAccepted);
            int j = i;
            BigE.WriteInt32(packet, ref i, _lobby.ID);
            BigE.WriteByte(packet, ref i, (byte)_status.Peer_id);
            BigE.WriteByte(packet, ref i, (byte)user.UserStatus.Peer_id);
            BigE.WriteInt64(packet, ref i, challengeNumber);
            BigE.WriteInt64(packet, ref i, listeningChallengeNumber);
            byte[] signed = _dsa_client.SignData(packet, j, i - j);
            BigE.WriteSizePrefixed(packet, ref i, 2, signed);
            int length = Dgmt.FinalizePacket(packet, i - Dgmt.HEADER_LENGTH);

            //Send
            ServerResponse response = new ServerResponse(user.Socket, packet, length);
            QueueResponse(response);
        }

        void Send_ChatSend(User user, string message)
        {
            byte[] packet = Dgmt.ForgeNewPacket();
            int i = Dgmt.HEADER_LENGTH;
            BigE.WritePacketIDP2P(packet, ref i, Dgmt.PacketP2PID.ChatSend);

            //On construit d'abord le reste du message, qui doit être signé
            byte[] reste = new byte[Dgmt.BUFFER_MAX_LENGTH];
            int j = 0;
            BigE.WriteInt64(reste, ref j, (ulong)_status.Session_id);
            BigE.WriteSizePrefixedUTF8(reste, ref j, 2, message);
            //[TOCHECK] Resize fait bien ce qu'on veut ?
            Array.Resize(ref reste, j);

            //Signature
            byte[] sign = _dsa_client.SignData(reste);
            BigE.WriteSizePrefixed(packet, ref i, 2, sign);
            //Ecriture du reste du message
            BigE.WriteRawBytes(packet, ref i, reste);
            int length = Dgmt.FinalizePacket(packet, i - Dgmt.HEADER_LENGTH);

            //Send
            ServerResponse response = new ServerResponse(user.Socket, packet, length);
            QueueResponse(response);
        }
    }
}
