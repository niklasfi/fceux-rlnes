#pragma once

#include "../types.h"

template<typename BotType>
class StateProxy{
public:
	StateProxy() = default;
	~StateProxy(){
		delete current_state;
		delete old_state;
	}
	uint8 process(uint8* RAM, uint8& action){
		delete old_state;
		old_state = current_state;

		current_state = new typename BotType::stateType(RAM);

		uint8 response = bot.react(*current_state);

		if (old_state)
			bot.learn(*old_state, response, *current_state);

		return response;
	}
private:
	BotType bot;
	typename BotType::stateType* old_state;
	typename BotType::stateType* current_state;
};
