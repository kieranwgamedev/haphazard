#include "GL\glew.h"
#include "Background.h"
#include "Shaders.h"

GLuint BackgroundComponent::m_UniTexBox = 0;
GLuint BackgroundComponent::m_UniTexLayer = 0;
Screen::Mesh* BackgroundComponent::m_Mesh;

static GLuint VAO, VBO;
static bool firstBackground = true;

BackgroundComponent::BackgroundComponent(Texture* texture, BACKGROUND_TYPE type, int layer)
	: m_Texture{texture}, m_Type{type}, m_Layer{layer}
{
	if (firstBackground)
	{
		firstBackground = false;

		Shaders::backgroundShader->Use();
		m_UniTexBox = glGetUniformLocation(Shaders::backgroundShader->GetProgramID(), "TexBox");
		m_UniTexLayer =glGetUniformLocation(Shaders::backgroundShader->GetProgramID(), "TexLayer");
		Texture::BindArray();
		// Make screen mesh "upside down" so sprites are rendered in correct orientation
		m_Mesh = new Screen::Mesh(1,0,0,1);
	}
}

void BackgroundComponent::SetParallax(glm::vec2 minimumPosition, glm::vec2 maximumPosition, glm::vec2 subTextureSize)
{
	m_ParallaxBounds = glm::vec4(minimumPosition, maximumPosition);
	m_SubTextureSize = subTextureSize;
}

void BackgroundComponent::Render(glm::vec2 pos)
{
	if(!m_Texture)
		return;

	glm::vec4 box;
	glm::vec2 t = m_Texture->GetBounds();

	switch (m_Type)
	{
	case BACKGROUND_TYPE::BACKGROUND:
	case BACKGROUND_TYPE::FOREGROUND:
		box.x = 0; // Lower x
		box.y = 0; // Lower y
		box.z = t.x; // Upper x
		box.w = t.y; // Upper y
		break;
	case BACKGROUND_TYPE::PARALLAX:
		// Get progress in x and y directions
		glm::vec2 p = glm::vec2( (pos.x + m_ParallaxBounds.x) / m_ParallaxBounds.z,
														 (pos.y + m_ParallaxBounds.y) / m_ParallaxBounds.w);


		if (m_ParallaxBounds.z == 0 || p.x > 1)
			p.x = 1;
		if (m_ParallaxBounds.w == 0 || p.y > 1)
			p.y = 1;
		// Linearly interpolate between minimum and maximum 

		// Lower bounds
		glm::vec2 l = glm::vec2((m_TextureXRange.x + p.x * ((m_TextureXRange.y - (m_TextureXRange.x)))) * t.x, // Percent from 0 to max
														0);

		// Upper bound (lower + offset)
		glm::vec2 u = glm::vec2(t.x * m_SubTextureSize.x + l.x,
														t.y);

		box.x = l.x; // Lower x
		box.y = l.y; // Lower y
		box.z = u.x; // Upper x
		box.w = u.y; // Upper y
		break;
	}

	//Render
	Shaders::backgroundShader->Use();
	m_Mesh->Bind();
	Texture::BindArray();
	glUniform4f(m_UniTexBox, box.x, box.y, box.z, box.w);
	glUniform1ui(m_UniTexLayer, m_Texture->GetID());
	m_Mesh->DrawTris();
}