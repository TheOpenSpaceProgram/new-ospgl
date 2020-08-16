#pragma once 
#include <functional>
#include <type_traits>

// We have to do this so we can use function style template
// return(arg1, arg2, ...)
template<typename T>
class Signal;

template<typename R, typename... Args>
class Signal<R(Args...)>
{
public:

	typedef std::function<R(Args...)> Handler;

	std::vector<Handler> handlers;

	void add_handler(Handler handler)
	{
		handlers.push_back(handler);
	}


	R call(Args&& ...args)
	{
		for(Handler h : handlers)
		{
			if constexpr ( std::is_void<R>::value)
			{
				h(std::forward<Args>(args)...);
			}
			else
			{
				return h(std::forward<Args>(args)...);
			}
		}
	}

	R operator()(Args&& ...args)
	{
		if constexpr (std::is_void<R>::value)
		{
			call(std::forward<Args>(args)...);
		}
		else
		{
			return call(std::forward<Args>(args)...);
		}
	}
	



};
