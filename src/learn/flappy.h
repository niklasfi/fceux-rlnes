#pragma once

#include "singleton.h"
#include "stateproxy.h"
#include <unordered_map>
#include <algorithm>
#include <numeric>
#include <map>
#include <tuple>
#include <string>
#include <iostream>
#include <istream>
#include <fstream>

#include "../types.h"
#include "../ppu.h"

namespace rl{
	struct FlappyState {
		static const unsigned ADDR_PIPE_TYPE = 0x03b;
		static const unsigned ADDR_SCORE_1 = 0x041;
		static const unsigned ADDR_SCORE_2 = 0x042;
		static const unsigned ADDR_SCORE_3 = 0x043;
		static const unsigned ADDR_SCORE_4 = 0x044;
		static const unsigned ADDR_FLAPPY_DIRECTION = 0x004b;
		static const unsigned ADDR_TOP_DIST = 0x210;
		static const unsigned ADDR_FLIGHT_MODE = 0x216;
		static const unsigned ADDR_ALIVE1 = 0x188;
		static const unsigned ADDR_ALIVE2 = 0x189;
		static const unsigned ADDR_FRAME_SKIP = 0x001;

		static const int SCREEN_X_SIZE = 256;
		static const int SCREEN_Y_SIZE = 256;
		static const int BOARD_X_SIZE = 512;
		static const int BOARD_Y_SIZE = 480;
		static const int BLOCK_X_SIZE = 8;
		static const int BLOCK_Y_SIZE = 8;
		static const int BIRD_BEFORE_SCROLL_OFFSET = BLOCK_X_SIZE * 11 + 4;
		static const int PIPE_X_DIST = BLOCK_X_SIZE * 12;

		enum FLIGHT_MODE { UP = 0, DOWN = 1, INTRO = 2 };

		class PipeOutOfBoundsException : public std::exception {};

		FlappyState(uint8* RAM){
			int y_scroll;
			ppu_getScroll(x_scroll, y_scroll);
			assert(0 <= x_scroll);
			assert(x_scroll < BOARD_X_SIZE);
			assert(0 <= y_scroll);
			assert(y_scroll < BOARD_Y_SIZE);
			
			x_abs = (x_scroll + BIRD_BEFORE_SCROLL_OFFSET) % BOARD_X_SIZE;
			assert(0 <= x_scroll);
			assert(x_abs < BOARD_X_SIZE);

			y_abs = RAM[ADDR_TOP_DIST] >= BOARD_Y_SIZE / 2 ? SCREEN_Y_SIZE : SCREEN_Y_SIZE - RAM[ADDR_TOP_DIST];
			assert(0 <= y_abs);
			assert(y_abs <= SCREEN_Y_SIZE);

			try{
				std::tie(x_pipe, y_pipe) = findNext(x_abs);
			}
			catch(PipeOutOfBoundsException e){
				x_pipe = BOARD_X_SIZE;
				y_pipe = SCREEN_X_SIZE * 3 / 4;
			}
			x_rel = (x_abs - x_pipe) % SCREEN_X_SIZE;
			y_rel = y_abs - y_pipe;

			score = RAM[ADDR_SCORE_1] + 10 * RAM[ADDR_SCORE_2] + 100 * RAM[ADDR_SCORE_3] + 1000 * RAM[ADDR_SCORE_4];
			//std::cout << "abs: " << x_abs << "," << y_abs << ", pip: " << x_pipe << "," << y_pipe << ", rel: " << x_rel << "," << y_rel << "\n";

			flightMode = (FLIGHT_MODE)RAM[ADDR_FLIGHT_MODE];
			intro = RAM[0x21a] == 2 || RAM[0x19] == 133;

			flappyDirection = RAM[ADDR_FLAPPY_DIRECTION];
			alive = RAM[0x03c] == 1;
			accepts_input = alive && !intro;
			frame_skip = RAM[ADDR_FRAME_SKIP] == 4;

			/*
			static bool alive_old = 0, intro_old = 0;
			if (alive_old != alive || intro_old != intro){
				std::cout << "alive: " << alive << ", intro: " << intro << ", accepts_input: " << accepts_input << "\n";
				alive_old = alive;
				intro_old = intro;
			}
			*/
		};
		int x_scroll;
		int x_abs;
		int y_abs;
		int x_rel;
		int y_rel;
		int x_pipe;
		int y_pipe;
		int score;

