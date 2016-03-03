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

#include "GltfContainer.h"

namespace gltf {
	
class Scene;
class File;
using FileRef = std::shared_ptr<File>;
	
struct Accessor;
struct Animation;
struct BufferView;
struct Buffer;
struct Camera;
struct Image;
struct Material;
struct Mesh;
struct Node;
struct Program;
struct Sampler;
struct Scene;
struct Shader;
struct Skin;
struct Technique;
struct Texture;
	
struct Asset {
	struct Profile {
		std::string api = "WebGL", version = "1.0.3";
	};
	std::string		copyright, generator, version;
	Profile			profile;
	bool			premultipliedAlpha = false;
	Json::Value		extras;
};
	
class File {
public:
	static FileRef create( const ci::DataSourceRef &gltfFile, bool cacheFile = false );
	~File();
	
	bool hasExtension( const std::string &extension ) const;
	const std::vector<std::string>& getExtensions() const;
	const ci::fs::path& getGltfPath() const;
	const Json::Value& getTree() const;
	
	Accessor			getAccessorInfo( const std::string &key ) const;
	Animation			getAnimationInfo( const std::string &key ) const;
	const Asset&		getAssetInfo() const;
	BufferView			getBufferViewInfo( const std::string &key ) const;
	gltf::Buffer		getBufferInfo( const std::string &key ) const;
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
	

	//! Returns the converted string as a geom::Attrib. Attribute semantics
	//! include POSITION, NORMAL, TEXCOORD, COLOR, JOINT, JOINTMATRIX, and
	//! WEIGHT.  Attribute semantics can be of the form [semantic]_[set_index],
	//! e.g, TEXCOORD_0, TEXCOORD_1, etc."
	static ci::geom::Attrib getAttribEnum( const std::string &attrib );
	static ci::geom::Primitive convertToPrimitive( GLenum primitive );
	static ci::gl::UniformSemantic getUniformEnum( const std::string &uniform );
	static uint8_t getNumComponentsForType( const std::string &type );
	static uint8_t getNumBytesForComponentType( GLuint type );
	
	ci::CameraPersp		getPerspCameraByName( const std::string &name );
	ci::CameraOrtho		getOrthoCameraByName( const std::string &name );
	
private:
	File( const ci::DataSourceRef &gltfFile, bool cacheFile );
	void load();
	void loadAssetInfo();
	void loadExtensions();
	
	Json::Value			mTree;
	cinder::fs::path	mGltfPath;
	std::vector<std::string> mExtensions;
	Asset				mAssetInfo;
	
	std::map<std::string, Accessor>		mAccessors;
	std::map<std::string, Animation>	mAnimations;
	std::map<std::string, BufferView>	mBufferViews;
	std::map<std::string, gltf::Buffer> mBuffers;
	std::map<std::string, Camera>		mCameras;
	std::map<std::string, Image>		mImages;
	std::map<std::string, Material>		mMaterials;
	std::map<std::string, Mesh>			mMeshes;
	std::map<std::string, Node>			mNodes;
	std::map<std::string, Program>		mPrograms;
	std::map<std::string, Sampler>		mSamplers;
	std::map<std::string, Scene>		mScenes;
	std::map<std::string, Shader>		mShaders;
	std::map<std::string, Skin>			mSkins;
	std::map<std::string, Technique>	mTechniques;
	std::map<std::string, Texture>		mTextures;
};

class Scene {
public:
	Scene( const Json::Value &val );
	Scene( const ci::DataSourceRef gltfFile );
	
	const FileRef& getFile() const;
	const std::string& getSceneName() const;
	
	class Iter {
		
	};
	
	Iter begin();
	Iter end();
	
private:
	FileRef				mFile;
	std::string			mSceneName;
	std::vector<Node>	mNodes;
};
	
struct Accessor {
	std::string			bufferView;	// Required Pointer to bufferView
	uint32_t			byteOffset; // Required
	uint32_t			byteStride = 0;
	uint32_t			componentType; // Required
	uint32_t			count;		// Required
	std::string			type;		// Required, type of data in string form Aka VEC4
	std::vector<float>	min, max;
	std::string			name;		// The user-defined name of this object.
	Json::Value			extras;
};

struct Animation {
	struct Channel {
		std::string sampler, targetId, targetPath;
		Json::Value channelExtras, targetExtras;
	};
	struct Sampler {
		std::string input, interpolation = "LINEAR", output;
	};
	
