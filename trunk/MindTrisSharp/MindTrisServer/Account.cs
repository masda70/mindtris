using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace MindTrisServer
{
    class Account
    {
        public string UserName { get; set; }
        public string DisplayedName { get; set; }
        public string Password { get; set; }
        public string Email { get; set; }

        public override string ToString()
        {
            return DisplayedName;
        }
    }
}
