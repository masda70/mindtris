using System;
using System.IO;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.GamerServices;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Input;
using MindTris;
using System.Collections.Generic;
using MindTrisCore;
using Microsoft.Xna.Framework.Audio;

namespace Tetris
{
    /// <summary>
    /// This is the main type for your game
    /// </summary>
    public class Engine : Microsoft.Xna.Framework.Game
    {
        // Graphics
        GraphicsDeviceManager graphics;
        SpriteBatch spriteBatch;
        Texture2D tetrisBackground, tetrisTextures;
        SpriteFont gameFont;
        readonly Rectangle[] blockRectangles = new Rectangle[8];
        
        // Game
        Client _client;
        bool _game_active;

        List<byte> _pieces;

        Board _board;
        byte[] _first_pieces;
        uint _roundNumber = 0;
        LinkedList<Move> _pendingMoves = new LinkedList<Move>();
        TimeSpan _elapsedSinceLastRound = TimeSpan.Zero;
        const int FRAME_PER_SECONDS = 30;
        Score _score;
        bool pause = false;

        TimeSpan _elapsedMusic;
        SoundEffect _music;
        SoundEffect _degrix;
        SoundEffect _stoicien;
        Random _rand;

        Dictionary<byte, Observator> _observators;
        TimeSpan _elapsedSwitch;
        LinkedList<Peer> _initPeers;
        static TimeSpan SWITCH_DELAY = new TimeSpan(0, 0, 3);

        // Input
        KeyboardState oldKeyboardState = Keyboard.GetState();

        public Engine(Client client, ICollection<Peer> peers, byte[] pieces)
        {
            graphics = new GraphicsDeviceManager(this);
            _first_pieces = pieces;
            _pieces = new List<byte>(_first_pieces);
            _client = client;
            _game_active = false;
            _initPeers = new LinkedList<Peer>(peers);
            _observators = new Dictionary<byte, Observator>();
            _rand = new Random();

            Content.RootDirectory = "Content";

            // Create sprite rectangles for each figure in texture file
            int k = 0;
            // I figure
            blockRectangles[k++] = new Rectangle(0, 24, 24, 24);
            // J figure
            blockRectangles[k++] = new Rectangle(120, 0, 24, 24);
            // L figure
            blockRectangles[k++] = new Rectangle(216, 24, 24, 24);
            // O figure
            blockRectangles[k++] = new Rectangle(312, 0, 24, 24);
            // S figure
            blockRectangles[k++] = new Rectangle(48, 96, 24, 24);
            // T figure
            blockRectangles[k++] = new Rectangle(144, 96, 24, 24);
            // Z figure
            blockRectangles[k++] = new Rectangle(240, 72, 24, 24);
            //Ghost piece
            blockRectangles[k++] = new Rectangle(0, 48, 24, 24);
        }

        /// <summary>
        /// Allows the game to perform any initialization it needs to before starting to run.
        /// This is where it can query for any required services and load any non-graphic
        /// related content.  Calling base.Initialize will enumerate through any components
        /// and initialize them as well.
        /// </summary>
        protected override void Initialize()
        {
            Window.Title = "Mindtris# XNA Edition 0.1";
            
            graphics.PreferredBackBufferHeight = 600;
            graphics.PreferredBackBufferWidth = 1600;
            graphics.ApplyChanges();

            this.TargetElapsedTime = TimeSpan.FromSeconds(1.0f / (float)FRAME_PER_SECONDS);

            // Try to open file if it exists, otherwise create it
            using (FileStream fileStream = File.Open("record.dat", FileMode.OpenOrCreate))
            {
                fileStream.Close();
            }

            base.Initialize();

            //On signale au serveur qu'on est ready
            _client.BeginGame += new Client.VoidFunction(_client_BeginGame);
            _client.SignalGameLoaded(false, null);
            _client.NewPiecesIncoming += new Client.ReceivingNewPieces(_client_NewPiecesIncoming);
            _client.MoveMade += new Client.MoveFunction(_client_MoveMade);
        }

        void _client_MoveMade(Peer peer, uint roundNumber, Move[] moves)
        {
            //[TODO] Attention à l'horrible cast
            for (int i = 0; i < moves.Length; i++)
            {
                Console.WriteLine("Received move {0} : ({1}, {2})", moves[i].PieceNumber, moves[i].X, moves[i].Y);
                _observators[peer.ID].Board.PlacePiece(_pieces[(int)moves[i].PieceNumber], moves[i].Orientation, moves[i].X, TransformY(moves[i].Y));
            }
        }

        void _client_NewPiecesIncoming(uint offset, byte[] pieces)
        {
            _pieces.AddRange(pieces);
            _board.FeedPieces(pieces);
        }

        void _client_BeginGame()
        {
            _game_active = true;
            _elapsedMusic = TimeSpan.Zero;
            _elapsedSwitch = TimeSpan.Zero;
            _music.Play(0.3f, 0.0f, 0.0f);
        }

