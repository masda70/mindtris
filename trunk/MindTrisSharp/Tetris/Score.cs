using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using MindTrisCore;
using System;

namespace Tetris
{
    /// <summary>
    /// This is a game component that implements IUpdateable.
    /// </summary>
    public class Score : Microsoft.Xna.Framework.DrawableGameComponent
    {
        // Graphic
        protected SpriteBatch sBatch;
        protected SpriteFont font;

        // Counters
        protected int value;
        protected int level;
        protected int recordScore = 0;
        protected string recordPlayer = "Player 1";

        Vector2 _trans;
        Peer _peer;

        public Score(Game game, Peer peer, SpriteFont font, int x, int y)
            : base(game)
        {
            _trans = new Vector2(x, y);
            _peer = peer;
            sBatch = (SpriteBatch)Game.Services.GetService(typeof(SpriteBatch));
            this.font = font;
        }

        /// <summary>
        /// Allows the game component to perform any initialization it needs to before starting
        /// to run.  This is where it can query for any required services and load content.
        /// </summary>
        public override void Initialize()
        {
            value = 0;
            level = 1;
            base.Initialize();
        }

        public int Value
        {
            set { this.value = value; }
            get { return value; }
        }
        public int Level
        {
            set { level = value; }
            get { return level; }
        }
        public uint PiecesPlayed
        {
            get;
            set;
        }
        public int RecordScore
        {
            set { recordScore = value; }
            get { return recordScore; }
        }
        public string RecordPlayer
        {
            set { recordPlayer = value; }
            get { return recordPlayer; }
        }

        /// <summary>
        /// This is called when the game should draw itself.
        /// </summary>
        /// <param name="gameTime">Provides a snapshot of timing values.</param>
        public override void Draw(GameTime gameTime)
        {
            sBatch.DrawString(font, "Player:\n" + _peer.DisplayName.Substring(0, Math.Min(10, _peer.DisplayName.Length)) + "\nPieces: " + PiecesPlayed,
                new Vector2(1.5f * 24, 3 * 24) + _trans, Color.Green);

            sBatch.DrawString(font, "Score:\n" + value,
                new Vector2(1.5f * 24, 13 * 24) + _trans, Color.Orange);

            base.Draw(gameTime);
        }
    }
}