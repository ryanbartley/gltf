//
//  Skeleton.cpp
//  Test
//
//  Created by ryan bartley on 3/5/16.
//
//

#include "Skeleton.h"

using namespace ci;

Skeleton::Skeleton( std::vector<Joint> joints, std::vector<std::string> jointNames, const ci::mat4 &bindShapeMatrix )
: mJointArray( std::move( joints ) ), mJointNames( std::move( jointNames ) ), mBindShapeMatrix( bindShapeMatrix )
{
}

bool Skeleton::hasJoint( const std::string &name ) const
{
	auto endIt = end( mJointNames );
	return std::find( begin( mJointNames ), endIt, name ) != endIt;
}

const Skeleton::Joint* Skeleton::getJoint( const std::string &name ) const
{
	auto begIt = begin( mJointNames );
	auto endIt = end( mJointNames );
	auto jointIt = std::find( begIt, endIt, name );
	if( jointIt != endIt ) {
		auto dist = std::distance( begIt, jointIt );
		return getJoint( static_cast<uint8_t>( dist ) );
	}
	else
		return nullptr;
}

const Skeleton::Joint* Skeleton::getJoint( uint8_t id ) const
{
	CI_ASSERT( id < mJointArray.size() );
	return &mJointArray[id];
}

const std::string* Skeleton::getJointName( const Skeleton::Joint &joint ) const
{
	return getJointName( joint.getNameId() );
}

const std::string* Skeleton::getJointName( uint8_t nameId ) const
{
	CI_ASSERT( nameId < mJointNames.size() );
	return &mJointNames[nameId];
}

bool Skeleton::jointIsChildOf( uint8_t childIndex, uint8_t parentIndex ) const
{
	uint8_t currentParent = mJointArray[childIndex].getParentId();
	while ( currentParent != parentIndex && currentParent != 0xFF ) {
		currentParent = mJointArray[currentParent].getParentId();
	}
	return currentParent == parentIndex;
}

Skeleton::Anim::Anim( std::vector<TransformClip> transformClips )
{
	joints.reserve( transformClips.size() );
	for( auto &transformClip : transformClips ) {
		joints.emplace_back( transformClip );
	}
}


