//
//  GLTF.cpp
//  GLTFWork
//
//  Created by Ryan Bartley on 6/19/14.
//
//

#include "GLTF.h"

#include "cinder/gl/Vbo.h"

using namespace ci;
using namespace ci::app;
using namespace ci::gl;
using namespace std;

namespace gltf {

Gltf::Gltf( const std::string &fileName, bool cache )
: mFileName( fileName ), mTree( loadAsset( fileName ) )
{
	load( cache );
}

void Gltf::load( bool cache )
{
	// GltfContents.json
	mTree = JsonTree( loadAsset( mFileName ) );

	if( cache ) {
//		for( const NameHandlerPair& aPair : mCategories ) {
//			const string& name = aPair.first;
//			console() << "// " << name << endl;
//			const JsonHandler& handler = aPair.second;
//			
//			if ( mTree.hasChild( name ) ) {
//				handler( mTree.getChild( name ) );
//			}
//			console() << endl;
//		}
//		makeGlReady();
	}
}

void Gltf::makeGlReady()
{
	
}

BatchRef Gltf::getBatchFromMeshByName( const std::string &name )
{
	static std::map<std::string, BatchRef> BatchCache;
	
	auto found = BatchCache.find( name );
	
	if( found != BatchCache.end() ) return found->second;
	
	BatchRef ret;
	
	
	
//	auto glsl = getGlslProgramByName( );
	
	return ret;
}

VboMeshRef Gltf::getVboMeshFromMeshByName( const std::string &name )
{
	static std::map<std::string, VboMeshRef> VboMeshCache;
	
	auto found = VboMeshCache.find( name );
	
	if( found != VboMeshCache.end() ) return found->second;
	
	VboMeshRef ret;
	std::vector<std::pair<geom::BufferLayout, VboRef>> arrayVbos;
	std::map<std::string, geom::BufferLayout> layoutsForVbo;
	Mesh mesh = getMeshInfo( name );
	
	uint32_t numVertices = 0;
	for( auto & attribute : mesh.attributes ) {
		
		auto attribAccessor = getAccessorInfo( attribute.first );
		auto attribBufferView = getBufferViewInfo( attribAccessor.bufferView );
		auto attribBuffer = getBufferInfo( attribBufferView.buffer );
		
		auto numComponents = getNumComponentsForType( attribAccessor.type );
//		auto numBytesComponent = getNumBytesForComponentType( attribAccessor.componentType );
		
		if( numVertices == 0 ) {
			numVertices = attribAccessor.count;
		}
		else if( numVertices != attribAccessor.count ) {
			std::string warning = "Vertices don't match in ";
			warning += attribAccessor.name;
			warning += " accessor";
			CI_LOG_W( warning );
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
		auto indexAccessor = getAccessorInfo( mesh.indices );
		auto indexBufferView = getBufferViewInfo( indexAccessor.bufferView );
		auto indexBuffer = getBufferInfo( indexBufferView.buffer );
		
		numIndices = indexAccessor.count;
		indexComponentType = indexAccessor.componentType;
		auto ptr = (uint8_t*)indexBuffer.data->getData();
		indices = gl::Vbo::create( GL_ELEMENT_ARRAY_BUFFER, indexBufferView.byteLength, &ptr[indexAccessor.byteOffset + indexBufferView.byteOffset], GL_STATIC_DRAW );
	}
	for( auto & bufferLayout : layoutsForVbo ) {
		auto bufferView = getBufferViewInfo( bufferLayout.first );
		auto buffer = getBufferInfo( bufferView.buffer );
		
		auto ptr = (uint8_t*)buffer.data->getData();
		VboRef temp = gl::Vbo::create( GL_ARRAY_BUFFER, buffer.byteLength, &ptr[bufferView.byteOffset], GL_STATIC_DRAW );
		arrayVbos.push_back( make_pair( bufferLayout.second, temp ) );
	}
	
	ret = VboMesh::create( numVertices, mesh.primitive, arrayVbos, numIndices, indexComponentType, indices );
	
	VboMeshCache.insert( make_pair( name, ret ) );
	return ret;
}

TriMeshRef Gltf::getTriMeshFromMeshByName( const std::string &name )
{
	static std::map<std::string, TriMeshRef> TriMeshCache;
	
	auto found = TriMeshCache.find( name );
	
	if( found != TriMeshCache.end() ) return found->second;
	
	TriMesh::Format format;
	format.positions(3);
	format.texCoords0(2);
	format.normals();
	TriMeshRef ret = TriMesh::create( format );
	Mesh mesh = getMeshInfo( name );
	
	for( auto & attribute : mesh.attributes ) {
		
		auto attribAccessor = getAccessorInfo( attribute.first );
		auto attribBufferView = getBufferViewInfo( attribAccessor.bufferView );
		auto attribBuffer = getBufferInfo( attribBufferView.buffer );
		
		auto numComponents = getNumComponentsForType( attribAccessor.type );
		auto numBytesComponent = getNumBytesForComponentType( attribAccessor.componentType );
		
		float * floatContainer = new float[attribAccessor.count * numComponents];
		uint8_t * data = (uint8_t*)attribBuffer.data->getData();
		
		memcpy( floatContainer,
			   &data[attribBufferView.byteOffset + attribAccessor.byteOffset],
			   attribAccessor.count * numComponents * numBytesComponent);
		
		// TODO: Decipher how many components and use the correct one.
		if( attribute.second == geom::Attrib::POSITION ) {
			ret->appendVertices( (vec3*)floatContainer, attribAccessor.count );
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
	
	auto indexAccessor = getAccessorInfo( mesh.indices );
	auto indexBufferView = getBufferViewInfo( indexAccessor.bufferView );
	auto indexBuffer = getBufferInfo( indexBufferView.buffer );
	
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

Accessor Gltf::getAccessorInfo( const std::string& key ) const
{
	static std::map<std::string, Accessor> AccessorCache;
	
	auto found = AccessorCache.find( key );
	
	if( found != AccessorCache.end() ) return found->second;
	
	Accessor ret;
	auto accessors = mTree.getChild( "accessors" );
	auto accessor = accessors.getChild( key );
	ret.bufferView = accessor["bufferView"].getValue();
	ret.byteOffset = accessor["byteOffset"].getValue<uint32_t>();
	ret.byteStride = accessor["byteStride"].getValue<uint32_t>();
	ret.count = accessor["count"].getValue<uint32_t>();
	ret.type = accessor["type"].getValue();
	ret.componentType = accessor["componentType"].getValue<uint32_t>();
	ret.name = key;
	if( accessor.hasChild( "max" ) ) {
		float minNum[3], maxNum[3];
		memset(minNum, 0, 3);
		memset(maxNum, 0, 3);
		auto minContainer = accessor.getChild( "min" ).getChildren();
		auto maxContainer = accessor.getChild( "max" ).getChildren();
		auto minIt = minContainer.begin();
		auto minEnd = minContainer.end();
		auto maxIt = maxContainer.begin();
		auto maxEnd = maxContainer.end();
		for( int i = 0; minIt != minEnd && maxIt != maxEnd ; ++i ) {
			minNum[i] = (*minIt++).getValue<float>();
			maxNum[i] = (*maxIt++).getValue<float>();
		}
		vec3 maxVec( maxNum[0], maxNum[1], maxNum[2] );
		vec3 minVec( minNum[0], minNum[1], maxNum[2] );
		ret.box = AxisAlignedBox3f( maxVec, minVec );
		ret.boxSet = true;
	}
	
	AccessorCache.insert( make_pair( key, ret ) );
	
	return ret;
}

BufferView Gltf::getBufferViewInfo( const std::string &name ) const
{
	static std::map<std::string, BufferView> BufferViewCache;
	
	auto found = BufferViewCache.find( name );
	
	if( found != BufferViewCache.end() ) return found->second;
	
	BufferView ret;
	auto bufferViews = mTree.getChild( "bufferViews" );
	auto bufferView = bufferViews.getChild( name );
	ret.buffer = bufferView["buffer"].getValue();
	ret.byteLength = bufferView["byteLength"].getValue<uint32_t>();
	ret.byteOffset = bufferView["byteOffset"].getValue<uint32_t>();
	ret.target = bufferView["target"].getValue<uint32_t>();
	ret.name = name;
	
	BufferViewCache.insert( make_pair(name, ret) );
	return ret;
}
	
Buffer Gltf::getBufferInfo( const std::string &name ) const
{
	static std::map<std::string, Buffer> BufferCache;
	
	auto found = BufferCache.find( name );
	
	if( found != BufferCache.end() ) return found->second;
	
	Buffer ret;
	auto buffers = mTree.getChild("buffers");
	auto buffer = buffers.getChild( name );
	ret.uri = buffer["uri"].getValue();
	ret.type = buffer["type"].getValue();
	ret.byteLength = buffer["byteLength"].getValue<uint32_t>();
	
	ret.data = make_shared<ci::Buffer>( ci::app::loadAsset( ret.uri )->getBuffer() );
	
	BufferCache.insert( make_pair( name, ret ) );
	return ret;
}
	
Mesh Gltf::getMeshInfo( const std::string &key ) const
{
	static std::map<std::string, Mesh> MeshCache;
	
	auto found = MeshCache.find( key );
	
	if( found != MeshCache.end() ) return found->second;
	
	Mesh ret;
	auto meshes = mTree.getChild( "meshes" );
	auto mesh = meshes.getChild( key );
	ret.key = key;
	ret.name = mesh["name"].getValue();
	auto primitives = mesh.getChild( "primitives" ).getChild(0);
	auto attributes = primitives.getChild("attributes").getChildren();
	ret.material = primitives["material"].getValue();
	ret.primitive = primitives["primitive"].getValue<uint32_t>();
	
	for( auto & attribute : attributes ) {
		auto attribString = attribute.getKey();
		ret.attributes.insert( make_pair( attribute.getValue(),
										 getAttribEnum( attribString ) ) );
	}
	
	ret.indices = primitives.getChild( "indices" ).getValue();
	
	return ret;
}
	
Program Gltf::getProgramInfo( const std::string &key ) const
{
	static std::map<std::string, Program> ProgramCache;
	
	auto found = ProgramCache.find( key );
	
	if( found != ProgramCache.end() ) return found->second;
	
	Program ret;
	
	auto programs = mTree.getChild( "programs" );
	auto program = programs.getChild( key );
	ret.name = key;
	ret.vert = program["vertexShader"].getValue();
	ret.frag = program["fragmentShader"].getValue();
	auto attributes = program["attributes"].getChildren();
	
	for( auto & attribute : attributes ) {
		ret.attributes.push_back( attribute.getValue() );
	}
	
	ProgramCache.insert( make_pair( key, ret ) );

	return ret;
}
	
Shader Gltf::getShaderInfo( const std::string &key ) const
{
	static std::map<std::string, Shader> ShaderCache;
	
	auto found = ShaderCache.find( key );
	
	if( found != ShaderCache.end() ) return found->second;
	
	Shader ret;
	
	auto shaders = mTree.getChild( "shaders" );
	auto shader = shaders.getChild( key );
	ret.name = key;
	ret.type = shader["type"].getValue<uint32_t>();
	ret.uri = shader["uri"].getValue();
	ret.source = ci::app::loadAsset( ret.uri );
	
	ShaderCache.insert( make_pair( key, ret ) );
	
	return ret;
}
	
Sampler Gltf::getSamplerInfo( const std::string &key ) const
{
	static std::map<std::string, Sampler> SamplerCache;
	
	auto found = SamplerCache.find( key );
	
	if( found != SamplerCache.end() ) return found->second;
	
	Sampler ret;
	
	auto samplers = mTree.getChild( "samplers" );
	auto sampler = samplers.getChild( key );
	ret.magFilter = sampler["magFilter"].getValue<GLenum>();
	ret.minFilter = sampler["minFilter"].getValue<GLenum>();
	ret.wrapS = sampler["wrapS"].getValue<GLenum>();
	ret.wrapT = sampler["wrapT"].getValue<GLenum>();
	ret.name = key;
	
	SamplerCache.insert( make_pair( key, ret ) );
	
	return ret;
}
	
Image Gltf::getImageInfo( const std::string &key ) const
{
	static std::map<std::string, Image> ImageCache;
	
	auto found = ImageCache.find( key );
	
	if( found != ImageCache.end() ) return found->second;
	
	Image ret;
	
	auto images = mTree.getChild( "images" );
	auto image = images.getChild( key );
	ret.name = key;
	ret.uri = image["uri"].getValue();
	
	//TODO: Make this deal with image data type. Basically, we're supplied with image type so we should create a surface of that image type.
	ret.surface = make_shared<Surface>( loadImage( loadAsset( ret.uri ) ) );
	
	ImageCache.insert( make_pair( key, ret ) );
	
	return ret;
}
	
Texture Gltf::getTextureInfo( const std::string &key ) const
{
	static std::map<std::string, Texture> TextureCache;
	
	auto found = TextureCache.find( key );
	
	if( found != TextureCache.end() ) return found->second;
	
	Texture ret;
	
	auto textures = mTree.getChild( "textures" );
	auto texture = textures.getChild( key );
	ret.name = key;
	ret.target = texture["target"].getValue<GLenum>();
	ret.format = texture["format"].getValue<GLenum>();
	ret.internalFormat = texture["internalFormat"].getValue<GLenum>();
	ret.type = texture["type"].getValue<GLenum>();
	ret.source = texture["source"].getValue();
	ret.sampler = texture["sampler"].getValue();
	
	TextureCache.insert( make_pair( key, ret ) );
	
	return ret;
}
	
TextureRef Gltf::getTextureByName( const std::string &name )
{
	static std::map<std::string, TextureRef> TextureRefCache;
	
	auto found = TextureRefCache.find( name );
	
	if( found != TextureRefCache.end() ) return found->second;
	
	TextureRef ret;
	
	auto texture = getTextureInfo( name );
	auto sampler = getSamplerInfo( texture.sampler );
	auto source = getImageInfo( texture.source );
	
	gl::Texture::Format format;
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
	
	ret = gl::Texture::create( *(source.surface), gl::Texture::Format().loadTopDown() );
	
	return ret;
}
	
Camera Gltf::getCameraInfo( const std::string &key ) const
{
	static std::map<std::string, Camera> CameraCache;
	
	auto found = CameraCache.find( key );
	
	if( found != CameraCache.end() ) return found->second;
	
	Camera ret;
	
	auto cameras = mTree.getChild( "cameras" );
	auto camera = cameras.getChild( key );
	ret.name = key;
	ret.type = camera["type"].getValue();
	if ( ret.type == "perspective" ) {
		auto perspectiveInfo = camera["perspective"];
		ret.aspectRatio = perspectiveInfo["aspectRatio"].getValue<float>();
		ret.yfov = perspectiveInfo["yfov"].getValue<float>();
		ret.znear = perspectiveInfo["znear"].getValue<float>();
		ret.zfar = perspectiveInfo["zfar"].getValue<float>();
	}
	else if( ret.type == "orthographic" ) {
		auto orthographicInfo = camera["orthographic"];
		ret.aspectRatio = orthographicInfo["aspectRatio"].getValue<float>();
		ret.yfov = orthographicInfo["yfov"].getValue<float>();
		ret.znear = orthographicInfo["znear"].getValue<float>();
		ret.zfar = orthographicInfo["zfar"].getValue<float>();
	}
	
	CameraCache.insert( make_pair( key, ret ) );
	return ret;
}
	

CameraOrtho Gltf::getOrthoCameraByName( const std::string &name )
{
	Camera cam = getCameraInfo( name );
	if( cam.type != "orthographic" ) throw "This should be orthographic but it's not";
	
	//TODO: This is most likely wrong need to change it.
	CameraOrtho ret( -cam.xmag, cam.xmag, -cam.ymag, cam.ymag, cam.znear, cam.zfar);
	
	return ret;
}

CameraPersp Gltf::getPerspCameraByName( const std::string &name )
{
	Camera cam = getCameraInfo( name );
	if( cam.type != "orthographic" ) throw "This should be perspective but it's not";
	
	auto app = app::App::get();
	
	CameraPersp ret( app->getWindowWidth(), app->getWindowHeight(), cam.yfov, cam.znear, cam.zfar );
	
	return ret;
}
	
Technique Gltf::getTechniqueInfo( const std::string &key ) const
{
	static std::map<std::string, Technique> TechniqueCache;
	
	auto found = TechniqueCache.find( key );
	
	if( found != TechniqueCache.end() ) return found->second;
	
	Technique ret;
	
	auto techniques = mTree.getChild( "techniques" );
	auto technique = techniques.getChild( key );
	
	ret.name = key;
	
	// Parameters Excavation...
	auto parameters = technique["parameters"].getChildren();

	for( auto paramIt = parameters.begin(); paramIt != parameters.end(); ++paramIt ) {
		Parameter param;
		param.name = paramIt->getKey();
		param.type = (*paramIt)["type"].getValue<GLenum>();
		if( (*paramIt).hasChild( "semantic" ) ) {
			param.semantic = (*paramIt)["semantic"].getValue();
		}
		if( (*paramIt).hasChild( "source" ) ) {
			param.source = (*paramIt)["source"].getValue();
		}
		if( (*paramIt).hasChild( "value" ) ) {
			auto values = (*paramIt).getChild( "value" ).getChildren();
			param.values.resize( values.size() );
			auto valIt = values.begin();
			auto retValIt = param.values.begin();
			for( ; valIt != values.end() && retValIt != param.values.end(); ) {
				(*retValIt++) = (*valIt++).getValue<float>();
			}
		}
		ret.parameters.insert( make_pair( param.name, param ) );
	}
	
	// Pass Name..
	ret.defaultPass = technique["pass"].getValue();
	
	// Passes...
	auto passes = technique["passes"].getChildren();
	ret.passes.resize( passes.size() );
	auto passIt = passes.begin();
	auto retPassIt = ret.passes.begin();
	for (; passIt != passes.end() && retPassIt != ret.passes.end() ; ++passIt, ++retPassIt ) {
		
		// Add the pass name
		retPassIt->name = passIt->getKey();
		
		// Grab the details
		auto details = (*passIt)["details"];
		Details retDetail;
		retDetail.type = details["type"].getValue();
		auto detailProfile = details["commonProfile"];
		retDetail.lightingModel = detailProfile["lightingModel"].getValue();
		// Detail Parameters inside CommonProfile
		auto profileParameters = detailProfile["parameters"].getChildren();
		retDetail.parameters.resize( profileParameters.size() );
		auto paramIt = profileParameters.begin();
		auto detailParamIt = retDetail.parameters.begin();
		for( ; paramIt != profileParameters.end() && detailParamIt != retDetail.parameters.end(); ++paramIt, ++detailParamIt ) {
			*detailParamIt = paramIt->getValue();
		}
		// Tex Coord Bindings inside Common Profile
		auto profileTexCoordBindings = detailProfile["texcoordBindings"].getChildren();
		auto texIt = profileTexCoordBindings.begin();
		for( ; texIt != profileTexCoordBindings.end(); ++texIt ) {
			retDetail.texcoordBindings.insert( make_pair( texIt->getKey(), texIt->getValue() ) );
		}
		
		// Add the pass detail
		retPassIt->detail = retDetail;
		
		// Grab the instance program
		auto instanceProgram = (*passIt)["instanceProgram"];
		
		InstanceProgram retInstanceProgram;
		retInstanceProgram.program = instanceProgram["program"].getValue();
		
		// Get Instance Program Attributes
		auto attributes = instanceProgram["attributes"].getChildren();
		auto attribIt = attributes.begin();
		for( ; attribIt != attributes.end(); ++attribIt ) {
			retInstanceProgram.attributes.insert( make_pair( (*attribIt).getKey(), (*attribIt).getValue() ) );
		}
		
		// Get Instance Program Uniforms
		auto uniforms = instanceProgram["uniforms"].getChildren();
		auto uniformIt = uniforms.begin();
		for( ; uniformIt != uniforms.end(); ++uniformIt ) {
			retInstanceProgram.uniforms.insert( make_pair( (*uniformIt).getKey(), (*uniformIt).getValue() ) );
		}
		
		// Add the pass Instance Program
		retPassIt->instanceProgram = retInstanceProgram;
		
		auto states = (*passIt)["states"];
		State state;
		state.depthMask = states["depthMask"].getValue<bool>();
		auto enables = states["enable"].getChildren();
		auto enableIt = enables.begin();
		for(; enableIt != enables.end(); ++enableIt ) {
			state.enables.push_back( enableIt->getValue<GLenum>() );
		}
		
		retPassIt->states = state;
	}
	
	
	TechniqueCache.insert( make_pair( key, ret ) );
	
	return ret;
}


GlslProgRef	Gltf::getGlslProgramFromMaterial( const std::string &name )
{
	std::map<std::string, GlslProgRef> GlslCache;
	
	auto found = GlslCache.find( name );
	
	if( found != GlslCache.end() ) return found->second;
	
	GlslProgRef ret;
	
	auto material = getMaterialInfo( name );
	auto technique = getTechniqueInfo( material.instanceTechnique.technique );
	auto pass = technique.passes[0];
	
	GlslProg::Format format;
	
	auto attribs = pass.instanceProgram.attributes;
	for( auto attrib = attribs.begin(); attrib != attribs.end(); ++attrib ) {
		// attrib->first is the name of the glsl variable
		// attrib->second is the name into the parameter
		// which offers type and semantic
		auto found = technique.parameters.find( attrib->second );
		if( found != technique.parameters.end() ) {
			format.attrib( getAttribEnum( found->second.semantic ), attrib->first );
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
				format.uniform( getUniformEnum( found->second.semantic ), uniform->first );
		}
	}
	
	auto program = getProgramInfo( pass.instanceProgram.program );
	auto fragShader = getShaderInfo( program.frag );
	auto vertShader = getShaderInfo( program.vert );
	
	format.fragment( fragShader.source ).vertex( vertShader.source );
	
	ret = GlslProg::create( format );
	
	return ret;
}
	
Material Gltf::getMaterialInfo( const std::string &key ) const
{
	std::map<std::string, Material> MaterialCache;
	
	auto found = MaterialCache.find( key );
	
	if( found != MaterialCache.end() ) return found->second;
	
	Material ret;
	
	auto materials = mTree.getChild( "materials" );
	auto material = materials.getChild( key );
	
	ret.name = material["name"].getValue();
	
	InstanceTechnique retTech;
	
	auto instanceTech = material["instanceTechnique"];
	retTech.technique = instanceTech["technique"].getValue();
	auto values = instanceTech["values"];
	
	if( values["ambient"].getNodeType() == JsonTree::NODE_ARRAY ) {
		std::array<float, 4> ambient;
		auto ambientArray = values["ambient"].getChildren();
		auto ambientIt = ambientArray.begin();
		cout << "Ambient: ";
		for( int i = 0; i < 4 && ambientIt != ambientArray.end(); ++ambientIt, ++i ) {
			ambient[i] = ambientIt->getValue<float>();
			cout << ambient[i] << " ";
		}
		cout << endl;
		retTech.colors.insert( make_pair( "ambient", ci::ColorAf( ambient[0], ambient[1], ambient[2], ambient[3] ) ) );
	}
	else if( values["ambient"].getNodeType() == JsonTree::NODE_VALUE ) {
		retTech.textures.insert( make_pair( "ambient", values["ambient"].getValue() ) );
	}
	
	if( values["diffuse"].getNodeType() == JsonTree::NODE_ARRAY ) {
		std::array<float, 4> diffuse;
		auto diffuseArray = values["diffuse"].getChildren();
		auto diffuseIt = diffuseArray.begin();
		cout << "Diffuse: ";
		for( int i = 0; i < 4 && diffuseIt != diffuseArray.end(); ++diffuseIt, ++i ) {
			diffuse[i] = diffuseIt->getValue<float>();
			cout << diffuse[i] << " ";
		}
		cout << endl;
		retTech.colors.insert( make_pair( "diffuse", ColorAf( diffuse[0], diffuse[1], diffuse[2], diffuse[3] ) ) );
	}
	else if( values["diffuse"].getNodeType() == JsonTree::NODE_VALUE ) {
		retTech.textures.insert( make_pair( "diffuse", values["diffuse"].getValue() ) );
	}
	
	if( values["emission"].getNodeType() == JsonTree::NODE_ARRAY ) {
		std::array<float, 4> emission;
		auto emissionArray = values["emission"].getChildren();
		auto emissionIt = emissionArray.begin();
		cout << "Emission: ";
		for( int i = 0; i < 4 && emissionIt != emissionArray.end(); ++emissionIt, ++i ) {
			emission[i] = emissionIt->getValue<float>();
			cout << emission[i] << " ";
		}
		cout << endl;
		retTech.colors.insert( make_pair( "emission", ColorAf( emission[0], emission[1], emission[2], emission[3] ) ) );
	}
	else if( values["emission"].getNodeType() == JsonTree::NODE_VALUE ) {
		retTech.textures.insert( make_pair( "emission", values["emission"].getValue() ) );
	}
	
	if( values["specular"].getNodeType() == JsonTree::NODE_ARRAY ) {
		std::array<float, 4> specular;
		auto specularArray = values["specular"].getChildren();
		auto specularIt = specularArray.begin();
		cout << "Specular: ";
		for( int i = 0; i < 4 && specularIt != specularArray.end(); ++specularIt, ++i ) {
			specular[i] = specularIt->getValue<float>();
			cout << specular[i] << " ";
		}
		cout << endl;
		retTech.colors.insert( make_pair( "specular", ColorAf( specular[0], specular[1], specular[2], specular[3] ) ) );
	}
	else if( values["specular"].getNodeType() == JsonTree::NODE_VALUE ) {
		retTech.textures.insert( make_pair( "specular", values["specular"].getValue() ) );
	}
	
	retTech.shininess = values["shininess"].getValue<float>();
	
	ret.instanceTechnique = retTech;
	
	MaterialCache.insert( make_pair( key, ret ) );
	
	return ret;
}
	
Node Gltf::getNodeInfo( const std::string &key ) const
{
	std::map<std::string, Node> NodeCache;
	
	auto found = NodeCache.find( key );
	
	if( found != NodeCache.end() ) return found->second;
	
	Node ret;
	
	auto nodes = mTree.getChild( "nodes" );
	auto node = nodes.getChild( key );
	
	ret.name = node["name"].getKey();
	
	if( node.hasChild( "camera" ) ) {
		ret.camera = node["camera"].getValue();
	}
	else if( node.hasChild( "meshes" ) ) {
		auto meshes = node["meshes"].getChildren();
		ret.meshes.resize( meshes.size() );
		auto meshIt = meshes.begin();
		auto retMeshIt = ret.meshes.begin();
		for( ; meshIt != meshes.end(); ++meshIt, ++retMeshIt ) {
			(*retMeshIt) = meshIt->getValue();
		}
	}
	else if( node.hasChild( "light" ) ) {
		ret.light = node["light"].getValue();
	}
	else if( node.hasChild( "instanceSkin" ) ) {
		ret.instanceSkin = node["instanceSkin"].getValue();
		if( node.hasChild( "joint" ) ) {
			ret.joint = node["joint"].getValue();
		}
	}
	
	if( node.hasChild( "matrix" ) ) {
		array<float, 16> tempMat;
		auto matrix = node["matrix"].getChildren();
		auto matIt = matrix.begin();
		for( int i = 0; matIt != matrix.end(); i++, ++matIt ) {
			tempMat[i] = matIt->getValue<float>();
		}
		ret.transformMatrix = ci::mat4( tempMat[0] );
	}
	else {
		if( node.hasChild( "translation" ) ) {
			array<float, 3> tempTrans;
			auto translation = node["translation"].getChildren();
			auto transIt = translation.begin();
			for( int i = 0; transIt != translation.end(); i++, ++transIt ) {
				tempTrans[i] = transIt->getValue<float>();
			}
			ret.translation = ci::vec3( tempTrans[0], tempTrans[1], tempTrans[2] );
		}
		if( node.hasChild( "rotation" ) ) {
			array<float, 4> tempRot;
			auto rotation = node["rotation"].getChildren();
			auto rotIt = rotation.begin();
			for( int i = 0; rotIt != rotation.end(); i++, ++rotIt ) {
				tempRot[i] = rotIt->getValue<float>();
			}
			ret.rotation = ci::quat( tempRot[0], tempRot[1], tempRot[2], tempRot[3] );
		}
		if( node.hasChild( "scale" ) ) {
			array<float, 3> tempScale;
			auto scale = node["scale"].getChildren();
			auto scaleIt = scale.begin();
			for( int i = 0; scaleIt != scale.end(); i++, ++scaleIt ) {
				tempScale[i] = scaleIt->getValue<float>();
			}
			ret.scale = ci::vec3( tempScale[0], tempScale[1], tempScale[2] );
		}
	}
	
	NodeCache.insert( make_pair( key, ret ) );
	return ret;
}
	
Scene Gltf::getSceneInfo( const std::string &key ) const
{
	std::map<std::string, Scene> SceneCache;
	
	auto found = SceneCache.find( key );
	
	if( found != SceneCache.end() ) return found->second;
	
	Scene ret;
	
	auto scenes = mTree.getChild( "scenes" );
	auto scene = scenes.getChild( key );
	
	ret.name = key;
	
	auto nodes = scene["nodes"].getChildren();
	ret.nodes.resize( nodes.size() );
	auto nodeIt = nodes.begin();
	auto retNodeIt = ret.nodes.begin();
	for( ; nodeIt != nodes.end(); ++nodeIt, ++retNodeIt ) {
		(*retNodeIt) = nodeIt->getValue();
	}
	
	return ret;
}
	
void Gltf::render( const std::string &sceneKey )
{
	auto scene = getSceneInfo( sceneKey );
	
	static std::vector<Node> nodes;
	if( nodes.empty() ) {
		for( auto nodeIt = scene.nodes.begin(); nodeIt != scene.nodes.end(); ++nodeIt ) {
			nodes.push_back( getNodeInfo( *nodeIt ) );
		}
		
	}
	
}

	
}


