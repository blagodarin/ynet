#pragma once

namespace ynet
{
	class Trigger
	{
	public:

		~Trigger()
		{
			if (_action)
				_action();
		}

		void operator=(const std::function<void()>& action)
		{
			_action = action;
		}

		Trigger() = default;
		Trigger(const Trigger&) = delete;
		Trigger(Trigger&&) = delete;

		Trigger& operator=(const Trigger&) = delete;
		Trigger& operator=(Trigger&&) = delete;

	private:

		std::function<void()> _action;
	};
}
