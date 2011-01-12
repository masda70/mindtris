#ifndef MT_BLOCK_H
#define MT_BLOCK_H

#include <SFML/Graphics.hpp>

class Block {
	public:
		Block() :
			m_used( false )
		{
		}

		Block( bool used, const sf::Image &image, float x, float y ) :
			m_used( used ),
			m_sprite( image )
		{

			m_sprite.SetPosition( x, y );
		}

		void setUsed( bool used ) { m_used = used; }
		void setImage( const sf::Image &image, float x, float y ) {
			//m_used = true;
			m_sprite.SetImage( image );
			m_sprite.SetPosition( x, y );
		}

		void setScale( float scale ) { m_sprite.SetScale( scale, scale ); }
		void setColor( sf::Color color ) { m_sprite.SetColor( color ); }

		void setPosition( float x, float y ) {
			m_sprite.SetPosition( x, y );
		}

		bool isUsed() const { return m_used; }
		const sf::Sprite &getSprite() { return m_sprite; }

	private:
		bool       m_used;
		sf::Sprite m_sprite;
};

#endif

