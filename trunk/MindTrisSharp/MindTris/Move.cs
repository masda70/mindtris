using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace MindTris
{
    public class Move
    {
        uint _moveTimeStamp;
        uint _pieceNumber;
        byte _orientation;
        byte _x;
        byte _y;

        public Move(uint moveTimeStamp, uint pieceNumber, byte orientation, byte x, byte y)
        {
            _moveTimeStamp = moveTimeStamp;
            _pieceNumber = pieceNumber;
            _orientation = orientation;
            _x = x;
            _y = y;
        }

        public uint MoveTimeStamp { get { return _moveTimeStamp; } }
        public uint PieceNumber { get { return _pieceNumber; } }
        public byte Orientation { get { return _orientation; } }
        public byte X { get { return _x; } }
        public byte Y { get { return _y; } }
    }
}
