#version 330 core

in vec3 vNormal;

out vec4 oColor;

uniform vec4 u_ambient = vec4( 0.2, 0.2, 0.2, 1.0 );
uniform vec4 u_emission = vec4( .6, .3, .1, 1.0 );
uniform vec4 u_specular = vec4( 1.0, 1.0, 1.0, 1.0 );
uniform float u_shininess = 128.0;

void main(void) {
	vec3 normal = normalize(vNormal);
	vec4 color = vec4(0., 0., 0., 0.);
	vec4 diffuse = vec4(.6, .6, .6, 1.);
	vec4 emission;
	vec4 ambient;
	vec4 specular;
	ambient = u_ambient;
	emission = u_emission;
	specular = u_specular;
	diffuse.xyz *= max(dot(normal,vec3(0.,0.,1.)), 0.);
	color.xyz += diffuse.xyz;
	color.xyz += emission.xyz;
	color = vec4(color.rgb * diffuse.a, diffuse.a);
	oColor = color;
}
