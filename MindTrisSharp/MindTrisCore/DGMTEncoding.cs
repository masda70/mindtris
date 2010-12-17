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
using System.Diagnostics;
using ZenithMFramework;

namespace MindTrisCore.DGMTEncoding
{
	static public class BigE
	{
		public static void E(byte[] buffer, int index, int length)
		{
			//Ensures that number types are encoded in big endian, as specified in the protocol.
			if (BitConverter.IsLittleEndian)
			{
				Array.Reverse(buffer, index, length);
			}
		}

        public static int WriteRawBytes(byte[] buffer, ref int i, byte[] value)
        {
            value.CopyTo(buffer, i);
            i += value.Length;
            return value.Length;
        }

        public static int WriteSizePrefixed(byte[] buffer, ref int i, int byteCount, byte[] value)
        {
            switch (byteCount)
            {
                case 1:
                    byte size_8 = (byte)value.Length;
                    buffer[i] = size_8;
                    break;
                case 2:
                    ushort size_16 = (ushort)value.Length;
                    BitConverter.GetBytes(size_16).CopyTo(buffer, i);
                    break;
                case 4:
                    uint size_32 = (uint)value.Length;
                    BitConverter.GetBytes(size_32).CopyTo(buffer, i);
                    break;
                case 8:
                    ulong size_64 = (ulong)value.Length;
                    BitConverter.GetBytes(size_64).CopyTo(buffer, i);
                    break;
                default:
                    throw new Exception("Taille invalide d'entier");

            }
            //Ensures Big endianness
            BigE.E(buffer, i, byteCount);
            i += byteCount;
            value.CopyTo(buffer, i);
            i += value.Length;
            return byteCount + value.Length;
        }

        public static int WriteSizePrefixedUTF8(byte[] buffer, ref int i, int byteCount, string value)
        {
            return WriteSizePrefixed(buffer, ref i, byteCount, Encoding.UTF8.GetBytes(value));
        }

        public static int WriteSizePrefixedASCII(byte[] buffer, ref int i, int byteCount, string value)
        {
            return WriteSizePrefixed(buffer, ref i, byteCount, Encoding.ASCII.GetBytes(value));
        }

        public static int WriteInt16(byte[] buffer, ref int i, ushort value)
        {
            BitConverter.GetBytes(value).CopyTo(buffer, i);
            BigE.E(buffer, i, 2);
            i += 2;
            return 2;
        }

        public static int WriteInt32(byte[] buffer, ref int i, uint value)
        {
            BitConverter.GetBytes(value).CopyTo(buffer, i);
            BigE.E(buffer, i, 4);
            i += 4;
            return 4;
        }

        public static int WriteInt64(byte[] buffer, ref int i, ulong value)
        {
            BitConverter.GetBytes(value).CopyTo(buffer, i);
            BigE.E(buffer, i, 8);
            i += 8;
            return 8;
        }

        public static int WriteIPAddress(byte[] buffer, ref int i, IPAddress address)
        {
            byte[] ip = address.GetAddressBytes();
            Debug.Assert(ip.Length == 4);
            BigE.E(ip, 0, 4);
            for (int k = 0; k < 4; k++)
            {
                buffer[i + k] = ip[k];
            }
            i += 4;
            return 4;
        }

        public static int WriteRSAPublicKey(byte[] buffer, ref int i, string xml)
        {
            string modulusBase64;
            string exponentbase64;
            WebParser parser = new WebParser(xml);
            parser.JumpAfter(">", ">");
            parser.GetStringBefore("<", out modulusBase64);
            parser.JumpAfter(">", ">");
            parser.GetStringBefore("<", out exponentbase64);
            byte[] modulus = System.Convert.FromBase64String(modulusBase64);
            byte[] exponent = System.Convert.FromBase64String(exponentbase64);
            //Big Endianness is already insured by base64 encoding of the xml string representation
            BigE.WriteSizePrefixed(buffer, ref i, 2, modulus);
            BigE.WriteSizePrefixed(buffer, ref i, 1, exponent);
            return 1 + 2 + modulus.Length + exponent.Length;
        }

        public static int WriteDSAPublicKey(byte[] buffer, ref int i, string xml)
        {
            string p;
            string q;
            string g;
            string y;
            WebParser parser = new WebParser(xml);
            parser.JumpAfter(">", ">");
            parser.GetStringBefore("<", out p);
            parser.JumpAfter(">", ">");
            parser.GetStringBefore("<", out q);
            parser.JumpAfter(">", ">");
            parser.GetStringBefore("<", out g);
            parser.JumpAfter(">", ">");
            parser.GetStringBefore("<", out y);
            string[] a = { p, q, g, y };
            int length = 0;
            for (int k = 0; k < 4; k++)
            {
                byte[] bytes = System.Convert.FromBase64String(a[k]);
                length += BigE.WriteSizePrefixed(buffer, ref i, 2, bytes);
            }
            return length;
        }

        public static int WriteByte(byte[] buffer, ref int i, byte value)
        {
            buffer[i++] = value;
            return 1;
        }