		FLIGHT_MODE flightMode; // 3val
		uint8 flappyDirection; // -1 , 0 , 1 , 2 , 3
		bool alive; //boolean
		bool intro;
		bool accepts_input;
		bool frame_skip;

		FlappyState& operator=(const FlappyState& s) = default;

		FlappyState() = default;
	private:
		static uint8& posToPPAVal(size_t x, size_t y){
			assert(x < 64);
			assert(y < 32);

			size_t offset = 0;
			if (x >= 32){
				offset = 0x0400;
				x -= 32;
			}
			offset += x + 32 * y;
			return NTARAM[offset];
		}

		static std::tuple<int, int> findNext(int x_abs){
			uint8 x_block = x_abs / 8;
			for (uint8 xoff = 0; xoff < BOARD_X_SIZE / BLOCK_X_SIZE; xoff++){
				uint8 x = (x_block + xoff) % (BOARD_X_SIZE / BLOCK_X_SIZE);
				bool found = false;
				uint8 y = 0;
				for (y = 0; y < BOARD_Y_SIZE / BLOCK_Y_SIZE / 2; ++y){
					// /2: only search upper screen
					uint8 mem = posToPPAVal(x, y);
					if (mem == 0) break;
					else {
						if (mem == 0x2d || mem == 0x2e || mem == 0x2f || mem == 0x30) {
							found = true;
							break;
						}
					}
				}

				if (found){
					return std::make_tuple(x * BLOCK_X_SIZE, SCREEN_Y_SIZE - y * BLOCK_Y_SIZE);
				}
			}
			throw PipeOutOfBoundsException();
		}

		friend std::ostream& operator<<(std::ostream& os, const FlappyState& state);
	};

	std::ostream& operator<<(std::ostream& os, const FlappyState& state){
		os << "@" << state.x_rel << "," << state.y_rel;
		if (!state.alive){
			os << "d";
		}
		if (state.intro){
			os << "i";
		}
		return os;
	}


	template<size_t x_stepSize, size_t y_stepSize>
	class FlappyDiscretizedState{
	public:
		FlappyDiscretizedState(const FlappyState& state){
			discrete = std::to_string((state.x_rel / x_stepSize) * x_stepSize) + ";" + std::to_string((state.y_rel / y_stepSize) * y_stepSize); // +";" + std::to_string(state.ADDR_FLAPPY_DIRECTION);
		}
		FlappyDiscretizedState(){}

		std::string serialize() const{
			//DO STUFF!!!
			return discrete;
		};

		bool operator==(const FlappyDiscretizedState& other) const{
			return serialize() == other.serialize();
		}
		bool operator!=(const FlappyDiscretizedState& other) const{
			return !(*this == other);
		}
		static std::ostream& osOp(std::ostream& os, const FlappyDiscretizedState<x_stepSize, y_stepSize>& state)
		{
			return os << state.serialize(); //TODO
		};
		static std::istream& isOp(std::istream& is, FlappyDiscretizedState<x_stepSize, y_stepSize>& state)
		{
			is >> state.discrete;
			return is;
		};
	private:
		double x;
		double y;
		std::string discrete;
	};

	class QTable
	{
	public:
		using StateT = FlappyState;
		using ActionT = uint8;
		using xT = decltype(FlappyState::x_rel);
		using yT = decltype(FlappyState::y_rel);
		std::vector<double> Q;
		static const xT x_min = -512;
		static const xT x_max = 256;
		static const yT y_min = -256;
		static const yT y_max = 256;
		static const xT x_dim = x_max - x_min;
		static const yT y_dim = y_max - y_min;
		static const int a_dim = 2;
		static const int x_compr = 64;
		static const int y_compr = 2;

