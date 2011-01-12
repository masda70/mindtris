#include "mindtriscore/includes.h"
#include "mindtriscore/util.h"
#include "mindtriscore/bytearray.h"
#include "mindtriscore/bytebuffer.h"
#include "mindtriscore/socket.h"
#include "mindtriscore/commprotocol.h"
#include "mindtriscore/packet.h"
#include "mindtriscore/messagestreamer.h"
#include "mindtriscore/serverprotocol.h"
#include "mindtriscore/p2pprotocol.h"
#include "mindtriscore/tetromino.h"

#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include <math.h>
#include "gfxmanager.h"
#include "sfxmanager.h"
#include "state.h"
#include "globals.h"
#include "block.h"
#include "mover.h"
#include "moverprovider.h"
#include "board.h"
#include "ingame.h"



const float UPDATE_DELAY = .2f;
const float LEVEL_WARN_TIME = 10.f;
const float LEVEL_CLOCK_TIME = 5.f;
const float BASE_SPEED = 1.f;
const float MIN_SPEED = .1f;
const float SPEED_DEDUCTION = .084f;
const float BASE_TIME = 18.f;
const float TIME_PER_LEVEL = 2.f;
const float MAX_DELAY = 1.f;
const float DROP_DELAY = .2f;
const float GAMEMUSIC_VOLUME = 70.f;
const float ROUND_TIME = .1f;

InGame::~InGame() {
}




unsigned long InGame::touch() {
	static std::vector<sf::String>::iterator iter;
	static float reftime;
	static sf::Color color;

	reftime = m_clock.GetElapsedTime(); // Reference time for operations.
	m_phase += .25f;
	if( m_phase >= 360.f ) {
		m_phase -= 360.f;
	}

	if( m_running ) {
		// Check if level is completed

		if( m_round_time <= reftime){

			m_round_time = m_round_time + ROUND_TIME;

			boost::mutex::scoped_lock l(m_mutex);
			pair<uint32_t,vector<DGMTP2PProtocol::Move>> p = pair<uint32_t,vector<DGMTP2PProtocol::Move>>(m_round_number,m_round_moves);
			m_round_data.push_back(p);
			m_round_moves.clear();

			m_round_number++;
		}

		// Move moverblock!
		if( m_nextmovement <= reftime && !m_blinkleft ) {
			CollisionTest result = m_board.moveMover( 0, -1 );
			m_nextmovement = reftime + m_movementdelay;

			if( result == OutOfBounds || result == Collision ) {
				m_board.applyMover();
				m_round_moves.push_back(DGMTP2PProtocol::Move(m_piecenumber,m_board.getMover().getOrientation(),m_board.getMover().getX(),m_board.getMover().getY()));

				m_piecenumber ++;

				unsigned char fullrows = m_board.countFullRows( m_rowsvector );

				if( fullrows ) {
					m_blinkleft = 4;
				}
				else {
					// Check if new mover collides, i.e. game is over.
					if( !m_board.setMover( m_next ) ) {
						m_running = false;
						m_gamemusic.Stop();
					}
					else {
						createNext();
					}

					m_woodensound.Play();
				}

				m_rows = m_board.countRows();

			}

		}

		// Do an update regularly.
		if( m_nextupdate <= reftime ) {
			updateTexts();
			m_nextupdate = m_clock.GetElapsedTime() + UPDATE_DELAY;

			if( m_blinkleft && m_running ) {
				m_board.blinkRows( m_rowsvector );
				--m_blinkleft;

				if( !m_blinkleft ) {
					m_board.destroyRows( m_rowsvector );
					m_destroysound.Play();

					m_rows = m_board.countRows();
					m_nextmovement = reftime + m_movementdelay;

					// Check if new mover collides, i.e. game is over.
					if( !m_board.setMover( m_next ) ) {
						m_running = false;
					}
					else {
						createNext();
					}

					m_rowsvector.clear();
				}
			}
		}
	}


	//////////////// RENDERING ////////////////////
	m_window.Draw( m_background );

	for( iter = m_alignedstrings.begin(); iter != m_alignedstrings.end(); ++iter ) {
		m_window.Draw( *iter );
	}

	m_board.draw();
	m_next.draw();

	if( !m_running ) {
		if( !m_paused ) {
			m_window.Draw( m_gameover );
			m_window.Draw( m_pressesc );
		}
	}

	return m_returnval;
}

