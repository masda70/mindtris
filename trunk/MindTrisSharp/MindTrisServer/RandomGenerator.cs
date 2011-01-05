using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace MindTrisServer
{
    class RandomGenerator
    {
        int _seed;
        uint _count;

        List<byte> _saved_pieces;

        Random _rand;
        List<byte> _pieces;
        LinkedList<byte> _to_draw_from;
        const int MAX_PIECES = 7;

        public RandomGenerator(int seed)
        {
            _seed = seed;
            _rand = new Random(seed);
            _saved_pieces = new List<byte>();
            _count = 0;
            _pieces = new List<byte>(MAX_PIECES);
            _to_draw_from = new LinkedList<byte>();
        }

        void FillPieces()
        {
            _pieces.Clear();
            for (byte i = 0; i < MAX_PIECES; i++)
            {
                _pieces.Add(i);
            }
        }

        void CreatePermutation()
        {
            FillPieces();
            _to_draw_from.Clear();
            while (_pieces.Count > 0)
            {
                int i = _rand.Next(0, _pieces.Count);
                _to_draw_from.AddLast(_pieces[i]);
                _pieces.RemoveAt(i);
            }
        }

        public byte Next()
        {
            if (_to_draw_from.Count == 0) CreatePermutation();
            //pop
            byte to_return = _to_draw_from.First.Value;
            _to_draw_from.RemoveFirst();
            _count++;
            //Save
            _saved_pieces.Add(to_return);
            return to_return;
        }

        public byte[] Next(int count)
        {
            return Next(0, count);
        }

        public byte[] Next(uint offset, int count)
        {
            byte[] result = new byte[count];
            uint j = 0;
            while (NextPieceIndex > j + offset)
            {
                //[TOCHECK] C'est horrible (le cast)
                //On chope dans les saved
                result[j] = _saved_pieces[(int)(j + offset)];
                j++;
            }
            for (; j < count; j++)
            {
                result[j] = Next();
            }
            return result;
        }

        public void Reset()
        {
            _rand = new Random(_seed);
            _count = 0;
            _saved_pieces = new List<byte>();
            _to_draw_from.Clear();
            _pieces.Clear();
        }

        public int Seed { get { return _seed; } }

        public uint NextPieceIndex { get { return _count; } }
    }
}
