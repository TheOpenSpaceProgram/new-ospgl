#include "FlightInput.h"



void FlightInput::set_ctx(InputContext* ctx)
{
	if(cur_ctx != nullptr)
	{
		cur_ctx->input = nullptr;
	}

	if(ctx != nullptr)
	{
		cur_ctx = ctx;
		cur_ctx->input = this;
	}
}

void FlightInput::update(GLFWwindow* window, double dt)
{
	if(cur_ctx != nullptr)
	{
		cur_ctx->update(window, dt);
	}
}

