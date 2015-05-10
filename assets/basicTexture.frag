#version 330

uniform sampler2D tex0;

in vec2 oTexCoord;

out vec4 oColor;

void main() {
	oColor = texture(tex0, oTexCoord);
}