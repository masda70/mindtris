#include "mindtriscore/includes.h"
#include "mindtriscore/tetromino.h"
#include <SFML/Graphics.hpp>
#include "gfxmanager.h"
#include "block.h"

#include "mover.h"
#include "board.h"



Mover::Mover( uint8_t size, float blocksize, sf::RenderWindow &wnd ) :
	m_blocks( 0 ),
	m_size( size ),
	m_totalsize( size * size ),
	m_blocksize( blocksize ),
	m_xoffset( 0.f ),
	m_yoffset( 0.f ),
	m_posx( 0 ), m_posy( 0 ),
	m_lastx( 0 ), m_lasty( 0 ),
	m_scale( 1.f ),
	m_window( wnd )
{
	m_blocks = new Block[size*size];
}

Mover::~Mover() {
	delete [] m_blocks;
}

Mover::Mover( Mover& mover ) :
	m_blocks( 0 ),
	m_window( mover.m_window )
{
	*this = mover;
}

Mover &Mover::operator=( Mover &mover ) {
	if( m_blocks ) {
		delete [] m_blocks;
	}

	m_size = mover.m_size;

	m_blocks = new Block[mover.m_size * mover.m_size];

	uint8_t x, y;
	for( y = 0; y < m_size; ++y ) {
		for( x = 0; x < m_size; ++x ) {
			m_blocks[y*m_size+x] = mover.m_blocks[y*m_size+x];
		}
	}

	m_totalsize = mover.m_totalsize;
	m_blocksize = mover.m_blocksize;
	m_xoffset = mover.m_xoffset;
	m_yoffset = mover.m_yoffset;
	m_firstx = mover.m_firstx;
	m_firsty = mover.m_firsty;
	m_lastx = mover.m_lastx;
	m_lasty = mover.m_lasty;

	m_tetromino_id = mover.m_tetromino_id;
	m_tetromino_orientation = mover.m_tetromino_orientation;
	m_tetromino_xoffset = mover.m_tetromino_xoffset;
	m_tetromino_yoffset = mover.m_tetromino_yoffset;

	setPosition( mover.m_posx, mover.m_posy );

	return *this;
}

void Mover::rotate(const Board & board, bool clockwise ) {

	Mover          rotated( *this );

	const Tetromino * tetromino  = Tetromino::GetTetrominoByID(m_tetromino_id);

	Tetromino::State new_orientation;

	switch(m_tetromino_orientation)
	{
		case Tetromino::STATE_0:
		{
			new_orientation = clockwise?Tetromino::STATE_R:Tetromino::STATE_L;
			break;
		}
		case Tetromino::STATE_R:
		{
			new_orientation = clockwise?Tetromino::STATE_2:Tetromino::STATE_0;
			break;
		}
		case Tetromino::STATE_2:
		{
			new_orientation = clockwise?Tetromino::STATE_L:Tetromino::STATE_R;
			break;
		}
		case Tetromino::STATE_L:
		{
			new_orientation = clockwise?Tetromino::STATE_0:Tetromino::STATE_2;
			break;
		}
	}

	for(unsigned long i = 0; i<tetromino->GetOffsetNumber(); i++)
	{
		uint8_t kick_x = tetromino->GetXOffset(m_tetromino_orientation,i) - tetromino->GetXOffset(new_orientation,i);
		uint8_t kick_y = tetromino->GetYOffset(m_tetromino_orientation,i) - tetromino->GetYOffset(new_orientation,i);

		bool valid = true;

		for(int x = 0; x < tetromino->GetSize();x++)
		{
			for(int y = 0; y < tetromino->GetSize();y++)
			{
				if(tetromino->GetPieceXY(new_orientation,x,y) && board.GetCell(m_posx-m_firstx+x+kick_x,m_posy-m_firsty+y+kick_y)){
					valid = false;
					y = tetromino->GetSize(); x = tetromino->GetSize();
				}
			}
		}
		if(valid){
			m_posx = m_posx - m_firstx + kick_x;
			m_posy = m_posy - m_firsty + kick_y;
			m_tetromino_orientation = new_orientation;
			for(int x = 0; x < tetromino->GetSize();x++)
			{
				for(int y = 0; y < tetromino->GetSize();y++)
				{
					m_blocks[y*m_size+x].setUsed( tetromino->GetPieceXY(m_tetromino_orientation,x,y));
				}
			}
			break;

		}
	}


	updateFirstLastPositions();
	m_posx = m_posx + m_firstx;
	m_posy = m_posy + m_firsty;

	updateSprites();
}

