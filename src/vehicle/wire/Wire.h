#pragma once
#include "Port.h"

class Wire
{
public:
	// Has the wire been written to this cycle?
	bool blocked;

	Port* from;
	Port* to;
};