        int _turn = 0;
        void SwitchObservator()
        {
            if (_observators.Count > 0)
            {
                int i = 0;
                byte peer = Byte.MaxValue;
                foreach (byte peerouze in _observators.Keys)
                {
                    if (i == _turn) peer = peerouze;
                    i++;
                }
                _turn = (_turn + 1) % _observators.Count;
                if (peer < Byte.MaxValue)
                {
                    foreach (byte peerouze in _observators.Keys)
                    {
                        if (peerouze != peer)
                        {
                            _observators[peerouze].Board.Visible = false;
                            _observators[peerouze].Score.Visible = false;
                        }
                    }
                    _observators[peer].Board.Visible = true;
                    _observators[peer].Score.Visible = true;
                }
            }
        }

        /// <summary>
        /// LoadContent will be called once per game and is the place to load
        /// all of your content.
        /// </summary>
        protected override void LoadContent()
        {
            // Create a new SpriteBatch, which can be used to draw textures.
            spriteBatch = new SpriteBatch(GraphicsDevice);

            // Add the SpriteBatch service
            Services.AddService(typeof(SpriteBatch), spriteBatch);            

            //Load 2D textures
            tetrisBackground = Content.Load<Texture2D>("background");
            tetrisTextures = Content.Load<Texture2D>("tetris");

            // Load game font
            gameFont = Content.Load<SpriteFont>("font");

            _music = Content.Load<SoundEffect>("ost");
            _degrix = Content.Load<SoundEffect>("degrix");
            _stoicien = Content.Load<SoundEffect>("stoicien");

            
            // Create game field
            _board = new Board(this, _first_pieces, ref tetrisTextures, blockRectangles);

            _board.PiecesRequired += new Board.VoidFunction(_board_PiecesRequired);
            _board.MoveExecuted += new Board.MoveExecutedFunction(_board_MoveExecuted);
            _board.LinesDeleted += new Board.IntFunction(_board_LinesDeleted);

            _board.Initialize();
            Components.Add(_board);
            _board.Speed = (float)(1.0 / (float)FRAME_PER_SECONDS);

            _score = new Score(this, new Peer() { DisplayName = _client.Name }, gameFont, 0, 0);
            _score.Initialize();
            Components.Add(_score);

            foreach (Peer peer in _initPeers)
            {
                BoardObserver boardObserver = new BoardObserver(this, ref tetrisTextures, blockRectangles, 800, 0);
                boardObserver.Initialize();
                Components.Add(boardObserver);


                // Save player's score and game level
                Score score = new Score(this, peer, gameFont, 800, 0);
                score.Initialize();
                Components.Add(score);

                _observators[peer.ID] = new Observator(boardObserver, score);
            }

            // Load game record
            using (StreamReader streamReader = File.OpenText("record.dat"))
            {
                string player = null;
                if ((player = streamReader.ReadLine()) != null)
                    _score.RecordPlayer = player;
                int record = 0;
                if ((record = Convert.ToInt32(streamReader.ReadLine())) != 0)
                    _score.RecordScore = record;
            }
        }

        void _board_LinesDeleted(int n)
        {
            _stoicien.Play();
        }

        void _board_MoveExecuted(uint pieceNumber, byte orientation, byte x, byte y)
        {
            _degrix.Play();
            y = TransformY(y);
            Move move = new Move(pieceNumber, orientation, x, y);
            Console.WriteLine("Move {0} : piece {1}, orientation {2},({3}, {4})", pieceNumber, _pieces[(int)pieceNumber], orientation, x, y);
            _pendingMoves.AddLast(move);
        }

        byte TransformY(byte y)
        {
            return (byte)((Board.HEIGHT - 1) - y);
        }

        void _board_PiecesRequired()
        {
            _client.RequestNewPieces((uint)_pieces.Count, 0x10);
        }

        bool _isKeyLeftDown = false;
        bool _isKeyRightDown = false;
        bool _isKeyUpDown = false;
        bool _isKeyDownDown = false;
        bool _isKeySpaceDown = false;
        int _updates_frequency_X = 2;
        int _tous_les_X_updates = 0;

