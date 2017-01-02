#pragma once

#include "singleton.h"
#include "stateproxy.h"

#include "../types.h"

namespace rl{
	struct MarioState{
		static const unsigned ADDR_MARIO_MODE = 0x756;
		static const unsigned ADDR_LIVES = 0x75A;
		static const unsigned ADDR_COINS = 0x75E;
		static const unsigned ADDR_WORLD = 0x75F;
		static const unsigned ADDR_LEVEL = 0x760;

		enum MARIO_MODE { SMALL = 0, LARGE = 1, FIRE = 2};
	
		MarioState(uint8* RAM);

		uint8 marioMode;
		uint8 lives;
		uint8 coins;
		uint8 world;
		uint8 level;

		uint8 marioMode_char() const;
	};

	class MarioBot {
	public:
		MarioBot();

		typedef MarioState stateType;

		uint8 react(const MarioState& state);
		void learn(const MarioState& oldState, uint8 action, const MarioState& newState);
		friend std::ostream& operator<<(std::ostream& os, const MarioState& bot);
		friend std::istream& operator>>(std::istream& is, MarioState& bot);

		typedef SingletonWrapper<StateProxy<MarioBot>> MarioBotPS;
	};
}
