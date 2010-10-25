using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace MindTris
{
    class Program
    {
        static void Main(string[] args)
        {
			string ip;
			if (args.Length > 0)
			{
				ip = args[0];
			}
			else ip = "127.0.0.1";
            Client client = new Client("ZenithM", ip);
            client.HelloFromClient();
            Console.ReadLine();
        }
    }
}
