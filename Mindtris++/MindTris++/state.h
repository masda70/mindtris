#ifndef MT_STATE_H
#define MT_STATE_H

class State {
	public:
		static const unsigned long NOSIGNAL = 0;

		State( sf::RenderWindow &window ):
			m_window( window ) {};

		virtual ~State() {};

		virtual unsigned long touch() = 0;

		virtual void onEnter() = 0;
		virtual void onLeave() = 0;

		virtual void onKeyPress( sf::Key::Code key ) = 0;

		virtual void registerSignal( const std::string &id, unsigned long value );
		virtual void Pause(bool b){};
	protected:
		unsigned long getSignalValue( const std::string &id ) { return m_signals[id]; }

		sf::RenderWindow &m_window;

	private:
		std::map<const std::string,unsigned long> m_signals;
};

#endif

