#include "mindtriscore/includes.h"
#include "mindtriscore/tetromino.h"

#include <SFML/Graphics.hpp>
#include "globals.h"
#include "block.h"
#include "mover.h"
#include "gfxmanager.h"
#include "board.h"



Board::Board( sf::RenderWindow &window, float xoffset, float yoffset, uint8_t moversize, float blocksize ) :
	m_xoffset( xoffset ),
	m_yoffset( yoffset ),
	m_moversize( moversize ),
	m_blocksize( blocksize ),
	m_mover( moversize, blocksize, window ),
	m_skipmover( false ),
	m_window( window )
{
	clear();
}

Board::~Board() {
}

void Board::clear() {
	uint8_t x, y;

	for( y = 0; y < BOARD_HEIGHT; ++y ) {
		for( x = 0; x < BOARD_WIDTH; ++x ) {
			m_board[x][y].setUsed( false );
		}
	}
}

void Board::draw() {
	static uint8_t x, y;

	for( y = 0; y < BOARD_HEIGHT_VISIBLE; ++y ) {
		for( x = 0; x < BOARD_WIDTH_VISIBLE; ++x ) {
			if( m_board[x][y].isUsed() ) {
				m_window.Draw( m_board[x][y].getSprite() );
			}
		}
	}

	if( !m_skipmover ) {
		m_mover.draw();
	}
}

bool Board::setMover( Mover &mover ) {
	m_skipmover = false;

	m_mover = mover;
	m_mover.setScale( 1.f );
	m_mover.setBlocksize( m_blocksize );

	m_mover.setOffset( m_xoffset, m_yoffset + m_blocksize*(BOARD_HEIGHT_VISIBLE-1) );
	m_mover.setPosition( (BOARD_WIDTH/2) -(m_mover.getLastX()- m_mover.getFirstX()+2)/2, BOARD_HEIGHT_VISIBLE);

	if( testCollision( m_mover, m_mover.getX(), m_mover.getY() ) != NoCollision ) {
		m_mover.clear();
		return false;
	}

	return true;
}

CollisionTest Board::moveMover(uint8_t  x, uint8_t  y ) {
	uint8_t        tx, ty;
	CollisionTest result;

	tx = m_mover.getX() + x;
	ty = m_mover.getY() + y;
	result = testCollision( m_mover, tx, ty );

	// Only move the mover if either no collision will occure or the
	// bottom will be reached.
	if( result == NoCollision || result == BottomReached ) {
		m_mover.setPosition( static_cast<uint8_t>( tx ), static_cast<uint8_t>( ty ) );
	}

	return result;
}

CollisionTest Board::testCollision( Mover &mover, short targetx, short targety ) {
	uint8_t mx, my;
	short tx, ty;

	// Test if mover would be out of bounds.
	tx = targetx + mover.getLastX()-mover.getFirstX();
	ty = targety + mover.getLastY()-mover.getFirstY();

	if( targetx >= BOARD_WIDTH || targety >= BOARD_HEIGHT ||
			targetx < 0 || targety < 0 ||
			tx >= BOARD_WIDTH  || ty >= BOARD_HEIGHT ) {
		return OutOfBounds;
	}

	// Test each block.
	for( my = 0; my <= mover.getLastY()-mover.getFirstY(); ++my ) {
		ty = targety + my ;

		for( mx = 0; mx <=  mover.getLastX()-mover.getFirstX(); ++mx ) {
			tx = targetx + mx;

			if( mover.isUsed( mover.getFirstX()+mx, mover.getFirstY()+my ) && m_board[tx][ty].isUsed() ) {
				return Collision;
			}
		}
	}

	// Test if bottom reached. 
	if( targety == -1 ) {
		return BottomReached;
	}

	return NoCollision;
}

CollisionTest Board::rotateMover( bool clockwise ) {
	Mover         rotated( m_mover );
	CollisionTest result;

	rotated.rotate(*this, clockwise);

	result = testCollision( rotated, rotated.getX(), rotated.getY() );

	if( result != NoCollision && result != BottomReached ) {
		return RotateWouldCollide;
	}

	m_mover = rotated;
	return result;
}

void Board::applyMover( Mover &mover ) {
	uint8_t x, y;

	for( y = 0; y <= mover.getLastY()-mover.getFirstY(); ++y ) {
		for( x = 0; x <= mover.getLastX()-mover.getFirstX(); ++x ) {
			if( mover.isUsed( x + mover.getFirstX(), y + mover.getFirstY() ) ) {
				m_board[mover.getX() + x][mover.getY() + y] = mover.getBlock( mover.getFirstX()+x, mover.getFirstY()+y );
				m_board[mover.getX() + x][mover.getY() + y].setUsed( true );
			}
		}
	}

	m_skipmover = true;
}

void Board::applyMover() {
	applyMover( m_mover );
}

uint8_t Board::countRows() {
	uint8_t row, col;

	for( row = BOARD_HEIGHT-1; row >=0 ; --row ) {
		for( col = 0; col < BOARD_WIDTH; ++col ) {
			if( m_board[col][row].isUsed() ) {
				return row+1;
			}
		}
	}

	return 0;
}

uint8_t Board::countFullRows( RowsVector &rows ) {
	uint8_t x, y, full;

	full = 0;

	for( y = 0; y < BOARD_HEIGHT; ++y ) {
		for( x = 0; x < BOARD_WIDTH; ++x ) {
			// If we find a row which is NOT full, we can stop counting and
			// return the previously counted full rows amount.
			if( !m_board[x][y].isUsed() ) {
				break;
			}
		}

		if( x == BOARD_WIDTH ) {
			rows.push_back( y );
			++full;
		}
	}

	return full;
}

void Board::destroyRows( RowsVector &rows ) {
	uint8_t                row, destroyed;
	uint8_t                y, x;
	RowsVector::iterator iter;

	destroyed = 0;

	for( y = 0; y < BOARD_HEIGHT; ++y ) {

		row = y;

		iter = find( rows.begin(), rows.end(), row );
		if( iter != rows.end() ) {
			++destroyed;
			continue;
		}

		for( x = 0; x < BOARD_WIDTH; ++x ) {
			if( BOARD_HEIGHT-1 - row < rows.size() ) {
				m_board[x][row].setUsed( false );
			}

			if( destroyed ) {
				m_board[x][row-destroyed] = m_board[x][row];
				m_board[x][row-destroyed].setPosition(
						m_xoffset + x * m_blocksize,
						m_yoffset + m_blocksize*(BOARD_HEIGHT_VISIBLE-1)-(row-destroyed) * m_blocksize
				);
			}

		}
	}
}

void Board::blinkRows( RowsVector &rows ) {
	uint8_t               x;
	RowsVector::iterator iter;
	sf::Color            color;

	color = m_board[0][rows[0]].getSprite().GetColor();
	if( color.a != 255 ) {
		color.a = 255;
	}
	else {
		color.a = 100;
	}

	for( iter = rows.begin(); iter != rows.end(); ++iter ) {
		for( x = 0; x < BOARD_WIDTH; ++x ) {
			m_board[x][*iter].setColor( color );
		}
	}
}

unsigned long Board::dropMover() {
	Mover dropper( m_mover );
	unsigned long deep( 0 );

	while( testCollision( dropper, dropper.getX(), dropper.getY() - 1 ) == NoCollision ) {
		dropper.setPosition( dropper.getX(), dropper.getY() - 1 );
		++deep;
	}

	m_mover = dropper;
	return deep;
}
