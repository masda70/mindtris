#include "includes.h"
#include "tetromino.h"
	

	Tetromino * Tetromino::TYPE_J;
	Tetromino * Tetromino::TYPE_L;
	Tetromino * Tetromino::TYPE_T;
	Tetromino * Tetromino::TYPE_S;
	Tetromino * Tetromino::TYPE_Z;
	Tetromino * Tetromino::TYPE_I;
	Tetromino * Tetromino::TYPE_O;

void Tetromino::Initialize(){

	TYPE_I = new Tetromino("I",ID_I,"block_0.png",4,5);
	TYPE_J = new Tetromino("J",ID_J,"block_1.png",3,5);
	TYPE_L = new Tetromino("L",ID_L,"block_2.png",3,5);
	TYPE_O = new Tetromino("O",ID_O,"block_3.png",2,1);
	TYPE_S = new Tetromino("S",ID_S,"block_4.png",3,5);
	TYPE_T = new Tetromino("T",ID_T,"block_5.png",3,5);
	TYPE_Z = new Tetromino("Z",ID_Z,"block_6.png",3,5);


	{								
	uint8_t map[] = {1,0,0, 1,1,1, 0,0,0};
	uint8_t offsets[][2] = {{0,0},{0,0},{0,0},{0,0},{0,0}};
	TYPE_J->SetupPiece(STATE_0,map,offsets);
	}
	{
	uint8_t map[] = {0,1,1, 0,1,0, 0,1,0};
	uint8_t offsets[][2] = {{0,0},{+1,0},{+1,-1},{0,+2},{+1,+2}};
	TYPE_J->SetupPiece(STATE_R,map,offsets);
	}
	{
	uint8_t map[] ={0,0,0, 1,1,1, 0,0,1};
	uint8_t offsets[][2] =  {{0,0},{0,0},{0,0},{0,0},{0,0}};
	TYPE_J->SetupPiece(STATE_2,map,offsets);
	}
	{
	uint8_t map[] = {0,1,0, 0,1,0, 1,1,0};
	uint8_t offsets[][2] =  {{0,0},{-1,0},{-1,-1},{0,+2},{-1,+2}};
	TYPE_J->SetupPiece(STATE_L,map,offsets);
	}
	{
	uint8_t map[] = {0,0,1, 1,1,1, 0,0,0};
	uint8_t offsets[][2] = {{0,0},{0,0},{0,0},{0,0},{0,0}};
	TYPE_L->SetupPiece(STATE_0,map,offsets);
	}
	{
	uint8_t map[] = {0,1,0, 0,1,0, 0,1,1};
	uint8_t offsets[][2] = {{0,0},{+1,0},{+1,-1},{0,+2},{+1,+2}};
	TYPE_L->SetupPiece(STATE_R,map,offsets);
	}
	{
	uint8_t map[] ={0,0,0, 1,1,1, 1,0,0};
	uint8_t offsets[][2] =  {{0,0},{0,0},{0,0},{0,0},{0,0}};
	TYPE_L->SetupPiece(STATE_2,map,offsets);
	}
	{
	uint8_t map[] = {1,1,0, 0,1,0, 0,1,0};
	uint8_t offsets[][2] =  {{0,0},{-1,0},{-1,-1},{0,+2},{-1,+2}};
	TYPE_L->SetupPiece(STATE_L,map,offsets);
	}
	{
	uint8_t map[] = {0,1,1, 1,1,0, 0,0,0};
	uint8_t offsets[][2] = {{0,0},{0,0},{0,0},{0,0},{0,0}};
	TYPE_S->SetupPiece(STATE_0,map,offsets);
	}
	{
	uint8_t map[] = {0,1,0, 0,1,1, 0,0,1};
	uint8_t offsets[][2] = {{0,0},{+1,0},{+1,-1},{0,+2},{+1,+2}};
	TYPE_S->SetupPiece(STATE_R,map,offsets);
	}
	{
	uint8_t map[] ={0,0,0, 0,1,1, 1,1,0};
	uint8_t offsets[][2] =  {{0,0},{0,0},{0,0},{0,0},{0,0}};
	TYPE_S->SetupPiece(STATE_2,map,offsets);
	}
	{
	uint8_t map[] = {1,0,0, 1,1,0, 0,1,0};
	uint8_t offsets[][2] =  {{0,0},{-1,0},{-1,-1},{0,+2},{-1,+2}};
	TYPE_S->SetupPiece(STATE_L,map,offsets);
	}
	{
	uint8_t map[] = {0,1,0, 1,1,1, 0,0,0};
	uint8_t offsets[][2] = {{0,0},{0,0},{0,0},{0,0},{0,0}};
	TYPE_T->SetupPiece(STATE_0,map,offsets);
	}
	{
	uint8_t map[] = {0,1,0, 0,1,1, 0,1,0};
	uint8_t offsets[][2] = {{0,0},{+1,0},{+1,-1},{0,+2},{+1,+2}};
	TYPE_T->SetupPiece(STATE_R,map,offsets);
	}
	{
	uint8_t map[] ={0,0,0, 1,1,1, 0,1,0};
	uint8_t offsets[][2] =  {{0,0},{0,0},{0,0},{0,0},{0,0}};
	TYPE_T->SetupPiece(STATE_2,map,offsets);
	}
	{
	uint8_t map[] = {0,1,0, 1,1,0, 0,1,0};
	uint8_t offsets[][2] =  {{0,0},{-1,0},{-1,-1},{0,+2},{-1,+2}};
	TYPE_T->SetupPiece(STATE_L,map,offsets);
	}
	{
	uint8_t map[] = {1,1,0, 0,1,1, 0,0,0};
	uint8_t offsets[][2] = {{0,0},{0,0},{0,0},{0,0},{0,0}};
	TYPE_Z->SetupPiece(STATE_0,map,offsets);
	}
	{
	uint8_t map[] = {0,0,1, 0,1,1, 0,1,0};
	uint8_t offsets[][2] = {{0,0},{+1,0},{+1,-1},{0,+2},{+1,+2}};
	TYPE_Z->SetupPiece(STATE_R,map,offsets);
	}
	{
	uint8_t map[] ={0,0,0, 1,1,0, 0,1,1};
	uint8_t offsets[][2] =  {{0,0},{0,0},{0,0},{0,0},{0,0}};
	TYPE_Z->SetupPiece(STATE_2,map,offsets);
	}
	{
	uint8_t map[] = {0,1,0, 1,1,0, 1,0,0};
	uint8_t offsets[][2] =  {{0,0},{-1,0},{-1,-1},{0,+2},{-1,+2}};
	TYPE_Z->SetupPiece(STATE_L,map,offsets);
	}
	{
	uint8_t map[] = {0,0,0,0, 1,1,1,1, 0,0,0,0, 0,0,0,0};
	uint8_t offsets[][2] = {{0,0},{-1,0},{+2,0},{-1,0},{+2,0}};
	TYPE_I->SetupPiece(STATE_0,map,offsets);
	}
	{
	uint8_t map[] = {0,0,1,0, 0,0,1,0, 0,0,1,0, 0,0,1,0};;
	uint8_t offsets[][2] ={{0,0},{1,0},{1,0},{1,+1},{1,-2}};
	TYPE_I->SetupPiece(STATE_R,map,offsets);
	}
	{
	uint8_t map[] ={0,0,0,0, 0,0,0,0, 1,1,1,1, 0,0,0,0};
	uint8_t offsets[][2] = {{0,0},{+2,0},{-1,0},{+2,-1},{-1,-1}};
	TYPE_I->SetupPiece(STATE_2,map,offsets);
	}
	{
	uint8_t map[] = {0,1,0,0, 0,1,0,0, 0,1,0,0, 0,1,0,0};
	uint8_t offsets[][2] = {{0,0},{0,0},{0,0},{0,-2},{0,+1}};
	TYPE_I->SetupPiece(STATE_L,map,offsets);									
	}
	{
	uint8_t map[] = {1,1, 1,1};
	uint8_t offsets[][2] =  {{0,0}};
	TYPE_O->SetupPiece(STATE_0,map,offsets);
	}
	{
	uint8_t map[] = {1,1, 1,1};
	uint8_t offsets[][2] =  {{0,0}};
	TYPE_O->SetupPiece(STATE_R,map,offsets);
	}
	{
	uint8_t map[] = {1,1, 1,1};
	uint8_t offsets[][2] =  {{0,0}};
	TYPE_O->SetupPiece(STATE_2,map,offsets);
	}
	{
	uint8_t map[] = {1,1, 1,1};
	uint8_t offsets[][2] =  {{0,0}};
	TYPE_O->SetupPiece(STATE_L,map,offsets);
	}
}

