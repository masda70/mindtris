#ifndef MT_GFXMANAGER_H
#define MT_GFXMANAGER_H

class GfxManager {
	public:
		~GfxManager();

		static GfxManager &get() {
			static GfxManager mgr;
			return mgr;
		}

		sf::Image &loadImage( const std::string filename );
		bool releaseImage( const std::string filename );
		void clear();

	private:
		GfxManager() {};
		GfxManager( GfxManager& mgr ) { *this = mgr; }
		GfxManager& operator =( GfxManager& mgr ) { return mgr; }

		std::map<const std::string,sf::Image*> m_images;
};

#endif

