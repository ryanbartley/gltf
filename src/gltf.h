//
//  GLTF.h
//  GLTFWork
//
//  Created by Ryan Bartley on 6/19/14.
//
//

#pragma once

#include <math.h>
#include <queue>
#include <stack>

#include "jsoncpp/json.h"
#include "cinder/Utilities.h"
#include "cinder/gl/gl.h"
#include "Transformation.hpp"
#include "Animation.h"

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
struct Light;
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
	static FileRef create( const ci::DataSourceRef &gltfFile );
	~File() = default;
	
	const ci::fs::path&	getGltfPath() const { return mGltfPath; }
	const Json::Value&	getTree() const { return mGltfTree; }
	
	const Asset&		getAssetInfo() const;
	void				setAssetInfo( const Json::Value &val );
	
	const Scene&		getDefaultScene() const;
	
	bool							hasExtension( const std::string &extension ) const;
	const std::vector<std::string>& getExtensions() const { return mExtensions; }

	template<typename T>
	void				add( const std::string &key, T type );
	template<typename T>
	void				get( const std::string &key, T& type );
	
	const Accessor&		getAccessorInfo( const std::string &key ) const;
	void				addAccessorInfo( const std::string &key, const Json::Value &val );
	const Animation&	getAnimationInfo( const std::string &key ) const;
	void				addAnimationInfo( const std::string &key, const Json::Value &val );
	const gltf::Buffer&	getBufferInfo( const std::string &key ) const;
	void				addBufferInfo( const std::string &key, const Json::Value &val );
	const BufferView&	getBufferViewInfo( const std::string &key ) const;
	void				addBufferViewInfo( const std::string &key, const Json::Value &val );
	const Camera&		getCameraInfo( const std::string &key ) const;
	void				addCameraInfo( const std::string &key, const Json::Value &val );
	const Image&		getImageInfo( const std::string &key ) const;
	void				addImageInfo( const std::string &key, const Json::Value &val );
	const Light&		getLightInfo( const std::string &key ) const;
	void				addLightInfo( const std::string &key, const Json::Value &val );
	const Material&		getMaterialInfo( const std::string &key ) const;
	void				addMaterialInfo( const std::string &key, const Json::Value &val );
	const Mesh &        getMeshInfo( const std::string &key ) const;
	void				addMeshInfo( const std::string &key, const Json::Value &val );
	const Node&			getNodeInfo( const std::string &key ) const;
	void				addNodeInfo( const std::string &key, const Json::Value &val );
	const Program&		getProgramInfo( const std::string &key ) const;
	void				addProgramInfo( const std::string &key, const Json::Value &val );
	const Sampler&		getSamplerInfo( const std::string &key ) const;
	void				addSamplerInfo( const std::string &key, const Json::Value &val );
	const Scene&		getSceneInfo( const std::string &key ) const;
	void				addSceneInfo( const std::string &key, const Json::Value &val );
	const Shader&		getShaderInfo( const std::string &key ) const;
	void				addShaderInfo( const std::string &key, const Json::Value &val );
	const Skin&			getSkinInfo( const std::string &key ) const;
	void				addSkinInfo( const std::string &key, const Json::Value &val );
	const Technique&	getTechniqueInfo( const std::string &key ) const;
	void				addTechniqueInfo( const std::string &key, const Json::Value &val );
	const Texture&		getTextureInfo( const std::string &key ) const;
	void				addTextureInfo( const std::string &key, const Json::Value &val );
	
	const std::map<std::string, Animation>& getAnimations() { return mAnimations; }
	
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
	File( const ci::DataSourceRef &gltfFile );
	void load();
	void loadExtensions();
	void verifyFile( const ci::DataSourceRef &data, std::string &gltfJson );
	
	Json::Value			mGltfTree;
	cinder::fs::path	mGltfPath;
	
	std::vector<std::string> mExtensions;
	
	Asset				mAssetInfo;
	std::string			mDefaultScene;
	
	std::map<std::string, Accessor>		mAccessors;
	std::map<std::string, Animation>	mAnimations;
	std::map<std::string, BufferView>	mBufferViews;
	std::map<std::string, gltf::Buffer> mBuffers;
	std::map<std::string, Camera>		mCameras;
	std::map<std::string, Image>		mImages;
	std::map<std::string, Light>		mLights;
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
	
	ci::BufferRef	mBuffer;
	
	friend std::ostream& operator<<( std::ostream &lhs, const File &rhs );
};

