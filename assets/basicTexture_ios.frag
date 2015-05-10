precision mediump float;

uniform sampler2D tex0;

varying vec2 oTexCoord;

void main() {
	gl_FragColor = texture2D(tex0, oTexCoord);
}