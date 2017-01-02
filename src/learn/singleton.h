#pragma once

namespace rl{
	template <typename T>
	class SingletonWrapper {
	public: 
		static T& getInstance(){
			static T instance;
			return instance;
		}
	private:
		SingletonWrapper() = default;
		SingletonWrapper(SingletonWrapper const&) = delete;
		void operator=(SingletonWrapper const&) = delete;
	};
}