struct Scene {
	std::vector<std::string>	nodes;
	std::string					name;
	Json::Value					extras;
};
	
struct Accessor {
	
	static void* getDataPtr( const FileRef &file, const Accessor &accessor );
	
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
		std::string sampler, id, path;
		Json::Value channelExtras, targetExtras;
	};
	struct Sampler {
		enum class LerpType { LINEAR };
		std::string input, output;
		LerpType type = LerpType::LINEAR;
	};
	struct Parameter {
		std::string parameter, accessor;
	};
	struct ParameterData {
		std::string			paramName;
		uint32_t			numComponents;
		std::vector<float>	data;
	};
	
	std::vector<ParameterData> getParameters( const FileRef &file ) const;
	
	static Clip<Transform>	createTransformClip( const std::vector<ParameterData> &paramData );
	static Clip<ci::vec3>	createTranslationClip( const std::vector<ParameterData> &paramData );
	static Clip<ci::vec3>	createScaleClip( const std::vector<ParameterData> &paramData );
	static Clip<ci::quat>	createRotationClip( const std::vector<ParameterData> &paramData );
	
	std::vector<Channel>	channels;
	std::vector<Sampler>	samplers;
	std::string				timeAccessor;
	std::vector<Parameter>	parameters;
	std::string				name;
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
	std::string		buffer; // Pointer to buffer
	uint32_t		byteLength = 0;
	uint32_t		byteOffset;
	uint32_t		target;
	std::string		name;
	Json::Value		extras;
};

struct Camera {
	enum class Type { PERSPECTIVE, ORTHOGRAPHIC };

	std::string		name;
	Type			type;
	float			zfar = 0.0f,
					znear = 0.0f,
					// only for perspective
					yfov = 0.0f,
					aspectRatio = 0.0f,
					// only for orthographic
					xmag = 0.0f,
					ymag = 0.0f;
	Json::Value		extras, camSpecificExtras;
};

struct Image {
	std::string			name;
	std::string			uri; // path
	ci::ImageSourceRef	imageSource;
	Json::Value			extras;
};
	
struct Light {
	enum class Type { AMBIENT, DIRECTIONAL, POINT, SPOT };
	
	ci::vec4	color = ci::vec4( 0.0f, 0.0f, 0.0f, 1.0f );
	float		distance = 0.0f,
				constantAttenuation = 0.0f,
				linearAttenuation = 1.0f,
				quadraticAttenuation = 1.0f,
				falloffAngle = M_PI / 2,
				falloffExponent = 0;
	Type		type;
};

struct Material {
	std::string		name;
	std::string		technique;
	
	struct Source {
		enum class Type { DIFFUSE, SPECULAR, EMISSION };
		Type		type;
		std::string texture;
		ci::vec4	color = ci::vec4( 0.0f, 0.0f, 0.0f, 1.0f );
	};
	
	uint32_t			jointCount = 0;
	ci::vec4			ambient = ci::vec4( 0.0f, 0.0f, 0.0f, 1.0f );
	std::vector<Source> sources;
	float				shininess = 0.0f,
						transparency = 1.0f;
	bool				doubleSided = false,
						transparent = false;
	
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
	
	void outputToConsole( std::ostream &os, uint8_t tabAmount ) const;
	
