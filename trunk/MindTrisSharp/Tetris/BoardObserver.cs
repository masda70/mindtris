using System;
using System.Collections.Generic;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;

namespace Tetris
{
    class BoardObserver : Microsoft.Xna.Framework.DrawableGameComponent
    {
        SpriteBatch sBatch;
        Texture2D textures;
        Rectangle[] rectangles;
        enum FieldState { Free, Static, Dynamic };
        FieldState[,] boardFields;
        Vector2[, ,] Figures;
        readonly Vector2 StartPositionForNewFigure = new Vector2(3, 0);
        Vector2 PositionForDynamicFigure;
        Vector2[] DynamicFigure = new Vector2[BLOCKS_COUNT_IN_FIGURE];
        Random random = new Random();
        int[,] BoardColor;
        const int HEIGHT = 20;
        const int WIDTH = 10;
        const int BLOCKS_COUNT_IN_FIGURE = 4;
        int DynamicFigureNumber;
        int DynamicFigureModificationNumber;
        int DynamicFigureColor;
        bool BlockLine;
        bool showNewBlock;
        float movement;
        float speed;

        float _offset_x;
        float _offset_y;
        
        Queue<int> nextFigures = new Queue<int>();
        Queue<int> nextFiguresModification = new Queue<int>();

        public delegate void IntFunction(int n);
        public event IntFunction LinesDeleted;

        public float Movement
        {
            set { movement = value; }
            get { return movement; }
        }
        public float Speed
        {
            set { speed = value; }
            get { return speed; }
        }

        public BoardObserver(Game game, ref Texture2D textures, Rectangle[] rectangles, float x, float y)
            : base(game)
        {
            sBatch = (SpriteBatch)Game.Services.GetService(typeof(SpriteBatch));

            // Load textures for blocks
            this.textures = textures;

            // Rectangles to draw figures
            this.rectangles = rectangles;

            _offset_x = x;
            _offset_y = y;

            // Create tetris board
            boardFields = new FieldState[WIDTH, HEIGHT];
            BoardColor = new int[WIDTH, HEIGHT];

            Figures = Tetrominos.CreateFiguresSRSTrueRotations();

            nextFigures.Enqueue(random.Next(7));
            nextFigures.Enqueue(random.Next(7));
            nextFigures.Enqueue(random.Next(7));
            nextFigures.Enqueue(random.Next(7));

            /*
            nextFiguresModification.Enqueue(random.Next(4));
            nextFiguresModification.Enqueue(random.Next(4));
            nextFiguresModification.Enqueue(random.Next(4));
            nextFiguresModification.Enqueue(random.Next(4));
            //*/
        }

        public override void Initialize()
        {
            showNewBlock = true;
            movement = 0;
            speed = 0.1f / 3;

            for (int i = 0; i < WIDTH; i++)
                for (int j = 0; j < HEIGHT; j++)
                    ClearBoardField(i, j);

            base.Initialize();
        }

        public void FindDynamicFigure()
        {
            int BlockNumberInDynamicFigure = 0;
            for (int i = 0; i < WIDTH; i++)
                for (int j = 0; j < HEIGHT; j++)
                    if (boardFields[i, j] == FieldState.Dynamic)
                        DynamicFigure[BlockNumberInDynamicFigure++] = new Vector2(i, j);
        }

        /// <summary>
        /// Find, destroy and save lines's count
        /// </summary>
        /// <returns>Number of destoyed lines</returns>
        public int DestroyLines()
        {
            // Find total lines
            int BlockLineCount = 0;
            for (int j = 0; j < HEIGHT; j++)
            {
                BlockLine = true;
                for (int i = 0; i < WIDTH; i++)
                    if (boardFields[i, j] == FieldState.Free) BlockLine = false;
                //Destroy total lines
                if (BlockLine)
                {
                    // Save number of total lines
                    BlockLineCount++;
                    for (int l = j; l > 0; l--)
                        for (int k = 0; k < WIDTH; k++)
                        {
                            boardFields[k, l] = boardFields[k, l - 1];
                            BoardColor[k, l] = BoardColor[k, l - 1];
                        }
                    for (int l = 0; l < WIDTH; l++)
                    {
                        boardFields[l, 0] = FieldState.Free;
                        BoardColor[l, 0] = -1;
                    }
                }
            }
            if (BlockLineCount > 0)
            {
                if (LinesDeleted != null) LinesDeleted(BlockLineCount);
            }
            return BlockLineCount;
        }

