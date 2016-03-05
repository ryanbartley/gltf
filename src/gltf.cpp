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
using namespace ci::gl;
using namespace std;

namespace gltf {
	
struct BinaryHeader {
	std::array<uint8_t, 4>	magic;
	uint32_t				version;
	uint32_t				length;
	uint32_t				sceneLength;
	uint32_t				sceneFormat;
};
	
FileRef File::create( const ci::DataSourceRef &gltfFile )
{
	return FileRef( new File( gltfFile ) );
}
	
File::File( const ci::DataSourceRef &gltfFile )
: mGltfPath( gltfFile->getFilePath().parent_path() )
{
	std::string gltfJson;
	verifyFile( gltfFile, gltfJson );
	
	Json::Features features;
	features.allowComments_ = true;
	features.strictRoot_ = true;
	
	Json::Reader reader( features );
	try {
		reader.parse( gltfJson, mGltfTree );
	}
	catch ( const std::runtime_error &e ) {
		CI_LOG_E( "Error parsing gltf file " << e.what() );
	}
	cout << mGltfTree.toStyledString() << endl;
	loadExtensions();
	if( ! mGltfTree["asset"].isNull() )
		setAssetInfo( mGltfTree["asset"] );
	load();
}
	
void File::verifyFile( const ci::DataSourceRef &data, std::string &gltfJson )
{
	auto pathExtension = data->getFilePath().extension().string();
	auto binary = pathExtension == ".glb";
	if( binary ) {
		auto buffer = data->getBuffer();
		auto header = reinterpret_cast<BinaryHeader*>( buffer->getData() );
		
		auto sceneStart = reinterpret_cast<uint8_t*>( header + 1 );
		gltfJson.append( sceneStart, sceneStart + header->sceneLength );
		
		auto binaryStart = sceneStart + header->sceneLength;
		auto binarySize = header->length - header->sceneLength - sizeof( BinaryHeader );
		
		mBuffer = ci::Buffer::create( binarySize );
		memcpy( mBuffer->getData(), binaryStart, binarySize );
	}
	else
		gltfJson = loadString( data );
}
	
void File::load()
{
	auto gltfTypes = mGltfTree.getMemberNames();
	for( auto &typeName : gltfTypes ) {
		auto &typeObj = mGltfTree[typeName];
		if( typeName == "scene" ) {
			mDefaultScene = typeObj.asString();
			continue;
		}
		else if( typeName == "extensionsUsed" )
			continue;
		auto typeKeys = typeObj.getMemberNames();
		for( auto &typeKey : typeKeys ) {
			auto &obj = typeObj[typeKey];
			if( typeName == "accessors" )
				addAccessorInfo( typeKey, obj );
			else if( typeName == "animations" )
				addAnimationInfo( typeKey, obj );
			else if( typeName == "bufferViews" )
				addBufferViewInfo( typeKey, obj );
			else if( typeName == "buffers" )
				addBufferInfo( typeKey, obj );
			else if( typeName == "cameras" )
				addCameraInfo( typeKey, obj );
			else if( typeName == "images" )
				addImageInfo( typeKey, obj );
			else if( typeName == "materials" )
				addMaterialInfo( typeKey, obj );
			else if( typeName == "meshes" )
				addMeshInfo( typeKey, obj );
			else if( typeName == "nodes" )
				addNodeInfo( typeKey, obj );
			else if( typeName == "programs" )
				addProgramInfo( typeKey, obj );
			else if( typeName == "samplers" )
				addSamplerInfo( typeKey, obj );
			else if( typeName == "scenes" )
				addSceneInfo( typeKey, obj );
			else if( typeName == "shaders" )
				addShaderInfo( typeKey, obj );
			else if( typeName == "skins" )
				addSkinInfo( typeKey, obj );
			else if( typeName == "techniques" )
				addTechniqueInfo( typeKey, obj );
			else if( typeName == "textures" )
				addTextureInfo( typeKey, obj );
		}
	}
}
	
void File::loadExtensions()
{
	if( ! mGltfTree["extensionsUsed"].isNull() && mGltfTree["extensionsUsed"].isArray() ) {
		auto &extensions = mGltfTree["extensionsUsed"];
		std::transform( begin( extensions ), end( extensions ), std::back_inserter( mExtensions ),
		[]( const Json::Value &val ){ return val.asString(); } );
		std::sort( begin( mExtensions ), end( mExtensions ) );
	}
}

bool File::hasExtension( const std::string &extension ) const
{
	return std::binary_search( begin( mExtensions ), end( mExtensions ), extension );
}

const Accessor& File::getAccessorInfo( const std::string& key ) const
{
	auto found = mAccessors.find( key );
	return found->second;
}
	
template<>
void File::add( const std::string &key, Accessor accessor )
{
	mAccessors[key] = move(accessor);
}

void File::addAccessorInfo( const std::string &key, const Json::Value &accessorInfo )
{
	// Required points.
	CI_ASSERT( accessorInfo["bufferView"].isString() );
	CI_ASSERT( accessorInfo["byteOffset"].isNumeric() );
	CI_ASSERT( accessorInfo["componentType"].isNumeric() );
	CI_ASSERT( accessorInfo["type"].isString() );
	CI_ASSERT( accessorInfo["count"].isNumeric() );

	Accessor ret;
	ret.bufferView = accessorInfo["bufferView"].asString();
	ret.byteOffset = accessorInfo["byteOffset"].asUInt();
	ret.count = accessorInfo["count"].asUInt();
	ret.type = accessorInfo["type"].asString();
	ret.componentType = accessorInfo["componentType"].asUInt();
	ret.name = accessorInfo["name"].asString();
	ret.extras = accessorInfo["extras"];

	if( !accessorInfo["byteStride"].isNull() )
		ret.byteStride = accessorInfo["byteStride"].asUInt();

	auto &maxElem = accessorInfo["max"];
	auto &minElem = accessorInfo["min"];
	if( !maxElem.isNull() && !minElem.isNull() ) {
		auto maxSize = maxElem.size();
		auto minSize = minElem.size();
		CI_ASSERT( maxSize == minSize );
		ret.max.resize( maxSize );
		ret.min.resize( minSize );
		for( int i = 0; i < maxSize; i++ ) {
			ret.max[i] = maxElem[i].asFloat();
			ret.min[i] = minElem[i].asFloat();
		}
	}
	add( key, move(ret) );
}
	
const Animation& File::getAnimationInfo( const std::string &key ) const
{
	auto found = mAnimations.find( key );
	return found->second;
}
	
template<>
void File::add( const std::string &key, Animation animation )
{
	mAnimations[key] = move( animation );
}

void File::addAnimationInfo( const std::string &key, const Json::Value &animationInfo )
{
	Animation ret;
	auto &channels = animationInfo["channels"];
	for( auto &channel : channels ) {
		auto &target = channel["target"];

		CI_ASSERT( channel["sampler"].isString() );
		CI_ASSERT( channel["target"].isObject() );
		CI_ASSERT( target["id"].isString() );
		CI_ASSERT( target["path"].isString() );

		Animation::Channel animChannel;

		animChannel.sampler = channel["sampler"].asString();
		animChannel.targetId = target["id"].asString();
		animChannel.targetPath = target["path"].asString();
		animChannel.channelExtras = channel["extras"];
		animChannel.targetExtras = channel["extras"];
		ret.channels.emplace_back( move( animChannel ) );
	}

	auto &samplers = animationInfo["samplers"];
	for( auto &sampler : samplers ) {
		CI_ASSERT( sampler["input"].isString() );
		CI_ASSERT( sampler["output"].isString() );
		
		Animation::Sampler animSampler;
		animSampler.input = sampler["input"].asString();
		animSampler.output = sampler["output"].asString();
		if( sampler["interpolation"].isString() )
			animSampler.interpolation = sampler["interpolation"].asString();
		
		ret.samplers.emplace_back( move( animSampler ) );
	}

	ret.name = animationInfo["name"].asString();
	ret.parameters = animationInfo["parameters"];
	ret.extras = animationInfo["extras"];

	add( key, move( ret ) );
}
	
const Asset& File::getAssetInfo() const
{
	return mAssetInfo;
}
	
void File::setAssetInfo( const Json::Value &assetInfo )
{
	CI_ASSERT( ! assetInfo["version"].isNull() );
	mAssetInfo.version = assetInfo["version"].asString();
	
	if( assetInfo["profile"].isObject() ) {
		if( assetInfo["profile"]["api"].isString() )
			mAssetInfo.profile.api = assetInfo["profile"]["api"].asString();
		if( assetInfo["profile"]["version"].isString() )
			mAssetInfo.profile.version = assetInfo["profile"]["version"].asString();
	}
	
	mAssetInfo.copyright = assetInfo["copyright"].asString();
	mAssetInfo.generator = assetInfo["generator"].asString();
	mAssetInfo.premultipliedAlpha = assetInfo["premultipliedAlpha"].asBool();
}
	
const gltf::Buffer& File::getBufferInfo( const std::string &name ) const
{
	auto found = mBuffers.find( name );
	return found->second;
}
	
template<>
void File::add( const std::string &key, gltf::Buffer buffer )
{
	mBuffers[key] = move( buffer );
}
	
void File::addBufferInfo( const std::string &key, const Json::Value &bufferInfo )
{
	CI_ASSERT( bufferInfo["uri"].isString() );
	
	gltf::Buffer ret;
	auto uri = bufferInfo["uri"].asString();
	
	auto pos = uri.find_first_of(',');
	if( pos != std::string::npos ) {
		ret.uri = uri.substr( 0, pos );
		auto data = uri.substr( pos + 1, uri.size() );
		auto buffer = fromBase64( data );
		ret.data = BufferRef( new ci::Buffer( std::move( buffer ) ) );
	}
	else {
		if( key == "binary_glTF" )
			ret.data = mBuffer;
		else
			ret.data = loadFile( mGltfPath / uri )->getBuffer();
	}
	
	ret.type = bufferInfo["type"].asString();
	ret.byteLength = bufferInfo["byteLength"].asUInt();
	ret.extras = bufferInfo["extras"];
	ret.name = bufferInfo["name"].asString();
	
	add( key, move( ret ) );
}

const BufferView& File::getBufferViewInfo( const std::string &name ) const
{
	auto found = mBufferViews.find( name );
	return found->second;
}
	
template<>
void File::add( const std::string &key, gltf::BufferView bufferView )
{
	mBufferViews[key] = move( bufferView );
}

void File::addBufferViewInfo( const std::string &key, const Json::Value &bufferViewInfo )
{
	CI_ASSERT( bufferViewInfo["byteOffset"].isNumeric() );
	CI_ASSERT( bufferViewInfo["buffer"].isString() );
	
	BufferView ret;
	
	ret.buffer = bufferViewInfo["buffer"].asString();
	ret.byteOffset = bufferViewInfo["byteOffset"].asUInt();
	ret.byteLength = bufferViewInfo["byteLength"].asUInt();
	ret.target = bufferViewInfo["target"].asUInt();
	ret.name = bufferViewInfo["name"].asString();
	ret.extras = bufferViewInfo["extras"];
	
	add( key, ret );
}
	
const Camera& File::getCameraInfo( const std::string &key ) const
{
	auto found = mCameras.find( key );
	return found->second;
}
	
template<>
void File::add( const std::string &key, Camera camera )
{
	mCameras[key] = move( camera );
}
	
void File::addCameraInfo( const std::string &key, const Json::Value &cameraInfo )
{
	CI_ASSERT( cameraInfo["type"].isString() );
	
	Camera ret;
	auto cameraType = cameraInfo["type"].asString();
	ret.type = cameraType == "perspective" ? Camera::Type::PERSPECTIVE : Camera::Type::ORTHOGRAPHIC;
	if ( ret.type == Camera::Type::PERSPECTIVE ) {
		auto &perspectiveInfo = cameraInfo["perspective"];
		
		CI_ASSERT( perspectiveInfo["yfov"].isNumeric() );
		CI_ASSERT( perspectiveInfo["znear"].isNumeric() );
		CI_ASSERT( perspectiveInfo["zfar"].isNumeric() );
		
		ret.aspectRatio = perspectiveInfo["aspectRatio"].asFloat();
		ret.yfov = perspectiveInfo["yfov"].asFloat();
		ret.znear = perspectiveInfo["znear"].asFloat();
		ret.zfar = perspectiveInfo["zfar"].asFloat();
		ret.camSpecificExtras = perspectiveInfo["extras"];
	}
	else if( ret.type == Camera::Type::ORTHOGRAPHIC ) {
		auto &orthographicInfo = cameraInfo["orthographic"];
		
		CI_ASSERT( orthographicInfo["xmag"].isNumeric() );
		CI_ASSERT( orthographicInfo["ymag"].isNumeric() );
		CI_ASSERT( orthographicInfo["znear"].isNumeric() );
		CI_ASSERT( orthographicInfo["zfar"].isNumeric() );
		
		ret.xmag = orthographicInfo["xmag"].asFloat();
		ret.ymag = orthographicInfo["ymag"].asFloat();
		ret.znear = orthographicInfo["znear"].asFloat();
		ret.zfar = orthographicInfo["zfar"].asFloat();
		ret.camSpecificExtras = orthographicInfo["extras"];
	}
	ret.name = cameraInfo["name"].asString();
	ret.extras = cameraInfo["extras"];
	
	add( key, move( ret ) );
}
	
const Image& File::getImageInfo( const std::string &key ) const
{
	auto found = mImages.find( key );
	return found->second;
}
	
template<>
void File::add( const std::string &key, Image image )
{
	mImages[key] = move( image );
}
	
void File::addImageInfo( const std::string &key, const Json::Value &imageInfo )
{
	CI_ASSERT( imageInfo["uri"].isString() );
	
	Image ret;
	ret.uri = imageInfo["uri"].asString();
	ret.name = imageInfo["name"].asString();
	
	// in embedded use this to look at type
//	auto beginning = uri.find('/');
//	auto end = uri.find( ';' );
//	auto typeStr = uri.substr( beginning + 1, end );
	
	auto hasExtInfo = imageInfo["extensions"].size() > 9;
	if( hasExtension( "KHR_binary_glTF" ) && hasExtInfo ) {
		cout << imageInfo.toStyledString() << endl;
		CI_ASSERT( hasExtInfo );
//		auto binaryExt = imageInfo["extensions"]["KHR_binary_glTF"];
//		auto bufferView = binaryExt["bufferView"].asString();
//		auto size = ivec2( binaryExt["width"].asUInt(), binaryExt["height"].asUInt() );
//		auto mimetype = binaryExt["mimeType"].asString();
	}
	else
		ret.imageSource = loadImage( loadFile( mGltfPath / ret.uri ) );
	
	add( key, move( ret ) );
}
	
const Material& File::getMaterialInfo( const std::string &key ) const
{
	auto found = mMaterials.find( key );
	return found->second;
}
	
template<>
void File::add( const std::string &key, Material material )
{
	mMaterials[key] = move( material );
}
	
void File::addMaterialInfo( const std::string &key, const Json::Value &materialInfo )
{
	Material ret;
	ret.name = materialInfo["name"].asString();
	ret.technique = materialInfo["technique"].asString();
	ret.values = materialInfo["values"];
	ret.extras = materialInfo["extras"];
	
	add( key, move( ret ) );
}
	
const Mesh& File::getMeshInfo( const std::string &key ) const
{
	auto found = mMeshes.find( key );
	return found->second;
}
	
template<>
void File::add( const std::string &key, Mesh mesh )
{
	mMeshes[key] = move( mesh );
}
	
void File::addMeshInfo( const std::string &key, const Json::Value &meshInfo )
{
	Mesh ret;
	for( auto &primitive : meshInfo["primitives"] ) {
		CI_ASSERT( primitive["material"].isString() );
		
		Mesh::Primitive meshPrim;
		meshPrim.material = primitive["material"].asString();
		meshPrim.indices = primitive["indices"].asString();
		meshPrim.primitive = primitive["mode"].asUInt();
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
	ret.name = meshInfo["name"].asString();
	ret.extras = meshInfo["extras"];
	
	add( key, move( ret ) );
}
	
const Node& File::getNodeInfo( const std::string &key ) const
{
	auto found = mNodes.find( key );
	return found->second;
}
	
template<>
void File::add( const std::string &key, Node node )
{
	mNodes[key] = move( node );
}
	
void File::addNodeInfo( const std::string &key, const Json::Value &nodeInfo )
{
	Node ret;
	
	int i = 0;
	if( ! nodeInfo["matrix"].isNull() ) {
		auto matrix = nodeInfo["matrix"];
		for( auto & matVal : matrix ) {
			ret.transformMatrix[i / 4][i % 4] = matVal.asFloat();
			i++;
		}
	}
	else {
		if( ! nodeInfo["translation"].isNull() ) {
			auto transArray = nodeInfo["translation"];
			i = 0;
			for( auto & transVal : transArray )
				ret.translation[i++] = transVal.asFloat();
		}
		if( ! nodeInfo["rotation"].isNull() ) {
			auto rotArray = nodeInfo["rotation"];
			i = 0;
			for( auto & rotVal : rotArray )
				ret.rotation[i++] = rotVal.asFloat();
		}
		if( ! nodeInfo["scale"].isNull() ) {
			auto & scaleArray = nodeInfo["scale"];
			i = 0;
			for( auto & scaleVal : scaleArray )
				ret.scale[i++] = scaleVal.asFloat();
		}
	}
	
	if( ! nodeInfo["camera"].isNull() ) {
		ret.camera = nodeInfo["camera"].asString();
	}
	else if( ! nodeInfo["jointName"].isNull() ) {
		ret.jointName = nodeInfo["jointName"].asString();
	}
	else {
		if( ! nodeInfo["meshes"].isNull() ) {
			auto &meshes = nodeInfo["meshes"];
			std::transform( meshes.begin(), meshes.end(), std::back_inserter( ret.meshes ),
						   []( const Json::Value &val ){ return val.asString(); } );
		}
		if( ! nodeInfo["skin"].isNull() ) {
			ret.skin = nodeInfo["skin"].asString();
		}
		if( ! nodeInfo["skeletons"].isNull() ) {
			auto &skeletons = nodeInfo["skeletons"];
			std::transform( skeletons.begin(), skeletons.end(), std::back_inserter( ret.skeletons ),
						   []( const Json::Value &val ){ return val.asString(); } );
		}
	}
	
	ret.name = nodeInfo["name"].asString();
	ret.extras = nodeInfo["extras"];
	
	add( key, move( ret ) );
}
	
const Program& File::getProgramInfo( const std::string &key ) const
{
	auto found = mPrograms.find( key );
	return found->second;
}
	
template<>
void File::add( const std::string &key, Program program )
{
	mPrograms[key] = move( program );
}
	
void File::addProgramInfo( const std::string &key, const Json::Value &programInfo )
{
	CI_ASSERT( programInfo["vertexShader"].isString() );
	CI_ASSERT( programInfo["fragmentShader"].isString() );
	
	Program ret;
	ret.vertexShader = programInfo["vertexShader"].asString();
	ret.fragmentShader = programInfo["fragmentShader"].asString();
	
	auto &attributes = programInfo["attributes"];
	for( auto & attribute : attributes )
		ret.attributes.push_back( attribute.asString() );
	
	ret.name = programInfo["name"].asString();
	ret.extras = programInfo["extras"].asString();
	
	add( key, move( ret ) );
}

const Sampler& File::getSamplerInfo( const std::string &key ) const
{
	auto found = mSamplers.find( key );
	return found->second;
}
	
template<>
void File::add( const std::string &key, Sampler sampler )
{
	mSamplers[key] = move( sampler );
}
	
void File::addSamplerInfo( const std::string &key, const Json::Value &samplerInfo )
{
	Sampler ret;
	
	if( samplerInfo["magFilter"].isNumeric() )
		ret.magFilter = samplerInfo["magFilter"].asUInt();
	if( samplerInfo["minFilter"].isNumeric() )
		ret.minFilter = samplerInfo["minFilter"].asUInt();
	if( samplerInfo["wrapS"].isNumeric() )
		ret.wrapS = samplerInfo["wrapS"].asUInt();
	if( samplerInfo["wrapT"].isNumeric() )
		ret.wrapT = samplerInfo["wrapT"].asUInt();
	
	ret.name = samplerInfo["name"].asString();
	ret.extras = samplerInfo["extras"];
	
	add( key, move( ret ) );
}
	
const Scene& File::getSceneInfo( const std::string &key ) const
{
	auto found = mScenes.find( key );
	return found->second;
}
	
template<>
void File::add( const std::string &key, Scene scene )
{
	mScenes[key] = move( scene );
}
	
void File::addSceneInfo( const std::string &key, const Json::Value &sceneInfo )
{
	Scene ret;
	
	auto &nodes = sceneInfo["nodes"];
	ret.nodes.resize( nodes.size() );
	int i = 0;
	for( auto & node : nodes )
		ret.nodes[i++] = node.asString();
	
	ret.name = sceneInfo["names"].asString();
	ret.extras = sceneInfo["extras"];
	
	add( key, move( ret ) );
}
	
const Shader& File::getShaderInfo( const std::string &key ) const
{
	auto found = mShaders.find( key );
	return found->second;
}
	
template<>
void File::add( const std::string &key, Shader shader )
{
	mShaders[key] = move( shader );
}
	
void File::addShaderInfo( const std::string &key, const Json::Value &shaderInfo )
{
	CI_ASSERT( shaderInfo["uri"].isString() );
	CI_ASSERT( shaderInfo["type"].isNumeric() );
	
	Shader ret;
	
	auto uri = shaderInfo["uri"].asString();
	
	// either it's embeded, binary, or separate
	
	auto pos = uri.find(",");
	if( pos != std::string::npos ) {
		if( pos + 1 == uri.size() && hasExtension( "KHR_binary_glTF" ) ) {
			auto binaryExt = shaderInfo["extensions"]["KHR_binary_glTF"];
			auto bufferView = binaryExt["bufferView"].asString();
			auto &bufferViewInfo = mGltfTree["bufferViews"][bufferView];
			auto offset = bufferViewInfo["byteOffset"].asUInt();
			auto length = bufferViewInfo["byteLength"].asUInt();
			ret.source.append( reinterpret_cast<char*>( mBuffer->getData() ) + offset, length );
			ret.uri = uri;
		}
		else {
			auto data = uri.substr( pos + 1, uri.size() );
			auto buffer = fromBase64( data );
			ret.source = std::string( static_cast<char*>(buffer.getData()), buffer.getSize() );
		}
	}
	else {
		ret.source = loadString( loadFile( mGltfPath / uri ) );
	}
	ret.type = shaderInfo["type"].asUInt();
	ret.uri = shaderInfo["uri"].asString();
	ret.name = shaderInfo["name"].asString();
	ret.extras = shaderInfo["extras"];
	
	add( key, move( ret ) );
}
	
const Skin& File::getSkinInfo( const std::string &key ) const
{
	auto found = mSkins.find( key );
	return found->second;
}
	
template<>
void File::add( const std::string &key, Skin skin )
{
	mSkins[key] = move( skin );
}
	
void File::addSkinInfo( const std::string &key, const Json::Value &skinInfo )
{
	CI_ASSERT( skinInfo["inverseBindMatrices"].isString() );
	CI_ASSERT( ! skinInfo["jointNames"].isNull() );
	
	Skin ret;
	
	ret.inverseBindMatrices = skinInfo["inverseBindMatrices"].asString();
	auto &jointNames = skinInfo["jointName"];
	std::transform( jointNames.begin(), jointNames.end(), std::back_inserter( ret.jointNames ),
	[]( const Json::Value &val ){ return val.asString(); } );
	if( ! skinInfo["bindShapeMatrix"].isNull() ) {
		auto &bindShapeMatrix = skinInfo["bindShapeMatrix"];
		int i = 0;
		for( auto &bind : bindShapeMatrix ) {
			ret.bindShapeMatrix[i / 4][i % 4] = bind.asFloat();
			i++;
		}
	}
	ret.name = skinInfo["name"].asString();
	ret.extras = skinInfo["extras"];
	
	add( key, move( ret ) );
}

const Technique& File::getTechniqueInfo( const std::string &key ) const
{
	auto found = mTechniques.find( key );
	return found->second;
}
	
template<>
void File::add( const std::string &key, Technique technique )
{
	mTechniques[key] = move( technique );
}
	
void File::addTechniqueInfo( const std::string &key, const Json::Value &techniqueInfo )
{
	CI_ASSERT( techniqueInfo["program"].isString() );
	
	Technique ret;
	ret.program = techniqueInfo["program"].asString();
	
	auto &attribs = techniqueInfo["attributes"];
	auto attribNames = attribs.getMemberNames();
	ret.attributes.reserve( attribNames.size() );
	for( int i = 0; i < attribNames.size(); i++ ) {
		auto &attribName = attribNames[i];
		auto pair = make_pair( attribName, attribs[attribName].asString() );
		ret.attributes.emplace_back( move( pair ) );
	}
	
	auto &uniforms = techniqueInfo["uniforms"];
	auto uniformNames = uniforms.getMemberNames();
	ret.attributes.reserve( uniformNames.size() );
	for( int i = 0; i < uniformNames.size(); i++ ) {
		auto &uniformName = uniformNames[i];
		auto pair = make_pair( uniformName, attribs[uniformName].asString() );
		ret.uniforms.emplace_back( move( pair ) );
	}
	
	if( ! techniqueInfo["states"].isNull() ) {
		auto &states = techniqueInfo["states"];
		if( ! states["enable"].isNull() ) {
			auto &enables = states["enable"];
			ret.states.enables.reserve( enables.size() );
			std::transform( enables.begin(), enables.end(), std::back_inserter( ret.states.enables ),
						   []( const Json::Value &val ){ return val.asUInt(); } );
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
	auto &parameters = techniqueInfo["parameters"];
	ret.parameters.reserve( parameters.size() );
	for ( auto & param : parameters ) {
		CI_ASSERT( ! param["type"].isNull() );
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
	
	ret.name = techniqueInfo["name"].asString();
	ret.extras = techniqueInfo["extras"];
	
	add( key, move( ret ) );
}
	
const Texture& File::getTextureInfo( const std::string &key ) const
{
	auto found = mTextures.find( key );
	return found->second;
}
	
template<>
void File::add( const std::string &key, Texture texture )
{
	mTextures[key] = move( texture );
}

void File::addTextureInfo( const std::string &key, const Json::Value &textureInfo )
{
	CI_ASSERT( textureInfo["sampler"].isString() );
	CI_ASSERT( textureInfo["source"].isString() );
	
	Texture ret;
	ret.source = textureInfo["source"].asString();
	ret.sampler = textureInfo["sampler"].asString();
	if( textureInfo["target"].isNumeric() )
		ret.target = textureInfo["target"].asUInt();
	if( textureInfo["format"].isNumeric() )
		ret.format = textureInfo["format"].asUInt();
	if( textureInfo["internalFormat"].isNumeric() )
		ret.internalFormat = textureInfo["internalFormat"].asUInt();
	if( textureInfo["type"].isNumeric() )
		ret.type = textureInfo["type"].asUInt();
	
	ret.name = textureInfo["name"].asString();
	ret.extras = textureInfo["extras"];
	
	add( key, move( ret ) );
}

CameraOrtho File::getOrthoCameraByName( const std::string &name )
{
	Camera cam = getCameraInfo( name );
	if( cam.type != Camera::Type::ORTHOGRAPHIC ) throw "This should be orthographic but it's not";
	
	//TODO: This is most likely wrong need to change it.
	CameraOrtho ret( -cam.xmag, cam.xmag, -cam.ymag, cam.ymag, cam.znear, cam.zfar);
	
	return ret;
}

CameraPersp File::getPerspCameraByName( const std::string &name )
{
	Camera cam = getCameraInfo( name );
	if( cam.type != Camera::Type::PERSPECTIVE ) throw "This should be perspective but it's not";
	
	CameraPersp ret; //( app->getWindowWidth(), app->getWindowHeight(), cam.yfov, cam.znear, cam.zfar );
	ret.setPerspective( cam.aspectRatio, cam.yfov, cam.znear, cam.zfar );

	return ret;
}
	
ci::geom::Primitive File::convertToPrimitive( GLenum primitive )
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
	
ci::geom::Attrib File::getAttribEnum( const std::string &attrib )
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
	
ci::gl::UniformSemantic File::getUniformEnum( const std::string &uniform )
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
	
uint8_t File::getNumComponentsForType( const std::string &type )
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

uint8_t File::getNumBytesForComponentType( GLuint type )
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
	
GlslProgRef	getGlslProgramFromMaterial( const File &gltf, const std::string &name )
{
	//std::map<std::string, GlslProgRef> GlslCache;
	//
	//auto found = GlslCache.find( name );
	//
	//if( found != GlslCache.end() ) return found->second;
	//
	//GlslProgRef ret;
	//
	//auto material = gltf.getMaterialInfo( name );
	//auto technique = gltf.getTechniqueInfo( material.instanceTechnique.technique );
	//auto pass = technique.passes[0];
	//
	//GlslProg::Format format;
	//
	//auto attribs = pass.instanceProgram.attributes;
	//for( auto attrib = attribs.begin(); attrib != attribs.end(); ++attrib ) {
	//	// attrib->first is the name of the glsl variable
	//	// attrib->second is the name into the parameter
	//	// which offers type and semantic
	//	auto found = technique.parameters.find( attrib->second );
	//	if( found != technique.parameters.end() ) {
	//		format.attrib( gltf.getAttribEnum( found->second.semantic ), attrib->first );
	//	}
	//}
	//
	//auto uniforms = pass.instanceProgram.uniforms;
	//for( auto uniform = uniforms.begin(); uniform != uniforms.end(); ++uniform ) {
	//	// uniform->first is the name of the glsl variable
	//	// uniform->second is the name into the parameter
	//	// which offers type and semantic
	//	auto found = technique.parameters.find( uniform->second );
	//	
	//	if( found != technique.parameters.end() ) {
	//		if( !found->second.semantic.empty() )
	//			format.uniform( gltf.getUniformEnum( found->second.semantic ), uniform->first );
	//	}
	//}
	//
	//auto program = gltf.getProgramInfo( pass.instanceProgram.program );
	//auto fragShader = gltf.getShaderInfo( program.frag );
	//auto vertShader = gltf.getShaderInfo( program.vert );
	//
	//format.fragment( fragShader.source ).vertex( vertShader.source );
	//
	//ret = GlslProg::create( format );
	//
	//return ret;
	return  GlslProgRef();
}

TextureRef getTextureByName( const File &gltf, const std::string &name )
{
	//static std::map<std::string, TextureRef> TextureRefCache;
	//
	//auto found = TextureRefCache.find( name );
	//
	//if( found != TextureRefCache.end() ) return found->second;
	//
	//TextureRef ret;
	//
	//auto texture = gltf.getTextureInfo( name );
	//auto sampler = gltf.getSamplerInfo( texture.sampler );
	//auto source = gltf.getImageInfo( texture.source );
	//
	//ci::gl::Texture2d::Format format;
	//format.wrapS( sampler.wrapS )
	//.wrapT( sampler.wrapT )
	//.magFilter( sampler.magFilter )
	//.minFilter( sampler.minFilter )
	//.target( texture.target )
	//.internalFormat( texture.internalFormat )
	////		.pixelDataFormat( texture.format )
	//.dataType( texture.type )
	//// TODO: Test what should be here!
	//.loadTopDown();
	//
	//ret = ci::gl::Texture::create( *(source.surface), ci::gl::Texture2d::Format().loadTopDown() );
	//
	//return ret;
	return TextureRef();
}

BatchRef getBatchFromMeshByName( const File &gltf, const std::string &name )
{
	//static std::map<std::string, BatchRef> BatchCache;
	//
	//auto found = BatchCache.find( name );
	//
	//if( found != BatchCache.end() ) return found->second;
	//
	//BatchRef ret;
	//
	////	auto glsl = getGlslProgramByName( );
	//
	//return ret;
	return BatchRef();
}

VboMeshRef getVboMeshFromMeshByName( const File &gltf, const std::string &name )
{
	//static std::map<std::string, VboMeshRef> VboMeshCache;
	//
	//auto found = VboMeshCache.find( name );
	//
	//if( found != VboMeshCache.end() ) return found->second;
	//
	//VboMeshRef ret;
	//std::vector<std::pair<geom::BufferLayout, VboRef>> arrayVbos;
	//std::map<std::string, geom::BufferLayout> layoutsForVbo;
	//Mesh mesh = gltf.getMeshInfo( name );
	//
	//uint32_t numVertices = 0;
	//for( auto & attribute : mesh.primitives[0].attributes ) {
	//	
	//	auto attribAccessor = gltf.getAccessorInfo( attribute.first );
	//	auto attribBufferView = gltf.getBufferViewInfo( attribAccessor.bufferView );
	//	auto attribBuffer = gltf.getBufferInfo( attribBufferView.buffer );
	//	
	//	auto numComponents = gltf.getNumComponentsForType( attribAccessor.type );
	//	//		auto numBytesComponent = getNumBytesForComponentType( attribAccessor.componentType );
	//	
	//	if( numVertices == 0 ) {
	//		numVertices = attribAccessor.count;
	//	}
	//	else if( numVertices != attribAccessor.count ) {
	//		CI_LOG_W( "Vertices don't match in " << attribAccessor.name << " accessor." );
	//	}
	//	
	//	auto foundBuffer = layoutsForVbo.find( attribBufferView.name );
	//	if( foundBuffer != layoutsForVbo.end() ) {
	//		auto & layout = foundBuffer->second;
	//		layout.append( attribute.second, numComponents, attribAccessor.byteStride, attribAccessor.byteOffset );
	//	}
	//	else {
	//		geom::BufferLayout layout;
	//		layout.append( attribute.second, numComponents, attribAccessor.byteStride, attribAccessor.byteOffset );
	//		layoutsForVbo.insert( make_pair( attribBufferView.name, layout ) );
	//	}
	//}
	//VboRef indices;
	//uint32_t numIndices = 0;
	//uint32_t indexComponentType = 0;
	//{
	//	auto indexAccessor = gltf.getAccessorInfo( mesh.primitives[0].indices );
	//	auto indexBufferView = gltf.getBufferViewInfo( indexAccessor.bufferView );
	//	auto indexBuffer = gltf.getBufferInfo( indexBufferView.buffer );
	//	
	//	numIndices = indexAccessor.count;
	//	indexComponentType = indexAccessor.componentType;
	//	auto ptr = (uint8_t*)indexBuffer.data->getData();
	//	indices = ci::gl::Vbo::create( GL_ELEMENT_ARRAY_BUFFER, indexBufferView.byteLength, &ptr[indexAccessor.byteOffset + indexBufferView.byteOffset], GL_STATIC_DRAW );
	//}
	//for( auto & bufferLayout : layoutsForVbo ) {
	//	auto bufferView = gltf.getBufferViewInfo( bufferLayout.first );
	//	auto buffer = gltf.getBufferInfo( bufferView.buffer );
	//	
	//	auto ptr = (uint8_t*)buffer.data->getData();
	//	VboRef temp = ci::gl::Vbo::create( GL_ARRAY_BUFFER, buffer.byteLength, &ptr[bufferView.byteOffset], GL_STATIC_DRAW );
	//	arrayVbos.push_back( make_pair( bufferLayout.second, temp ) );
	//}
	//
	//ret = VboMesh::create( numVertices, mesh.primitives[0].primitive, arrayVbos, numIndices, indexComponentType, indices );
	//
	//VboMeshCache.insert( make_pair( name, ret ) );
	//return ret;
	return VboMeshRef();
}
	
TriMeshRef getTriMeshFromMeshByName( const File &gltf, const std::string &name )
{
	//static std::map<std::string, TriMeshRef> TriMeshCache;
	//
	//auto found = TriMeshCache.find( name );
	//
	//if( found != TriMeshCache.end() ) return found->second;
	//
	//TriMesh::Format format;
	//format.positions(3);
	//format.texCoords0(2);
	//format.normals();
	//TriMeshRef ret = TriMesh::create( format );
	//Mesh mesh = gltf.getMeshInfo( name );
	//
	//for( auto & attribute : mesh.primitives[0].attributes ) {
	//	
	//	auto attribAccessor = gltf.getAccessorInfo( attribute.first );
	//	auto attribBufferView = gltf.getBufferViewInfo( attribAccessor.bufferView );
	//	auto attribBuffer = gltf.getBufferInfo( attribBufferView.buffer );
	//	
	//	auto numComponents = gltf.getNumComponentsForType( attribAccessor.type );
	//	auto numBytesComponent = gltf.getNumBytesForComponentType( attribAccessor.componentType );
	//	
	//	float * floatContainer = new float[attribAccessor.count * numComponents];
	//	uint8_t * data = (uint8_t*)attribBuffer.data->getData();
	//	
	//	memcpy( floatContainer,
	//		   &data[attribBufferView.byteOffset + attribAccessor.byteOffset],
	//		   attribAccessor.count * numComponents * numBytesComponent);
	//	
	//	// TODO: Decipher how many components and use the correct one.
	//	if( attribute.second == geom::Attrib::POSITION ) {
	//		ret->appendPositions( (vec3*)floatContainer, attribAccessor.count );
	//	}
	//	else if( attribute.second == geom::Attrib::NORMAL ) {
	//		ret->appendNormals( (vec3*)floatContainer, attribAccessor.count );
	//	}
	//	else if( attribute.second == geom::Attrib::TEX_COORD_0 ) {
	//		ret->appendTexCoords0( (vec2*)floatContainer, attribAccessor.count );
	//	}
	//	else if( attribute.second == geom::Attrib::TEX_COORD_1 ) {
	//		ret->appendTexCoords1( (vec2*)floatContainer, attribAccessor.count );
	//	}
	//	else if( attribute.second == geom::Attrib::TEX_COORD_2 ) {
	//		ret->appendTexCoords2( (vec2*)floatContainer, attribAccessor.count );
	//	}
	//	else if( attribute.second == geom::Attrib::TEX_COORD_3 ) {
	//		ret->appendTexCoords3( (vec3*)floatContainer, attribAccessor.count );
	//	}
	//	else if( attribute.second == geom::Attrib::COLOR ) {
	//		ret->appendColors( (Color*)floatContainer, attribAccessor.count );
	//	}
	//	
	//	delete [] floatContainer;
	//}
	//
	//auto indexAccessor = gltf.getAccessorInfo( mesh.primitives[0].indices );
	//auto indexBufferView = gltf.getBufferViewInfo( indexAccessor.bufferView );
	//auto indexBuffer = gltf.getBufferInfo( indexBufferView.buffer );
	//
	//std::vector<uint16_t> indices(indexAccessor.count);
	//char * data = (char*)indexBuffer.data->getData();
	//memcpy(indices.data(), &data[indexBufferView.byteOffset], indexAccessor.count * sizeof(uint16_t) );
	//std::vector<uint32_t> convertedIndices(indices.size());
	//
	//
	//for( int i = 0; i < indices.size() && i < convertedIndices.size(); ++i ) {
	//	convertedIndices[i] = indices[i];
	//}
	//
	//ret->appendIndices( convertedIndices.data(), convertedIndices.size() );
	//
	//TriMeshCache.insert( make_pair(name, ret) );
	//return ret;
	return TriMeshRef();
}
	
} // namespace gl

std::ostream& operator<<( std::ostream &lhs, const File &rhs )
{
	
	return lhs;
}
	
std::ostream& operator<<( std::ostream &lhs, const Accessor &rhs )
{
	return lhs;
}

std::ostream& operator<<( std::ostream &lhs, const Animation &rhs )
{
	return lhs;
}

std::ostream& operator<<( std::ostream &lhs, const Asset &rhs )
{
	return lhs;
}

std::ostream& operator<<( std::ostream &lhs, const BufferView &rhs )
{
	return lhs;
}

std::ostream& operator<<( std::ostream &lhs, const Buffer &rhs )
{
	return lhs;
}

std::ostream& operator<<( std::ostream &lhs, const Camera &rhs )
{
	return lhs;
}

std::ostream& operator<<( std::ostream &lhs, const Image &rhs )
{
	return lhs;
}

std::ostream& operator<<( std::ostream &lhs, const Material &rhs )
{
	return lhs;
}

std::ostream& operator<<( std::ostream &lhs, const Mesh &rhs )
{
	return lhs;
}

std::ostream& operator<<( std::ostream &lhs, const Node &rhs )
{
	return lhs;
}

std::ostream& operator<<( std::ostream &lhs, const Program &rhs )
{
	return lhs;
}

std::ostream& operator<<( std::ostream &lhs, const Sampler &rhs )
{
	return lhs;
}

std::ostream& operator<<( std::ostream &lhs, const Scene &rhs )
{
	return lhs;
}

std::ostream& operator<<( std::ostream &lhs, const Shader &rhs )
{
	return lhs;
}

std::ostream& operator<<( std::ostream &lhs, const Skin &rhs )
{
	return lhs;
}

std::ostream& operator<<( std::ostream &lhs, const Technique &rhs )
{
	return lhs;
}

std::ostream& operator<<( std::ostream &lhs, const Texture &rhs )
{
	return lhs;
}

} // namespace gltf


