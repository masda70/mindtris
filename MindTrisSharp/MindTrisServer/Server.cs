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
using MindTrisCore.DGMTEncoding;
using System.Security.Cryptography;
using System.Text.RegularExpressions;
using System.Diagnostics;

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
        RSACryptoServiceProvider _rsa;
        Random _rand;

        //Demands handling
        Thread _listening;

        //Users
        Dictionary<Socket, User> _users;
        Dictionary<User, Account> _logged_users;
        Dictionary<string, Account> _accounts;
        LobbyServer[] _lobbies;

        //Lag timings
        const int WELCOMING_LAG = 200;
        const int LISTENING_LAG = 50;
        const int CHECKING_ALIVE_LAG = 90000;

        //Scalability
        int _connection_count;
        const int MAX_CONNECTION_COUNT = 1000;
        const int MAX_LOBBIES_COUNT = 1000;

        //Game mechanics
        RandomGenerator _generator;
        const byte PIECES_INITIAL_COUNT = 10;
        const byte PIECES_UPDATE_COUNT = 10;

        public Server(string name, IPAddress address, int port)
        {
            _name = name;
            _address = address;
            _port = port;
            _connection_count = 0;
            _listener = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
            _listener.Blocking = false;
            _users = new Dictionary<Socket, User>(MAX_CONNECTION_COUNT);
            _logged_users = new Dictionary<User, Account>(MAX_CONNECTION_COUNT);
            _accounts = new Dictionary<string, Account>(10);
            _lobbies = new LobbyServer[MAX_LOBBIES_COUNT];
            _rsa = new RSACryptoServiceProvider();
            _rand = new Random();
        }

        public Server(string name)
            : this(name, IPAddress.Any, Dgmt.DEFAULT_PORT) { }

        public void Start()
        {
            //Local endpoint for the socket.
            IPEndPoint localEndPoint = new IPEndPoint(_address, _port);
            //Binding!
            _listener.Bind(localEndPoint);
            Console.WriteLine("Binding...");
            _listener.Listen(MAX_CONNECTION_COUNT);
            Console.WriteLine("Listening...");

            _listening = new Thread(new ThreadStart(Listening));
            _listening.Start();
        }

        public void Stop()
        {
            foreach (Socket socket in _users.Keys)
            {
                DisconnectUser(socket);
            }
            _listening.Abort();
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
                Welcoming();
                IList list = _users.Keys.ToList<Socket>();
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
                User user = _users[socket];
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
                            user.Buffer.Length - user.Buffer.BufferPosition)),
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
            foreach (User user in _users.Values)
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
                //On assure l'endianness
                BigE.E(minibuffer, Dgmt.PROTOCOL_ID_LENGTH, Dgmt.PACKET_LENGTH_LENGTH);
                //Attention, dépend du protocole : sizeof(ushort) == PACKET_LENGTH_LENGTH
                ushort packet_length = (ushort)BitConverter.ToUInt16(minibuffer, Dgmt.PROTOCOL_ID_LENGTH);
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
                        case Dgmt.PacketID.HelloFromClient:
                            Process_HelloFromClient(user, content_length);
                            break;
                        case Dgmt.PacketID.CreateUser:
                            Process_CreateUser(user, content_length);
                            break;
                        case Dgmt.PacketID.Login:
                            Process_Login(user, content_length);
                            break;
                        case Dgmt.PacketID.CreateLobby:
                            Process_CreateLobby(user, content_length);
                            break;
                        case Dgmt.PacketID.GetLobbyList:
                            Process_GetLobbyList(user, content_length);
                            break;
                        case Dgmt.PacketID.JoinLobby:
                            Process_JoinLobby(user, content_length);
                            break;
                        case Dgmt.PacketID.LeaveLobby:
                            Process_LeaveLobby(user, content_length);
                            break;
                        case Dgmt.PacketID.StartGame:
                            Process_StartGame(user, content_length);
                            break;
                        case Dgmt.PacketID.LoadedGame:
                            Process_LoadedGame(user, content_length);
                            break;
                        case Dgmt.PacketID.GiveNewPieces:
                            Process_GiveNewPieces(user, content_length);
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
                //[TOCHECK] Je fais quoi là ?
                Response_HelloFromServer(user, 0x02);
                DisconnectUser(user.Socket);
            }
        }

        void Process_HelloFromClient(User user, int content_length)
        {
            if (!user.UserStatus.Connected && content_length == Dgmt.PROTOCOL_VERSION_LENGTH + 1)
            {
                Console.WriteLine("{0}: HelloFromClient passed.", user.Socket.RemoteEndPoint);
                //On vérifie la version du protocole
                int i = Dgmt.HEADER_LENGTH + 1;
                byte[] minibuffer = user.Buffer.GetSubbufferCopy(i, content_length - 1);
                Console.WriteLine("{0}: Version number #{1}", user.Socket.RemoteEndPoint, BitConverter.ToUInt32(minibuffer, 0));
                if (minibuffer[0] == Dgmt.VERSION[0] && minibuffer[1] == Dgmt.VERSION[1] && minibuffer[2] == Dgmt.VERSION[2] && minibuffer[3] == Dgmt.VERSION[3])
                {
                    //Connection success
                    Response_HelloFromServer(user, 0x00);
                }
                else
                {
                    Response_HelloFromServer(user, 0x01);
                }
            }
            else Response_HelloFromServer(user, 0x02);
        }

        void Process_CreateUser(User user, int content_length)
        {
            if (user.UserStatus.Connected && !user.UserStatus.Logged_on)
            {
                Console.WriteLine("{0}: Attempting user creation.", user.Socket.RemoteEndPoint);
                int i = Dgmt.HEADER_LENGTH;
                byte[] plain = user.Buffer.GetSubbufferCopy(i, content_length);
                i = 1;
                string username = BigE.ReadSizePrefixedUTF8(plain, ref i, 1);
                string displayed_username = BigE.ReadSizePrefixedUTF8(plain, ref i, 1);
                string email = BigE.ReadSizePrefixedASCII(plain, ref i, 2);
                byte[] rgb_pass = BigE.ReadSizePrefixedRawBytes(plain, ref i, 2);
                string pass = Encoding.ASCII.GetString(Decrypt(rgb_pass));
                //Do some validity checks
                /*
                if (!Regex.IsMatch(username, Dgmt.REGEX_LOGIN))
                {
                    Response_CreateUser(user, 0x02);
                    return;
                }
                //*/
                if (_accounts.ContainsKey(username))
                {
                    Response_CreateUser(user, 0x01);
                    return;
                }
                /* [TODO] réactiver le check fat
                if (!Regex.IsMatch(pass, Dgmt.REGEX_PASSWORD))
                {
                    Response_CreateUser(user, 0x03);
                    return;
                }
                //*/
                if (!Regex.IsMatch(email, Dgmt.REGEX_EMAIL))
                {
                    Response_CreateUser(user, 0x04);
                    return;
                }
                //Add new account
                Account account = new Account() { UserName = username, DisplayedName = displayed_username, Password = pass, Email = email };
                _accounts.Add(username, account);
                Response_CreateUser(user, 0x00);
            }
        }

        void Process_Login(User user, int content_length)
        {
            if (user.UserStatus.Connected && !user.UserStatus.Logged_on)
            {
                Console.WriteLine("{0}: Logging in.", user.Socket.RemoteEndPoint);
                int i = Dgmt.HEADER_LENGTH;
                byte[] packet = user.Buffer.GetSubbufferCopy(i, content_length);
                i = 1;
                string username = BigE.ReadSizePrefixedUTF8(packet, ref i, 1);
                byte[] rgb = BigE.ReadSizePrefixedRawBytes(packet, ref i, 2);
                string pass = Encoding.ASCII.GetString(Decrypt(rgb));
                //Check
                if (_accounts.ContainsKey(username))
                {
                    Account account = _accounts[username];
                    if (pass == account.Password)
                    {
                        _logged_users.Add(user, account);
                        user.UserStatus.Logged_on = true;
                        user.UserStatus.User = username;
                        Response_Login(user, 0x00, account.DisplayedName);
                    }
                    else Response_Login(user, 0x02, "");
                }
                else Response_Login(user, 0x01, "");
            }
            //[TODO] Gérer les autres codes d'erreurs : too many tries, double login
        }

        int NextFreeLobbyID()
        {
            for (int k = 1; k < MAX_LOBBIES_COUNT; k++)
            {
                if (_lobbies[k] == null) return k;
            }
            return -1;
        }
        void Process_CreateLobby(User user, int content_length)
        {
            if (user.UserStatus.Connected)
            {
                Console.WriteLine("{0}: Creating lobby.", user.Socket.RemoteEndPoint);
                int i = Dgmt.HEADER_LENGTH;
                byte[] packet = user.Buffer.GetSubbufferCopy(i, content_length);
                //Skip the type
                i = 1;
                int id = NextFreeLobbyID();
                if (id < 0)
                {
                    //Not enough slots
                    Response_CreateLobby(user, 0x02, null);
                    return;
                }
                LobbyServer lobby = new LobbyServer();
                lobby.Name = BigE.ReadSizePrefixedUTF8(packet, ref i, 1);
                lobby.PlayerMaxCount = BigE.ReadByte(packet, ref i);
                lobby.PasswordProtected = BigE.ReadBool(packet, ref i);
                if (lobby.PasswordProtected)
                {
                    byte[] rgb_pass = BigE.ReadSizePrefixedRawBytes(packet, ref i, 2);
                    byte[] pass = Decrypt(rgb_pass);
                    lobby.Password = Encoding.ASCII.GetString(pass);
                    if (!Regex.IsMatch(lobby.Password, Dgmt.REGEX_PASSWORD))
                    {
                        Response_CreateLobby(user, 0x01, lobby);
                        return;
                    }
                }
                lobby.ID = (uint)id;
                lobby.Players = new LinkedList<PeerServer>();
                byte[] uint64 = new byte[8];
                _rand.NextBytes(uint64);
                lobby.SessionID = BitConverter.ToUInt64(uint64, 0);
                lobby.Creator = _logged_users[user].DisplayedName;
                lobby.UserCreator = user;
                //We connect the creator to its own lobby
                user.Port = BigE.ReadInt16(packet, ref i);
                user.PublicKey = BigE.ReadDSAPublicKey(packet, ref i);
                PeerServer player = new PeerServer(lobby, user, user.UserStatus.User);
                lobby.Players.AddLast(player);
                lobby.PlayerCount++;
                user.UserStatus.Lobby_id = lobby.ID;
                user.UserStatus.Creator_lobby_id = lobby.ID;
                user.UserStatus.Peer_id = player.ID;
                user.UserStatus.Session_id = lobby.SessionID;
                lobby.CreatorPeerID = player.ID;

                //Add lobby
                _lobbies[id] = lobby;

                Response_CreateLobby(user, 0x00, lobby);
            }
            //Gérer les message d'erreur 0x02 : not enough rights
        }

        void Process_GetLobbyList(User user, int content_length)
        {
            //No request to process, skip directly to response
            Response_GetLobbyList(user);
        }

        void Process_JoinLobby(User user, int content_length)
        {
            if (user.UserStatus.Connected)
            {
                Console.WriteLine("{0}: Joining lobby.", user.Socket.RemoteEndPoint);
                int i = Dgmt.HEADER_LENGTH;
                byte[] packet = user.Buffer.GetSubbufferCopy(i, content_length);
                //Skip the type
                i = 1;
                uint lobby_id = BigE.ReadInt32(packet, ref i);
                string pass = BigE.ReadSizePrefixedASCII(packet, ref i, 1);
                user.Port = BigE.ReadInt16(packet, ref i);
                user.PublicKey = BigE.ReadDSAPublicKey(packet, ref i);

                //Process
                if (lobby_id > MAX_LOBBIES_COUNT
                    || _lobbies[lobby_id] == null
                    || (_lobbies[lobby_id].PasswordProtected && _lobbies[lobby_id].Password != pass))
                {
                    //If ID is invalid, unallocated or password doesn't match
                    Response_JoinLobby(user, 0x01, lobby_id, null, 0);
                    return;
                }

                LobbyServer lobby = _lobbies[lobby_id];
                if (lobby.PlayerCount >= lobby.PlayerMaxCount)
                {
                    //Lobby is full
                    Response_JoinLobby(user, 0x02, lobby_id, null, 0);
                    return;
                }
                //User has the right password and there is room for him, go for it
                LinkedList<PeerServer> peers = new LinkedList<PeerServer>();
                foreach (PeerServer peer in lobby.Players) peers.AddLast(peer);
                PeerServer player = new PeerServer(lobby, user, _logged_users[user].DisplayedName);
                //Update status
                user.UserStatus.Lobby_id = lobby.ID;
                user.UserStatus.Peer_id = player.ID;
                user.UserStatus.Session_id = lobby.SessionID;
                //Update peers
                Response_UpdateClientLobbyStatus(user, player.ID, 0x00, player, peers);
                //Player is added server side, after the response
                lobby.Players.AddLast(player);
                lobby.PlayerCount++;
                //Send response
                Response_JoinLobby(user, 0x00, lobby.ID, lobby, player.ID);
            }
        }

        void Process_LeaveLobby(User user, int content_length)
        {
            if (user.UserStatus.Lobby_id != null)
            {
                Console.WriteLine("{0}: Has left lobby.", user.Socket.RemoteEndPoint);
                LobbyServer lobby = _lobbies[(uint)user.UserStatus.Lobby_id];
                PeerServer peer = null;
                foreach (PeerServer peerouze in lobby.Players)
                {
                    if (Object.ReferenceEquals(peerouze.User, user))
                    {
                        peer = peerouze;
                        break;
                    }
                }
                Debug.Assert(peer != null);
                //Update user status
                user.UserStatus.Am_playing = false;
                user.UserStatus.Lobby_id = null;
                user.UserStatus.Peer_id = null;
                user.UserStatus.Session_id = null;

                //Updates peer
                Response_UpdateClientLobbyStatus(user, peer.ID, 0x01, peer, lobby.Players);
                //Removing peer
                lobby.Players.Remove(peer);
                lobby.PlayerCount--;
            }
        }

        void Process_StartGame(User user, int content_length)
        {
            if (user.UserStatus.Creator_lobby_id != null)
            {
                Console.WriteLine("{0}: Starting game.", user.Socket.RemoteEndPoint);
                int i = Dgmt.HEADER_LENGTH;
                byte[] packet = user.Buffer.GetSubbufferCopy(i, content_length);
                //Skip the type
                i = 1;
                uint lobby_id = (uint)user.UserStatus.Creator_lobby_id;
                LobbyServer lobby = _lobbies[lobby_id];

                //Send response
                Response_GameStarting(user, 0x00);

                //Tirer les premières pièces
                lobby.Generator = new RandomGenerator(_rand.Next());
                byte[] pieces = lobby.Generator.Next(PIECES_INITIAL_COUNT);

                //Notify every player in the lobby to load the game
                foreach (PeerServer peer in _lobbies[lobby_id].Players)
                {
                    Response_LoadGame(peer.User, pieces);
                }
            }
        }

        void Process_LoadedGame(User user, int content_length)
        {
            if (user.UserStatus.Connected && user.UserStatus.Lobby_id != null)
            {
                Console.WriteLine("{0}: Loaded game.", user.Socket.RemoteEndPoint);
                int i = Dgmt.HEADER_LENGTH;
                byte[] packet = user.Buffer.GetSubbufferCopy(i, content_length);
                //Skip the type
                i = 1;
                byte answer = BigE.ReadByte(packet, ref i);
                if (answer != 0x00)
                {
                    //[TODO] Gérer le cas où il y a une erreur, lol
                }
                else
                {
                    LobbyServer lobby = _lobbies[(uint)user.UserStatus.Lobby_id];
                    PeerServer peer = lobby[user];
                    peer.IsGameLoaded = true;
                    //If all peers have loaded the game, send BeginGame
                    bool all_loaded = true;
                    foreach (PeerServer peerouze in lobby.Players)
                    {
                        all_loaded = all_loaded && peerouze.IsGameLoaded;
                    }
                    if (all_loaded)
                    {
                        foreach (PeerServer peerouze in lobby.Players)
                        {
                            peerouze.User.UserStatus.Am_playing = true;
                            Response_BeginGame(peerouze.User);
                        }
                    }
                }
            }
        }

        void Process_GiveNewPieces(User user, int content_length)
        {
            if (user.UserStatus.Am_playing && user.UserStatus.Lobby_id != null)
            {
                Console.WriteLine("{0}: Requiring new pieces.", user.Socket.RemoteEndPoint);
                int i = Dgmt.HEADER_LENGTH;
                byte[] packet = user.Buffer.GetSubbufferCopy(i, content_length);
                //Skip the type
                i = 1;
                uint offset = BigE.ReadInt32(packet, ref i);
                byte count = BigE.ReadByte(packet, ref i);
                LobbyServer lobby = _lobbies[(uint)user.UserStatus.Lobby_id];

                foreach (PeerServer peer in lobby.Players)
                {
                    Response_NewPieces(peer.User, lobby, offset, count);
                }
            }
        }

        /*
        void ReceiveFromSingleClient(IAsyncResult res)
        {
            var state = (Pair<Socket, byte[]>)res.AsyncState;
            Socket socket = state.Fst;
            byte[] buffer = state.Snd;
            User user = _users[socket];
            //End receive
            int qte = socket.EndReceive(res);
            //Continue receiving until a complete header is read
            while (qte < DGMT.HEADER_LENGTH)
            {
                qte += socket.Receive(buffer, qte, DGMT.HEADER_LENGTH - qte, SocketFlags.None);
            }
            //Header check
            if (DGMT.DGMTCheck(buffer, 0))
            {
                Console.WriteLine("{0}: DGMT passed.", socket.RemoteEndPoint);
                //On assure l'endianness
                BigE.E(buffer, DGMT.PROTOCOL_ID_LENGTH, DGMT.PACKET_LENGTH_LENGTH);
                //Attention, dépend du protocole : sizeof(ushort) == PACKET_LENGTH_LENGTH
                ushort content_length = (ushort)(BitConverter.ToUInt16(buffer, DGMT.PROTOCOL_ID_LENGTH) - DGMT.HEADER_LENGTH);
                byte id = buffer[DGMT.PROTOCOL_ID_LENGTH + DGMT.PACKET_LENGTH_LENGTH];
                //Pour l'instant on fait qu'un test kikoo pour valider la connection
                //On check le HELLO FROM CLIENT
                if (user.UserState == UserState.ConnectionPending &&
                    (id == (byte)DGMT.PacketID.HelloFromClient) &&
                    content_length == DGMT.PROTOCOL_VERSION_LENGTH)
                {
                    Console.WriteLine("{0}: HelloFromClient passed.", socket.RemoteEndPoint);
                    //On vérifie la version du protocole
                    buffer = new byte[DGMT.PROTOCOL_VERSION_LENGTH];
                    //Read all
                    qte = 0;
                    while (qte < content_length)
                    {
                        qte += socket.Receive(buffer, qte, content_length - qte, SocketFlags.None);
                    }
                    Console.WriteLine("{0}: Version number #{1}", socket.RemoteEndPoint, BitConverter.ToUInt32(buffer, 0));
                    BigE.E(buffer, 0, DGMT.PROTOCOL_VERSION_LENGTH);
                    if (BitConverter.ToUInt32(buffer, 0) == DGMT.VERSION)
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
//*/
        


        void QueueResponse(ServerResponse response)
        {
            if (_users.ContainsKey(response.Socket))
            {
                User user = _users[response.Socket];
                user.SendingPending.AddLast(response);
            }
        }

        void Response_HelloFromServer(User user, byte response)
        {
            user.UserStatus.Connected = true;
            string motd = String.Format(_motd, _name);
            byte[] packet = Dgmt.ForgeNewPacket();
            int i = Dgmt.HEADER_LENGTH;
            packet[i] = (byte)Dgmt.PacketID.HelloFromServer;
            i++;
            packet[i] = response;
            i++;
            //Sending server public key
            string xml = _rsa.ToXmlString(false);
            BigE.WriteRSAPublicKey(packet, ref i, xml);
            if (response != 0x00) motd = "";
            BigE.WriteSizePrefixed(packet, ref i, 2, Encoding.UTF8.GetBytes(motd));
            //MOTD
            if (response == 0x00)
            {
                //Queuing response
                Console.WriteLine("{0}: Sending MOTD to client...", user.Socket.RemoteEndPoint);
            }
            int content_length = i - Dgmt.HEADER_LENGTH;
            int length = Dgmt.FinalizePacket(packet, content_length);
            ServerResponse to_send = new ServerResponse(user.Socket, packet, length);
            QueueResponse(to_send);
        }

        void Response_CreateUser(User user, byte response)
        {
            byte[] packet = Dgmt.ForgeNewPacket();
            packet[Dgmt.HEADER_LENGTH] = (byte)Dgmt.PacketID.UserCreation;
            packet[Dgmt.HEADER_LENGTH + 1] = response;
            int length = Dgmt.FinalizePacket(packet, 2);
            ServerResponse to_send = new ServerResponse(user.Socket, packet, length);
            QueueResponse(to_send);
        }

        void Response_Login(User user, byte response, string displayed_name)
        {
            byte[] packet = Dgmt.ForgeNewPacket();
            int i = Dgmt.HEADER_LENGTH;
            packet[i++] = (byte)Dgmt.PacketID.LoginReply;
            packet[i++] = response;
            int size = 2;
            if (response == 0x00)
            {
                size += BigE.WriteSizePrefixedUTF8(packet, ref i, 1, displayed_name);
            }
            int length = Dgmt.FinalizePacket(packet, size);
            ServerResponse to_send = new ServerResponse(user.Socket, packet, length);
            QueueResponse(to_send);
        }

        void Response_CreateLobby(User user, byte response, LobbyServer lobby)
        {
            byte[] packet = Dgmt.ForgeNewPacket();
            int i = Dgmt.HEADER_LENGTH;
            BigE.WriteByte(packet, ref i, (byte)Dgmt.PacketID.LobbyCreation);
            BigE.WriteByte(packet, ref i, response);
            if (response == 0x00)
            {
                BigE.WriteInt32(packet, ref i, lobby.ID);
                BigE.WriteByte(packet, ref i, lobby.Players.First.Value.ID);
                BigE.WriteInt64(packet, ref i, lobby.SessionID);
            }
            else
            {
                BigE.WriteInt32(packet, ref i, 0);
                BigE.WriteByte(packet, ref i, 0);
                BigE.WriteInt64(packet, ref i, 0);
            }
            int length = Dgmt.FinalizePacket(packet, i - Dgmt.HEADER_LENGTH);
            ServerResponse res = new ServerResponse(user.Socket, packet, length);
            QueueResponse(res);
        }

        void Response_GetLobbyList(User user)
        {
            byte[] packet = Dgmt.ForgeNewPacket();
            int i = Dgmt.HEADER_LENGTH;
            BigE.WritePacketID(packet, ref i, Dgmt.PacketID.LobbyList);
            var list = _lobbies.Where(x => { return x != null; });
            BigE.WriteByte(packet, ref i, (byte)list.Count());
            foreach (LobbyServer lobby in list)
            {
                BigE.WriteInt32(packet, ref i, lobby.ID);
                BigE.WriteSizePrefixedUTF8(packet, ref i, 1, lobby.Name);
                BigE.WriteByte(packet, ref i, lobby.PlayerCount);
                BigE.WriteByte(packet, ref i, lobby.PlayerMaxCount);
                BigE.WriteBool(packet, ref i, lobby.PasswordProtected);
                BigE.WriteSizePrefixedUTF8(packet, ref i, 1, lobby.Creator);
            }
            int length = Dgmt.FinalizePacket(packet, i - Dgmt.HEADER_LENGTH);
            //Send the shit
            ServerResponse res = new ServerResponse(user.Socket, packet, length);
            QueueResponse(res);
        }

        void Response_JoinLobby(User user, byte response, uint lobbyID, LobbyServer lobby, byte peerID)
        {
            byte[] packet = Dgmt.ForgeNewPacket();
            int i = Dgmt.HEADER_LENGTH;
            BigE.WritePacketID(packet, ref i, Dgmt.PacketID.JoinedLobby);
            BigE.WriteInt32(packet, ref i, lobbyID);
            BigE.WriteByte(packet, ref i, response);
            if (response == 0x00)
            {
                BigE.WriteSizePrefixedUTF8(packet, ref i, 1, lobby.Name);
                BigE.WriteByte(packet, ref i, lobby.PlayerMaxCount);
                BigE.WriteByte(packet, ref i, lobby.CreatorPeerID);
                BigE.WriteByte(packet, ref i, peerID);
                BigE.WriteInt64(packet, ref i, lobby.SessionID);
                BigE.WriteByte(packet, ref i, (byte)(lobby.Players.Count - 1));
                foreach (Peer peer in lobby.Players)
                {
                    if (peer.ID != peerID)
                    {
                        BigE.WriteByte(packet, ref i, peer.ID);
                        BigE.WriteSizePrefixedUTF8(packet, ref i, 1, peer.DisplayName);
                        BigE.WriteIPAddress(packet, ref i, peer.IpAddress);
                        BigE.WriteInt16(packet, ref i, peer.Port);
                        BigE.WriteDSAPublicKey(packet, ref i, peer.PublicKey);
                    }
                }
            }

            //Send
            int length = Dgmt.FinalizePacket(packet, i - Dgmt.HEADER_LENGTH);
            ServerResponse res = new ServerResponse(user.Socket, packet, length);
            QueueResponse(res);
        }

        void Response_UpdateClientLobbyStatus(User user, byte clientID, byte statusUpdate, Peer peer, ICollection<PeerServer> peers)
        {
            byte[] packet = Dgmt.ForgeNewPacket();
            int i = Dgmt.HEADER_LENGTH;
            BigE.WritePacketID(packet, ref i, Dgmt.PacketID.UpdateClientStatus);
            Debug.Assert(user.UserStatus.Lobby_id != null);
            BigE.WriteInt32(packet, ref i, (uint)user.UserStatus.Lobby_id);
            BigE.WriteByte(packet, ref i, statusUpdate);
            BigE.WriteByte(packet, ref i, clientID);
            if (statusUpdate == 0x00)
            {
                BigE.WriteSizePrefixedUTF8(packet, ref i, 1, peer.DisplayName);
                BigE.WriteIPAddress(packet, ref i, peer.IpAddress);
                BigE.WriteInt16(packet, ref i, peer.Port);
                BigE.WriteDSAPublicKey(packet, ref i, peer.PublicKey);
            }
            int length = Dgmt.FinalizePacket(packet, i - Dgmt.HEADER_LENGTH);
            //Multiple send to other players
            foreach (PeerServer peerless in peers)
            {
                ServerResponse res = new ServerResponse(peerless.User.Socket, packet, length);
                QueueResponse(res);
            }
        }

        void Response_GameStarting(User user, byte response)
        {
            byte[] packet = Dgmt.ForgeNewPacket();
            int i = Dgmt.HEADER_LENGTH;
            BigE.WritePacketID(packet, ref i, Dgmt.PacketID.GameStarting);
            BigE.WriteByte(packet, ref i, response);

            int length = Dgmt.FinalizePacket(packet, i - Dgmt.HEADER_LENGTH);
            //Send the shit
            ServerResponse res = new ServerResponse(user.Socket, packet, length);
            QueueResponse(res);
        }

        void Response_LoadGame(User user, byte[] pieces)
        {
            byte[] packet = Dgmt.ForgeNewPacket();
            int i = Dgmt.HEADER_LENGTH;
            BigE.WritePacketID(packet, ref i, Dgmt.PacketID.LoadGame);
            BigE.WriteByte(packet, ref i, (byte)pieces.Length);
            for (int j = 0; j < pieces.Length; j++)
            {
                BigE.WriteByte(packet, ref i, pieces[j]);
            }

            int length = Dgmt.FinalizePacket(packet, i - Dgmt.HEADER_LENGTH);
            //Send the shit
            ServerResponse res = new ServerResponse(user.Socket, packet, length);
            QueueResponse(res);
        }

        void Response_BeginGame(User user)
        {
            byte[] packet = Dgmt.ForgeNewPacket();
            int i = Dgmt.HEADER_LENGTH;
            BigE.WritePacketID(packet, ref i, Dgmt.PacketID.BeginGame);

            int length = Dgmt.FinalizePacket(packet, i - Dgmt.HEADER_LENGTH);
            //Send the shit
            ServerResponse res = new ServerResponse(user.Socket, packet, length);
            QueueResponse(res);
        }

        void Response_NewPieces(User user, LobbyServer lobby, uint offset, byte count)
        {
            byte[] packet = Dgmt.ForgeNewPacket();
            int i = Dgmt.HEADER_LENGTH;
            
            BigE.WritePacketID(packet, ref i, Dgmt.PacketID.NewPieces);
            BigE.WriteInt32(packet, ref i, offset);
            count = (count == 0) ? PIECES_UPDATE_COUNT : count;
            byte[] pieces = lobby.Generator.Next(offset, count);
            BigE.WriteByte(packet, ref i, count);
            BigE.WriteRawBytes(packet, ref i, pieces);
            int length = Dgmt.FinalizePacket(packet, i - Dgmt.HEADER_LENGTH);
            //Send the shit
            ServerResponse res = new ServerResponse(user.Socket, packet, length);
            QueueResponse(res);
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
            if (_logged_users.ContainsKey(user)) _logged_users.Remove(user);
            if (user.UserStatus.Creator_lobby_id != null)
            {
                LobbyServer lobby = _lobbies[(uint)user.UserStatus.Lobby_id];
                //[TOCHECK] Je sais plus le behavior de cette fonction lol, au pire c'est un peu overkill
                foreach (PeerServer peerouze in lobby.Players)
                {
                    Response_UpdateClientLobbyStatus(peerouze.User, peerouze.ID, 0x02, peerouze, lobby.Players);
                }
                _lobbies[(uint)user.UserStatus.Lobby_id] = null;
            }
            else if (user.UserStatus.Lobby_id != null)
            {
                LobbyServer lobby = _lobbies[(uint)user.UserStatus.Lobby_id];
                PeerServer peer = null;
                if (lobby != null && lobby.Players != null)
                {
                    foreach (PeerServer peerouze in lobby.Players)
                    {
                        if (Object.ReferenceEquals(peerouze.User, user)) peer = peerouze;
                    }
                    lobby.Players.Remove(peer);
                    lobby.PlayerCount--;
                }
            }
            
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

        byte[] Decrypt(byte[] rgb)
        {
            return _rsa.Decrypt(rgb, true);
        }
    }
}
