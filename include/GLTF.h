//
//  GLTF.h
//  GLTFWork
//
//  Created by Ryan Bartley on 6/19/14.
//
//

#pragma once

#include "cinder/Json.h"
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
	Gltf( const std::string &fileName, bool cache = false );
	
	void load( bool cache );
	void makeGlReady();
	
	static std::vector<std::pair<std::string, void(Gltf::*)(const ci::JsonTree &)>> kSlugs;
	
	ci::gl::BatchRef	getBatchFromMeshByName( const std::string &name );
	ci::gl::VboMeshRef	getVboMeshFromMeshByName( const std::string &name );
	ci::TriMeshRef		getTriMeshFromMeshByName( const std::string &name );
	ci::gl::GlslProgRef	getGlslProgramFromMaterial( const std::string &name );
	ci::gl::TextureRef	getTextureByName( const std::string &name );
	ci::CameraPersp		getPerspCameraByName( const std::string &name );
	ci::CameraOrtho		getOrthoCameraByName( const std::string &name );
	Accessor			getAccessorInfo( const std::string &key ) const;
	BufferView			getBufferViewInfo( const std::string &key ) const;
	Buffer				getBufferInfo( const std::string &key ) const;
	Mesh                getMeshInfo( const std::string &key ) const;
	Program				getProgramInfo( const std::string &key ) const;
	Shader				getShaderInfo( const std::string &key ) const;
	Sampler				getSamplerInfo( const std::string &key ) const;
	Image				getImageInfo( const std::string &key ) const;
	Texture				getTextureInfo( const std::string &key ) const;
	Camera				getCameraInfo( const std::string &key ) const;
	Technique			getTechniqueInfo( const std::string &key ) const;
	Material			getMaterialInfo( const std::string &key ) const;
	Node				getNodeInfo( const std::string &key ) const;
	Scene				getSceneInfo( const std::string &key ) const;
	
	void				render( const std::string &scene );
	
	static ci::geom::Primitive convertPrimitive( GLenum primitive )
	{
		switch (primitive) {
			case GL_LINES: return ci::geom::LINES; break;
			case GL_LINE_STRIP:	return ci::geom::LINE_STRIP; break;
			case GL_TRIANGLES: return ci::geom::TRIANGLES; break;
			case GL_TRIANGLE_STRIP: return ci::geom::TRIANGLE_STRIP; break;
			case GL_TRIANGLE_FAN: return ci::geom::TRIANGLE_FAN; break;
			default: CI_LOG_E("Don't know this primitive"); return (ci::geom::Primitive)-1; break;
		}
	}
	
	//! Returns the converted string as a geom::Attrib. Attribute semantics include POSITION, NORMAL, TEXCOORD, COLOR, JOINT, JOINTMATRIX, and WEIGHT.  Attribute semantics can be of the form [semantic]_[set_index], e.g, TEXCOORD_0, TEXCOORD_1, etc."
	static ci::geom::Attrib getAttribEnum( const std::string &attrib )
	{
		using namespace ci::geom;
		if( attrib == "POSITION" )			return Attrib::POSITION;
		else if( attrib == "NORMAL" )		return Attrib::NORMAL;
		else if( attrib == "TEXCOORD_0" )	return Attrib::TEX_COORD_0;
		else if( attrib == "TEXCOORD_1" )	return Attrib::TEX_COORD_1;
		else if( attrib == "TEXCOORD_2" )	return Attrib::TEX_COORD_2;
		else if( attrib == "TEXCOORD_3" )	return Attrib::TEX_COORD_3;
		else if( attrib == "COLOR" )		return Attrib::COLOR;
		else if( attrib == "JOINT" )		return Attrib::BONE_INDEX;
		else if( attrib == "JOINTMATRIX" ) {
			CI_LOG_E( "UNDEFINED Attib JOINTMATRIX Using CUSTOM_0" );
			return Attrib::CUSTOM_0;
		}
		else if( attrib == "WEIGHT" )		return Attrib::BONE_WEIGHT;
		else								return Attrib::NUM_ATTRIBS;
	}
	
	static ci::gl::UniformSemantic getUniformEnum( const std::string &uniform )
	{
		auto & u = uniform;
		using namespace ci::gl;
		if( uniform == "MODEL" )				return UniformSemantic::UNIFORM_MODEL_MATRIX;
		else if( u == "VIEW" )					return UniformSemantic::UNIFORM_VIEW_MATRIX;
		else if( u == "PROJECTION" )			return UniformSemantic::UNIFORM_PROJECTION_MATRIX;
		else if( u == "MODELVIEW" )				return UniformSemantic::UNIFORM_MODEL_VIEW;
		else if( u == "MODELVIEWPROJECTION" )	return UniformSemantic::UNIFORM_MODEL_VIEW_PROJECTION;
		else if( u == "MODELINVERSE" )			return UniformSemantic::UNIFORM_MODEL_MATRIX_INVERSE;
		else if( u == "VIEWINVERSE" )			return UniformSemantic::UNIFORM_VIEW_MATRIX_INVERSE;
		else if( u == "PROJECTIONINVERSE" )		return UniformSemantic::UNIFORM_PROJECTION_MATRIX_INVERSE;
		else if( u == "MODELVIEWINVERSE" )		return UniformSemantic::UNIFORM_MODEL_VIEW;
		else if( u == "MODELVIEWPROJECTIONINVERSE" ) return UniformSemantic::UNIFORM_MODEL_VIEW_PROJECTION;
		else if( u == "MODELINVERSETRANSPOSE" ) return UniformSemantic::UNIFORM_MODEL_MATRIX_INVERSE;
		else if( u == "MODELVIEWINVERSETRANSPOSE" ) return UniformSemantic::UNIFORM_MODEL_VIEW_INVERSE_TRANSPOSE;
		else if( u == "VIEWPORT" )				return UniformSemantic::UNIFORM_VIEWPORT_MATRIX;
		else return (UniformSemantic)-1;
	}
	
	static uint8_t getNumComponentsForType( const std::string &type )
	{
		auto typeC = type.c_str(); auto size = type.size();
		if( ! strncmp( typeC, "SCALAR", size ) )	return 1;
		else if( ! strncmp( typeC, "VEC2", size ) ) return 2;
		else if( ! strncmp( typeC, "VEC3", size ) ) return 3;
		else if( ! strncmp( typeC, "VEC4", size ) ) return 4;
		else if( ! strncmp( typeC, "MAT2", size ) ) return 4;
		else if( ! strncmp( typeC, "MAT3", size ) ) return 9;
		else if( ! strncmp( typeC, "MAT4", size ) ) return 16;
		else										return 0;
	}
	
	static uint8_t getNumBytesForComponentType( GLuint type )
	{
		switch (type) {
			case 5120: // BYTE
			case 5121: // UNSIGNED_BYTE
				return 1;
				break;
			case 5122: // SHORT
			case 5123: // UNSIGNED_SHORT
				return 2;
				break;
			case 5126: // FLOAT
				return 4;
				break;
			default: {
				CI_LOG_E("ERROR: That enum doesn't have a dimmension/size.");
				return 0;
			}
				break;
		}
	}
	
private:
	
	
	
	std::string						mFileName;
	ci::JsonTree					mTree;
};
	
}
