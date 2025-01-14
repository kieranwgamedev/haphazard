/*
FILE: screenBlur.frag
PRIMARY AUTHOR: Max Rauffer

Copyright (c) 2017 DigiPen (USA) Corporation.
*/
#version 330 core

in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D screenTexture;

uniform bool Horizontal;
uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

uniform float Intensity;

void main()
{
    vec2 tex_offset = Intensity / textureSize(screenTexture, 0); // gets size of single texel
    vec4 result = texture(screenTexture, TexCoord) * weight[0]; // current fragment's contribution
    if(Horizontal)
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(screenTexture, TexCoord + vec2(tex_offset.x * i, 0.0)) * weight[i];
            result += texture(screenTexture, TexCoord - vec2(tex_offset.x * i, 0.0)) * weight[i];
        }
    }
    else
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(screenTexture, TexCoord + vec2(0.0, tex_offset.y * i)) * weight[i];
            result += texture(screenTexture, TexCoord - vec2(0.0, tex_offset.y * i)) * weight[i];
        }
    }
    FragColor = vec4(result);
}