void Mover::setTetromino( const Tetromino & tetromino  ) {
	m_tetromino_id = tetromino.GetID();

	const sf::Image &image = GfxManager::get().loadImage( "media/"+  tetromino.GetImage());
	m_tetromino_orientation = Tetromino::STATE_0;
	uint8_t x, y;

	for( y = 0; y < tetromino.GetSize(); ++y ) {
		for( x = 0; x < tetromino.GetSize(); ++x ) {
			m_blocks[y*m_size+x].setUsed( tetromino.GetPieceXY(m_tetromino_orientation ,x,y) );
			m_blocks[y*m_size+x].setImage(
					image,
					m_xoffset + x * m_blocksize,
					m_yoffset - y * m_blocksize
			);
		}
	}

	updateFirstLastPositions();
	updateSprites();
}

void Mover::setOffset( float x, float y ) {
	m_xoffset = x;
	m_yoffset = y;
	updateSprites();
}

void Mover::setPosition( uint8_t x, uint8_t y ) {
	m_posx = x;
	m_posy = y;
	updateSprites();
}

void Mover::updateSprites() {
	uint8_t bx, by;

	for( by = 0; by < m_size; ++by ) {
		for( bx = 0; bx < m_size; ++bx ) {

			m_blocks[by*m_size+bx].setPosition(
					m_xoffset + (bx+m_posx-m_firstx) * m_blocksize,
					m_yoffset - (by+m_posy-m_firsty) * m_blocksize
			);
			m_blocks[by*m_size+bx].setScale( m_scale );
		}
	}
}

void Mover::updateFirstLastPositions() {
	uint8_t x, y;

	m_firstx = 0; m_firsty = 0;

	for( y = 0; y < m_size; ++y ) {
		for( x = 0; x < m_size; ++x ) {
			if( m_blocks[y*m_size+x].isUsed() ) {
				break;
			}
		}

		if( x != m_size ) {
			break;
		}

		++m_firsty;
	}

	for( x = 0; x < m_size; ++x ) {
		for( y = 0; y < m_size; ++y ) {
			if( m_blocks[y*m_size+x].isUsed() ) {
				break;
			}
		}

		if( y != m_size ) {
			break;
		}

		++m_firstx;
	}


	m_lastx = 0;
	m_lasty = 0;

	for( y = 0; y < m_size; ++y ) {
		for( x = 0; x < m_size; ++x ) {

			if( m_blocks[y*m_size+x].isUsed() ) {
				if( x > m_lastx ) {
					m_lastx = x;
				}
				if( y > m_lasty ) {
					m_lasty = y;
				}
			}

		}
	}
}

void Mover::clear() {
	uint8_t blockid;

	for( blockid = 0; blockid < (m_size*m_size); ++blockid ) {
		m_blocks[blockid].setUsed( false );
	}
}


void Mover::draw() {

	for( uint8_t by = 0; by <=m_lasty-m_firsty; ++by ) {
		if(by+m_posy >= Board::BOARD_HEIGHT_VISIBLE )
			return;
		for( uint8_t bx =0; bx <= m_lastx- m_firstx; ++bx ) {

			if(m_blocks[(by+m_firsty)*m_size+bx+m_firstx].isUsed())
			{
				m_window.Draw( m_blocks[(by+m_firsty)*m_size+bx+m_firstx].getSprite() );
			}
				
		}
	}
}