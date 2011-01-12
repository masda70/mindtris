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
    public class Client
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
        DateTime _lastTimeSeen;


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
        Dictionary<byte, Peer> _remaining_unconnected_peers;
        Dictionary<byte, PeerPlayer> _remaining_untrusted_peers;
        Dictionary<User, PeerPlayer> _players;

        SHA1CryptoServiceProvider _sha;
        const int EXPECTED_HASH_SIZE = 20;
        byte[] _roundDataHash;

        //Lag timings
        const int WELCOMING_LAG = 200;
        const int LISTENING_LAG = 50;
        const int CHECKING_ALIVE_LAG = 90000;
        const int SEND_KEEP_ALIVE_LAG = 30000;

        //Server side scalability
        int _connection_count;
        const int MAX_CONNECTION_COUNT = 1000;

        //Game Mechanics
        public const int ROUND_DELAY_MILLISECONDS = 100;

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
            _remaining_unconnected_peers = new Dictionary<byte, Peer>();
            _remaining_untrusted_peers = new Dictionary<byte, PeerPlayer>();
            _rsa_server = new RSACryptoServiceProvider();
            _dsa_client = new DSACryptoServiceProvider();
            _dsa_peer = new DSACryptoServiceProvider();
            _sha = new SHA1CryptoServiceProvider();
            Debug.Assert(_sha.HashSize / 8 == EXPECTED_HASH_SIZE);
            _rand = new Random();
            string lol = _dsa_client.ToXmlString(false);
            lol.ToString();
            _status = new ClientStatus();
            _lastTimeSeen = DateTime.Now;
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

        public void CreateLobby(string name, byte max_players, bool hasPassword, string pass, ushort port)
        {
            if (_status.Logged_on)
            {
                Send_CreateLobby(name, max_players, hasPassword, pass, port);
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

        public void StartGame()
        {
            if (_status.Creator_lobby_id == _lobby.ID)
            {
                Send_StartGame();
            }
        }

        public void RequestNewPieces(uint offset, byte count)
        {
            if (_status.Am_playing)
            {
                Send_RequestNewPieces(offset, count);
            }
        }

        public void SignalGameLoaded(bool error, ICollection<Peer> not_connected)
        {
            if (_status.Connected && _status.Lobby_id != null)
            {
                Send_LoadedGame(error, not_connected);
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
        public delegate void ReceivingNewPieces(uint offset, byte[] pieces);
        public event ConfirmationFunction GameStarting;
        public event ReceivingNewPieces GameLoaded;
        public event VoidFunction BeginGame;
        public event ReceivingNewPieces NewPiecesIncoming;
        public delegate void MoveFunction(Peer peer, uint roundNumber, Move[] moves);
        public event MoveFunction MoveMade;

        public delegate void MessageReceivedFunction(Peer peer, string message);
        public event MessageReceivedFunction MessageReceived;
        public delegate void PeerHandshakenFunction(Peer peer);
        public event PeerHandshakenFunction PeerHandshaken;
        



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

        void Send_CreateLobby(string name, byte max_players, bool hasPass, string pass, ushort port)
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
            _port = port;
            BigE.WriteInt16(packet, ref i, _port);
            string key = _dsa_client.ToXmlString(false);
            BigE.WriteDSAPublicKey(packet, ref i, key);

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

        void Send_KeepAlive()
        {
            byte[] packet = Dgmt.ForgeNewPacket();
            int length = Dgmt.FinalizePacket(packet, 0);

            ServerResponse response = new ServerResponse(_socket_server, packet, length);
            QueueResponse(response);
        }

        void Send_StartGame()
        {
            byte[] packet = Dgmt.ForgeNewPacket();
            packet[Dgmt.HEADER_LENGTH] = (byte)Dgmt.PacketID.StartGame;
            int length = Dgmt.FinalizePacket(packet, 1);

            ServerResponse response = new ServerResponse(_socket_server, packet, length);
            QueueResponse(response);
        }

        void Send_RequestNewPieces(uint offset, byte count)
        {
            byte[] packet = Dgmt.ForgeNewPacket();
            int i = Dgmt.HEADER_LENGTH;
            BigE.WritePacketID(packet, ref i, Dgmt.PacketID.GiveNewPieces);
            BigE.WriteInt32(packet, ref i, offset);
            BigE.WriteByte(packet, ref i, count);

            int length = Dgmt.FinalizePacket(packet, i - Dgmt.HEADER_LENGTH);

            //Send to server
            ServerResponse response = new ServerResponse(_socket_server, packet, length);
            QueueResponse(response);
        }

        void Send_LoadedGame(bool error, ICollection<Peer> not_connected)
        {
            byte[] packet = Dgmt.ForgeNewPacket();
            int i = Dgmt.HEADER_LENGTH;
            BigE.WritePacketID(packet, ref i, Dgmt.PacketID.LoadedGame);
            if (error && not_connected != null)
            {
                BigE.WriteByte(packet, ref i, 0x01);
                BigE.WriteByte(packet, ref i, (byte)not_connected.Count);
                foreach (Peer peer in not_connected)
                {
                    BigE.WriteByte(packet, ref i, peer.ID);
                }
            }
            else BigE.WriteByte(packet, ref i, 0x00);

            int length = Dgmt.FinalizePacket(packet, i - Dgmt.HEADER_LENGTH);

            //Send to server
            ServerResponse response = new ServerResponse(_socket_server, packet, length);
            QueueResponse(response);
        }

        byte[] Encrypt(byte[] rgb)
        {
            return _rsa_server.Encrypt(rgb, true);
        }

        //Client side
        public void ConnectPeer(Peer peer)
        {
            //[TODO] Retirer le thread ici
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
                        user.UserStatus.Peer_id = peer.ID;
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

        public void SendRoundPacket(uint roundNumber, ICollection<Move> moves)
        {
            if (_status.Am_playing)
            {
                if (moves != null)
                {
                    Send_Round(roundNumber, moves);
                }
                else Send_Round(roundNumber, new List<Move>());
            }
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
                    lock (_users)
                    {
                        _users.Add(client, user);
                    }
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
                    //Keep yourself alive
                    if (DateTime.Now.Subtract(_lastTimeSeen).TotalMilliseconds > SEND_KEEP_ALIVE_LAG)
                    {
                        _lastTimeSeen = DateTime.Now;
                        Send_KeepAlive();
                    }
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
                        Math.Min(socket.Available, Math.Max(
                            user.Buffer.WindowStart - user.Buffer.BufferPosition,
                            user.Buffer.Length - user.Buffer.BufferPosition
                            )),
                        SocketFlags.None
                        );
                    user.Buffer.WindowLength += qte;
                    //If extra window space is available, write additional data to fill the window
                    if (socket.Available > 0 && user.Buffer.BufferPosition < user.Buffer.WindowStart)
                    {
                        qte = socket.Receive(user.Buffer.BufferRaw,
                            user.Buffer.BufferPosition,
                            Math.Min(socket.Available, user.Buffer.WindowStart - user.Buffer.BufferPosition),
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
            //On effectue une copie, des fois que le processing modifie _users
            LinkedList<User> copie = new LinkedList<User>(_users.Values);
            //Peers
            foreach (User user in copie)
            {
                ProcessUserPacket(user);
            }
        }

        void ProcessUserPacket(User user)
        {
            //If the data do not even contain a full header, skip
            if (user.Buffer.WindowLength < Dgmt.HEADER_P2P_LENGTH) return;
            //Header check
            byte[] minibuffer = user.Buffer.GetSubbufferCopy(0, Dgmt.HEADER_P2P_LENGTH);
            if (Dgmt.DGMTP2PCheck(minibuffer, 0))
            {
                int i = Dgmt.PROTOCOL_P2P_ID_LENGTH;
                ushort packet_length = BigE.ReadInt16(minibuffer, ref i);
                //If the whole packet has not been read yet, skip
                if (user.Buffer.WindowLength < packet_length) return;
                ushort content_length = (ushort)(packet_length - Dgmt.HEADER_P2P_LENGTH);
                if (content_length <= 0)
                {
                    //It's a keep-alive, we update
                    user.LastTimeSeen = DateTime.Now;
                }
                else
                {
                    //Grab the message type
                    Dgmt.PacketP2PID id = (Dgmt.PacketP2PID)user.Buffer[Dgmt.HEADER_P2P_LENGTH];
                    //Process accordingly
                    switch (id)
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
                        case Dgmt.PacketP2PID.Round:
                            Process_Round(user, content_length);
                            break;
                        default:
                            throw new NotImplementedException("Unknown message type, or the feature is not implemented yet.");
                    }
                }
                //Update the window
                user.Buffer.WindowStart = (user.Buffer.WindowStart + packet_length) % user.Buffer.Length;
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
                    Dgmt.PacketID id = (Dgmt.PacketID)user.Buffer[Dgmt.PROTOCOL_ID_LENGTH + Dgmt.PACKET_LENGTH_LENGTH];
                    //Process accordingly
                    switch (id)
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
                        case Dgmt.PacketID.GameStarting:
                            Process_GameStarting(user, content_length);
                            break;
                        case Dgmt.PacketID.LoadGame:
                            Process_LoadGame(user, content_length);
                            break;
                        case Dgmt.PacketID.BeginGame:
                            Process_BeginGame(user, content_length);
                            break;
                        case Dgmt.PacketID.NewPieces:
                            Process_NewPieces(user, content_length);
                            break;
                        default:
                            throw new NotImplementedException("Unknown message type, or the feature is not implemented yet.");
                    }
                }
                //Update the window
                user.Buffer.WindowStart = (user.Buffer.WindowStart + packet_length) % user.Buffer.Length;
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
            if (_users.ContainsKey(socket))
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
                _lobby.ID = lobbyID;
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
                //[TOCHECK] On charge le programme utilisant la classe "Client" de se connecter au peers, renvoyés dans l'event LobbyJoined
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
            uint lobbyID = BigE.ReadInt32(response, ref i);
            byte answer = BigE.ReadByte(response, ref i);
            byte peerID = BigE.ReadByte(response, ref i);
            Peer peer = null;
            if (answer == 0x00)
            {
                string displayName = BigE.ReadSizePrefixedUTF8(response, ref i, 1);
                IPAddress address = BigE.ReadIPAddress(response, ref i);
                ushort port = BigE.ReadInt16(response, ref i);
                string public_key = BigE.ReadDSAPublicKey(response, ref i);
                if (_remaining_untrusted_peers.ContainsKey(peerID))
                {
                    peer = _remaining_untrusted_peers[peerID];
                    //On complète les infos de l'user
                    User userouze = UserFromPeerID(peerID);
                    userouze.Port = port;
                    userouze.PublicKey = public_key;
                    userouze.UserStatus.User = displayName;
                    _remaining_untrusted_peers.Remove(peerID);
                }
                else
                {
                    peer = new Peer();
                    _remaining_unconnected_peers[peerID] = peer;
                }
                peer.DisplayName = displayName;
                peer.ID = peerID;
                peer.IpAddress = address;
                peer.Port = port;
                peer.PublicKey = public_key;
            }
            else
            {
                User userouze = UserFromPeerID(peerID);
                if (userouze == null) return;
                //peer = _players[user];
            }

            if (StatusUpdated != null) StatusUpdated(answer, peer);
            if (peer.ID == _status.Peer_id)
            {
                //[TOCHECK] On fire les 2 events pour l'instant
                if (LobbyLeft != null) LobbyLeft();
            }
        }

        void Process_GameStarting(User user, int content_length)
        {
            int i = Dgmt.HEADER_LENGTH;
            byte[] response = user.Buffer.GetSubbufferCopy(i, content_length);
            i = 1;
            byte answer = BigE.ReadByte(response, ref i);
            if (answer == 0x00)
            {
                if (GameStarting != null) GameStarting(answer);
            }
        }

        void Process_LoadGame(User user, int content_length)
        {
            int i = Dgmt.HEADER_LENGTH;
            byte[] response = user.Buffer.GetSubbufferCopy(i, content_length);
            i = 1;
            byte count = BigE.ReadByte(response, ref i);
            byte[] pieces = new byte[count];
            for (int j = 0; j < count; j++)
            {
                pieces[j] = BigE.ReadByte(response, ref i);
            }
            if (GameLoaded != null) GameLoaded(0, pieces);
        }

        void Process_BeginGame(User user, int content_length)
        {
            int i = Dgmt.HEADER_LENGTH;
            byte[] response = user.Buffer.GetSubbufferCopy(i, content_length);
            i = 1;
            _status.Am_playing = true;
            foreach (User userouze in _players.Keys)
            {
                userouze.UserStatus.Am_playing = true;
            }
            if (BeginGame != null) BeginGame();
        }

        void Process_NewPieces(User user, int content_length)
        {
            int i = Dgmt.HEADER_LENGTH;
            byte[] response = user.Buffer.GetSubbufferCopy(i, content_length);
            i = 1;
            uint offset = BigE.ReadInt32(response, ref i);
            byte[] pieces = BigE.ReadSizePrefixedRawBytes(response, ref i, 1);

            if (NewPiecesIncoming != null) NewPiecesIncoming(offset, pieces);
        }

        //Process Packets from peers
        void Process_ConnectionRequest(User user, int content_length)
        {
            if (!user.UserStatus.Logged_on)
            {
                Console.WriteLine("{0}: Handshaking.", user.Socket.RemoteEndPoint);
                int i = Dgmt.HEADER_P2P_LENGTH;
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
                PeerPlayer player;
                if (_remaining_unconnected_peers.ContainsKey(peerID))
                {
                    Peer matching_peer = _remaining_unconnected_peers[peerID];
                    user.Port = matching_peer.Port;
                    user.PublicKey = matching_peer.PublicKey;
                    player = new PeerPlayer(matching_peer, user);
                    lock (_remaining_unconnected_peers) { _remaining_unconnected_peers.Remove(peerID); }
                }
                else
                {
                    //We never saw him coming
                    player = new PeerPlayer(peerID, user);
                    _remaining_untrusted_peers[peerID] = player;
                }
                lock (_players) { _players.Add(user, player); }
                //On update l'user status
                user.UserStatus.Logged_on = true;
                user.UserStatus.Peer_id = peerID;
                user.UserStatus.Lobby_id = lobbyID;

                //Send the ConnectionAccepted packet : 2nd step of the 3-way handshake
                Send_ConnectionAccepted(user, challengeNumber);
            }
        }

        void Process_ConnectionAccepted(User user, int content_length)
        {
            if (user.UserStatus.Logged_on)
            {
                Console.WriteLine("{0}: is accepting the connection.", user.Socket.RemoteEndPoint);
                int i = Dgmt.HEADER_P2P_LENGTH;
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
                byte[] to_sign = new byte[i - j];
                Array.Copy(packet, j, to_sign, 0, i - j);
                byte[] signatureDSA = BigE.ReadSizePrefixedRawBytes(packet, ref i, 2);

                //Là faire des tests
                if (lobbyID == request.LobbyID &&
                    peerID == request.PeerID &&
                    listeningPeerID == request.ListeningPeerID &&
                    challengeNumber == request.ChallengeNumber)
                {
                    _dsa_peer.FromXmlString(user.PublicKey);
                    try
                    {
                        if (!_dsa_peer.VerifyData(to_sign, signatureDSA)) Console.WriteLine("{0}: Bad signature on round packet message.", user.Socket.RemoteEndPoint);
                    }
                    catch (CryptographicException e)
                    {
                        Console.WriteLine("{0}: Bad signature on round packet message + Exception", user.Socket.RemoteEndPoint);
                    }
                    //Tout est bon, dans le cochon
                    Peer peer = _players[user];

                    //Proceed to the last step of the 3-way handshake
                    Send_ConnectionAcknowledged(user, challengeNumber, listeningChallengeNumber);

                    if (PeerHandshaken != null) PeerHandshaken(peer);
                    return;
                }
                DisconnectUser(user.Socket);
            }
        }

        void Process_ConnectionAcknowledged(User user, int content_length)
        {
            if (user.UserStatus.Logged_on)
            {
                Console.WriteLine("{0}: is acknowledging your accepting the connection.", user.Socket.RemoteEndPoint);
                int i = Dgmt.HEADER_P2P_LENGTH;
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
                byte[] to_sign = new byte[i - j];
                Array.Copy(packet, j, to_sign, 0, i - j);
                byte[] signatureDSA = BigE.ReadSizePrefixedRawBytes(packet, ref i, 2);

                //Là faire des tests
                if (lobbyID == request.LobbyID &&
                    peerID == request.PeerID &&
                    listeningPeerID == request.ListeningPeerID &&
                    challengeNumber == request.ChallengeNumber)
                {
                    _dsa_peer.FromXmlString(user.PublicKey);
                    try
                    {
                        if (!_dsa_peer.VerifyData(to_sign, signatureDSA)) Console.WriteLine("{0}: Bad signature on round packet message.", user.Socket.RemoteEndPoint);
                    }
                    catch (CryptographicException e)
                    {
                        Console.WriteLine("{0}: Bad signature on round packet message + Exception", user.Socket.RemoteEndPoint);
                    }
                    //Tout est bon, dans le cochon
                    Peer peer = _players[user];
                    if (PeerHandshaken != null) PeerHandshaken(peer);
                    return;
                }
                DisconnectUser(user.Socket);
            }
        }

        void Process_ChatSend(User user, int content_length)
        {
            if (user.UserStatus.Logged_on)
            {
                int i = Dgmt.HEADER_P2P_LENGTH;
                byte[] packet = user.Buffer.GetSubbufferCopy(i, content_length);
                //Skip the type
                i = 1;
                int j = i;
                ulong sessionID = BigE.ReadInt64(packet, ref i);
                string message = BigE.ReadSizePrefixedUTF8(packet, ref i, 2);
                //On copie le bordel dont on doit vérifier la signature
                byte[] to_sign = new byte[i - j];
                Array.Copy(packet, j, to_sign, 0, i - j);
                byte[] signature = BigE.ReadSizePrefixedRawBytes(packet, ref i, 2);

                _dsa_peer.FromXmlString(user.PublicKey);
                if (!_dsa_peer.VerifyData(to_sign, signature)) Console.WriteLine("{0}: Bad signature on sent message.", user.Socket.RemoteEndPoint);

                if (MessageReceived != null) MessageReceived(_players[user], message);
            }
        }

        void Process_Round(User user, int content_length)
        {
            if (user.UserStatus.Am_playing)
            {
                int i = Dgmt.HEADER_P2P_LENGTH;
                byte[] packet = user.Buffer.GetSubbufferCopy(i, content_length);
                //Skip the type
                i = 1;
                int j = i;
                ulong sessionID = BigE.ReadInt64(packet, ref i);
                uint roundNumber = BigE.ReadInt32(packet, ref i);
                byte move_size = BigE.ReadByte(packet, ref i);
                Move[] moves = new Move[move_size];
                for (int k = 0; k < move_size; k++)
                {
                    uint pieceNumber = BigE.ReadInt32(packet, ref i);
                    byte orientation = BigE.ReadByte(packet, ref i);
                    byte x = BigE.ReadByte(packet, ref i);
                    byte y = BigE.ReadByte(packet, ref i);
                    moves[k] = new Move(pieceNumber, orientation, x, y);
                }
                //Round hash
                byte hash_size = BigE.ReadByte(packet, ref i);
                for (int k = 0; k < hash_size; k++)
                {
                    byte id = BigE.ReadByte(packet, ref i);
                    byte[] hash = BigE.ReadRawBytes(packet, ref i, EXPECTED_HASH_SIZE);
                    if (id != _status.Peer_id)
                    {
                        User userouze = UserFromPeerID(id);
                        PeerPlayer player = _players[userouze];
                        player.RoundDataHash = hash;
                    }
                }

                //Signature
                //On copie le bordel dont on doit vérifier la signature
                byte[] to_sign = new byte[i - j];
                Array.Copy(packet, j, to_sign, 0, i - j);
                byte[] signature = BigE.ReadSizePrefixedRawBytes(packet, ref i, 2);

                _dsa_peer.FromXmlString(user.PublicKey);
                try
                {
                    if (!_dsa_peer.VerifyData(to_sign, signature)) Console.WriteLine("{0}: Bad signature on round packet message.", user.Socket.RemoteEndPoint);
                }
                catch (CryptographicException e)
                {
                    Console.WriteLine("{0}: Bad signature on round packet message + Exception", user.Socket.RemoteEndPoint);
                }

                //Send to UI, if a move happened
                if (move_size > 0)
                {
                    if (MoveMade != null) MoveMade(_players[user], roundNumber, moves);
                }
            }
        }

        //Send Packets to peers
        void Send_ConnectionRequest(User user)
        {
            byte[] packet = Dgmt.ForgeNewPacket();
            int i = Dgmt.HEADER_P2P_LENGTH;
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
            int length = Dgmt.FinalizePacketP2P(packet, i - Dgmt.HEADER_P2P_LENGTH);

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
            int i = Dgmt.HEADER_P2P_LENGTH;
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
            int length = Dgmt.FinalizePacketP2P(packet, i - Dgmt.HEADER_P2P_LENGTH);

            ClientRequestConnectionAccepted request = new ClientRequestConnectionAccepted(_lobby.ID, (byte)user.UserStatus.Peer_id, (byte)_status.Peer_id, challengeNumber, listeningChallengeNumber);
            RegisterRequest(request);
            //Send
            ServerResponse response = new ServerResponse(user.Socket, packet, length);
            QueueResponse(response);
        }

        void Send_ConnectionAcknowledged(User user, ulong challengeNumber, ulong listeningChallengeNumber)
        {
            byte[] packet = Dgmt.ForgeNewPacket();
            int i = Dgmt.HEADER_P2P_LENGTH;
            BigE.WritePacketIDP2P(packet, ref i, Dgmt.PacketP2PID.ConnectionAcknowledged);
            int j = i;
            BigE.WriteInt32(packet, ref i, _lobby.ID);
            BigE.WriteByte(packet, ref i, (byte)_status.Peer_id);
            BigE.WriteByte(packet, ref i, (byte)user.UserStatus.Peer_id);
            BigE.WriteInt64(packet, ref i, challengeNumber);
            BigE.WriteInt64(packet, ref i, listeningChallengeNumber);
            byte[] signed = _dsa_client.SignData(packet, j, i - j);
            BigE.WriteSizePrefixed(packet, ref i, 2, signed);
            int length = Dgmt.FinalizePacketP2P(packet, i - Dgmt.HEADER_P2P_LENGTH);

            //Send
            ServerResponse response = new ServerResponse(user.Socket, packet, length);
            QueueResponse(response);
        }

        void Send_ChatSend(User user, string message)
        {
            byte[] packet = Dgmt.ForgeNewPacket();
            int i = Dgmt.HEADER_P2P_LENGTH;
            BigE.WritePacketIDP2P(packet, ref i, Dgmt.PacketP2PID.ChatSend);

            int j = i;
            BigE.WriteInt64(packet, ref i, (ulong)_status.Session_id);
            BigE.WriteSizePrefixedUTF8(packet, ref i, 2, message);

            //Signature
            string public_key = _dsa_client.ToXmlString(false);
            byte[] sign = _dsa_client.SignData(packet, j, i - j);
            BigE.WriteSizePrefixed(packet, ref i, 2, sign);
            int length = Dgmt.FinalizePacketP2P(packet, i - Dgmt.HEADER_P2P_LENGTH);

            //Send
            ServerResponse response = new ServerResponse(user.Socket, packet, length);
            QueueResponse(response);
        }

        void Send_Round(uint roundNumber, ICollection<Move> moves)
        {
            byte[] packet = Dgmt.ForgeNewPacket();
            int i = Dgmt.HEADER_P2P_LENGTH;
            BigE.WritePacketIDP2P(packet, ref i, Dgmt.PacketP2PID.Round);

            int j = i;
            BigE.WriteInt64(packet, ref i, (ulong)_status.Session_id);
            BigE.WriteInt32(packet, ref i, roundNumber);
            BigE.WriteByte(packet, ref i, (byte)moves.Count);
            foreach (Move move in moves)
            {
                BigE.WriteInt32(packet, ref i, move.PieceNumber);
                BigE.WriteByte(packet, ref i, move.Orientation);
                BigE.WriteByte(packet, ref i, move.X);
                BigE.WriteByte(packet, ref i, move.Y);
            }
            //Players
            BigE.WriteByte(packet, ref i, (byte)(_players.Count + 1));
            //Me
            BigE.WriteByte(packet, ref i, (byte)_status.Peer_id);
            byte[] dummy = new byte[EXPECTED_HASH_SIZE];
            for (int k = 0; k < EXPECTED_HASH_SIZE; k++) dummy[k] = 0;
            if (_roundDataHash != null)
            {
                BigE.WriteRawBytes(packet, ref i, _roundDataHash);
            }
            else
            {
                BigE.WriteRawBytes(packet, ref i, dummy);
            }
            //The others
            foreach (PeerPlayer player in _players.Values)
            {
                BigE.WriteByte(packet, ref i, player.ID);
                BigE.WriteRawBytes(packet, ref i, player.RoundDataHash != null ? player.RoundDataHash : dummy);
            }

            //Signature
            string public_key = _dsa_client.ToXmlString(false);
            byte[] sign = _dsa_client.SignData(packet, j, i - j);
            //Hash
            _roundDataHash = _sha.ComputeHash(packet, j, i - j);
            
            BigE.WriteSizePrefixed(packet, ref i, 2, sign);

            int length = Dgmt.FinalizePacketP2P(packet, i - Dgmt.HEADER_P2P_LENGTH);
            
            //Send to everyone
            foreach (User user in _players.Keys)
            {
                //Send
                ServerResponse response = new ServerResponse(user.Socket, packet, length);
                QueueResponse(response);
            }
        }


        User UserFromPeerID(byte peerID)
        {
            User user = null;
            foreach (User userouze in _players.Keys)
            {
                if (userouze.UserStatus.Peer_id == peerID) user = userouze;
            }
            return user;
        }

        public string Name { get { return _login; } }
        //public ClientStatus UserStatus { get { return _status; } }
    }
}
