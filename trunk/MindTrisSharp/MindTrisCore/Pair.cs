using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace MindTrisCore
{
    public class Pair<U, V>
    {
        public U Fst { get; set; }
        public V Snd { get; set; }
        public Pair(U u, V v) { Fst = u; Snd = v; }
    }
}
