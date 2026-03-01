#pragma once

namespace net
{
	enum class MessageType
	{
		ServerPing,
		MessageAll,
		Disconnect,
		Fold,
		Raise,
		Call,
		AllIn
	};
}