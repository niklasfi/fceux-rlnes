#include "learn.h"

#include "flappy.h"

#include <iostream>

namespace rl{
	void process(uint8* RAM, uint8& action){
		static int cooldown = 0;
		if (cooldown-- >= 0){
			return;
		}

		static auto game = FlappyGame<QTable>();
		action |= game.play(RAM);

		if (action)
			cooldown = 2;
	}
}