        public static int WritePacketID(byte[] buffer, ref int i, Dgmt.PacketID id)
        {
            return WriteByte(buffer, ref i, (byte)id);
        }

        public static int WritePacketIDP2P(byte[] buffer, ref int i, Dgmt.PacketP2PID id)
        {
            return WriteByte(buffer, ref i, (byte)id);
        }

        public static int WriteBool(byte[] buffer, ref int i, bool value)
        {
            return WriteByte(buffer, ref i, (byte)(value ? 1 : 0));
        }




        public static byte[] ReadRawBytes(byte[] buffer, ref int i, int count)
        {
            byte[] res = new byte[count];
            for (int k = 0; k < count; k++)
            {
                res[k] = buffer[i + k];
            }
            i += count;
            return res;
        }

        public static byte[] ReadSizePrefixedRawBytes(byte[] buffer, ref int i, int byteCount)
        {
            int size;
            switch (byteCount)
            {
                case 1:
                    size = buffer[i];
                    break;
                case 2:
                    BigE.E(buffer, i, 2);
                    size = BitConverter.ToUInt16(buffer, i);
                    break;
                default:
                    throw new Exception("Taille invalide d'entier");
            }
            i += byteCount;
            byte[] res = new byte[size];
            for (int k = 0; k < size; k++)
            {
                res[k] = buffer[i + k];
            }
            i += size;
            return res;
        }

        public static string ReadSizePrefixedUTF8(byte[] buffer, ref int i, int byteCount)
        {
            int size;
            switch (byteCount)
            {
                case 1:
                    size = buffer[i];
                    break;
                case 2:
                    BigE.E(buffer, i, 2);
                    size = BitConverter.ToUInt16(buffer, i);
                    break;
                default:
                    throw new Exception("Taille invalide d'entier");
            }
            i += byteCount;
            string res = Encoding.UTF8.GetString(buffer, i, size);
            i += size;
            return res;
        }

        public static string ReadSizePrefixedASCII(byte[] buffer, ref int i, int byteCount)
        {
            int size;
            switch (byteCount)
            {
                case 1:
                    size = buffer[i];
                    break;
                case 2:
                    BigE.E(buffer, i, 2);
                    size = BitConverter.ToUInt16(buffer, i);
                    break;
                default:
                    throw new Exception("Taille invalide d'entier");
            }
            i += byteCount;
            string res = Encoding.ASCII.GetString(buffer, i, size);
            i += size;
            return res;
        }

        public static IPAddress ReadIPAddress(byte[] buffer, ref int i)
        {
            BigE.E(buffer, i, 4);
            byte[] ip_b = new byte[4];
            for (int j = 0; j < 4; j++) ip_b[j] = buffer[i + j];
            i += 4;
            return new IPAddress(ip_b);
        }

        public static ushort ReadInt16(byte[] buffer, ref int i)
        {
            BigE.E(buffer, i, 2);
            ushort res = BitConverter.ToUInt16(buffer, i);
            i += 2;
            return res;
        }

        public static uint ReadInt32(byte[] buffer, ref int i)
        {
            BigE.E(buffer, i, 4);
            uint res = BitConverter.ToUInt32(buffer, i);
            i += 4;
            return res;
        }

        public static ulong ReadInt64(byte[] buffer, ref int i)
        {
            BigE.E(buffer, i, 8);
            ulong res = BitConverter.ToUInt64(buffer, i);
            i += 8;
            return res;
        }

        public static byte ReadByte(byte[] buffer, ref int i)
        {
            return buffer[i++];
        }

        public static bool ReadBool(byte[] buffer, ref int i)
        {
            byte lol = ReadByte(buffer, ref i);
            return lol > 0;
        }

        public static string ReadRSAPublicKey(byte[] buffer, ref int i)
        {
            byte[] modulus = BigE.ReadSizePrefixedRawBytes(buffer, ref i, 2);
            byte[] exponent = BigE.ReadSizePrefixedRawBytes(buffer, ref i, 1);
            string modulus64 = System.Convert.ToBase64String(modulus);
            string exponent64 = System.Convert.ToBase64String(exponent);
            //XML Format and base64 show numbers in bigendian, so no need to call BigE.E
            const string format = "<RSAKeyValue><Modulus>{0}</Modulus><Exponent>{1}</Exponent></RSAKeyValue>";
            return String.Format(format, modulus64, exponent64);
        }

        public static string ReadDSAPublicKey(byte[] buffer, ref int i)
        {
            byte[][] b = new byte[4][];
            string[] s = new string[4];
            for (int k = 0; k < 4; k++) b[k] = BigE.ReadSizePrefixedRawBytes(buffer, ref i, 2);
            for (int k = 0; k < 4; k++) s[k] = System.Convert.ToBase64String(b[k]);
            const string format = "<DSAKeyValue><P>{0}</P><Q>{1}</Q><G>{2}</G><Y>{3}</Y></DSAKeyValue>";
            return String.Format(format, s);
        }
	}
}
