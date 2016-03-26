#version 330 core

in vec3 vNormal;
in vec2 vTexcoord0;

uniform sampler2D uDiffuse;

out vec4 oColor;

//uniform vec4 u_ambient;
//uniform vec4 u_emission;
//uniform vec4 u_specular;
//uniform float u_shininess;

void main(void) {
//	vec3 normal = normalize(v_normal);
//	vec4 color = vec4(0., 0., 0., 0.);
//	vec4 diffuse = vec4(0., 0., 0., 1.);
//	vec4 emission;
//	vec4 ambient;
//	vec4 specular;
//	ambient = u_ambient;
//	diffuse.rgb = texture(u_diffuse, v_texcoord0).rgb;
//	emission = u_emission;
//	specular = u_specular;
//	diffuse.xyz *= max(dot(normal,vec3(0.,0.,1.)), 0.);
//	color.xyz += diffuse.xyz;
//	color.xyz += emission.xyz;
//	color = vec4(color.rgb * diffuse.a, diffuse.a);
	oColor = vec4( 1 );
}
