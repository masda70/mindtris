#ifndef MT_INGAME_H
#define MT_INGAME_H
#include <boost/thread/mutex.hpp>
class InGame : public State {
	public:
		InGame( sf::RenderWindow &window ) :
			State( window ),
			m_board( window, 25, 25, 4, 25.f ),
			m_next( 4, 15.f, window ),
			m_blinkleft( 0 )
		{

		};

		virtual ~InGame();

		unsigned long touch();

		void onEnter();
		void onLeave();
		void onKeyPress( sf::Key::Code key );

		unsigned long getScore() { return m_score; }
		MoverProvider & GetMoverProvider() { return m_moverprovider;}

		boost::mutex & GetMutex() { return m_mutex;}
		vector<pair<uint32_t,vector<DGMTP2PProtocol::Move>>> & GetRoundData() { return m_round_data;}
		void Pause(bool b);
	private:
		enum GameState {
			Prepare = 0,
			Running = 1,
			Paused = 2
		};

		static const unsigned long STR_LEVEL = 0;
		static const unsigned long STR_ROWS = 1;
		static const unsigned long STR_SCORE = 2;
		static const unsigned long STR_TIMELEFT = 3;
		static const unsigned long STR_PLAYTIME = 4;
		static const unsigned long STR_LAST = 5;

		void initLevel( unsigned char level );
		void updateTexts();
		void addScore( unsigned long value );
		bool createNext();

		boost::mutex m_mutex;
		uint32_t m_round_number;
		vector<DGMTP2PProtocol::Move> m_round_moves;
		vector<pair<uint32_t,vector<DGMTP2PProtocol::Move>>> m_round_data;


		uint32_t m_piecenumber;

		unsigned long m_returnval;
		bool  m_running;
		bool  m_paused;

		float m_phase;

		unsigned char m_level;
		unsigned long m_score;
		unsigned char m_rows;
		float m_round_time;
		float m_starttime;
		float m_finishtime;
		float m_pausetime;

		float m_movementdelay;
		float m_nextmovement;
		float m_nextupdate;

		bool m_waitnext;

		Board m_board;
		Board::RowsVector m_rowsvector;

		MoverProvider m_moverprovider;
		Mover         m_next;

		unsigned char m_blinkleft;

		sf::Clock m_clock;
		std::vector<sf::String> m_alignedstrings;

		sf::Sprite m_background;
		sf::Sprite m_gameover;

		sf::String m_pressesc;

		sf::Music m_gamemusic;
		sf::Sound m_alarmsound;
		sf::Sound m_notpossible;
		sf::Sound m_rotatesound;
		sf::Sound m_destroysound;
		sf::Sound m_woodensound;
};

#endif
