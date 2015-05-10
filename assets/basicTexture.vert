#version 330 

in vec3 position;
in vec2 texCoord;

out vec2 oTexCoord;

uniform mat4 mvp;

void main() {
	gl_Position = mvp * vec4(position, 1.0);
	oTexCoord = texCoord;
}