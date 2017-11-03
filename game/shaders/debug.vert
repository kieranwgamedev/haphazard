#version 330 core
layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 scale;
layout (location = 2) in float rotation;
layout (location = 3) in vec4 color;

out VS_OUT {
	vec4 Offsets[4];
	vec2 Scale;
	vec4 Color;
} vs_out;

layout (std140) uniform Matrices
{
	uniform mat4 view;
	uniform mat4 proj;
};

void main()
{
	float angle = rotation;

	mat4 s, t, r;
	s[0] = vec4( scale.x, 0, 0, 0); // Col 1
	s[1] = vec4( 0, scale.y, 0, 0); // Col 2
	s[2] = vec4( 0, 0, 1, 0); // Col 3
	s[3] = vec4( 0, 0, 0, 1); // Col 4

	t[0][0] = t[1][1] = t[2][2] = t[3][3] = 1;
	t[3][0] = pos.x; t[3][1] = pos.y;

	r[0] = vec4(cos(angle), sin(angle),0,0); // Col 1
	r[1] = vec4(-sin(angle), cos(angle),0,0); // Col 1
	r[2] = vec4(0,0,1,0);
	r[3] = vec4(0,0,0,1);
	
	mat4 model = t * r * s;


    mat4 PVM = proj * view * model;
	vs_out.Offsets[0] = PVM * vec4(-0.5, -0.5, 0.0, 1.0);
	vs_out.Offsets[1] = PVM * vec4(-0.5, 0.5, 0.0, 1.0);
	vs_out.Offsets[2] = PVM * vec4(0.5, 0.5, 0.0, 1.0);
	vs_out.Offsets[3] = PVM * vec4(0.5, -0.5, 0.0, 1.0);
	
	vs_out.Color = color;
}