		QTable(): Q(x_dim / x_compr * y_dim / y_compr * a_dim, 0.){
			//isOp(std::ifstream("../../roms/knowledge.bin", std::ios::in | std::ios::binary), *this);
		}
		QTable(const QTable& qtable) : Q(qtable.Q){}
		~QTable(){
			//osOp(std::ofstream("../../roms/knowledge.bin", std::ios::out | std::ios::binary), *this);
		}

		ActionT getBestAction(const StateT& state, const std::vector<ActionT>& possibleActions){
			double best_reward = std::numeric_limits<double>::lowest();
			uint8 best_action = possibleActions.front();

			for (const auto& action : possibleActions){
				const double reward = this->at(state, action);
				if (reward > best_reward){
					best_reward = reward;
					best_action = action;
				}
			}
			return best_action;
		}
		void update(const StateT& oldState, ActionT action, const StateT& newState, const double& reward,
			const std::vector<ActionT>& actions_new){

			double best_reward = std::numeric_limits<double>::lowest();
			for (const auto& action : actions_new){
				best_reward = std::max(at(newState, action), best_reward);
			}

			const double alpha = .7;
			const double gamma = .3;
			at(oldState, action) += alpha * (reward + gamma * best_reward - at(oldState, action));
		}

		static std::ostream& osOp(std::ostream& os, const QTable& qtable){
			os.write((char*)qtable.Q.data(), sizeof(qtable.Q.front()) * qtable.Q.size());
			os.flush();
			std::cout << "SUM: " << std::accumulate(qtable.Q.begin(), qtable.Q.end(), 0) << "\n";
			return os;
		}
		static std::istream& isOp(std::istream& is, const QTable& qtable){
			is.read((char*) qtable.Q.data(), sizeof(qtable.Q.front()) * qtable.Q.size());
			std::cout << "SUM: " << std::accumulate(qtable.Q.begin(), qtable.Q.end(), 0) << "\n";
				return is;
		}
	private:
		double& at(const xT& x, const yT& y, const ActionT& action){
			assert(x_min <= x);
			assert(x < x_max);
			assert(y_min <= y);
			assert(y < y_max);
			xT x_off = (x - x_min) / x_compr;
			yT y_off = (y - y_min) / y_compr;
			return Q.at(x_off * y_dim / y_compr * a_dim + y_off * a_dim + (action != 0 ? 1 : 0));
		}
		const double& at(const xT& x, const yT& y, const ActionT& action) const {
			assert(x_min <= x);
			assert(x < x_max);
			assert(y_min <= y);
			assert(y < y_max);
			xT x_off = (x - x_min) / x_compr;
			yT y_off = (y - y_min) / y_compr;
			return Q.at(x_off * y_dim / y_compr * a_dim + y_off * a_dim + (action != 0 ? 1 : 0));
		}
		double& at(const StateT& state, const ActionT& action){
			return at(state.x_rel, state.y_rel, action);
		}
		const double& at(const StateT& state, const ActionT& action) const{
			return at(state.x_rel, state.y_rel, action);
		}
	};



