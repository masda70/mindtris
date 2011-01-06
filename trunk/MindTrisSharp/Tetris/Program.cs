using System.Windows.Forms;
using MindTris;
using System.Collections.Generic;
using MindTrisCore;
using System.Threading;
using System;
using ZenithMFramework;
using System.Net;
using System.Diagnostics;

namespace Tetris
{
    static class Program
    {
        static Client _client;
        static Dictionary<byte, Peer> _peers;

        static string _ip_server;
        static string _login;
        static string _pass;
        static string _email = "zenithm@lolzorz1337.org";
        static ushort _port;

        static string _motd;

        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        static void Main(string[] args)
        {
            Application.EnableVisualStyles();

            Console.WriteLine("Server Address (default port will be used)?");
            string address = Console.ReadLine();
            IPAddress ip;
            if (IPAddress.TryParse(address, out ip))
            {
                _ip_server = address;
            }
            else
            {
                //Sinon on solve
                ip = Dns.GetHostAddresses(address)[0];
                _ip_server = ip.ToString();
            }

            _peers = new Dictionary<byte, Peer>();
            _client = new Client(_ip_server);
            _client.Connected += new Client.ConnectedFunction(client_Connected);
            _client.StatusUpdated += new Client.UpdateFunction(_client_StatusUpdated);
            _client.MessageReceived += new Client.MessageReceivedFunction(_client_MessageReceived);
            _client.Connect();

            while (true)
            {
                Thread.Sleep(100);
            }

            /*
            using (Engine game = new Engine())
            {
                game.Run();
            }
            //*/
        }

        static void client_Connected(string motd)
        {
            Thread t = new Thread(() =>
            {
                Console.WriteLine("MOTD: {0}", motd);
                _motd = motd;
                Console.WriteLine("Login?");
                _login = Console.ReadLine();
                Console.WriteLine("Pass?");
                _pass = Console.ReadLine();
                Console.WriteLine("Port?");
                while (!UInt16.TryParse(Console.ReadLine(), out _port)) ;
                _client.UserCreated += new Client.ConfirmationFunction(_client_UserCreated);
                Console.WriteLine("Create user? (y/other)");
                if (Console.ReadLine() == "y")
                {
                    _client.CreateUser(_login, _pass, _email);
                }
                else _client_UserCreated(0x00);
            });
            t.Start();
        }

        static void _client_UserCreated(byte response)
        {
            Thread t = new Thread(() =>
            {
                switch (response)
                {
                    case 0x00:
                        break;
                    case 0x01:
                        Console.WriteLine("[Server]: This username already exists.");
                        return;
                    case 0x02:
                        Console.WriteLine("[Server]: Invalid username.");
                        return;
                    case 0x03:
                        Console.WriteLine("[Server]: Invalid password.");
                        return;
                    case 0x04:
                        Console.WriteLine("[Server]: Invalid email.");
                        return;
                    default:
                        throw new Exception("Ne devrait pas arriver, ;D");
                }
                Console.WriteLine("[Server]: User successfully created.");
                _client.LoggedOn += new Client.LoggedOnFunction(_client_LoggedOn);
                _client.Login(_login, _pass);
            });
            t.Start();
        }

        static void _client_LoggedOn(byte response, string displayed_username)
        {
            Thread t = new Thread(() =>
            {
                switch (response)
                {
                    case 0x00:
                        break;
                    case 0x01:
                        Console.WriteLine("[Server]: Username does not exist.");
                        return;
                    case 0x02:
                        Console.WriteLine("[Server]: Bad username/password.");
                        return;
                    case 0x03:
                        Console.WriteLine("[Server]: Too many tries, try again later.");
                        return;
                    case 0x04:
                        Console.WriteLine("[Server]: Login success, but disconnected elsewhere.");
                        return;
                    default:
                        throw new Exception("Ne devrait pas arriver, ;D");
                }
                Console.WriteLine("[Server]: Your username is {0}.", displayed_username);
                _client.LobbiesRetrieved += new Client.RetrieveLobbiesFunction(_client_LobbiesRetrieved);
                _client.LobbyListRetrieval();
            });
            t.Start();
        }