        /// <summary>
        /// Allows the game to run logic such as updating the world,
        /// checking for collisions, gathering input, and playing audio.
        /// </summary>
        /// <param name="gameTime">Provides a snapshot of timing values.</param>
        protected override void Update(GameTime gameTime)
        {
            _elapsedSinceLastRound = _elapsedSinceLastRound.Add(gameTime.ElapsedGameTime);
            _elapsedMusic = _elapsedMusic.Add(gameTime.ElapsedGameTime);
            _elapsedSwitch = _elapsedSwitch.Add(gameTime.ElapsedGameTime);
            if (_elapsedMusic > _music.Duration)
            {
                //Restart music
                _elapsedMusic = TimeSpan.Zero;
                //_music.Play(0.3f, 0.0f, 0.0f);    
            }
            if (_elapsedSwitch > SWITCH_DELAY)
            {
                SwitchObservator();
                _elapsedSwitch = TimeSpan.Zero;
            }
            if (_game_active)
            {
                // Allows the game to exit
                KeyboardState keyboardState = Keyboard.GetState();
                if (keyboardState.IsKeyDown(Keys.Escape))
                    this.Exit();

                // Check pause
                bool pauseKey = (oldKeyboardState.IsKeyDown(Keys.P) &&
                    (keyboardState.IsKeyUp(Keys.P)));

                oldKeyboardState = keyboardState;

                if (pauseKey)
                    pause = !pause;

                if (!pause)
                {
                    // Find dynamic figure position
                    _board.FindDynamicFigure();

                    // Increase player score
                    int lines = _board.DestroyLines();
                    if (lines > 0)
                    {
                        _score.Value += lines;// (int)((5.0f / 2.0f) * lines * (lines + 3));
                        //_board.Speed += 0.005f;
                    }

                    _score.Level = (int)(10 * _board.Speed);

                    // Create new shape in game
                    if (!_board.CreateNewFigure())
                        GameOver();
                    else
                    {
                        //[TOCHECK] On peut sûrement améliorer les behaviors des pieces
                        if (_tous_les_X_updates == 0)
                        {
                            if (keyboardState.IsKeyDown(Keys.Space))
                                 _isKeySpaceDown = true;
                            if (keyboardState.IsKeyUp(Keys.Space) && _isKeySpaceDown)
                            {
                                _isKeySpaceDown = false;
                                _board.HardDrop();
                                goto Fin;
                            }

                            // If left key is pressed then released
                            if (keyboardState.IsKeyDown(Keys.Left))
                            {
                                _board.MoveFigureLeft();
                            }
                            // If right key is pressed
                            if (keyboardState.IsKeyDown(Keys.Right))
                            {
                                _board.MoveFigureRight();
                            }
                            // If down key is pressed
                            if (keyboardState.IsKeyDown(Keys.Down))
                            {
                                _board.MoveFigureDown();
                            }
                        }

                        // Rotate figure
                        if (keyboardState.IsKeyDown(Keys.Up)) _isKeyUpDown = true;
                        if (keyboardState.IsKeyUp(Keys.Up) && _isKeyUpDown)
                        {
                            _isKeyUpDown = false;
                            _board.RotateFigureSRSTrueRotation(true);
                        }

                        // Moving figure
                        if (_board.Movement >= 1)
                        {
                            _board.Movement = 0;
                            _board.MoveFigureDown();
                        }
                        else
                            _board.Movement += _board.Speed;
                        //*/
                    }
                }

            Fin:
                //Sending Round packets
                if (_elapsedSinceLastRound > new TimeSpan(0, 0, 0, 0, Client.ROUND_DELAY_MILLISECONDS - 20))
                {
                    _client.SendRoundPacket(_roundNumber, _pendingMoves);
                    _pendingMoves.Clear();
                    _roundNumber++;
                    _elapsedSinceLastRound = TimeSpan.Zero;
                }
            }
            _tous_les_X_updates = (_tous_les_X_updates + 1) % _updates_frequency_X;
            base.Update(gameTime);
        }

        private void GameOver()
        {
            if (_score.Value > _score.RecordScore)
            {
                _score.RecordScore = _score.Value;

                pause = true;

                Record record = new Record();
                record.ShowDialog();

                _score.RecordPlayer = record.Player;

                using (StreamWriter writer = File.CreateText("record.dat"))
                {
                    writer.WriteLine(_score.RecordPlayer);
                    writer.WriteLine(_score.RecordScore);
                }

                pause = false;
            }
            _board.Initialize();
            _score.Initialize();
        }

        /// <summary>
        /// This is called when the game should draw itself.
        /// </summary>
        /// <param name="gameTime">Provides a snapshot of timing values.</param>
        protected override void Draw(GameTime gameTime)
        {
            spriteBatch.Begin();
            spriteBatch.Draw(tetrisBackground, Vector2.Zero, Color.White);
            spriteBatch.Draw(tetrisBackground, new Vector2((float)tetrisBackground.Width, 0.0f), Color.White);

            base.Draw(gameTime);
            spriteBatch.End();
        }

        
    }

    class Observator
    {
        public Score Score { get; set; }
        public BoardObserver Board { get; set; }

        public Observator(BoardObserver board, Score score)
        {
            Board = board;
            board.LinesDeleted += new BoardObserver.IntFunction(board_LinesDeleted);
            Score = score;
        }

        void board_LinesDeleted(int n)
        {
            Score.Value += n;
        }
    }
}