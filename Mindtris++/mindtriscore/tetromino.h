#ifndef MT_TETROMINO_H
#define MT_TETROMINO_H

class Board;

class Tetromino
{

public:

	static Tetromino * TYPE_J;
	static Tetromino * TYPE_L;
	static Tetromino * TYPE_T;
	static Tetromino * TYPE_S;
	static Tetromino * TYPE_Z;
	static Tetromino * TYPE_I;
	static Tetromino * TYPE_O;

	static const unsigned int STATE_NUMBER = 4;

	enum TYPE_ID{
		ID_I = 0,
		ID_J = 1,
		ID_L = 2,
		ID_O = 3,
		ID_S = 4,
		ID_T = 5,
		ID_Z = 6,
	};

	enum State{
		STATE_0 = 0,
		STATE_R = 1,
		STATE_2 = 2,
		STATE_L = 3
	};

private:

	uint8_t m_id;
	string m_blockimage;

	string m_piecename;

	uint8_t m_piecesize;
	unsigned long m_offsets_number;

	unique_ptr<bool[]> m_piecemaps;
	unique_ptr<uint8_t[]> m_offsetmap;


	void SetupPiece(const State & s,const uint8_t map[], const uint8_t offsets[][2]);

public:
	unsigned long GetOffsetNumber() const {return m_offsets_number;}

	uint8_t GetXOffset(Tetromino::State state, unsigned long offset_test) const;
	uint8_t GetYOffset(Tetromino::State state, unsigned long offset_test) const;
	bool GetXY(Tetromino::State state, uint8_t x, uint8_t y) const;

	static const Tetromino * GetTetrominoByID(uint8_t id);

	uint8_t GetSize() const { return m_piecesize;}
	string GetName() const { return m_piecename;}
	uint8_t GetID() const {return m_id;}
	string GetImage() const {return m_blockimage;}


	Tetromino(string name, uint8_t id, string image, uint8_t size, unsigned long offsets_number);
	bool GetPieceXY(const State & state, uint8_t x, uint8_t y) const;

	static void Initialize();


};

#endif