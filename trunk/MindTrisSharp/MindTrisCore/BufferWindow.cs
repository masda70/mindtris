using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace MindTrisCore
{
    public class BufferWindow
    {
        byte[] _buffer;

        public BufferWindow(int size)
        {
            _buffer = new byte[size];
            WindowStart = 0;
            WindowLength = 0;
        }

        public byte[] GetSubbufferCopy(int i, int length)
        {
            if (length > _buffer.Length) throw new IndexOutOfRangeException("Sub-buffer length must be less than original buffer length.");
            byte[] res = new byte[length];
            for (int k = 0; k < length; k++)
            {
                res[k] = this[i + k];
            }
            return res;
        }

        public byte[] BufferRaw { get { return _buffer; } }
        public int Length { get { return _buffer.Length; } }
        public int BufferPosition
        {
            get { return (WindowStart + WindowLength) % BufferRaw.Length; }
        }
        public int WindowStart { get; set; }
        public int WindowLength { get; set; }
        public byte this[int i]
        {
            get
            {
                if (i >= WindowLength) throw new IndexOutOfRangeException("i must not be larger than WindowLength");
                int index = (i + WindowStart) % _buffer.Length;
                return _buffer[index];
            }
            set
            {
                if (i >= WindowLength) throw new IndexOutOfRangeException("i must not be larger than WindowLength");
                int index = (i + WindowStart) % _buffer.Length;
                _buffer[index] = value;
            }
        }
    }
}
