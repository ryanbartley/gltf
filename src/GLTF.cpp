//
//  GLTF.cpp
//  GLTFWork
//
//  Created by Ryan Bartley on 6/19/14.
//
//

#include "GLTF.h"

#include "cinder/gl/Vbo.h"
#include "cinder/Utilities.h"
#include "cinder/Log.h"
#include "cinder/DataSource.h"
#include "cinder/Base64.h"
#include "cinder/Log.h"

using namespace ci;
using namespace ci::app;
using namespace ci::gl;
using namespace std;

namespace gltf {
	
File::File( const ci::DataSourceRef &gltfFile )
: mGltfPath( gltfFile->getFilePath().parent_path() )
{
	Json::Features features;
	features.allowComments_ = true;
	features.strictRoot_ = true;
	Json::Reader reader( features );
	try {
		reader.parse( loadString( gltfFile ), mTree );
	}
	catch ( const std::runtime_error &e ) {
		CI_LOG_E( "Error parsing gltf file " << e.what() );
	}
	
	
}

Scene::Scene( const FileRef &file, const std::string &sceneName )
{
	
}

Accessor Scene::getAccessorInfo( const std::string& key ) const
{
	auto &accessors = mTree["accessors"];
	auto &accessor = accessors[key];
	
	// Required points.
	CI_ASSERT( accessor["bufferView"].isString() );
	CI_ASSERT( accessor["byteOffset"].isNumeric() );
	CI_ASSERT( accessor["componentType"].isNumeric() );
	CI_ASSERT( accessor["type"].isString() );
	CI_ASSERT( accessor["count"].isNumeric() );
	
	Accessor ret;
	ret.bufferView	= accessor["bufferView"].asString();
	ret.byteOffset	= accessor["byteOffset"].asUInt();
	ret.count		= accessor["count"].asUInt();
	ret.type		= accessor["type"].asString();
	ret.componentType = accessor["componentType"].asUInt();
	ret.name		= accessor["name"].asString();
	ret.extras		= accessor["extras"];
	
	if( ! accessor["byteStride"].isNull() )
		ret.byteStride	= accessor["byteStride"].asUInt();
	
	auto &maxElem = accessor["max"];
	auto &minElem = accessor["min"];
	if( ! maxElem.isNull() && ! minElem.isNull() ) {
		auto maxSize = maxElem.size();
		auto minSize = minElem.size();
		CI_ASSERT(maxSize == minSize);
		ret.max.resize( maxSize );
		ret.min.resize( minSize );
		for( int i = 0; i < maxSize; i++ ) {
			ret.max[i] = maxElem[i].asFloat();
			ret.min[i] = minElem[i].asFloat();
		}
	}
	
	return ret;
}
	
Animation Scene::getAnimationInfo( const std::string &key ) const
{
	auto &animations = mTree["animations"];
	auto &animation = animations[key];
	
	Animation ret;
	auto &channels = animation["channels"];
	for( auto &channel : channels ) {
		CI_ASSERT( channel["sampler"].isString() );
		Animation::Channel animChannel;
		animChannel.sampler = channel["sampler"].asString();
		
		CI_ASSERT( channel["target"].isObject() );
		auto &target = channel["target"];
		
		CI_ASSERT( target["id"].isString() );
		CI_ASSERT( target["path"].isString() );
		animChannel.targetId = target["id"].asString();
		animChannel.targetPath = target["path"].asString();
		ret.channels.emplace_back( move( animChannel ) );
		
		animChannel.channelExtras = channel["extras"];
		animChannel.targetExtras = channel["extras"];
	}
	
	auto &samplers = animation["samplers"];
	for( auto &sampler : samplers ) {
		Animation::Sampler animSampler;
		CI_ASSERT( sampler["input"].isString() );
		animSampler.input = sampler["input"].asString();
		CI_ASSERT( sampler["output"].isString() );
		animSampler.output = sampler["output"].asString();
		if( sampler["interpolation"].isString() )
			animSampler.interpolation = sampler["interpolation"].asString();
	}
	
	ret.name = animation["name"].asString();
	ret.parameters = animation["parameters"];
	ret.extras = animation["extras"];
	
	return ret;
}
	
Asset Scene::getAssetInfo() const
{
	auto &assetInfo = mTree["asset"];
	Asset ret;
	
	CI_ASSERT( assetInfo["version"].isString() );
	ret.version = assetInfo["version"].asString();
	
	if( assetInfo["profile"].isObject() ) {
		if( assetInfo["profile"]["api"].isString() )
			ret.profile.api = assetInfo["profile"]["api"].asString();
		if( assetInfo["profile"]["version"].isString() )
			ret.profile.version = assetInfo["profile"]["version"].asString();
	}
	
	ret.copyright = assetInfo["copyright"].asString();
	ret.generator = assetInfo["generator"].asString();
	ret.premultipliedAlpha = assetInfo["premultipliedAlpha"].asBool();
	
	return ret;
}
	
Buffer Scene::getBufferInfo( const std::string &name ) const
{
	auto &buffers = mTree["buffers"];
	auto &buffer = buffers[name];
	
	CI_ASSERT( buffer["uri"].isString() );
	
	Buffer ret;
	auto uri = buffer["uri"].asString();
	
	if( auto pos = uri.find("data:application/octet-stream;base64,") != std::string::npos ) {
		auto data = uri.substr( pos + 1, uri.size() );
		auto buffer = fromBase64( data );
		ret.data = BufferRef( new ci::Buffer( std::move( buffer ) ) );
	}
	else {
		ret.data = loadFile( mGltfPath / uri )->getBuffer();
	}
	
	ret.type = buffer["type"].asString();
	ret.byteLength = buffer["byteLength"].asUInt();
	ret.extras = buffer["extras"];
	ret.name = buffer["name"].asString();
	
	return ret;
}

BufferView Scene::getBufferViewInfo( const std::string &name ) const
{
	auto &bufferViews = mTree["bufferViews"];
	auto &bufferView = bufferViews[name];
	
	CI_ASSERT( bufferView["byteOffset"].isString() );
	CI_ASSERT( bufferView["buffer"].isString() );
	
	BufferView ret;
	
	ret.buffer = bufferView["buffer"].asString();
	ret.byteOffset = bufferView["byteOffset"].asUInt();
	ret.byteLength = bufferView["byteLength"].asUInt();
	ret.target = bufferView["target"].asUInt();
	ret.name = bufferView["name"].asString();
	ret.extras = bufferView["extras"];
	
	return ret;
}
	
Camera Scene::getCameraInfo( const std::string &key ) const
{
	auto &cameras = mTree["cameras"];
	auto &camera = cameras[key];
	
	CI_ASSERT( camera["type"].isString() );
	
	Camera ret;
	ret.type = camera["type"].asString();
	ret.name = camera["name"].asString();
	if ( ret.type == "perspective" ) {
		auto &perspectiveInfo = camera["perspective"];
		
		CI_ASSERT( perspectiveInfo["yfov"].isNumeric() );
		CI_ASSERT( perspectiveInfo["znear"].isNumeric() );
		CI_ASSERT( perspectiveInfo["zfar"].isNumeric() );
		
		ret.aspectRatio = perspectiveInfo["aspectRatio"].asFloat();
		ret.yfov = perspectiveInfo["yfov"].asFloat();
		ret.znear = perspectiveInfo["znear"].asFloat();
		ret.zfar = perspectiveInfo["zfar"].asFloat();
		ret.perspExtras = perspectiveInfo["extras"];
	}
	else if( ret.type == "orthographic" ) {
		auto &orthographicInfo = camera["orthographic"];
		
		CI_ASSERT( orthographicInfo["xmag"].isNumeric() );
		CI_ASSERT( orthographicInfo["ymag"].isNumeric() );
		CI_ASSERT( orthographicInfo["znear"].isNumeric() );
		CI_ASSERT( orthographicInfo["zfar"].isNumeric() );
		
		ret.xmag = orthographicInfo["xmag"].asFloat();
		ret.ymag = orthographicInfo["ymag"].asFloat();
		ret.znear = orthographicInfo["znear"].asFloat();
		ret.zfar = orthographicInfo["zfar"].asFloat();
		ret.orthoExtras = orthographicInfo["extras"];
	}
	ret.extras = camera["extras"];
	
	return ret;
}
	
Image Scene::getImageInfo( const std::string &key ) const
{
	auto &images = mTree["images"];
	auto &image = images[key];
	
	CI_ASSERT( image["uri"].isString() );
	
	Image ret;
	ret.uri = image["uri"].asString();
	ret.name = image["name"].asString();
	
	ret.imageSource = loadImage( loadFile( mGltfPath / ret.uri ) );
	
	return ret;
}
	
Material Scene::getMaterialInfo( const std::string &key ) const
{
	auto &materials = mTree["materials"];
	auto &material = materials[key];
	
	Material ret;
	ret.name = material["name"].asString();
	ret.technique = material["technique"].asString();
	ret.values = material["values"];
	ret.extras = material["extras"];
	
	return ret;
}
	
Mesh Scene::getMeshInfo( const std::string &key ) const
{
	auto &meshes = mTree["meshes"];
	auto &mesh = meshes[key];
	
	Mesh ret;
	ret.name = mesh["name"].asString();
	for( auto &primitive : mesh["primitives"] ) {
		CI_ASSERT( primitive["material"].isString() );
		
		Mesh::Primitive meshPrim;
		meshPrim.material = primitive["material"].asString();
		meshPrim.indices = primitive["indices"].asString();
		meshPrim.primitive = primitive["primitives"].asUInt();
		meshPrim.extras = primitive["extras"];
		
		auto &attributes = primitive["attributes"];
		auto attribNames = attributes.getMemberNames();
		for( int i = 0; i < attribNames.size(); i++ ) {
			auto &attribName = attribNames[i];
			Mesh::Primitive::AttribAccessor attrib;
			attrib.attrib = getAttribEnum( attribName );
			attrib.accessor = attributes[attribName].asString();
			meshPrim.attributes.emplace_back( move( attrib ) );
		}
		
		ret.primitives.emplace_back( move( meshPrim ) );
	}
	ret.extras = mesh["extras"];
	
	return ret;
}

Node Scene::getNodeInfo( const std::string &key ) const
{
	auto &nodes = mTree["nodes"];
	auto &node = nodes[key];
	
	Node ret;
	ret.name = node["name"].asString();
	
	int i = 0;
	if( ! node["matrix"].isNull() ) {
		auto matrix = node["matrix"];
		for( auto & matVal : matrix ) {
			ret.transformMatrix[i / 4][i % 4] = matVal.asFloat();
			i++;
		}
	}
	else {
		if( ! node["translation"].isNull() ) {
			auto transArray = node["translation"];
			i = 0;
			for( auto & transVal : transArray )
				ret.translation[i++] = transVal.asFloat();
		}
		if( ! node["rotation"].isNull() ) {
			auto rotArray = node["rotation"];
			i = 0;
			for( auto & rotVal : rotArray )
				ret.rotation[i++] = rotVal.asFloat();
		}
		if( ! node["scale"].isNull() ) {
			auto & scaleArray = node["scale"];
			i = 0;
			for( auto & scaleVal : scaleArray )
				ret.scale[i++] = scaleVal.asFloat();
		}
	}
	
	if( ! node["camera"].isNull() ) {
		ret.camera = node["camera"].asString();
	}
	else if( ! node["jointName"].isNull() ) {
		ret.jointName = node["jointName"].asString();
	}
	else {
		if( ! node["meshes"].isNull() ) {
			auto &meshes = node["meshes"];
			std::transform( meshes.begin(), meshes.end(), std::back_inserter( ret.meshes ),
						   []( const Json::Value &val ){ return val.asString(); } );
		}
		if( ! node["skin"].isNull() ) {
			ret.skin = node["skin"].asString();
		}
		if( ! node["skeletons"].isNull() ) {
			auto &skeletons = node["skeletons"];
			std::transform( skeletons.begin(), skeletons.end(), std::back_inserter( ret.skeletons ),
						   []( const Json::Value &val ){ return val.asString(); } );
		}
	}
	
	return ret;
}
	
Program Scene::getProgramInfo( const std::string &key ) const
{
	auto &programs = mTree["programs"];
	auto &program = programs[key];
	
	Program ret;
	ret.name = program["name"].asString();
	
	CI_ASSERT( program["vertexShader"].isString() );
	CI_ASSERT( program["fragmentShader"].isString() );
	
	ret.vertexShader = program["vertexShader"].asString();
	ret.fragmentShader = program["fragmentShader"].asString();
	
	auto &attributes = program["attributes"];
	for( auto & attribute : attributes )
		ret.attributes.push_back( attribute.asString() );
	
	ret.extras = program["extras"].asString();
	
	return ret;
}
	
Sampler Scene::getSamplerInfo( const std::string &key ) const
{
	auto &samplers = mTree["samplers"];
	auto &sampler = samplers[key];
	
	Sampler ret;
	ret.name = sampler["name"].asString();
	
	if( sampler["magFilter"].isNumeric() )
		ret.magFilter = sampler["magFilter"].asUInt();
	if( sampler["minFilter"].isNumeric() )
		ret.minFilter = sampler["minFilter"].asUInt();
	if( sampler["wrapS"].isNumeric() )
		ret.wrapS = sampler["wrapS"].asUInt();
	if( sampler["wrapT"].isNumeric() )
		ret.wrapT = sampler["wrapT"].asUInt();
	
	ret.extras = sampler["extras"];
	
	return ret;
}
	
Scene Scene::getSceneInfo( const std::string &key ) const
{
	auto &scenes = mTree["scenes"];
	auto &scene = scenes[key];
	
	Scene ret;
	
	auto nodes = scene["nodes"];
	ret.nodes.resize( nodes.size() );
	int i = 0;
	for( auto & node : nodes )
		ret.nodes[i++] = node.asString();
	ret.extras = scene["extras"];
	ret.name = key;
	
	return ret;
}
	
Shader Scene::getShaderInfo( const std::string &key ) const
{
	auto &shaders = mTree["shaders"];
	auto &shader = shaders[key];
	
	Shader ret;
	
	CI_ASSERT( shader["uri"].isString() );
	CI_ASSERT( shader["type"].isNumeric() );
	
	auto uri = shader["uri"].asString();
	if( auto pos = uri.find("data:text/plain;base64,") != std::string::npos ) {
		auto data = uri.substr( pos + 1, uri.size() );
		auto buffer = fromBase64( data );
		ret.source = std::string( static_cast<char*>(buffer.getData()), buffer.getSize() );
	}
	else {
		ret.source = loadString( loadFile( mGltfPath / uri ) );
	}
	ret.type = shader["type"].asUInt();
	ret.uri = shader["uri"].asString();
	ret.name = shader["name"].asString();
	ret.extras = shader["extras"];
	
	return ret;
}
	
Skin Scene::getSkinInfo( const std::string &key ) const
{
	auto &skins = mTree["skins"];
	auto &skin = skins[key];
	
	Skin ret;
	
	CI_ASSERT( skin["inverseBindMatrices"].isString() );
	CI_ASSERT( ! skin["jointName"].isNull() );
	
	ret.inverseBindMatrices = skin["inverseBindMatrices"].asString();
	auto &jointNames = skin["jointName"];
	std::transform( jointNames.begin(), jointNames.end(), std::back_inserter( ret.jointNames ),
	[]( const Json::Value &val ){ return val.asString(); } );
	if( ! skin["bindShapeMatrix"].isNull() ) {
		auto &bindShapeMatrix = skin["bindShapeMatrix"];
		int i = 0;
		for( auto &bind : bindShapeMatrix ) {
			ret.bindShapeMatrix[i / 4][i % 4] = bind.asFloat();
			i++;
		}
	}
	ret.name = skin["name"].asString();
	ret.extras = skin["extras"];
	
	return ret;
}

Technique Scene::getTechniqueInfo( const std::string &key ) const
{
	auto &techniques = mTree["techniques"];
	auto &technique = techniques[key];
	
	Technique ret;
	
	CI_ASSERT( technique["program"].isString() );
	
	ret.program = technique["program"].asString();
	ret.name = technique["name"].asString();
	ret.extras = technique["extras"];
	
	auto &attribs = technique["attributes"];
	auto attribNames = attribs.getMemberNames();
	ret.attributes.reserve( attribNames.size() );
	for( int i = 0; i < attribNames.size(); i++ ) {
		auto &attribName = attribNames[i];
		auto pair = make_pair( attribName, attribs[attribName].asString() );
		ret.attributes.emplace_back( move( pair ) );
	}
	
	auto &uniforms = technique["uniforms"];
	auto uniformNames = uniforms.getMemberNames();
	ret.attributes.reserve( uniformNames.size() );
	for( int i = 0; i < uniformNames.size(); i++ ) {
		auto &uniformName = uniformNames[i];
		auto pair = make_pair( uniformName, attribs[uniformName].asString() );
		ret.uniforms.emplace_back( move( pair ) );
	}
	
	if( ! technique["states"].isNull() ) {
		auto &states = technique["states"];
		if( ! states["enable"].isNull() ) {
			auto &enables = states["enable"];
			ret.states.enables.reserve( enables.size() );
			std::transform( enables.begin(), enables.end(), std::back_inserter( ret.states.enables ),
			[]( const Json::Value &val ){ return val.asString(); } );
		}
		if( ! states["functions"].isNull() ) {
			auto &functions = states["functions"];
			int i = 0;
			if( ! functions["blendColor"].isNull() ) {
				i = 0;
				for( auto &color : functions["blendColor"] )
					ret.states.functions.blendColor[i++] = color.asFloat();
			}
			if( ! functions["blendEquationSeparate"].isNull() ) {
				i = 0;
				for( auto &equation : functions["blendEquationSeparate"] )
					ret.states.functions.blendEquationSeparate[i++] = equation.asUInt();
			}
			if( ! functions["blendFuncSeparate"].isNull() ) {
				i = 0;
				for( auto &func : functions["blendFuncSeparate"] )
					ret.states.functions.blendFuncSeparate[i++] = func.asUInt();
			}
			if( ! functions["colorMask"].isNull() ) {
				i = 0;
				for( auto &mask : functions["colorMask"] )
					ret.states.functions.colorMask[i++] = mask.asBool();
			}
			if( ! functions["depthRange"].isNull() ) {
				i = 0;
				for( auto &range : functions["depthRange"] )
					ret.states.functions.depthRange[i++] = range.asFloat();
			}
			if( ! functions["polygonOffset"].isNull() ) {
				i = 0;
				for( auto &offset : functions["polygonOffset"] )
					ret.states.functions.polygonOffset[i++] = offset.asFloat();
			}
			if( ! functions["scissor"].isNull() ) {
				i = 0;
				for( auto &scissor : functions["scissor"] )
					ret.states.functions.scissor[i++] = scissor.asFloat();
			}
			if( ! functions["lineWidth"].isNull() )
				ret.states.functions.lineWidth = functions["lineWidth"].asFloat();
			if( ! functions["cullFace"].isNull() )
				ret.states.functions.cullFace = functions["cullFace"].asUInt();
			if( ! functions["depthFunc"].isNull() )
				ret.states.functions.depthFunc = functions["depthFunc"].asUInt();
			if( ! functions["frontFace"].isNull() )
				ret.states.functions.frontFace = functions["frontFace"].asUInt();
			if( ! functions["depthMask"].isNull() )
				ret.states.functions.depthMask = functions["depthMask"].asBool();
			ret.states.functions.extras = functions["extras"];
		}
		ret.states.extras = states["extras"];
	}
	
	// Parameters Excavation...
	auto &parameters = technique["parameters"];
	ret.parameters.reserve( parameters.size() );
	for ( auto & param : parameters ) {
		CI_ASSERT( param["type"].isNull() );
		Technique::Parameter techParam;
		techParam.type = param["type"].asUInt();
		if( ! param["count"].isNull() )
			techParam.count = param["count"].asUInt();
		if( ! param["node"].isNull() )
			techParam.node = param["node"].asString();
		if( ! param["semantic"].isNull() )
			techParam.semantic = param["semantic"].asString();
		techParam.name = param["name"].asString();
		techParam.extras = param["extras"];
		ret.parameters.emplace_back( move( techParam ) );
	}
	
	return ret;
}
	
Texture Scene::getTextureInfo( const std::string &key ) const
{
	auto &textures = mTree["textures"];
	auto &texture = textures[key];
	
	Texture ret;
	
	CI_ASSERT( ! texture["sampler"].isString() );
	CI_ASSERT( ! texture["source"].isString() );
	
	ret.source = texture["source"].asString();
	ret.sampler = texture["sampler"].asString();
	ret.name = texture["name"].asString();
	if( texture["target"].isNumeric() )
		ret.target = texture["target"].asUInt();
	if( texture["format"].isNumeric() )
		ret.format = texture["format"].asUInt();
	if( texture["internalFormat"].isNumeric() )
		ret.internalFormat = texture["internalFormat"].asUInt();
	if( texture["type"].isNumeric() )
		ret.type = texture["type"].asUInt();
	
	return ret;
}

CameraOrtho Scene::getOrthoCameraByName( const std::string &name )
{
	Camera cam = getCameraInfo( name );
	if( cam.type != "orthographic" ) throw "This should be orthographic but it's not";
	
	//TODO: This is most likely wrong need to change it.
	CameraOrtho ret( -cam.xmag, cam.xmag, -cam.ymag, cam.ymag, cam.znear, cam.zfar);
	
	return ret;
}

CameraPersp Scene::getPerspCameraByName( const std::string &name )
{
	Camera cam = getCameraInfo( name );
	if( cam.type != "orthographic" ) throw "This should be perspective but it's not";
	
	auto app = app::App::get();
	
	CameraPersp ret( app->getWindowWidth(), app->getWindowHeight(), cam.yfov, cam.znear, cam.zfar );
	
	return ret;
}
	
ci::geom::Primitive Scene::convertToPrimitive( GLenum primitive )
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
	
ci::geom::Attrib Scene::getAttribEnum( const std::string &attrib )
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
		CI_LOG_W( "UNDEFINED Attib JOINTMATRIX Using CUSTOM_0" );
		return Attrib::CUSTOM_0;
	}
	else if( attrib == "WEIGHT" )		return Attrib::BONE_WEIGHT;
	else								return Attrib::NUM_ATTRIBS;
}
	