        bool DrawFigureOnBoard(Vector2[] vector, int color)
        {
            if (!TryPlaceFigureOnBoard(vector))
                return false;
            for (int i = 0; i <= vector.GetUpperBound(0); i++)
            {
                boardFields[(int)vector[i].X, (int)vector[i].Y] = FieldState.Dynamic;
                BoardColor[(int)vector[i].X, (int)vector[i].Y] = color;
            }
            return true;
        }

        bool TryPlaceFigureOnBoard(Vector2[] vector)
        {
            for (int i = 0; i <= vector.GetUpperBound(0); i++)
                if ((vector[i].X < 0) || (vector[i].X >= WIDTH) ||
                    (vector[i].Y >= HEIGHT))
                    return false;
            for (int i = 0; i <= vector.GetUpperBound(0); i++)
                if (boardFields[(int)vector[i].X, (int)vector[i].Y] == FieldState.Static)
                    return false;
            return true;
        }

        public Vector2 GetPositionOfPlacedPiece(Vector2[] piece)
        {
            float Y_max = float.MinValue;
            for (int i = 0; i < piece.Length; i++)
            {
                if (piece[i].Y > Y_max) Y_max = piece[i].Y;
            }
            //On a chopé le truc le plus bas, on chope le truc le plus à gauche
            float X_min = float.MaxValue;
            for (int i = 0; i < piece.Length; i++)
            {
                if (piece[i].X < X_min) X_min = piece[i].X;
            }
            return new Vector2(X_min, Y_max);
        }

        public bool PlacePiece(byte piece, byte orientation, byte x, byte y)
        {
            Vector2[] piece_v = new Vector2[BLOCKS_COUNT_IN_FIGURE];
            for (int i = 0; i < BLOCKS_COUNT_IN_FIGURE; i++)
            {
                piece_v[i] = Figures[piece, orientation, i];
            }
            Vector2 reference = GetPositionOfPlacedPiece(piece_v);
            //Translater tout ça
            Vector2 absolute_pos = new Vector2(x, y);
            for (int i = 0; i < BLOCKS_COUNT_IN_FIGURE; i++)
            {
                piece_v[i] = absolute_pos + (piece_v[i] - reference);
            }
            //On draw ça sur le board
            bool result = DrawFigureOnBoard(piece_v, piece);
            DestroyLines();
            return result;
        }

        public void SortingVector2(ref Vector2[] vector, bool sortByX, int a, int b)
        {
            if (a >= b)
                return;
            int i = a;
            for (int j = a; j <= b; j++)
            {
                if (sortByX)
                {
                    if (vector[j].X <= vector[b].X)
                    {
                        Vector2 tempVector = vector[i];
                        vector[i] = vector[j];
                        vector[j] = tempVector;
                        i++;
                    }
                }
                else
                {
                    if (vector[j].Y <= vector[b].Y)
                    {
                        Vector2 tempVector = vector[i];
                        vector[i] = vector[j];
                        vector[j] = tempVector;
                        i++;
                    }
                }
            }
            int c = i - 1;
            SortingVector2(ref vector, sortByX, a, c - 1);
            SortingVector2(ref vector, sortByX, c + 1, b);
        }
        void ClearBoardField(int i, int j)
        {
            boardFields[i, j] = FieldState.Free;
            BoardColor[i, j] = -1;
        }
        public override void Draw(GameTime gameTime)
        {
            Vector2 startPosition;
            // Draw the blocks
            for (int i = 0; i < WIDTH; i++)
                for (int j = 0; j < HEIGHT; j++)
                    if (boardFields[i, j] != FieldState.Free)
                    {
                        startPosition = new Vector2((10 + i) * rectangles[0].Width,
                            (2 + j) * rectangles[0].Height) + new Vector2(_offset_x, _offset_y);
                        sBatch.Draw(textures, startPosition, rectangles[BoardColor[i, j]], Color.White);
                    }

            base.Draw(gameTime);
        }
    }
}