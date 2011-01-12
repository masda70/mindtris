#include "includes.h"

uint32_t GetTime( )
{
	return GetTicks( ) / 1000;
}

uint32_t GetTicks( )
{
#ifdef WIN32
	return timeGetTime( );
#elif __APPLE__
	uint64_t current = mach_absolute_time( );
	static mach_timebase_info_data_t info = { 0, 0 };
	// get timebase info
	if( info.denom == 0 )
		mach_timebase_info( &info );
	uint64_t elapsednano = current * ( info.numer / info.denom );
	// convert ns to ms
	return elapsednano / 1e6;
#else
	uint32_t ticks;
	struct timespec t;
	clock_gettime( CLOCK_MONOTONIC, &t );
	ticks = t.tv_sec * 1000;
	ticks += t.tv_nsec / 1000000;
	return ticks;
#endif
}

uint64_t GenerateRandomUINT64(CryptoPP::RandomNumberGenerator &rng){
	byte_t output[8];
	rng.GenerateBlock (output, 8); 
	return ( ((uint64_t) output[0]) << 56 |((uint64_t) output[1]) << 48 | ((uint64_t) output[2]) << 40 | ((uint64_t) output[3]) <<  32 | ((uint64_t) output[4]) << 24 | ((uint64_t) output[5]) << 16 | ((uint64_t) output[6]) << 8| ((uint64_t) output[7]));
}