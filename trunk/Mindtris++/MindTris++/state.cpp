#include "mindtriscore/includes.h"
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include "globals.h"
#include "state.h"

void State::registerSignal( const std::string &id, unsigned long value ) {
	m_signals[id] = value;
}
