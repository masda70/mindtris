using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net;

namespace MindTrisCore
{
    class Peer
    {
        public byte ID { get; set; }
        public string DisplayName { get; set; }
        public IPAddress IpAddress { get; set; }
        public ushort Port { get; set; }
        public string PublicKey { get; set; }
    }
}
