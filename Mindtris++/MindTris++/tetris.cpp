#include "mindtriscore/includes.h"
#include "mindtriscore/util.h"
#include "mindtriscore/bytearray.h"
#include "mindtriscore/bytebuffer.h"
#include "mindtriscore/socket.h"
#include "mindtriscore/commprotocol.h"
#include "mindtriscore/packet.h"
#include "mindtriscore/messagestreamer.h"
#include "mindtriscore/p2pprotocol.h"


#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include "mindtriscore/tetromino.h"

#include "state.h"
#include "gfxmanager.h"
#include "sfxmanager.h"

#include "signals.h"
#include "block.h"

#include "mover.h"
#include "moverprovider.h"
#include "board.h"
#include "ingame.h"
#include "globals.h"
#include "tetris.h"

Tetris::Tetris( bool * start, vector<uint8_t> first_pieces, unsigned long screen_width, unsigned long screen_height, unsigned char screen_bpp, bool fullscreen ) :
	m_activestate( nullptr ),
	m_gamestate(nullptr),
	m_firstpieces(first_pieces),
	m_screen_width(screen_width),
	m_screen_height(screen_height),
	m_screen_bpp(screen_bpp),
	m_start(start),
	m_fullscreen(fullscreen)
{

    m_thread.reset(new boost::thread(boost::bind(&Tetris::run, this)));


}

uint8_t Tetris::RequestMorePieces(){
	for(vector<uint8_t>::iterator iter = m_queuedpieces.begin(); iter!=m_queuedpieces.end(); iter++){
		m_gamestate->GetMoverProvider().SetNext(*iter);
	}
	if(m_gamestate)
		return m_gamestate->GetMoverProvider().RequestMorePieces();
	else
		return 0;
}

boost::mutex * Tetris::GetMutex(){
	if(m_gamestate)
		return &m_gamestate->GetMutex();
	else
		return nullptr;
}


vector<pair<uint32_t,vector<DGMTP2PProtocol::Move>>> * Tetris::GetRoundData(){
	if(m_gamestate)
		return &m_gamestate->GetRoundData();
	else
		return nullptr;
}

uint32_t Tetris::GetPieceOffset(){
	if(m_gamestate)
		return m_gamestate->GetMoverProvider().GetPieceOffset();
	else
		return 0;
}

void Tetris::AddNextPiece(uint8_t piece){
	if(m_gamestate)
		m_gamestate->GetMoverProvider().SetNext(piece);
	else
		m_queuedpieces.push_back(piece);
}

Tetris::~Tetris() {
	setState( NULL );
}

int Tetris::run() {
	m_window.Create(
			sf::VideoMode( m_screen_width, m_screen_height, m_screen_bpp ),
			"Mindtris++",
			sf::Style::Close | (m_fullscreen ? sf::Style::Fullscreen : 0)
	);
	m_font.LoadFromFile( "media/diego.ttf" );
	sf::Event         event;
	bool              goon = true;
	unsigned long     signal;
	sf::String        fpsstr( L"FPS: ---", m_font, FONTSIZE );
	std::stringstream sstream;

	setup();

	fpsstr.SetColor( sf::Color( 0, 130, 255 ) );

	startGameState();

	m_gamestate->Pause(true);
	while(true)
	{
		boost::mutex::scoped_lock l(m_mutex);
		if(*m_start)
		{
			break;
		}
	}
	m_gamestate->Pause(false);
	while( goon ) {
		while( m_window.GetEvent( event ) ) {
			if( event.Type == sf::Event::Closed ) {
				goon = false;
			}
			else if( event.Type == sf::Event::KeyPressed ) {
				switch( event.Key.Code ) {
					default:
						m_activestate->onKeyPress( event.Key.Code );
						break;
				}
			}
		}

		signal = m_activestate->touch();

		//m_window.Draw( fpsstr );

		m_window.Display();

		if( countFPS() ) {
			sstream.str( "" );
			sstream <<"FPS: "<< static_cast<int>( m_fps );
			fpsstr.SetText( sstream.str() );
		}

		switch( signal ) {
			case Signals::INGAME_QUIT:
				goon = false;
				m_gamestate = 0;
				break;
				
			default:
				break;
		}
	}

	return 0;
}

void Tetris::setup() {
//	m_window.SetBackgroundColor( sf::Color( 0, 0, 0 ) );

	m_window.UseVerticalSync( false );
	m_window.SetFramerateLimit( 60 );

}

void Tetris::setState( State *state ) {
	if( m_activestate ) {
		m_activestate->onLeave();
		delete m_activestate;
		m_activestate = NULL;
	}

	// Every state has to ensure that all resources get loaded
	// properly. When states are changed, resources get freed.
	GfxManager::get().clear();
	SfxManager::get().clear();

	if( state ) {
		m_activestate = state;
		m_activestate->onEnter();
	}
}

bool Tetris::takeScreenshot( std::string filename ) {
	sf::Image screen = m_window.Capture();
	return screen.SaveToFile( filename );
}

void Tetris::startGameState() {
	m_gamestate = new InGame( m_window );

	for(vector<uint8_t>::iterator iter = m_firstpieces.begin(); iter!= m_firstpieces.end(); iter++){
		AddNextPiece(*iter);
	}

	m_gamestate->registerSignal( "onQuit", Signals::INGAME_QUIT );

	setState( m_gamestate );
}

