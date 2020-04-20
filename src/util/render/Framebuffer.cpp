#include "Framebuffer.h"
#include "../Logger.h"


void Framebuffer::bind()
{
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &unbind_to);
	glGetIntegerv(GL_VIEWPORT, &unbind_viewport[0]);

	glBindFramebuffer(GL_FRAMEBUFFER, fbuffer);

	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
}

void Framebuffer::unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)unbind_to);
	glViewport(unbind_viewport[0], unbind_viewport[1], unbind_viewport[2], unbind_viewport[3]);
}

void Framebuffer::set_viewport()
{
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
}

glm::ivec2 Framebuffer::get_size()
{
	return glm::ivec2(width, height);
}

Framebuffer::Framebuffer(size_t width, size_t height, GLuint rbo_override)
{
	this->width = width;
	this->height = height;
	glGenFramebuffers(1, &fbuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, fbuffer);

	glGenTextures(1, &tex_color_buffer);
	glBindTexture(GL_TEXTURE_2D, tex_color_buffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)width, (GLsizei)height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_color_buffer, 0);

	if (rbo_override == 0)
	{
		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, (GLsizei)width, (GLsizei)height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}
	else
	{
		rbo = rbo_override;
	}

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		logger->fatal("Couldn't create framebuffer! Check your drivers for updated OpenGL");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


Framebuffer::~Framebuffer()
{
}
