#ifndef MT_BOARD_H
#define MT_BOARD_H


enum CollisionTest {
	NoCollision = 0,
	OutOfBounds = 1,
	BottomReached = 2,
	RotateWouldCollide = 3,
	Collision = 4
};

class Mover;

class Board {
	public:

		static const uint8_t BOARD_WIDTH = 10;
		static const uint8_t BOARD_HEIGHT = 22;

		static const uint8_t BOARD_WIDTH_VISIBLE = 10;
		static const uint8_t BOARD_HEIGHT_VISIBLE = 20;

		typedef std::vector<uint8_t> RowsVector;

		Board( sf::RenderWindow &window, float xoffset, float yoffset, unsigned char moversize, float blocksize );
		~Board();

		void clear();
		void draw();

		//void setMover( BlockMatrix &matrix );
		bool setMover( Mover &mover );

		CollisionTest  moveMover(uint8_t x, uint8_t  y );
		unsigned long dropMover();
		CollisionTest rotateMover( bool clockwise = true );

		void applyMover();
		void applyMover( Mover &mover );

		bool testGameOver();
		uint8_t countRows();
		uint8_t countFullRows( RowsVector &rows );
		void destroyRows( RowsVector &rows );
		void blinkRows( RowsVector &rows );

		bool GetCell(uint8_t x, uint8_t y) const { if(x<0 || y <0 || y >= BOARD_HEIGHT || x >= BOARD_WIDTH) return true; else return m_board[x][y].isUsed();}
		Mover & getMover() {return m_mover;}
	private:
		void setBlock( uint8_t x, uint8_t y, bool used, unsigned char gfxid );
		CollisionTest testCollision( Mover &mover, short targetx, short targety );

		/** Variable definitions **/
		Block m_board[BOARD_WIDTH][BOARD_HEIGHT];
		float m_xoffset, m_yoffset;
		uint8_t m_moversize;
		float m_blocksize;
		Mover m_mover;
		bool  m_skipmover;

		sf::RenderWindow &m_window;
};

#endif