void InGame::onEnter() {
	// Load resources.
	m_background.SetImage( GfxManager::get().loadImage( "media/pingu_gamebg.png" ) );

	m_gameover.SetImage( GfxManager::get().loadImage( "media/gameover.png" ) );

	sf::Vector2f size = m_gameover.GetSize();
	m_gameover.SetPosition(
			m_window.GetWidth() / 2 - size.x / 2,
			m_window.GetHeight() / 2 - size.y / 2
	);

	m_gamemusic.OpenFromFile( "media/datahell_beta.ogg" );
	m_gamemusic.SetVolume( GAMEMUSIC_VOLUME );
	m_gamemusic.SetLoop( true );

	m_alarmsound.SetBuffer( SfxManager::get().loadBuffer( "media/alarm.wav" ) );
	m_notpossible.SetBuffer( SfxManager::get().loadBuffer( "media/notpossible.wav" ) );
	m_rotatesound.SetBuffer( SfxManager::get().loadBuffer( "media/buttonsel.wav" ) );
	m_destroysound.SetBuffer( SfxManager::get().loadBuffer( "media/destroy.wav" ) );
	m_woodensound.SetBuffer( SfxManager::get().loadBuffer( "media/wooden.wav" ) );

	m_alignedstrings.resize( STR_LAST );

	// Setup scene.
	m_alignedstrings[STR_ROWS].SetPosition( 324, 390 );
	m_alignedstrings[STR_ROWS].SetColor( sf::Color( 0, 0, 0 ) );
	m_alignedstrings[STR_SCORE].SetPosition( 324, 253 );
	m_alignedstrings[STR_SCORE].SetColor( sf::Color( 255, 255, 255 ) );
	m_alignedstrings[STR_SCORE].SetRotation( -5.f );

	m_pressesc.SetText( L"Press ESC to exit!" );
	m_pressesc.SetColor( sf::Color( 255, 255, 255 ) );
	m_pressesc.SetSize( 20 );
	sf::FloatRect rect = m_pressesc.GetRect();
	m_pressesc.SetCenter( rect.GetWidth() / 2, rect.GetHeight() / 2 );
	m_pressesc.SetPosition( m_window.GetWidth() / 2, m_window.GetHeight() - 20.f );


	m_round_number = 0;
	m_piecenumber = 0;

	m_phase = 0.f;
	m_starttime = m_clock.GetElapsedTime();
	m_round_time = m_starttime+ ROUND_TIME;
	m_finishtime = 0.f;
	m_nextupdate = 0.f;
	m_nextmovement = 0.f;
	m_returnval = State::NOSIGNAL;
	m_running = true;
	m_paused = false;
	m_rows = 0;
	m_score = 0;

	initLevel( 0 );

	// Setup "next" mover

	Tetromino::Initialize();

	m_next.setScale( .6f );
	createNext();
	m_board.setMover( m_next );
	createNext();

	updateTexts();
	m_gamemusic.Play();
}

void InGame::onLeave() {
	m_gamemusic.Stop();
}

void InGame::onKeyPress( sf::Key::Code key ) {
	if( m_paused && key != sf::Key::P && key != sf::Key::Escape ) {
		return;
	}

	switch( key ) {
		case sf::Key::Escape:
			m_returnval = getSignalValue( "onQuit" );
			break;

		case sf::Key::W:
			if( m_board.rotateMover( true ) != NoCollision ) {
				m_notpossible.Play();
			}
			else {
				m_rotatesound.Play();
			}
			break;

		case sf::Key::Q: 
			if( m_board.rotateMover( false ) != NoCollision ) {
				m_notpossible.Play();
			}
			else {
				m_rotatesound.Play();
			}
			break;

		case sf::Key::Down:
			if( m_board.moveMover( 0, -1) == NoCollision ) {
				m_nextmovement = m_clock.GetElapsedTime() + m_movementdelay;
			}
			break;

		case sf::Key::Left:
			m_board.moveMover( -1, 0 );
			break;

		case sf::Key::Right:
			m_board.moveMover( 1, 0 );
			break;

		case sf::Key::Space:
			if( m_board.dropMover() ) {
				m_nextmovement = m_clock.GetElapsedTime() + DROP_DELAY;
			}
			else {
				m_nextmovement = 0;
			}

			break;
		case sf::Key::M: // Toggle music.
			if( m_gamemusic.GetStatus() == sf::Sound::Playing ) {
				m_gamemusic.Stop();
			}
			else {
				m_gamemusic.Play();
			}

			break;

		default:
			break;
	}
}


void InGame::Pause(bool b){
	if( b ) {
		m_running = false;
		m_paused = true;
		m_pausetime = m_clock.GetElapsedTime();
		m_gamemusic.SetVolume( 0.f );
	}
	else if( !m_running && m_paused ) {
		float diff( m_clock.GetElapsedTime() - m_pausetime );
					
		m_round_time += diff;

		m_starttime += diff;
		m_finishtime += diff;

		m_running = true;
		m_paused = false;
		m_gamemusic.SetVolume( GAMEMUSIC_VOLUME );
	}
}

void InGame::updateTexts() {
	std::stringstream sstream;
	sf::FloatRect     rect;

	// Rows.
	sstream.str( "" );
	sstream << std::setfill( '0' ) << std::setw( 2 ) << static_cast<unsigned short>( m_rows );
	m_alignedstrings[STR_ROWS].SetText( sstream.str() );

	// Score.
	sstream.str( "" );
	sstream << std::setfill( '0' ) << std::setw( 6 ) << m_score;
	m_alignedstrings[STR_SCORE].SetText( sstream.str() );
}

void InGame::initLevel( unsigned char level ) {
	m_level = level;
	m_finishtime = m_starttime + level * (BASE_TIME + level * TIME_PER_LEVEL );

	// Recalculate speed based on level.
	m_movementdelay = BASE_SPEED - level * SPEED_DEDUCTION;
	if( m_movementdelay < MIN_SPEED ) {
		m_movementdelay = MIN_SPEED;
	}
}

void InGame::addScore( unsigned long value ) {
	m_score += value;
	updateTexts();
}

bool InGame::createNext() {
	float wxy;
	float ox, oy;

	const Tetromino * tetromino = m_moverprovider.GetNext();
	if(tetromino)
	{
		m_next.clear();
		m_next.setTetromino(*tetromino);


		wxy = (m_next.getSize() * m_next.getBlocksize()) / 2.f;
		ox = wxy - (((m_next.getLastX() + 1) * m_next.getBlocksize()) / 2.f);
		oy = wxy - (((m_next.getLastY() + 1) * m_next.getBlocksize()) / 2.f);

		m_next.setOffset( 344 + ox, 74 + oy );
		return true;
	}else{
		return false;
	}
}
