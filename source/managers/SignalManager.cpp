#include "SignalManager.hpp"

sig_atomic_t SignalManager::_isInterrupted = 0;

SignalManager::SignalManager() {}

SignalManager::~SignalManager() {}

bool SignalManager::isInterrupted()
{
	return(_isInterrupted != 0);
}

void SignalManager::setupSignalHandlers()
{
	struct sigaction signalAction;
	signalAction.sa_handler = &SignalManager::_handleSignal;
	sigemptyset(&signalAction.sa_mask);
	signalAction.sa_flags = 0;

	sigaction(SIGINT, &signalAction, NULL);
	sigaction(SIGQUIT, &signalAction, NULL);

	return;
}

void SignalManager::_handleSignal(int signalNumber)
{
	(void)signalNumber;
	_isInterrupted = 1;

	return;
}