        static void _client_LobbiesRetrieved(Lobby[] lobbies)
        {
            Thread t = new Thread(() =>
            {
                _client.LobbyJoined += new Client.JoinedLobbyFunction(_client_LobbyJoined);

                const string lobby_format = "{0} | ID:{1} | {2}/{3}";

                string[] lobbies_name = new string[lobbies.Length + 1];
                lobbies_name[lobbies_name.Length - 1] = "[Create a new fucking lobby.]";
                for (int k = 0; k < lobbies.Length; k++)
                {
                    lobbies_name[k] = String.Format(lobby_format,
                        lobbies[k].Name,
                        lobbies[k].ID,
                        lobbies[k].PlayerCount,
                        lobbies[k].PlayerMaxCount);
                }
                ConsoleMenu menu = new ConsoleMenu("Lobbies", lobbies_name);
                int choix = menu.Show();

                if (choix != lobbies_name.Length - 1)
                {
                    //D'abord, on start le listening
                    _client.StartListening(IPAddress.Any, _port);
                    Console.WriteLine("Password?");
                    string pass = Console.ReadLine();
                    _client.JoinLobby(lobbies[choix].ID, pass);
                }
                else
                {
                    Console.WriteLine("[Server]: Create a lobby.");
                    Console.WriteLine("Name?");
                    string name = Console.ReadLine();
                    Console.WriteLine("Player allowed count?");
                    int count;
                    while (!Int32.TryParse(Console.ReadLine(), out count)) ;
                    _client.LobbyCreated += new Client.LobbyCreatedFunction(_client_LobbyCreated);
                    _client.CreateLobby(name, (byte)count, false, "", _port);
                }
            });
            t.Start();
        }

        static void _client_LobbyCreated(byte response, uint? lobbyID, byte? peerID, ulong? sessionID)
        {
            //D'abord, on start le listening
            _client.StartListening(IPAddress.Any, _port);
            Debug.Assert(lobbyID != null);
            //_client.JoinLobby((uint)lobbyID, "");
            _client_LobbyJoined(0x00, 0, 1337, null);
        }

        static void _client_LobbyJoined(byte response, byte? clientID, ulong? sessionID, Peer[] peers)
        {
            Thread t = new Thread(() =>
            {
                switch (response)
                {
                    case 0x00:
                        Console.WriteLine("[Server]: Joined lobby with success.");
                        break;
                    case 0x01:
                        Console.WriteLine("[Server]: Bad password.");
                        return;
                    case 0x02:
                        Console.WriteLine("[Server]: Lobby is full.");
                        return;
                    case 0x03:
                        Console.WriteLine("[Server]: Unknown error.");
                        return;
                    default:
                        throw new Exception("Ne devrait pas arriver, ;D");
                }
                if (peers != null)
                {
                    foreach (Peer peer in peers)
                    {
                        _peers[peer.ID] = peer;
                        _client.ConnectPeer(peer);
                    }
                }
                _client.GameLoaded += new Client.ReceivingNewPieces(_client_GameLoaded);
                Console.WriteLine("Starting chat...");
                StartChat();
            });
            t.Start();
        }

        static void _client_GameLoaded(uint offset, byte[] pieces)
        {
            Thread t = new Thread(() =>
            {
                //Ignore offset, normally equals to zero
                using (Engine game = new Engine(_client, _peers.Values, pieces))
                {
                    game.Run();
                }
            });
            t.Start();
        }

        private static void StartChat()
        {
            Thread t = new Thread(() =>
                {
                    while (true)
                    {
                        string msg = Console.ReadLine();
                        switch (msg)
                        {
                            case "@":
                                DisplayPeers();
                                break;
                            case "/run":
                                _client.StartGame();
                                break;
                            default:
                                _client.SendMessage(msg);
                                break;
                        }
                    }
                });
            t.Start();
        }

        static void DisplayPeers()
        {
            foreach (Peer peer in _peers.Values)
            {
                Console.WriteLine("{0}) {1} @ {2}:{3}",
                    peer.ID, peer.DisplayName, peer.IpAddress, peer.Port);
            }
        }

        static void _client_MessageReceived(Peer peer, string message)
        {
            Thread t = new Thread(() =>
            {
                Console.WriteLine("{0} says: {1}", peer.DisplayName, message);
            });
            t.Start();
        }

        static void _client_StatusUpdated(byte update, Peer peer)
        {
            Thread t = new Thread(() =>
            {
                switch (update)
                {
                    case 0x00:
                        Console.WriteLine("[Server]: {0} has joined the lobby.", peer.DisplayName);
                        break;
                    case 0x01:
                        Console.WriteLine("[Server]: {0} has left the lobby.", peer.DisplayName);
                        _peers.Remove(peer.ID);
                        return;
                    case 0x02:
                        Console.WriteLine("[Server]: {0} has been kicked from the lobby.");
                        _peers.Remove(peer.ID);
                        return;
                    default:
                        throw new Exception("Ne devrait pas arriver, ;D");
                }
                _peers[peer.ID] = peer;
            });
            t.Start();
        }
    }
}

