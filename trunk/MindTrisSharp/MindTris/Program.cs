using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using MindTrisCore;
using System.Threading;
using ZenithMFramework;
using System.Net;
using System.Security.Cryptography;
using System.Diagnostics;

namespace MindTris
{
    class LobbyListItem
    {
        public uint _0_lobbyID;
        public UTFString1 _2_lobbyName;
        public byte _3_playerCount;
        public byte _4_maxplayer;
        public byte _5_passProtected;
        public UTFString1 _7_creator;
    }

    class LobbyRetrievalResponsePacket : Packet
    {
        public byte _1_type = 0x84;
        public byte _2_list_size;
        public LobbyListItem[] _3_lobbyList;
    }

    class Program
    {
        static Client _client;
        static Dictionary<byte, Peer> _peers;

        static string _ip_server;
        static string _login;
        static string _pass;
        static string _email = "zenithm@lolzorz1337.org";
        static ushort _port;

        public static byte[] StringToByteArray(string hex)
        {
            return Enumerable.Range(0, hex.Length).
                   Where(x => 0 == x % 2).
                   Select(x => Convert.ToByte(hex.Substring(x, 2), 16)).
                   ToArray();
        }

        static void Main(string[] args)
        {
            string keys_0 = "<RSAKeyValue><Modulus>kVPEnS5vVgdySMJGDHLdBg+mPjB8FVx82tZWSHwY9PwFeXntZJ1pdal+L10ScdgHrj8kduz2x3FwgYQbuRy1SwhEI1Tia6rsCd1Asxl20ZhsllRXO4YprkuPCAWtro7xwSDpscr5OYV1oVnxwqbY9E5KyaVtHiFusdh8X+PuQO0=</Modulus><Exponent>AQAB</Exponent><P>xfje5exXOw239RbMU0e0dMEQfihCwtJQ9IlmGIp33HAHcbAhT/SDDPiZtXeRZhZiPJUUHEE+5PmqUgm+gRTFCQ==</P><Q>u+ybpZkyl8NiYgZyWUbYCR706J9ZausHlTKFpjKp6VtuvFgIf/Y9pLAqeQX78T9UuqB+Kr0WtGVJKfGHDf7ZxQ==</Q><DP>YlHrQX1TBT0W813TNDkUYjfQHReZsHALTKdAUUfTp2LsD4ZNxQvGWhbNH9a2G2FagIP5bN7qgYWNFRlJx4mtIQ==</DP><DQ>OGNx/MmwTauaDNHkY/eHAY2hbV/LQ/LMLq+fPNR8+YGvA6LiwgrQSmd2BySNicE4GlvoH8jnVDAEOSq5HLD5BQ==</DQ><InverseQ>hE4FodZRhsiee+AtJlNmci+EGce3jWHfNAKLB+bBCN5udVrHLPkgvQkh5oOTjDe/0/NXpa/yyN9F/4etR88ASw==</InverseQ><D>YF6fO+gl9nN7qpoBn5Gv1awc/pJiRjwNo4SMtQZt+k8BRlu/O4BTBa5+uePLLmkmNrsycBaVUw57n7c+NxO9d+M99EmLaUxIGFvoOPVbVS/U1Opj2jPlqzybxN2Z+JIO7ZaATg8Cuscgy+1b41o12kBcFlpttlbbsc1f41kvnmE=</D></RSAKeyValue>";
            string keys_1 = "<RSAKeyValue><Modulus>kVPEnS5vVgdySMJGDHLdBg+mPjB8FVx82tZWSHwY9PwFeXntZJ1pdal+L10ScdgHrj8kduz2x3FwgYQbuRy1SwhEI1Tia6rsCd1Asxl20ZhsllRXO4YprkuPCAWtro7xwSDpscr5OYV1oVnxwqbY9E5KyaVtHiFusdh8X+PuQO0=</Modulus><Exponent>AQAB</Exponent></RSAKeyValue>";
            string msg = "Est-ce que ca marche vraiment lolilol";
            RSACryptoServiceProvider p = new RSACryptoServiceProvider();
            p.FromXmlString(keys_1);
            byte[] lol = p.Encrypt(Encoding.ASCII.GetBytes(msg), true);
            string base64 = System.Convert.ToBase64String(lol);
            base64.ToString();

            p = new RSACryptoServiceProvider();
            p.FromXmlString(keys_0);
            //PAR DAVID base64 = "Mw+Cd7j27tvL36DH7jgfjrhGHK8eJc2XEXIkuwGpOruvLRdrkhEo/houQaJ9Qy5Zkxu7MWNIcQAJtyH4DWG9wa4QctdlCFR6aZ/2ryTuUz4YcQGFijjkllQ331m3u+YRM+YMOGset9h8FxRqqtcpJWK7MjJUh18agjzUlRuDK3A=";
            base64 = "Qh+V7nIpPZE5kLazNG0xmU4WnjAdjLugxIoN3B+MJpOrPRi9ClpK85kHFtT1y3kC0x/Z0UqkpB7valjIHspg/oZy82afey6h0zseE6EdT0Hgfc2TsXsifF4J2zDYijjkt1Qifny3Lo3mwSQXJ7P8k5Gy/CkXqhekbAovJU3T1K0=";
            //base64 = "PTKla7xLGPlxvi/LZ+nXjjhzzBW4ftt7p0QVXz4uGpsTX7vURbC9sMm41l68bF86H4k/R408RVV4vlTXFVmGZCmBCsrsD118D+7a62JILLk9V7L2/MhIEeeaAIOugtwfQCdR5C7hWmkT5831buhd59Q78c7UMVIIdbEqnMIcK6E=";
            byte[] rgb = System.Convert.FromBase64String(base64);
            byte[] decrypted = p.Decrypt(rgb, true);
            msg = Encoding.ASCII.GetString(decrypted);
            msg.ToString();



			if (args.Length > 0)
			{
				_ip_server = args[0];
			}
			else _ip_server = "127.0.0.1";
            
            //_ip_server = "138.231.142.111";
            
            //IPAddress address = Dns.GetHostAddresses("m70.crans.org")[0];
            //_ip_server = address.ToString();
            //*/
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
        }

        static void client_Connected(string motd)
        {
                        Thread t = new Thread(() =>
                {
            Console.WriteLine("MOTD: {0}", motd);
            Console.WriteLine("Login?");
            _login = Console.ReadLine();
            Console.WriteLine("Pass?");
            _pass = Console.ReadLine();
            Console.WriteLine("Port?");
            while (!UInt16.TryParse(Console.ReadLine(), out _port)) ;
            _client.UserCreated += new Client.ConfirmationFunction(_client_UserCreated);
            _client.CreateUser(_login, _pass, _email);
            //_client_UserCreated(0x00);
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
            if (lobbies.Length > 0)
            {
                string[] lobbies_name = new string[lobbies.Length];
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
                _client.CreateLobby(name, (byte)count, false, "");
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
            foreach (Peer peer in peers)
            {
                _peers[peer.ID] = peer;
            }
            Console.WriteLine("Starting chat...");
            StartChat();
                    });
            t.Start();
        }

        private static void StartChat()
        {
            while (true)
            {
                string msg = Console.ReadLine();
                switch (msg)
                {
                    case "@":

                        break;
                    default:
                        _client.SendMessage(msg);
                        break;
                }
            }
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
                    _client.ConnectPeer(peer);
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