	std::vector<Channel>	channels;
	std::vector<Sampler>	samplers;
	std::string				name;
	Json::Value				parameters;
	Json::Value				extras;
};

struct Buffer {
	ci::BufferRef	data;
	uint32_t		byteLength = 0;
	std::string		uri; // path
	std::string		type = "arrayBuffer";
	std::string		name;
	Json::Value		extras;
};

struct BufferView {
	std::string		name;
	std::string		buffer; // Pointer to buffer
	uint32_t		byteLength = 0;
	uint32_t		byteOffset;
	uint32_t		target;
	Json::Value		extras;
};

struct Camera {
	std::string		name;
	std::string		type;
	float			zfar = 0.0f,
	znear = 0.0f,
	// only for perspective
	yfov = 0.0f,
	aspectRatio = 0.0f,
	// only for orthographic
	xmag = 0.0f,
	ymag = 0.0f;
	Json::Value		extras, orthoExtras, perspExtras;
};

struct Image {
	std::string			name;
	std::string			uri; // path
	ci::ImageSourceRef	imageSource;
	Json::Value			extras;
};

struct Material {
	std::string		name;
	std::string		technique;
	Json::Value		values;
	Json::Value		extras;
};

struct Mesh {
	struct Primitive {
		struct AttribAccessor {
			ci::geom::Attrib	attrib;
			std::string			accessor;
		};
		std::vector<AttribAccessor>	attributes;
		std::string			indices; // Pointer to indices
		std::string			material; // Pointer to material
		GLenum				primitive = 4; // ex. GL_TRIANGLES
		Json::Value			extras;
	};
	
	
	std::string				name;
	std::vector<Primitive>	primitives;
	Json::Value				extras;
};

struct Node {
	std::vector<std::string> children, meshes, skeletons;
	std::string				 camera, jointName, skin;
	ci::quat				 rotation;
	ci::vec3				 translation, scale = ci::vec3( 1.0f );
	ci::mat4				 transformMatrix;
	std::string				 name;
	Json::Value				 extras;
};

struct Program {
	std::string				 name, fragmentShader, vertexShader;
	std::vector<std::string> attributes;
	Json::Value				 extras;
};

struct Sampler {
	std::string				name;
	GLenum					magFilter = GL_LINEAR,
	minFilter = GL_NEAREST_MIPMAP_LINEAR,
	wrapS = GL_REPEAT,
	wrapT = GL_REPEAT;
	Json::Value				extras;
};

struct Shader {
	std::string				name;
	std::string				uri; // path
	uint32_t				type;
	std::string				source;
	Json::Value				extras;
};

struct Skin {
	ci::mat4					bindShapeMatrix;
	std::string					inverseBindMatrices;
	std::vector<std::string>	jointNames;
	std::string					name;
	Json::Value					extras;
};

struct Technique {
	struct Parameter {
		std::string				name;
		std::string				node;
		std::string				semantic;
		uint32_t				count = 0;
		GLenum					type;
		Json::Value				extras;
	};
	
	struct State {
		struct Functions {
			std::array<float, 4>	blendColor = { 0.0f, 0.0f, 0.0f, 0.0f };
			std::array<float, 2>	blendEquationSeparate = { 32774, 32774 };
			std::array<int32_t, 4>	blendFuncSeparate = { 1, 1, 0, 0 };
			std::array<bool, 4>		colorMask = { true, true, true, true };
			std::array<float, 2>	depthRange = { 0.0f, 1.0f };
			std::array<float, 2>	polygonOffset = { 0.0f, 0.0f };
			std::array<float, 4>	scissor = { 0.0f, 0.0f, 0.0f, 0.0f };
			float		lineWidth	= 1.0f;
			GLenum		cullFace	= GL_BACK;
			GLenum		depthFunc	= GL_LESS;
			GLenum		frontFace	= GL_CCW;
			bool		depthMask	= true;
			Json::Value	extras;
		};
		std::vector<GLenum>	enables;
		Functions			functions;
		Json::Value			extras;
	};
	
	std::string				name;
	std::vector<Parameter>	parameters;
	State					states;
	std::string				program;
	std::vector<std::pair<std::string, std::string>> attributes, uniforms;
	Json::Value				extras;
};

struct Texture {
	std::string				sampler;
	std::string				source;
	std::string				name;
	GLenum					format = GL_RGBA,
	internalFormat = GL_RGBA,
	target = GL_TEXTURE_2D,
	type = GL_UNSIGNED_BYTE;
};

	
namespace gl {
	
ci::gl::BatchRef	getBatchFromMeshByName( const Scene &gltf, const std::string &name );
ci::gl::VboMeshRef	getVboMeshFromMeshByName( const Scene &gltf, const std::string &name );
ci::gl::GlslProgRef	getGlslProgramFromMaterial( const Scene &gltf, const std::string &name );
ci::gl::TextureRef	getTextureByName( const Scene &gltf, const std::string &name );
ci::TriMeshRef		getTriMeshFromMeshByName( const Scene &gltf, const std::string &name );
	
} } // namespace gl // namespace gltf
