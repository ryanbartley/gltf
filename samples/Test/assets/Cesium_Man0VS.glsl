#version 330 core

in vec3 ciPosition;
in vec3 ciNormal;
in vec2 ciTexCoord0;
in vec4 ciBoneIndex;
in vec4 ciBoneWeight;

uniform mat4 uJointMat[18];
uniform mat3 ciNormalMatrix;
uniform mat4 ciModelViewProjection;

out vec3 vNormal;
out vec2 vTexcoord0;

void main(void) {
	mat4 skinMat = ciBoneWeight.x * uJointMat[int(ciBoneIndex.x)];
		skinMat += ciBoneWeight.y * uJointMat[int(ciBoneIndex.y)];
		skinMat += ciBoneWeight.z * uJointMat[int(ciBoneIndex.z)];
		skinMat += ciBoneWeight.w * uJointMat[int(ciBoneIndex.w)];
	gl_Position = ciModelViewProjection * skinMat * vec4(ciPosition,1.0);
	
	vNormal = ciNormalMatrix * mat3(skinMat)* ciNormal;
	vTexcoord0 = ciTexCoord0;
}
