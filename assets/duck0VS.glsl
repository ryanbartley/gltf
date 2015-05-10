#version 330

in vec3 a_position;
in vec3 a_normal;
in vec2 a_texcoord0;

uniform mat3 u_normalMatrix;
uniform mat4 u_modelViewMatrix;
uniform mat4 u_projectionMatrix;
uniform mat4 u_light0Transform;

out vec3 v_normal;
out vec2 v_texcoord0;
out vec3 v_light0Direction;

void main(void) {
	vec4 pos = u_modelViewMatrix * vec4(a_position,1.0);
	v_normal = u_normalMatrix * a_normal;
	v_texcoord0 = a_texcoord0;
	v_light0Direction = mat3(u_light0Transform) * vec3(0.,0.,1.);
	gl_Position = u_projectionMatrix * pos;
}
