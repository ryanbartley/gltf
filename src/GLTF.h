//
//  GLTF.h
//  GLTFWork
//
//  Created by Ryan Bartley on 6/19/14.
//
//

#pragma once

#include "jsoncpp/json.h"
#include "cinder/Text.h"
#include "cinder/Utilities.h"

#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/VboMesh.h"
#include "cinder/params/Params.h"

#include "GltfContainer.h"

#include "cinder/gl/Batch.h"
#include "cinder/TriMesh.h"
#include "cinder/Log.h"

namespace gltf {

class Gltf {
public:
	Gltf( const ci::DataSourceRef &gltfFile );
	
	Accessor			getAccessorInfo( const std::string &key ) const;
	Animation			getAnimationInfo( const std::string &key ) const;
	Asset				getAssetInfo() const;
	BufferView			getBufferViewInfo( const std::string &key ) const;
	Buffer				getBufferInfo( const std::string &key ) const;
	Camera				getCameraInfo( const std::string &key ) const;
	Image				getImageInfo( const std::string &key ) const;
	Material			getMaterialInfo( const std::string &key ) const;
	Mesh                getMeshInfo( const std::string &key ) const;
	Node				getNodeInfo( const std::string &key ) const;
	Program				getProgramInfo( const std::string &key ) const;
	Sampler				getSamplerInfo( const std::string &key ) const;
	Scene				getSceneInfo( const std::string &key ) const;
	Shader				getShaderInfo( const std::string &key ) const;
	Skin				getSkinInfo( const std::string &key ) const;
	Technique			getTechniqueInfo( const std::string &key ) const;
	Texture				getTextureInfo( const std::string &key ) const;
	
	ci::CameraPersp		getPerspCameraByName( const std::string &name );
	ci::CameraOrtho		getOrthoCameraByName( const std::string &name );
	
	//! Returns the converted string as a geom::Attrib. Attribute semantics
	//! include POSITION, NORMAL, TEXCOORD, COLOR, JOINT, JOINTMATRIX, and
	//! WEIGHT.  Attribute semantics can be of the form [semantic]_[set_index],
	//! e.g, TEXCOORD_0, TEXCOORD_1, etc."
	static ci::geom::Attrib getAttribEnum( const std::string &attrib );
	static ci::geom::Primitive convertToPrimitive( GLenum primitive );
	static ci::gl::UniformSemantic getUniformEnum( const std::string &uniform );
	static uint8_t getNumComponentsForType( const std::string &type );
	static uint8_t getNumBytesForComponentType( GLuint type );
	ci::BufferRef getBufferFromUri( const std::string &uri, std::string *retUri ) const;
	
private:
	Json::Value			mTree;
	cinder::fs::path	mGltfPath;
	std::vector<std::string> mExtensions;
};
	
namespace gl {
	
ci::gl::BatchRef	getBatchFromMeshByName( const Gltf &gltf, const std::string &name );
ci::gl::VboMeshRef	getVboMeshFromMeshByName( const Gltf &gltf, const std::string &name );
ci::gl::GlslProgRef	getGlslProgramFromMaterial( const Gltf &gltf, const std::string &name );
ci::gl::TextureRef	getTextureByName( const Gltf &gltf, const std::string &name );
ci::TriMeshRef		getTriMeshFromMeshByName( const Gltf &gltf, const std::string &name );
	
} } // namespace gl // namespace gltf
