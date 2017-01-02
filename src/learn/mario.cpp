#include "mario.h"

namespace rl{
	MarioState::MarioState(uint8* RAM){
		marioMode = RAM[ADDR_MARIO_MODE];
		lives = RAM[ADDR_LIVES];
		coins = RAM[ADDR_COINS];
		world = RAM[ADDR_WORLD];
		level = RAM[ADDR_LEVEL];
	}

	uint8 MarioState::marioMode_char() const{
		switch (marioMode){
		case 0:
			return 's';
		case 1:
			return 'L'; 
		case 2:
			return 'F';
		default:
			return ' ';
		}
	}

	std::ostream& operator<<(std::ostream& os, const MarioState& state){
		/*os
			<< (state.lives + 1) << state.marioMode_char() << " "
			<< "C" << (state.coins < 10 ? "0" : "") << state.coins << " "
			<< (state.world + 1) << "-" << (state.lives)
			<< "\n";
		return os;*/
	}

	MarioBot::MarioBot(){
		//TODO
	}

	uint8 MarioBot::react(const MarioState& state){
		//TODO
		return 0;
	}

	void MarioBot::learn(const MarioState& oldState, uint8 action, const MarioState& newState){
		//TODO
	}

	std::ostream& operator<<(std::ostream& os, const MarioBot& bot){
		//TODO
		return os;
	}

	std::istream& operator>>(std::istream& is, MarioBot& bot){
		//TODO
		return is;
	}
}
