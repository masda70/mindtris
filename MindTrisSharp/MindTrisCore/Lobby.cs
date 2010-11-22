using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace MindTrisCore
{
    class Lobby
    {
        public uint ID { get; set; }
        public string Name { get; set; }
        public byte PlayerCount { get; set; }
        public byte PlayerMaxCount { get; set; }
        public bool PasswordProtected { get; set; }
        public string Creator { get; set; }
    }
}