ci::gl::UniformSemantic Scene::getUniformEnum( const std::string &uniform )
{
	auto & u = uniform;
	using namespace ci::gl;
	if( u == "MODEL" )						return UniformSemantic::UNIFORM_MODEL_MATRIX;
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
	
uint8_t Scene::getNumComponentsForType( const std::string &type )
{
	if( type == "SCALAR" )	  return 1;
	else if( type == "VEC2" ) return 2;
	else if( type == "VEC3" ) return 3;
	else if( type == "VEC4" ) return 4;
	else if( type == "MAT2" ) return 4;
	else if( type == "MAT3" ) return 9;
	else if( type == "MAT4" ) return 16;
	else					  return 0;
}

uint8_t Scene::getNumBytesForComponentType( GLuint type )
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
	
namespace gl {
	
GlslProgRef	getGlslProgramFromMaterial( const Gltf &gltf, const std::string &name )
{
	std::map<std::string, GlslProgRef> GlslCache;
	
	auto found = GlslCache.find( name );
	
	if( found != GlslCache.end() ) return found->second;
	
	GlslProgRef ret;
	
	auto material = gltf.getMaterialInfo( name );
	auto technique = gltf.getTechniqueInfo( material.instanceTechnique.technique );
	auto pass = technique.passes[0];
	
	GlslProg::Format format;
	
	auto attribs = pass.instanceProgram.attributes;
	for( auto attrib = attribs.begin(); attrib != attribs.end(); ++attrib ) {
		// attrib->first is the name of the glsl variable
		// attrib->second is the name into the parameter
		// which offers type and semantic
		auto found = technique.parameters.find( attrib->second );
		if( found != technique.parameters.end() ) {
			format.attrib( gltf.getAttribEnum( found->second.semantic ), attrib->first );
		}
	}
	
	auto uniforms = pass.instanceProgram.uniforms;
	for( auto uniform = uniforms.begin(); uniform != uniforms.end(); ++uniform ) {
		// uniform->first is the name of the glsl variable
		// uniform->second is the name into the parameter
		// which offers type and semantic
		auto found = technique.parameters.find( uniform->second );
		
		if( found != technique.parameters.end() ) {
			if( !found->second.semantic.empty() )
				format.uniform( gltf.getUniformEnum( found->second.semantic ), uniform->first );
		}
	}
	
	auto program = gltf.getProgramInfo( pass.instanceProgram.program );
	auto fragShader = gltf.getShaderInfo( program.frag );
	auto vertShader = gltf.getShaderInfo( program.vert );
	
	format.fragment( fragShader.source ).vertex( vertShader.source );
	
	ret = GlslProg::create( format );
	
	return ret;
}

TextureRef getTextureByName( const Gltf &gltf, const std::string &name )
{
	static std::map<std::string, TextureRef> TextureRefCache;
	
	auto found = TextureRefCache.find( name );
	
	if( found != TextureRefCache.end() ) return found->second;
	
	TextureRef ret;
	
	auto texture = gltf.getTextureInfo( name );
	auto sampler = gltf.getSamplerInfo( texture.sampler );
	auto source = gltf.getImageInfo( texture.source );
	
	ci::gl::Texture2d::Format format;
	format.wrapS( sampler.wrapS )
	.wrapT( sampler.wrapT )
	.magFilter( sampler.magFilter )
	.minFilter( sampler.minFilter )
	.target( texture.target )
	.internalFormat( texture.internalFormat )
	//		.pixelDataFormat( texture.format )
	.dataType( texture.type )
	// TODO: Test what should be here!
	.loadTopDown();
	
	ret = ci::gl::Texture::create( *(source.surface), ci::gl::Texture2d::Format().loadTopDown() );
	
	return ret;
}

BatchRef getBatchFromMeshByName( const Gltf &gltf, const std::string &name )
{
	static std::map<std::string, BatchRef> BatchCache;
	
	auto found = BatchCache.find( name );
	
	if( found != BatchCache.end() ) return found->second;
	
	BatchRef ret;
	
	//	auto glsl = getGlslProgramByName( );
	
	return ret;
}

VboMeshRef getVboMeshFromMeshByName( const Gltf &gltf, const std::string &name )
{
	static std::map<std::string, VboMeshRef> VboMeshCache;
	
	auto found = VboMeshCache.find( name );
	
	if( found != VboMeshCache.end() ) return found->second;
	
	VboMeshRef ret;
	std::vector<std::pair<geom::BufferLayout, VboRef>> arrayVbos;
	std::map<std::string, geom::BufferLayout> layoutsForVbo;
	Mesh mesh = gltf.getMeshInfo( name );
	
	uint32_t numVertices = 0;
	for( auto & attribute : mesh.primitives[0].attributes ) {
		
		auto attribAccessor = gltf.getAccessorInfo( attribute.first );
		auto attribBufferView = gltf.getBufferViewInfo( attribAccessor.bufferView );
		auto attribBuffer = gltf.getBufferInfo( attribBufferView.buffer );
		
		auto numComponents = gltf.getNumComponentsForType( attribAccessor.type );
		//		auto numBytesComponent = getNumBytesForComponentType( attribAccessor.componentType );
		
		if( numVertices == 0 ) {
			numVertices = attribAccessor.count;
		}
		else if( numVertices != attribAccessor.count ) {
			CI_LOG_W( "Vertices don't match in " << attribAccessor.name << " accessor." );
		}
		
		auto foundBuffer = layoutsForVbo.find( attribBufferView.name );
		if( foundBuffer != layoutsForVbo.end() ) {
			auto & layout = foundBuffer->second;
			layout.append( attribute.second, numComponents, attribAccessor.byteStride, attribAccessor.byteOffset );
		}
		else {
			geom::BufferLayout layout;
			layout.append( attribute.second, numComponents, attribAccessor.byteStride, attribAccessor.byteOffset );
			layoutsForVbo.insert( make_pair( attribBufferView.name, layout ) );
		}
	}
	VboRef indices;
	uint32_t numIndices = 0;
	uint32_t indexComponentType = 0;
	{
		auto indexAccessor = gltf.getAccessorInfo( mesh.primitives[0].indices );
		auto indexBufferView = gltf.getBufferViewInfo( indexAccessor.bufferView );
		auto indexBuffer = gltf.getBufferInfo( indexBufferView.buffer );
		
		numIndices = indexAccessor.count;
		indexComponentType = indexAccessor.componentType;
		auto ptr = (uint8_t*)indexBuffer.data->getData();
		indices = ci::gl::Vbo::create( GL_ELEMENT_ARRAY_BUFFER, indexBufferView.byteLength, &ptr[indexAccessor.byteOffset + indexBufferView.byteOffset], GL_STATIC_DRAW );
	}
	for( auto & bufferLayout : layoutsForVbo ) {
		auto bufferView = gltf.getBufferViewInfo( bufferLayout.first );
		auto buffer = gltf.getBufferInfo( bufferView.buffer );
		
		auto ptr = (uint8_t*)buffer.data->getData();
		VboRef temp = ci::gl::Vbo::create( GL_ARRAY_BUFFER, buffer.byteLength, &ptr[bufferView.byteOffset], GL_STATIC_DRAW );
		arrayVbos.push_back( make_pair( bufferLayout.second, temp ) );
	}
	
	ret = VboMesh::create( numVertices, mesh.primitives[0].primitive, arrayVbos, numIndices, indexComponentType, indices );
	
	VboMeshCache.insert( make_pair( name, ret ) );
	return ret;
}
	
TriMeshRef getTriMeshFromMeshByName( const Gltf &gltf, const std::string &name )
{
	static std::map<std::string, TriMeshRef> TriMeshCache;
	
	auto found = TriMeshCache.find( name );
	
	if( found != TriMeshCache.end() ) return found->second;
	
	TriMesh::Format format;
	format.positions(3);
	format.texCoords0(2);
	format.normals();
	TriMeshRef ret = TriMesh::create( format );
	Mesh mesh = gltf.getMeshInfo( name );
	
	for( auto & attribute : mesh.primitives[0].attributes ) {
		
		auto attribAccessor = gltf.getAccessorInfo( attribute.first );
		auto attribBufferView = gltf.getBufferViewInfo( attribAccessor.bufferView );
		auto attribBuffer = gltf.getBufferInfo( attribBufferView.buffer );
		
		auto numComponents = gltf.getNumComponentsForType( attribAccessor.type );
		auto numBytesComponent = gltf.getNumBytesForComponentType( attribAccessor.componentType );
		
		float * floatContainer = new float[attribAccessor.count * numComponents];
		uint8_t * data = (uint8_t*)attribBuffer.data->getData();
		
		memcpy( floatContainer,
			   &data[attribBufferView.byteOffset + attribAccessor.byteOffset],
			   attribAccessor.count * numComponents * numBytesComponent);
		
		// TODO: Decipher how many components and use the correct one.
		if( attribute.second == geom::Attrib::POSITION ) {
			ret->appendPositions( (vec3*)floatContainer, attribAccessor.count );
		}
		else if( attribute.second == geom::Attrib::NORMAL ) {
			ret->appendNormals( (vec3*)floatContainer, attribAccessor.count );
		}
		else if( attribute.second == geom::Attrib::TEX_COORD_0 ) {
			ret->appendTexCoords0( (vec2*)floatContainer, attribAccessor.count );
		}
		else if( attribute.second == geom::Attrib::TEX_COORD_1 ) {
			ret->appendTexCoords1( (vec2*)floatContainer, attribAccessor.count );
		}
		else if( attribute.second == geom::Attrib::TEX_COORD_2 ) {
			ret->appendTexCoords2( (vec2*)floatContainer, attribAccessor.count );
		}
		else if( attribute.second == geom::Attrib::TEX_COORD_3 ) {
			ret->appendTexCoords3( (vec3*)floatContainer, attribAccessor.count );
		}
		else if( attribute.second == geom::Attrib::COLOR ) {
			ret->appendColors( (Color*)floatContainer, attribAccessor.count );
		}
		
		delete [] floatContainer;
	}
	
	auto indexAccessor = gltf.getAccessorInfo( mesh.primitives[0].indices );
	auto indexBufferView = gltf.getBufferViewInfo( indexAccessor.bufferView );
	auto indexBuffer = gltf.getBufferInfo( indexBufferView.buffer );
	
	std::vector<uint16_t> indices(indexAccessor.count);
	char * data = (char*)indexBuffer.data->getData();
	memcpy(indices.data(), &data[indexBufferView.byteOffset], indexAccessor.count * sizeof(uint16_t) );
	std::vector<uint32_t> convertedIndices(indices.size());
	
	
	for( int i = 0; i < indices.size() && i < convertedIndices.size(); ++i ) {
		convertedIndices[i] = indices[i];
	}
	
	ret->appendIndices( convertedIndices.data(), convertedIndices.size() );
	
	TriMeshCache.insert( make_pair(name, ret) );
	return ret;
}
	
} } // namespace gl // namespace gltf


