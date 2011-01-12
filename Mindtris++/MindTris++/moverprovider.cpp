#include "mindtriscore/includes.h"

#include <boost/thread/mutex.hpp>

#include <SFML/Graphics.hpp>
#include "mindtriscore/tetromino.h"
#include "block.h"
#include "mover.h"
#include "moverprovider.h"


MoverProvider::~MoverProvider() {

}

uint8_t MoverProvider::RequestMorePieces(){
	if(m_newpieces.size()<14)
	{
		return 14;
	}
	return 0;
}

void MoverProvider::SetNext(uint8_t id)
{
    boost::mutex::scoped_lock l(m_mutex);
	m_pieceoffset++;
	m_newpieces.push_back(id);
}

const Tetromino * MoverProvider::GetNext() {

    boost::mutex::scoped_lock l(m_mutex);
	if(m_newpieces.empty()) 
	{
		return nullptr;
	}

	const Tetromino * tetromino = Tetromino::GetTetrominoByID(m_newpieces.front());
	m_newpieces.pop_front();
	return tetromino;

}

