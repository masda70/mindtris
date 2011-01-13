using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using MindTrisCore;

namespace Tetris
{
    class Penalty
    {
        byte _peerID;
        uint _roundNumber;
        int _lines;
        bool _me;

        Random _rand;

        protected Penalty(byte peerID, uint roundNumber, int lines, bool isForMe)
        {
            _peerID = peerID;
            _roundNumber = roundNumber;
            _lines = lines;
            _me = isForMe;
            _rand = new Random((int)roundNumber);
        }

        public Penalty(byte peer, uint roundNumber, int lines) : this(peer, roundNumber, lines, false) { }
        public Penalty(uint roundNumber, int lines) : this(0, roundNumber, lines, true) { }

        public byte NextHoleX()
        {
            return (byte)(_rand.Next() % 10);
        }

        public byte PeerID { get { return _peerID; } }
        public uint RoundNumber { get { return _roundNumber; } }
        public int Lines { get { return _lines; } }
        public bool IsForMe { get { return _me; } }
    }
}
