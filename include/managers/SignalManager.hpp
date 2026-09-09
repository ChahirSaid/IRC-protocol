#pragma once

#include <csignal>

class SignalManager
{
	private:
		static sig_atomic_t _isInterrupted;

	public:
		SignalManager();
		~SignalManager();

		bool isInterrupted();
		void setupSignalHandlers();

	private:
		static void _handleSignal(int signalNumber);
};