	template< class DiscretizedState, class State > class QHash
	{
	public:
		typedef DiscretizedState DiscretizedStateT;
		typedef State StateT;
		typedef uint8 ActionT;
		typedef std::tuple<DiscretizedState, ActionT> StateActionT;

		QHash() = default;
		QHash(std::istream& is) :QHash() {
			QHash::isOp(is, *this);
		}
		~QHash(){
			std::ofstream os("hello.data");
			QHash::osOp(os, *this);

			os.close();
		}
		ActionT getBestAction(const State& state, const std::vector<ActionT>& possibleActions){
			DiscretizedState Dstate(state);
			double bestReward = std::numeric_limits<double>::lowest();
			ActionT bestAction = possibleActions[0];
			//compute the max of Q(S',a)
			for (auto action : possibleActions){
				StateActionT currSA = std::make_tuple(Dstate, action);

				auto it = Q.find(currSA);
				double reward = it != Q.end() ? it->second : 0;

				if (reward > bestReward)
				{
					bestReward = reward;
					bestAction = action;
				}
			}
			return bestAction;
		}
		void update(const State& oldState, ActionT action, const State& newState, const double& reward,
			const std::vector<ActionT>& possibleActionsInNewState, double alpha, double gamma){
			DiscretizedState oldDState(oldState);
			DiscretizedState newDState(newState);
			StateActionT oldSA = std::make_tuple(oldDState, action);

			/*std::ofstream os("hello2.data", std::ofstream::app);
			DiscretizedState::osOp(os, Dstate);
			os << std::endl;
			DiscretizedState::osOp(os, DNstate);
			os << std::endl<<"ciao"<<std::endl;
			for (unsigned i = 0; i < possibleActionsInNewState.size(); ++i)
			os << i<< ":" << possibleActionsInNewState[i] << std::endl;*/

			double bestReward = std::numeric_limits<double>::lowest();
			double foundNextSA = false;

			//compute the max of Q(S',a)
			for (unsigned i = 0; i < possibleActionsInNewState.size(); ++i)
			{
				StateActionT nextSA = std::make_tuple(newDState, possibleActionsInNewState[i]);
				auto it = Q.find(nextSA);
				if (it != Q.end() && it->second > bestReward){
					bestReward = it->second;
					foundNextSA = true;
				}
			}

			if (!foundNextSA)
				bestReward = 0;

			//update the Q value
			auto it = Q.find(oldSA);
			if (it == Q.end()){
				bool inserted = false;
				std::tie(it, inserted) = Q.insert(std::make_pair(oldSA, 0));
				assert(inserted);
			}
			it->second = (1. - alpha) * it->second + alpha*(reward + gamma*bestReward);
		}

		static std::ostream& osOp(std::ostream& os, const QHash<DiscretizedState, State >& Qhash){
			for (auto it = Qhash.Q.begin(); it != Qhash.Q.end(); ++it)
			{
				const typename QHash::StateActionT& currSA = it->first;
				DiscretizedState::osOp(os, std::get<0>(currSA));
				os << std::endl
					<< std::get<1>(currSA) << std::endl
					<< it->second << std::endl;
			}

			return os;
		}

		static std::istream& isOp(std::istream& is, QHash<DiscretizedState, State >& Qhash){
			typename QHash<DiscretizedState, State >::DiscretizedStateT state;
			typename QHash<DiscretizedState, State >::ActionT action;
			double value;
			if (is.fail()) return is;
			while (!is.eof())
			{
				DiscretizedStateT::isOp(is, state);
				is >> action >> value;
				QHash::StateActionT currSA = std::make_tuple(state, action);
				Qhash.Q[currSA] = value;
			}
			return is;

		}

	private:
		struct StateAction_hash : public std::unary_function < StateActionT, std::size_t >
		{
			std::size_t operator()(const StateActionT& k) const
			{
				std::hash<std::string> hash_fn1;
				std::hash<ActionT> hash_fn2;
				return hash_fn1(std::get<0>(k).serialize()) ^ hash_fn2(std::get<1>(k));
			}
		};
		std::unordered_map<StateActionT, double, StateAction_hash> Q;
	};

	//RLBot < Q <DiscretizedState, State > >
	template <class Q>
	class RLBot {
	public:
		RLBot(const Q& Knowledge, const double alphaInit = 0.3, const double gammaInit = 0.7, const double epsilon = 0.0) : Knowledge(Knowledge), alpha(alphaInit), gamma(gammaInit){}
		typename Q::ActionT react(const typename Q::StateT& state, const typename std::vector<typename Q::ActionT>& possibleActions){
			//TODO: implement epsilon greedy exploration policy
			// something like: 
			// if(rand<epsilon)
			//	  chose randomly the action
			// else
			return Knowledge.getBestAction(state, possibleActions);
		}
		void learn(const typename Q::StateT& oldState, typename Q::ActionT action, const typename Q::StateT& newState, const double& reward, const std::vector<typename Q::ActionT>& possibleActionsInNewState){
			//update Q values
			std::cout << oldState << " " << (action != 0 ? "A" : "_") << " " << reward << "\n";
			Knowledge.update(oldState, action, newState, reward, possibleActionsInNewState, alpha, gamma);
		}
		friend std::ostream& operator<<(std::ostream& os, const RLBot& bot){
			os << bot.alpha << std::endl
				<< bot.gamma << std::endl;
			os << bot.Knowledge;
			return os;
		}
		friend std::istream& operator>>(std::istream& is, RLBot& bot){
			is >> bot.alpha >> bot.gamma >> bot.Knowledge;
			return is;
		}
		typedef typename Q::ActionT ActionT;
		typedef Q QT;
	private:
		Q Knowledge;
		double alpha;
		double gamma;
	};
	
