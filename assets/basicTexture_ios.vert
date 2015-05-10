attribute vec3 position;
attribute vec2 texCoord;

varying vec2 oTexCoord;

uniform mat4 mvp;

void main() {
	gl_Position = mvp * vec4(position, 1.0);
	oTexCoord = texCoord;
}