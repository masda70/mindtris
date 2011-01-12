#ifndef MT_MOVER_H
#define MT_MOVER_H

class Tetromino;
class Block;
class Board;

class Mover {
	public:
		Mover( uint8_t size, float blocksize, sf::RenderWindow &wnd );
		~Mover();

		Mover( Mover& mover );
		Mover &operator=( Mover &mover );

		void clear();

		void setPosition( uint8_t x, uint8_t y );
		uint8_t getX() { return m_posx; }
		uint8_t getY() { return m_posy; }
		uint8_t getLastX() { return m_lastx; }
		uint8_t getLastY() { return m_lasty; }
		uint8_t getFirstX() { return m_firstx; }
		uint8_t getFirstY() { return m_firsty; }
		Tetromino::State getOrientation() { return m_tetromino_orientation;}
		uint8_t getTetrominoID() { return m_tetromino_id;}


		bool isUsed( uint8_t x,uint8_t y ) { return m_blocks[y*m_size+x].isUsed(); }

		void setOffset( float x, float y );
		void setScale( float scale ) { m_scale = scale; updateSprites(); }
		void setBlocksize( float size ) { m_blocksize = size; }

		void setTetromino(const Tetromino & tetromino );

		uint8_t getSize() { return m_size; }
		float getBlocksize() { return m_blocksize; }

		const Block &getBlock( unsigned char x, unsigned char y ) { return m_blocks[y*m_size+x]; }

		void rotate( const Board & board, bool clockwise = true);
		void draw();

	private:
		void updateFirstLastPositions();
		void updateSprites();

		Block  *m_blocks;
		uint8_t  m_size;
		uint8_t m_totalsize;
		
		uint8_t m_tetromino_id;
		uint8_t m_tetromino_xoffset, m_tetromino_yoffset;
		Tetromino::State m_tetromino_orientation;

		float  m_blocksize, m_xoffset, m_yoffset;
		uint8_t  m_posx, m_posy, m_lastx, m_lasty, m_firstx, m_firsty;
		float  m_scale;

		sf::RenderWindow &m_window;
};



#endif

