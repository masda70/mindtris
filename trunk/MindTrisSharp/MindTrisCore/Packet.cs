using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Reflection;
using MindTrisCore.DGMTEncoding;

namespace MindTrisCore
{
    class UTFString1
    {
        public string s;
    }
    class UTFString2
    {
        public string s;
    }
    class UTFString4
    {
        public string s;
    }

    class ASCIIString1
    {
        public string s;
    }
    class ASCIIString2
    {
        public string s;
    }
    class ASCIIString4
    {
        public string s;
    }

    class Packet
    {
        public readonly byte[] _0_0_protocol = Encoding.ASCII.GetBytes("DGMT");
        public ushort _0_1_size;

        public int Write(byte[] buffer, int offset)
        {
            int i = offset;
            WriteValue(buffer, ref i, this);
            return i - offset;
        }

        public static void WriteType(byte[] buffer, ref int i, object value)
        {
            Type type = value.GetType();
            FieldInfo[] all_fields = type.GetFields();
            var sorted = all_fields.OrderBy(x => { return x.Name; });
            foreach (FieldInfo field in sorted)
            {
                WriteValue(buffer, ref i, field.GetValue(value));
            }
        }

        public static void WriteValue(byte[] buffer, ref int i, object value)
        {
            if (value is ASCIIString1) { BigE.WriteSizePrefixedASCII(buffer, ref i, 1, ((ASCIIString1)value).s); }
            else if (value is ASCIIString2) { BigE.WriteSizePrefixedASCII(buffer, ref i, 2, ((ASCIIString2)value).s); }
            else if (value is ASCIIString4) { BigE.WriteSizePrefixedASCII(buffer, ref i, 4, ((ASCIIString4)value).s); }
            else if (value is UTFString1) { BigE.WriteSizePrefixedASCII(buffer, ref i, 1, ((UTFString1)value).s); }
            else if (value is UTFString2) { BigE.WriteSizePrefixedASCII(buffer, ref i, 2, ((UTFString2)value).s); }
            else if (value is UTFString4) { BigE.WriteSizePrefixedASCII(buffer, ref i, 4, ((UTFString4)value).s); }
            else if (value is byte) { BigE.WriteByte(buffer, ref i, (byte)value); }
            else if (value is ushort) { BigE.WriteInt16(buffer, ref i, (ushort)value); }
            else if (value is uint) { BigE.WriteInt32(buffer, ref i, (uint)value); }
            else if (value is ulong) { BigE.WriteInt64(buffer, ref i, (ulong)value); }
            else if (value is bool) { BigE.WriteBool(buffer, ref i, (bool)value); }
            else if (value is Array)
            {
                Array array = (Array)value;
                for (int k = 0; k < array.Length; k++)
                {
                    WriteValue(buffer, ref i, array.GetValue(k));
                }
            }
            else
            {
                //C'est une classe custom
                WriteType(buffer, ref i, value);
            }
        }
    }
}