/*
The following renderer code is derived from code contained within the following...
 
ozz-animation is hosted at http://github.com/guillaumeblanc/ozz-animation and distributed under the MIT License (MIT).

Copyright (c) 2015 Guillaume Blanc

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

SkeletonRenderer::SkeletonRenderer()
{
	const float kInter = .2f;
	
	gl::GlslProg::Format format;
	format.vertex(
	"#ifdef ES_2\n" // If this is ES_2
	"\n"
	"precision mediump float;"
	"attribute vec3 ciPosition;\n"
	"attribute vec3 ciNormal;\n"
	"attribute vec4 ciColor;\n"
	"attribute mat4 joint;\n"
	"varying vec3 v_world_normal;\n"
	"varying vec4 v_vertex_color;\n"
	"\n"
	"#else\n" // Otherwise
	"\n"
	"in vec3 ciPosition;\n"
	"in vec3 ciNormal;\n"
	"in vec4 ciColor;\n"
	"in mat4 joint;\n"
	"out vec3 v_world_normal;\n"
	"out vec4 v_vertex_color;\n"
	"\n"
	"#endif\n" // ES_2
	"\n"
	"uniform mat4 ciModelViewProjection;\n"
	"\n"
	"#ifdef JOINT_SHADER\n" // Joint Shader math
	"\n"
	"mat4 GetWorldMatrix() {\n"
	"  // Rebuilds joint matrix.\n"
	"  mat4 joint_matrix;\n"
	"  joint_matrix[0] = vec4(normalize(joint[0].xyz), 0.);\n"
	"  joint_matrix[1] = vec4(normalize(joint[1].xyz), 0.);\n"
	"  joint_matrix[2] = vec4(normalize(joint[2].xyz), 0.);\n"
	"  joint_matrix[3] = vec4(joint[3].xyz, 1.);\n"
	
	"  // Rebuilds bone properties.\n"
	"  vec3 bone_dir = vec3(joint[0].w, joint[1].w, joint[2].w);\n"
	"  float bone_len = length(bone_dir);\n"
	
	"  // Setup rendering world matrix.\n"
	"  mat4 world_matrix;\n"
	"  world_matrix[0] = joint_matrix[0] * bone_len;\n"
	"  world_matrix[1] = joint_matrix[1] * bone_len;\n"
	"  world_matrix[2] = joint_matrix[2] * bone_len;\n"
	"  world_matrix[3] = joint_matrix[3];\n"
	"  return world_matrix;\n"
	"}\n"
	"\n"
	"#else\n" // Bone Shader math
	"\n"
	"mat4 GetWorldMatrix() {\n"
	"  // Rebuilds bone properties.\n"
	"  // Bone length is set to zero to disable leaf rendering.\n"
	"  float is_bone = joint[3].w;\n"
	"  vec3 bone_dir = vec3(joint[0].w, joint[1].w, joint[2].w) * is_bone;\n"
	"  float bone_len = length(bone_dir);\n"
	
	"  // Setup rendering world matrix.\n"
	"  float dot = dot(joint[2].xyz, bone_dir);\n"
	"  vec3 binormal = abs(dot) < .01 ? joint[2].xyz : joint[1].xyz;\n"
	
	"  mat4 world_matrix;\n"
	"  world_matrix[0] = vec4(bone_dir, 0.);\n"
	"  world_matrix[1] = \n"
	"    vec4(bone_len * normalize(cross(binormal, bone_dir)), 0.);\n"
	"  world_matrix[2] =\n"
	"    vec4(bone_len * normalize(cross(bone_dir, world_matrix[1].xyz)), 0.);\n"
	"  world_matrix[3] = vec4(joint[3].xyz, 1.);\n"
	"  return world_matrix;\n"
	"}\n"
	"\n"
	"#endif\n" // Joint Shader
	"\n"
	"void main() {\n"
	"  mat4 world_matrix = GetWorldMatrix();\n"
	"  vec4 vertex = vec4(ciPosition.xyz, 1.);\n"
	"  gl_Position = ciModelViewProjection * world_matrix * vertex;\n"
	"  mat3 cross_matrix = mat3(\n"
	"    cross(world_matrix[1].xyz, world_matrix[2].xyz),\n"
	"    cross(world_matrix[2].xyz, world_matrix[0].xyz),\n"
	"    cross(world_matrix[0].xyz, world_matrix[1].xyz));\n"
	"  float invdet = 1.0 / dot(cross_matrix[2], world_matrix[2].xyz);\n"
	"  mat3 normal_matrix = cross_matrix * invdet;\n"
	"  v_world_normal = normal_matrix * ciNormal;\n"
	"  v_vertex_color = ciColor;\n"
	"}\n" )
	.fragment(
	"precision mediump float;\n"
	"vec3 lerp(in vec3 alpha, in vec3 a, in vec3 b) {\n"
	"  return a + alpha * (b - a);\n"
	"}\n"
	"vec4 lerp(in vec4 alpha, in vec4 a, in vec4 b) {\n"
	"  return a + alpha * (b - a);\n"
	"}\n"
	"#ifdef ES_2\n"
	"varying vec3 v_world_normal;\n"
	"varying vec4 v_vertex_color;\n"
	"#else\n"
	"in vec3 v_world_normal;\n"
	"in vec4 v_vertex_color;\n"
	"out vec4 oColor;\n"
	"#endif\n"
	"void main() {\n"
	"  vec3 normal = normalize(v_world_normal);\n"
	"  vec3 alpha = (normal + 1.) * .5;\n"
	"  vec4 bt = lerp(\n"
	"    alpha.xzxz, vec4(.3, .3, .7, .7), vec4(.4, .4, .8, .8));\n"
	"  vec4 finalColor = vec4(\n"
	"     lerp(alpha.yyy, vec3(bt.x, .3, bt.y), vec3(bt.z, .8, bt.w)), 1.);\n"
	"  finalColor *= v_vertex_color;\n"
	"\n"
	"#ifdef ES_2\n"
	"  gl_FragColor = finalColor;\n"
	"#else\n"
	"  oColor = finalColor;\n"
	"#endif\n"
	"}\n" ).attrib( geom::CUSTOM_0, "joint" ).preprocess( true );
	
#if CINDER_GL_ES_2
	format.define("ES_2");
#endif
	
	auto matrixPaletteLayout = geom::BufferLayout({
		geom::AttribInfo( geom::CUSTOM_0, 16, sizeof(mat4), 0, 1 )
	});
	
	matrixPaletteVbo = ci::gl::Vbo::create( GL_ARRAY_BUFFER, GL_STREAM_DRAW );
	
	// Prepare bone mesh

	std::array<ci::vec3, 6> pos= {
		ci::vec3(1.f, 0.f, 0.f),
		ci::vec3(kInter, .1f, .1f),
		ci::vec3(kInter, .1f, -.1f),
		ci::vec3(kInter, -.1f, -.1f),
		ci::vec3(kInter, -.1f, .1f),
		ci::vec3(0.f, 0.f, 0.f)};
	std::array<ci::vec3, 8> normals = {
		ci::normalize(ci::cross(pos[2] - pos[1], pos[2] - pos[0])),
		ci::normalize(ci::cross(pos[1] - pos[2], pos[1] - pos[5])),
		ci::normalize(ci::cross(pos[3] - pos[2], pos[3] - pos[0])),
		ci::normalize(ci::cross(pos[2] - pos[3], pos[2] - pos[5])),
		ci::normalize(ci::cross(pos[4] - pos[3], pos[4] - pos[0])),
		ci::normalize(ci::cross(pos[3] - pos[4], pos[3] - pos[5])),
		ci::normalize(ci::cross(pos[1] - pos[4], pos[1] - pos[0])),
		ci::normalize(ci::cross(pos[4] - pos[1], pos[4] - pos[5]))};
	std::array<ci::vec3, 48> bones = {
		pos[0], normals[0], pos[2], normals[0],
		pos[1], normals[0], pos[5], normals[1],
		pos[1], normals[1], pos[2], normals[1],
		pos[0], normals[2], pos[3], normals[2],
		pos[2], normals[2], pos[5], normals[3],
		pos[2], normals[3], pos[3], normals[3],
		pos[0], normals[4], pos[4], normals[4],
		pos[3], normals[4], pos[5], normals[5],
		pos[3], normals[5], pos[4], normals[5],
		pos[0], normals[6], pos[1], normals[6],
		pos[4], normals[6], pos[5], normals[7],
		pos[4], normals[7], pos[1], normals[7]};
	
	// Note there's no define here for Joint next we'll define joint
	auto boneGlsl = gl::GlslProg::create( format );
	
	auto boneBufferLayout = geom::BufferLayout({
		geom::AttribInfo( geom::Attrib::POSITION, 3, sizeof(ci::vec3) * 2, 0, 0 ),
		geom::AttribInfo( geom::Attrib::NORMAL, 3, sizeof(ci::vec3) *2, sizeof(ci::vec3), 0 ),
	});
	auto boneVbo = ci::gl::Vbo::create( GL_ARRAY_BUFFER, sizeof(ci::vec3) * bones.size(), bones.data(), GL_STATIC_DRAW );
	
	auto boneVboMesh = ci::gl::VboMesh::create( 24, GL_TRIANGLES,
	{ { boneBufferLayout, boneVbo }, { matrixPaletteLayout, matrixPaletteVbo} } );
	boneJointBatches[0] = ci::gl::Batch::create( boneVboMesh, boneGlsl );
	
	
	// Prepares joint mesh.
	
	const int kNumSlices = 20;
	const int kNumPointsPerCircle = kNumSlices + 1;
	const int kNumPointsYZ = kNumPointsPerCircle;
	const int kNumPointsXY = kNumPointsPerCircle + kNumPointsPerCircle / 4;
	const int kNumPointsXZ = kNumPointsPerCircle;
	const int kNumPoints = kNumPointsXY + kNumPointsXZ + kNumPointsYZ;
	const float kRadius = kInter;  // Radius multiplier.
	const ci::Color red{0xff, 0xc0, 0xc0};
	const ci::Color green{0xc0, 0xff, 0xc0};
	const ci::Color blue{0xc0, 0xc0, 0xff};
	
	struct JointMesh {
		ci::vec3 pos, normal;
		ci::Color color;
	};
	std::array<JointMesh, kNumPoints> joints;
	
	// Fills vertices.
	float angleIncrement = static_cast<float>(M_PI * 2) / kNumSlices;
	int index = 0;
	for (int j = 0; j < kNumPointsYZ; ++j) { // YZ plan.
		float angle = j * angleIncrement;
		float s = sinf(angle), c = cosf(angle);
		JointMesh& vertex = joints[index++];
		vertex.pos = {0.f, c * kRadius, s * kRadius};
		vertex.normal = {0.f, c, s};
		vertex.color = red;
	}
	for (int j = 0; j < kNumPointsXY; ++j) { // XY plan.
		float angle = j * angleIncrement;
		float s = sinf(angle), c = cosf(angle);
		JointMesh& vertex = joints[index++];
		vertex.pos = {s * kRadius, c * kRadius, 0.f};
		vertex.normal = {s, c, 0.f};
		vertex.color = blue;
	}
	for (int j = 0; j < kNumPointsXZ; ++j) { // XZ plan.
		float angle = j * angleIncrement;
		float s = sinf(angle), c = cosf(angle);
		JointMesh& vertex = joints[index++];
		vertex.pos = {c * kRadius, 0.f, -s * kRadius};
		vertex.normal = {c, 0.f, -s};
		vertex.color = green;
	}
	CI_ASSERT(index == kNumPoints);
	
	format.define("JOINT_SHADER");
	
	auto jointGlsl = ci::gl::GlslProg::create( format );
	
	auto jointBufferLayout = geom::BufferLayout({
		geom::AttribInfo( geom::Attrib::POSITION, 3, sizeof(JointMesh), 0, 0 ),
		geom::AttribInfo( geom::Attrib::NORMAL, 3, sizeof(JointMesh), offsetof(JointMesh, normal), 0 ),
		geom::AttribInfo( geom::Attrib::COLOR, 3, sizeof(JointMesh), offsetof(JointMesh, color), 0 )
	});
	auto jointVbo = ci::gl::Vbo::create( GL_ARRAY_BUFFER, sizeof(JointMesh) * joints.size(), joints.data(), GL_STATIC_DRAW );
	
	auto jointVboMesh = ci::gl::VboMesh::create( kNumPoints, GL_LINE_STRIP,
	{ { jointBufferLayout, jointVbo }, { matrixPaletteLayout, matrixPaletteVbo} } );
	boneJointBatches[1] = ci::gl::Batch::create( jointVboMesh, jointGlsl );
}

void SkeletonRenderer::draw( const Skeleton &skeleton )
{
	std::vector<ci::mat4> finalPose;
	for( const auto &joint : skeleton.getJoints() ) {
		finalPose.emplace_back( ci::inverse( joint.getInverseBindMatrix() ) );
	}
	draw( skeleton, finalPose );
}

void SkeletonRenderer::draw( const Skeleton &skeleton, const std::vector<ci::mat4> &finalPose )
{
	if ( finalPose.empty() || finalPose.size() < skeleton.getNumJoints() )
		return;
	
	// Prepares computation constants.
	const auto &joints = skeleton.getJoints();
	
	// Convert matrices to uniforms.
	auto bufferSizeNeeded = skeleton.getNumJoints() * sizeof(ci::mat4);
	
	auto matrixBuffer = std::unique_ptr<float[]>( new float[(bufferSizeNeeded/sizeof(float))] );
	auto uniforms = matrixBuffer.get();
	
	int instances = 0;
	auto jointSize = static_cast< uint32_t >( joints.size() );
	for (uint32_t i = 1; i < jointSize; ++i) {
		
		// Selects joint matrices.
		const ci::mat4& parent = finalPose[joints[i].getParentId()];
		const ci::mat4& current = finalPose[i];
		
		// Copy parent joint's raw matrix, to render a bone between the parent
		// and current matrix.
		float* uniform = uniforms + instances * 16;
		memcpy( uniform, glm::value_ptr( parent ), sizeof( ci::mat4 ) );
		
		// Set bone direction (bone_dir). The shader expects to find it at index
		// [3,7,11] of the matrix.
		// Index 15 is used to store whether a bone should be rendered,
		// otherwise it's a leaf.
		auto bone_dir = current[3] - parent[3];
		uniform[3] = bone_dir[0];
		uniform[7] = bone_dir[1];
		uniform[11] = bone_dir[2];
		uniform[15] = 1.f;  // Enables bone rendering.
		
		// Next instance.
		++instances;
		uniform += 16;
	}
	
	if( matrixPaletteVbo->getSize() < bufferSizeNeeded )
		matrixPaletteVbo->bufferData( bufferSizeNeeded, uniforms, GL_STREAM_DRAW );
	else
		matrixPaletteVbo->bufferSubData( 0, bufferSizeNeeded, uniforms );
	
	for( auto & batch : boneJointBatches )
		batch->drawInstanced( instances );
}






