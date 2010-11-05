// This is the main DLL file.


#include "mindtriscore.h"

void CONSOLE_Print( string message )
{
	cout << message << endl;
}

void DEBUG_Print( string message )
{
	cout << message << endl;
}

void DEBUG_Print( BYTEARRAY b )
{
	cout << "{ ";

	for( unsigned int i = 0; i < b.size( ); i++ )
		cout << hex << (int)b[i] << " ";

	cout << "}" << endl;
}