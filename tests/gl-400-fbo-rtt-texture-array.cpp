//**********************************
// OpenGL FBO texture array
// 17/06/2010 - 01/11/2011
//**********************************
// Christophe Riccio
// ogl-samples@g-truc.net
//**********************************
// G-Truc Creation
// www.g-truc.net
//**********************************

#include <glf/glf.hpp>

namespace
{
	char const * VERT_SHADER_SOURCE("gl-400/rtt-array.vert");
	char const * FRAG_SHADER_SOURCE("gl-400/rtt-array.frag");
	int const FRAMEBUFFER_SIZE(2);
	int const SAMPLE_SIZE_WIDTH(640);
	int const SAMPLE_SIZE_HEIGHT(480);

	glf::window Window("gl-400-fbo-rtt-texture-array", glm::ivec2(SAMPLE_SIZE_WIDTH, SAMPLE_SIZE_HEIGHT));

	GLsizei const VertexCount(3);

	enum texture_type
	{
		TEXTURE_R,
		TEXTURE_G,
		TEXTURE_B,
		TEXTURE_MAX
	};

	GLuint FramebufferName(0);
	GLuint VertexArrayName(0);
	GLuint ProgramName(0);
	GLint UniformDiffuse(0);
	GLint UniformLayer(0);
	GLuint TextureName(0);

	glm::vec4 Viewport[TEXTURE_MAX];

}//namespace

bool initProgram()
{
	bool Validated = true;

	if(Validated)
	{
		GLuint VertShaderName = glf::createShader(GL_VERTEX_SHADER, glf::DATA_DIRECTORY + VERT_SHADER_SOURCE);
		GLuint FragShaderName = glf::createShader(GL_FRAGMENT_SHADER, glf::DATA_DIRECTORY + FRAG_SHADER_SOURCE);

		Validated = Validated && glf::checkShader(VertShaderName, VERT_SHADER_SOURCE);
		Validated = Validated && glf::checkShader(FragShaderName, FRAG_SHADER_SOURCE);

		ProgramName = glCreateProgram();
		glAttachShader(ProgramName, VertShaderName);
		glAttachShader(ProgramName, FragShaderName);
		glDeleteShader(VertShaderName);
		glDeleteShader(FragShaderName);
		glLinkProgram(ProgramName);
		Validated = Validated && glf::checkProgram(ProgramName);
	}

	if(Validated)
	{
		UniformDiffuse = glGetUniformLocation(ProgramName, "Diffuse");
		UniformLayer = glGetUniformLocation(ProgramName, "Layer");
	}

	return Validated && glf::checkError("initProgram");
}

bool initTexture()
{
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &TextureName);

	glBindTexture(GL_TEXTURE_2D_ARRAY, TextureName);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SWIZZLE_R, GL_RED);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SWIZZLE_G, GL_GREEN);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glm::ivec2 const FramebufferSize(Window.Size / FRAMEBUFFER_SIZE);
	
	glTexImage3D(
		GL_TEXTURE_2D_ARRAY,
		0, 
		GL_RGBA8, 
		GLsizei(FramebufferSize.x),
		GLsizei(FramebufferSize.y),
		GLsizei(3), //depth
		0,  
		GL_RGB, 
		GL_UNSIGNED_BYTE, 
		NULL);

	return glf::checkError("initTexture2D");
}

bool initFramebuffer()
{
	glGenFramebuffers(1, &FramebufferName);
	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
	glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, TextureName, 0, GLint(0));
	glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, TextureName, 0, GLint(1));
	glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, TextureName, 0, GLint(2));
	
	GLenum DrawBuffers[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
	glDrawBuffers(3, DrawBuffers);
	assert(glGetError() == GL_NO_ERROR);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return false;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return glf::checkError("initFramebuffer");
}

bool initVertexArray()
{
	glGenVertexArrays(1, &VertexArrayName);
	glBindVertexArray(VertexArrayName);
	glBindVertexArray(0);

	return glf::checkError("initVertexArray");
}

bool begin()
{
	glm::ivec2 const FramebufferSize(Window.Size / FRAMEBUFFER_SIZE);
	
	Viewport[TEXTURE_R] = glm::vec4(Window.Size.x >> 1, 0, FramebufferSize);
	Viewport[TEXTURE_G] = glm::vec4(Window.Size.x >> 1, Window.Size.y >> 1, FramebufferSize);
	Viewport[TEXTURE_B] = glm::vec4(0, Window.Size.y >> 1, FramebufferSize);

	bool Validated = true;

	if(Validated)
		Validated = initProgram();
	if(Validated)
		Validated = initVertexArray();
	if(Validated)
		Validated = initTexture();
	if(Validated)
		Validated = initFramebuffer();

	return Validated && glf::checkError("begin");
}

bool end()
{
	glDeleteProgram(ProgramName);
	glDeleteTextures(1, &TextureName);
	glDeleteFramebuffers(1, &FramebufferName);

	return glf::checkError("end");
}

void display()
{
	glm::ivec2 const FramebufferSize(Window.Size / FRAMEBUFFER_SIZE);
	
	// Pass 1
	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
	glViewport(0, 0, FramebufferSize.x, FramebufferSize.y);
	glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)[0]);
	glClearBufferfv(GL_COLOR, 1, &glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)[0]);
	glClearBufferfv(GL_COLOR, 2, &glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)[0]);
	glClearBufferfv(GL_COLOR, 3, &glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)[0]);

	// Pass 2
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, Window.Size.x, Window.Size.y);
	glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)[0]);

	glUseProgram(ProgramName);
	glUniform1i(UniformDiffuse, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, TextureName);
	glBindVertexArray(VertexArrayName);

	for(GLint i = 0; i < TEXTURE_MAX; ++i)
	{
		glViewport(
			GLint(Viewport[i].x), GLint(Viewport[i].y),
			GLsizei(Viewport[i].z), GLsizei(Viewport[i].w));
		glUniform1i(UniformLayer, i);

		glDrawArraysInstanced(GL_TRIANGLES, 0, VertexCount, 1);
	}

	glf::checkError("display");

}

int main(int argc, char* argv[])
{
	return glf::run(argc, argv, glf::CORE, 4, 0);
}
