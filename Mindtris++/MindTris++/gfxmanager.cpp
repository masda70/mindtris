#include <SFML/Graphics.hpp>
#include "mindtriscore/includes.h"
#include "gfxmanager.h"

GfxManager::~GfxManager() {
	clear();
}

sf::Image &GfxManager::loadImage( const std::string filename ) {
	std::map<const std::string,sf::Image*>::iterator iter;
	sf::Image *newimage;

	iter = m_images.find( filename );
	if( iter != m_images.end() ) {
		return *iter->second;
	}

	newimage = new sf::Image;
	newimage->LoadFromFile( filename );
	m_images[filename] = newimage;

	return *newimage;
}

bool GfxManager::releaseImage( const std::string filename ) {
	std::map<const std::string,sf::Image*>::iterator iter;

	iter = m_images.find( filename );
	if( iter == m_images.end() ) {
		return false;
	}

	delete iter->second;
	m_images.erase( iter );

	return true;
}

void GfxManager::clear() {
	std::map<const std::string,sf::Image*>::iterator iter;

	for( iter = m_images.begin(); iter != m_images.end(); ++iter ) {
		delete iter->second;
	}

	m_images.clear();
}
