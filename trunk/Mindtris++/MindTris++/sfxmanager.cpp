#include "mindtriscore/includes.h"

#include "sfxmanager.h"

SfxManager::~SfxManager() {
	clear();
}

sf::SoundBuffer &SfxManager::loadBuffer( const std::string filename ) {
	std::map<const std::string,sf::SoundBuffer*>::iterator iter;
	sf::SoundBuffer *newbuffer;

	iter = m_buffers.find( filename );
	if( iter != m_buffers.end() ) {
		return *iter->second;
	}

	newbuffer = new sf::SoundBuffer;
	newbuffer->LoadFromFile( filename );
	m_buffers[filename] = newbuffer;

	return *newbuffer;
}

bool SfxManager::releaseBuffer( const std::string filename ) {
	std::map<const std::string,sf::SoundBuffer*>::iterator iter;

	iter = m_buffers.find( filename );
	if( iter == m_buffers.end() ) {
		return false;
	}

	delete iter->second;
	m_buffers.erase( iter );

	return true;
}

void SfxManager::clear() {
	std::map<const std::string,sf::SoundBuffer*>::iterator iter;

	for( iter = m_buffers.begin(); iter != m_buffers.end(); ++iter ) {
		delete iter->second;
	}

	m_buffers.clear();
}
