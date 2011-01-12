#ifndef MT_SFXMANAGER_H
#define MT_SFXMANAGER_H

#include <SFML/Audio.hpp>
#include <map>

class SfxManager {
	public:
		~SfxManager();

		static SfxManager &get() {
			static SfxManager mgr;
			return mgr;
		}

		sf::SoundBuffer &loadBuffer( const std::string filename );
		bool releaseBuffer( const std::string filename );
		void clear();

	private:
		SfxManager() {};
		SfxManager( SfxManager& mgr ) { *this = mgr; }
		SfxManager& operator =( SfxManager& mgr ) { return mgr; }

		std::map<const std::string,sf::SoundBuffer*> m_buffers;
};

#endif
