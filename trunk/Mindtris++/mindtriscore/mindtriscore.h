#ifndef MINDTRISCORE_H
#define MINDTRISCORE_H


#include "includes.h"
#include "ms_stdint.h"
#include "util.h"
#include "socket.h"
#include "serverprotocol.h"


void CONSOLE_Print( string message );
void DEBUG_Print( string message );
void DEBUG_Print( BYTEARRAY b );

#endif // MINDTRISCORE_H