#ifndef MT_TETRIS_H
#define MT_TETRIS_H

#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

class State;
class InGame;

class Tetris {
	public:
		Tetris( bool * start, vector<uint8_t> first_pieces, unsigned long screen_width, unsigned long screen_height, unsigned char screen_bpp = 32, bool fullscreen = false );
		~Tetris();

		int run();
		bool takeScreenshot( std::string filename = "screenshot.jpg" );
		uint8_t RequestMorePieces();
		uint32_t GetPieceOffset();
		void AddNextPiece(uint8_t piece);

		boost::mutex * GetMutex();
		vector<pair<uint32_t,vector<DGMTP2PProtocol::Move>>> * GetRoundData();


	private:
		boost::mutex m_mutex;
		bool * m_start;
		void setup();

		void startGameState();

		void setState( State *state );

		inline bool countFPS();
		vector<uint8_t> m_queuedpieces;
		vector<uint8_t> m_firstpieces;
		State  *m_activestate;
		InGame *m_gamestate;
		unique_ptr<boost::thread> m_thread;
		unsigned long m_screen_width, m_screen_height;
		unsigned char m_screen_bpp;
		bool m_fullscreen;

		sf::Clock m_clock;
		float     m_fps;

		sf::Font  m_font;

		sf::RenderWindow m_window;
};

inline bool Tetris::countFPS() {
	static float lasttick = m_clock.GetElapsedTime();

	if( lasttick <= m_clock.GetElapsedTime() - .1f ) {
		m_fps = 1.f / m_window.GetFrameTime();
		lasttick = m_clock.GetElapsedTime();
		return true;
	}

	return false;
}

#endif