Tetromino::Tetromino(string name, uint8_t id, string image, uint8_t size,unsigned long offsets_number):
	m_piecename(name),
	m_piecesize(size),
	m_offsets_number(offsets_number),
	m_id(id),
	m_blockimage(image)
{
	m_piecemaps.reset(new bool[m_piecesize*m_piecesize*STATE_NUMBER]);
	m_offsetmap.reset(new uint8_t[m_offsets_number*2*STATE_NUMBER]);

}

const Tetromino * Tetromino::GetTetrominoByID(uint8_t id){
	switch(id)
	{
		case ID_I: return TYPE_I;
		case ID_J: return TYPE_J;
		case ID_L: return TYPE_L;
		case ID_S: return TYPE_S;
		case ID_T: return TYPE_T;
		case ID_Z: return TYPE_Z;
		case ID_O: return TYPE_O;
	}
	return nullptr;
	
}
void Tetromino::SetupPiece(const State & state,const uint8_t map[], const uint8_t offsets[][2]){

	for(uint8_t x = 0; x < m_piecesize; x++){
		for(uint8_t y = 0; y<m_piecesize; y++){
			m_piecemaps[state*m_piecesize*m_piecesize + y*m_piecesize + x] = (map[(m_piecesize-1-y)*m_piecesize + x]==1)?true:false;
		}
	}

	for(unsigned long i = 0; i < m_offsets_number; i++){
		m_offsetmap[state*m_offsets_number*2 + i*2 + 0] = offsets[i][0];
		m_offsetmap[state*m_offsets_number*2 + i*2 + 1] = offsets[i][1];
	}

}

bool Tetromino::GetPieceXY(const Tetromino::State & state, uint8_t x, uint8_t y) const
{
	if(x<0 || y<0 || x>=m_piecesize || y >=m_piecesize )
		return false;
	else
		return m_piecemaps[state*m_piecesize*m_piecesize+y*m_piecesize+x];
}


uint8_t Tetromino::GetXOffset(Tetromino::State state, unsigned long offset_test) const{
	return m_offsetmap[state*m_offsets_number*2 + offset_test*2 + 0];
}

uint8_t Tetromino::GetYOffset(Tetromino::State state, unsigned long offset_test) const{
	return m_offsetmap[state*m_offsets_number*2 + offset_test*2 + 1];
}

bool Tetromino::GetXY(Tetromino::State state, uint8_t x, uint8_t y) const{
	return m_piecemaps[state*m_piecesize*m_piecesize+y*m_piecesize+x];
}

