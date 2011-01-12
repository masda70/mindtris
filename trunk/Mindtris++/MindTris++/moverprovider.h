#ifndef MT_MOVERPROVIDER_H
#define MT_MOVERPROVIDER_H
#include <boost/thread/mutex.hpp>

class MoverProvider {
	public:
		MoverProvider() 
		{
			m_pieceoffset = 0;
		}
		~MoverProvider();
		uint32_t GetPieceOffset() { return m_pieceoffset;}
		uint8_t RequestMorePieces();
		void SetNext(uint8_t id);
		const Tetromino * GetNext();

	private:
		uint32_t m_pieceoffset;
		boost::mutex m_mutex;
		std::list<uint8_t> m_newpieces;
};

#endif