	ci::mat4 getTransformMatrix() const;
	ci::vec3 getTranslation() const;
	ci::quat getRotation() const;
	ci::vec3 getScale() const;
	
	bool isCamera() const { return ! camera.empty(); }
	bool isLight() const { return ! light.empty(); }
	bool hasMesh() const { return ! meshes.empty(); }
	bool hasSkeletons() const { return ! skeletons.empty(); }
	bool isJoint() const { return ! jointName.empty(); }
	bool hasChildren() const { return ! children.empty(); }
	
	std::string				 parent;
	std::vector<std::string> children, meshes, skeletons;
	std::string				 camera, jointName, skin, light;
	std::vector<float>		 transformMatrix,	// either 0 or 16
							 rotation,			// either 0 or 4
							 translation,		// either 0 or 3
							 scale;				// either 0 or 3
	std::string				 name;
	Json::Value				 extras;
};
	
struct NodeBreadthIter {
	NodeBreadthIter( const FileRef &file, const Node &root );
	NodeBreadthIter( const FileRef &file, const std::string &rootName );
	
	bool hasNext() const;
	const Node* next();
	
private:
	FileRef					mFile;
	std::queue<const Node*>	mQueue;
	
};
	
struct NodeDepthIter {
	NodeDepthIter( const FileRef &file, const Node &root );
	NodeDepthIter( const FileRef &file, const std::string &rootName );
	
	bool hasNext() const;
	const Node* next();
	
private:
	FileRef					mFile;
	std::stack<const Node*> mStack;
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
	std::string					inverseBindMatricesAccessor;
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
			std::array<uint32_t, 2>	blendEquationSeparate = { 32774, 32774 };
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
	std::string		sampler;
	std::string		source;
	std::string		name;
	GLenum			format = GL_RGBA,
					internalFormat = GL_RGBA,
					target = GL_TEXTURE_2D,
					type = GL_UNSIGNED_BYTE;
	Json::Value		extras;
};
	
namespace gl {
	
ci::gl::BatchRef	getBatchFromMeshByName( const Scene &gltf, const std::string &name );
ci::gl::VboMeshRef	getVboMeshFromMeshByName( const Scene &gltf, const std::string &name );
ci::gl::GlslProgRef	getGlslProgramFromMaterial( const Scene &gltf, const std::string &name );
ci::gl::TextureRef	getTextureByName( const Scene &gltf, const std::string &name );
ci::TriMeshRef		getTriMeshFromMeshByName( const Scene &gltf, const std::string &name );
	
} // namespace gl
	
std::ostream& operator<<( std::ostream &lhs, const File &rhs );
std::ostream& operator<<( std::ostream &lhs, const Accessor &rhs );
std::ostream& operator<<( std::ostream &lhs, const Animation &rhs );
std::ostream& operator<<( std::ostream &lhs, const Asset &rhs );
std::ostream& operator<<( std::ostream &lhs, const BufferView &rhs );
std::ostream& operator<<( std::ostream &lhs, const Buffer &rhs );
std::ostream& operator<<( std::ostream &lhs, const Camera &rhs );
std::ostream& operator<<( std::ostream &lhs, const Image &rhs );
std::ostream& operator<<( std::ostream &lhs, const Material &rhs );
std::ostream& operator<<( std::ostream &lhs, const Mesh &rhs );
std::ostream& operator<<( std::ostream &lhs, const Node &rhs );
std::ostream& operator<<( std::ostream &lhs, const Program &rhs );
std::ostream& operator<<( std::ostream &lhs, const Sampler &rhs );
std::ostream& operator<<( std::ostream &lhs, const Scene &rhs );
std::ostream& operator<<( std::ostream &lhs, const Shader &rhs );
std::ostream& operator<<( std::ostream &lhs, const Skin &rhs );
std::ostream& operator<<( std::ostream &lhs, const Technique &rhs );
std::ostream& operator<<( std::ostream &lhs, const Texture &rhs );

}  // namespace gltf
