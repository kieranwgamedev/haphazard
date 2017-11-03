#version 330 core

layout (location = 0) in float type;
layout (location = 1) in vec2 pos;
layout (location = 2) in vec2 vel;
layout (location = 3) in vec2 scale;
layout (location = 4) in float rotation;
layout (location = 5) in float life;
layout (location = 6) in float maxLife;

out float PType;
out vec2 PPos;
out vec2 PVel;
out vec2 PScale;
out float PRot;
out float PLife;
out float PMaxLife;

void main()
{
	PType = type;
	PPos = pos;
	PVel = vel;
	PScale = scale;
	PRot = rotation;
	PLife = life;
	PMaxLife = maxLife;
}