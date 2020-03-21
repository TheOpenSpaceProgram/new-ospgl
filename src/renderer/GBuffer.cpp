#include "GBuffer.h"
#include "../util/Logger.h"

void GBuffer::create(size_t width, size_t height)
{
	this->width = width;
	this->height = height;

	glGenFramebuffers(1, &g_buffer);
	glBindFramebuffer(GL_FRAMEBUFFER, g_buffer);

	// Ultra-High quality position buffer 
	// TODO: Maybe this is overkill, but it's needed for proper 
	// solar-system sized lighting. If we don't need that, we
	// could use a GL_RGB16F buffer, but then planets would not be
	// render properly on a lot of cases.
	glGenTextures(1, &g_pos);
	glBindTexture(GL_TEXTURE_2D, g_pos);
	// Turns out GL_RGB32F is not widely supported, but GL_RGBA32F is
	// So we have to use it for the thing to work in MESA drivers. 
	// No perfomance hit as we use the alpha for emissive. It's just a bit confusing...
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, (int)width, (int)height, 0, GL_RGB, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_pos, 0);

	// High quality normal buffer
	glGenTextures(1, &g_nrm);
	glBindTexture(GL_TEXTURE_2D, g_nrm);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, (int)width, (int)height, 0, GL_RGB, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, g_nrm, 0);

	// Low quality color and specular buffer
	glGenTextures(1, &g_col);
	glBindTexture(GL_TEXTURE_2D, g_col);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (int)width, (int)height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, g_col, 0);


	unsigned int attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
	glDrawBuffers(3, attachments);

	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, (int)width, (int)height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		logger->fatal("Could not create GBuffer, check you drivers.");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void GBuffer::destroy()
{
	glDeleteRenderbuffers(1, &rbo);
	glDeleteTextures(1, &g_col);
	glDeleteTextures(1, &g_nrm);
	glDeleteTextures(1, &g_pos);
	glDeleteFramebuffers(1, &g_buffer);
}


void GBuffer::bind()
{
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &unbind_to);
	glGetIntegerv(GL_VIEWPORT, &unbind_viewport[0]);

	glBindFramebuffer(GL_FRAMEBUFFER, g_buffer);

	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
}

void GBuffer::unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)unbind_to);
	glViewport(0, 0, unbind_viewport[2], unbind_viewport[3]);
}

void GBuffer::set_viewport()
{
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
}

glm::ivec2 GBuffer::get_size()
{
	return glm::ivec2(width, height);
}


GBuffer::GBuffer(size_t width, size_t height)
{
	create(width, height);
}


GBuffer::~GBuffer()
{
	destroy();
}
