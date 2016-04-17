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
#include "Animation.h"
#include "Skeleton.h"

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
	
	Skeleton::AnimRef		createSkeletonAnim( const SkeletonRef &skeleton ) const;
	
	ci::CameraPersp		getPerspCameraByName( const std::string &name );
	ci::CameraOrtho		getOrthoCameraByName( const std::string &name );
	
private:
	File( const ci::DataSourceRef &gltfFile );
	void load();
	void loadExtensions();
	void verifyFile( const ci::DataSourceRef &data, std::string &gltfJson );
	void setParentForChildren( Node *parent, const std::string &childKey );
	
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
	std::vector<Node*>			nodes;
	std::string					name;
	Json::Value					extras;
};
	
struct Accessor {
	
	enum class DataType { SCALAR, VEC2, VEC3, VEC4, MAT2, MAT3, MAT4 };
	enum class ComponentType {
		BYTE = GL_BYTE,
		UNSIGNED_BYTE = GL_UNSIGNED_BYTE,
		SHORT = GL_SHORT,
		UNSIGNED_SHORT = GL_UNSIGNED_SHORT,
		FLOAT = GL_FLOAT
	};
	
	void*	getDataPtr() const;
	uint8_t getNumComponents() const;
	uint8_t getNumBytesForComponentType() const;
	
	BufferView*			bufferView = nullptr;	// Required Pointer to bufferView
	DataType			dataType;
	ComponentType		componentType;
	uint32_t			byteOffset, // Required
						byteStride = 0,
						count;		// Required
	std::vector<float>	min, max;
	std::string			name;		// The user-defined name of this object.
	Json::Value			extras;
};

struct Animation {

	struct Channel {
		std::string sampler, path;
		Node		*target	= nullptr;
		std::string	targetId;
		Json::Value channelExtras, targetExtras;
	};
	struct Sampler {
		enum class LerpType { LINEAR };
		std::string input, output;
		LerpType type = LerpType::LINEAR;
	};
	struct Parameter {
		std::string parameter;
		Accessor* accessor = nullptr;
	};
	struct ParameterData {
		std::string			paramName;
		uint32_t			numComponents;
		std::vector<float>	data;
	};
	
	std::vector<ParameterData> getParameters() const;
	
	static TransformClip	createTransformClip( const std::vector<ParameterData> &paramData );
	static Clip<ci::vec3>	createTranslationClip( const std::vector<ParameterData> &paramData );
	static Clip<ci::vec3>	createScaleClip( const std::vector<ParameterData> &paramData );
	static Clip<ci::quat>	createRotationClip( const std::vector<ParameterData> &paramData );
	
	std::string				target;
	std::vector<Channel>	channels;
	std::vector<Sampler>	samplers;
	Accessor				*timeAccessor = nullptr;
	std::vector<Parameter>	parameters;
	std::string				name;
	Json::Value				extras;
};

struct Buffer {
	
	ci::BufferRef getBuffer() const { return data; }
	
	uint32_t		byteLength = 0;
	std::string		uri; // path
	std::string		type = "arrayBuffer";
	std::string		name;
	Json::Value		extras;
	
private:
	void cacheData() const;
	
	mutable ci::BufferRef		data;
	friend class File;
};

struct BufferView {
	enum class Target {
		ARRAY_BUFFER = GL_ARRAY_BUFFER,
		ELEMENT_ARRAY_BUFFER = GL_ELEMENT_ARRAY_BUFFER
	};
	
	Buffer			*buffer = nullptr; // Pointer to buffer
	uint32_t		byteLength = 0,
					byteOffset;
	Target			target;
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
	
	ci::ImageSourceRef getImage() const;
	
	std::string			name;
	std::string			uri; // path
	Json::Value			extras;
private:
	void cacheData() const;
	
	mutable ci::ImageSourceRef	imageSource;
	friend class File;
};
	
struct Light {
	enum class Type { AMBIENT, DIRECTIONAL, POINT, SPOT };
	
	ci::vec4	color = ci::vec4( 0.0f, 0.0f, 0.0f, 1.0f );
	float		distance = 0.0f,
				constantAttenuation = 0.0f,
				linearAttenuation = 1.0f,
				quadraticAttenuation = 1.0f,
				falloffAngle = M_PI / 2.0,
				falloffExponent = 0;
	Type		type;
	std::string name;
};

struct Material {
	std::string		name;
	Technique		*technique = nullptr;
	
	struct Source {
		enum class Type { DIFFUSE, SPECULAR, EMISSION };
		Type		type;
		Texture		*texture = nullptr;
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
			Accessor			*accessor = nullptr;
		};
		std::vector<AttribAccessor>	attributes;
		Accessor			*indices = nullptr; // Pointer to indices
		Material			*material = nullptr; // Pointer to material
		GLenum				primitive = 4; // ex. GL_TRIANGLES
		Json::Value			extras;
	};
	
	static ci::geom::Attrib getAttribEnum( const std::string &attrib );
	static ci::geom::Primitive convertToPrimitive( GLenum primitive );
	
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
	
	size_t getNumChildren() const { return children.size(); }
	const Node* getChild( size_t index ) const;
	const Node* getChild( const std::string &nodeName ) const;
	const Node* getParent() const;
	
	bool isCamera() const { return camera != nullptr; }
	bool isLight() const { return light != nullptr; }
	bool hasMeshes() const { return ! meshes.empty(); }
	bool hasSkeletons() const { return ! skeletons.empty(); }
	bool hasSkin() const { return skin != nullptr; }
	bool isJoint() const { return ! jointName.empty(); }
	bool hasChildren() const { return ! children.empty(); }
	bool isRoot() const { return parent == nullptr; }
	
	Node					*parent = nullptr;
	Camera					*camera = nullptr;
	Skin					*skin = nullptr;
	Light					*light = nullptr;
	std::vector<Node*>		children, skeletons;
	std::vector<Mesh*>		meshes;
	std::string				jointName;
	std::vector<float>		transformMatrix,	// either 0 or 16
							rotation,			// either 0 or 4
							translation,		// either 0 or 3
							scale;				// either 0 or 3
	std::string				name;
	Json::Value				extras;
};

struct Program {
	Shader					*frag = nullptr, *vert = nullptr;
	std::string				 name;
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
	enum class Type {
		VERTEX = GL_VERTEX_SHADER,
		FRAGMENT = GL_FRAGMENT_SHADER
	};
	
	const std::string& getSource() const;
	
	std::string				name;
	std::string				uri; // path
	Type					type;
	Json::Value				extras;
	
private:
	void cacheData() const;
	
	mutable std::string		source;
	friend class File;
};

struct Skin {
	
	SkeletonRef createSkeleton() const;
	
	ci::mat4			bindShapeMatrix;
	Accessor			*inverseBindMatrices = nullptr;
	std::vector<Node*>	joints;
	std::string			name;
	Json::Value			extras;
};

struct Technique {
	struct Parameter {
		std::string				name;
		Node*					node = nullptr;
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
	
	static ci::gl::UniformSemantic getUniformEnum( const std::string &uniform );
	
	Program					*program = nullptr;
	std::string				name;
	std::vector<Parameter>	parameters;
	State					states;
	std::vector<std::pair<std::string, std::string>> attributes, uniforms;
	Json::Value				extras;
};

struct Texture {
	Sampler			*sampler = nullptr;
	Image			*image = nullptr;
	std::string		name;
	GLenum			format = GL_RGBA,
					internalFormat = GL_RGBA,
					target = GL_TEXTURE_2D,
					type = GL_UNSIGNED_BYTE;
	Json::Value		extras;
};
	
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