	template<typename Bot>
	class FlappyGame{
	public:
		FlappyGame() = default;
		FlappyGame(const Bot& bot) : bot(bot){}
		uint8 play(uint8* RAM){
			currentState = FlappyState(RAM);
			
			/*
			static uint8 score_old = 0;
			uint8 score_cur = currentState.score;
			if(score_old != score_cur){
				std::cout << (int)score_cur << "\n";
			}
			score_old = score_cur;
			*/

			/*
			* This is the trivial AI
			*/

			static std::ofstream l("../../roms/log");
			static int x_pipe_old = 0;
			if (x_pipe_old != currentState.x_pipe){
				;//l << currentState << ", x_pipe: " << currentState.x_pipe << ", y_pipe: " << currentState.y_pipe << "\n" << std::flush;
			}
			x_pipe_old = currentState.x_pipe;

			//return -1 * (currentState.y_rel < -48 || currentState.intro) * (!currentState.frame_skip);

			uint8 response = 0;
			static uint8 last_response = 0;
			double reward = rewardFor(currentState);
			static FlappyState oldState;

			//std::cout << "current: " << currentState << "\n";
			if (oldState.accepts_input){
				int rem = currentState.score % 2048;
				std::vector<int> test{ 0, 683, 1365, 1366 };
				
				if(currentState.score == 0 || std::none_of(test.begin(), test.end(), [rem](int i){return i==rem ; }))
					bot.update(oldState, last_response, currentState, reward, viableActions(currentState));
				if (!currentState.alive)
					l << "   DEAD: " << currentState << ", s: " << currentState.score << std::endl;
			}

			if (currentState.accepts_input){
				response = bot.getBestAction(currentState, viableActions(currentState));
			} else {
				response = -1 * (currentState.y_rel < -48 || currentState.intro);
			}

			oldState = currentState;
			last_response = response;
			response *= !currentState.frame_skip; //don't respond, if the input is ignored anyway
			return response;

			/*
			uint8 response = 0;
			static int count = 0;
			static uint8 last_response = 0;
			static unsigned dead = 0;

			//std::cout << currentState.acceptsInput << ", a: " << currentState.alive << ", i: " << currentState.intro << ", t:" << currentState.overTheTop << ", g: " << currentState.onTheGround << ", r: " << currentState.gameRunning << "\n";
			double reward = rewardFor(currentState);

			if ((currentState.acceptsInput && dead == 0) || (dead > 100 && currentState.gameRunning))
			{
				if (dead)
					dead = 0;
				else
					bot.learn(oldState, last_response, currentState, reward, viableActions(oldState));
				response = bot.react(currentState, viableActions(currentState));
				//std::cout << "reward: " << reward << ", x: " << (int)currentState.x << ", y: " << currentState.y << "\n";
				oldState = currentState;
			}
			{
				response = 0xff * ((currentState.intro && dead % 2) || (currentState.y_rel < -72 && count % 2));

				if (response && !currentState.intro)
					//std::cout << " " << currentState.y << " ";

				if (dead < 1){
					//std::cout << "reward: " << reward << ", y: " << currentState.y << " KILL\n";
					bot.learn(oldState, last_response, currentState, reward, viableActions(oldState));
				}
				dead++;
			}

			last_response = response;
			return response;
			*/
		}
	private:
		Bot bot;
		FlappyState	currentState;
		FlappyState oldState;

		std::vector<typename Bot::ActionT> viableActions(const FlappyState& state){
			return std::vector < typename Bot::ActionT > { 0, 8 };
		}

		double rewardFor(const FlappyState& state){
			if (!state.alive)
				return -100;
			else
				return 1; // -abs(oldState.y - 36) + 36;
		}
	};
}
