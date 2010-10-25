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

namespace MindTrisCore.DGMTEncoding
{
	static public class BigE
	{
		static public void E(byte[] buffer, int index, int length)
		{
			//Ensures that number types are encoded in big endian, as specified in the protocol.
			if (BitConverter.IsLittleEndian)
			{
				Array.Reverse(buffer, index, length);
			}
		}
	}
}
