#include "Input.h"



void Input::set_ctx(InputContext* ctx)
{
	if(cur_ctx != nullptr)
	{
		cur_ctx->input = nullptr;
	}

	cur_ctx = ctx;
	cur_ctx->input = this;
}

void Input::update(GLFWwindow* window, double dt)
{
	if(cur_ctx != nullptr)
	{
		cur_ctx->update(window, dt);
	}
}

