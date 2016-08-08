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
		std::string api{"WebGL"}, version{"1.0.3"};
	};
	std::string		copyright, generator, version;
	Profile			profile;
	bool			premultipliedAlpha{false};
	Json::Value		extras;
};
	
class File {
public:
	//! Creates a FileRef from /a gltfFile.
	static FileRef create( const ci::DataSourceRef &gltfFile );
	~File() = default;
	//! Returns a const ref to the fs::path of this gltf File.
	const ci::fs::path&	getGltfPath() const { return mGltfPath; }
	//! Returns a const ref to the Json::Value of this gltf File.
	const Json::Value&	getTree() const { return mGltfTree; }

	//! Returns whether or not this glTF File has /a extension.
	bool							hasExtension( const std::string &extension ) const;
	//! Returns the list of glTF extensions associated with this glTF File.
	const std::vector<std::string>& getExtensions() const { return mExtensions; }

	//! Templated helper which copies T
	template<typename T>
	void				get( const std::string &key, T &type );
	template<typename T>
	Json::Value			getExtrasFrom( T &type );
	
	//! Returns the Asset info associated with this glTF File.
	const Asset&		getAssetInfo() const;
	//! Returns a const ref to the default scene of this glTF File.
	const Scene&		getDefaultScene() const;
	
	//! Returns a const ref to the Accessor associated with /a key.
	const Accessor&		getAccessorInfo( const std::string &key ) const;
	//! Returns a const ref to the Animation associated with /a key.
	const Animation&	getAnimationInfo( const std::string &key ) const;
	//! Returns a const ref to the Buffer associated with /a key.
	const gltf::Buffer&	getBufferInfo( const std::string &key ) const;
	//! Returns a const ref to the BufferView associated with /a key.
	const BufferView&	getBufferViewInfo( const std::string &key ) const;
	//! Returns a const ref to the Camera associated with /a key.
	const Camera&		getCameraInfo( const std::string &key ) const;
	//! Returns a const ref to the Image associated with /a key.
	const Image&		getImageInfo( const std::string &key ) const;
	//! Returns a const ref to the Light associated with /a key.
	const Light&		getLightInfo( const std::string &key ) const;
	//! Returns a const ref to the Material associated with /a key.
	const Material&		getMaterialInfo( const std::string &key ) const;
	//! Returns a const ref to the Mesh associated with /a key.
	const Mesh&			getMeshInfo( const std::string &key ) const;
	//! Returns a const ref to the Node associated with /a key.
	const Node&			getNodeInfo( const std::string &key ) const;
	//! Returns a const ref to the Program associated with /a key.
	const Program&		getProgramInfo( const std::string &key ) const;
	//! Returns a const ref to the Sampler associated with /a key.
	const Sampler&		getSamplerInfo( const std::string &key ) const;
	//! Returns a const ref to the Scene associated with /a key.
	const Scene&		getSceneInfo( const std::string &key ) const;
	//! Returns a const ref to the Shader associated with /a key.
	const Shader&		getShaderInfo( const std::string &key ) const;
	//! Returns a const ref to the Skin associated with /a key.
	const Skin&			getSkinInfo( const std::string &key ) const;
	//! Returns a const ref to the Technique associated with /a key.
	const Technique&	getTechniqueInfo( const std::string &key ) const;
	//! Returns a cons ref to the Texture associated with /a key.
	const Texture&		getTextureInfo( const std::string &key ) const;
	
	//! Returns a const ref to the Collection of T, which can be any of the types listed above.
	template<typename T>
	const std::map<std::string, T>& getCollectionOf() const;
	//! Creates and returns a Skeleton::AnimRef based on /a skeleton.
	Skeleton::AnimRef			createSkeletonAnim( const SkeletonRef &skeleton ) const;
	//! Creates and returns a vector of TransformClips based on /a skeleton.
	std::vector<TransformClip>	createSkeletonTransformClip( const SkeletonRef &skeleton ) const;
	
private:
	//! Constructor.
	File( const ci::DataSourceRef &gltfFile );
	//! Loads the glTF into this File.
	void load();
	//! Loads the associated extionsions from this File.
	void loadExtensions();
	//! Caches the Asset Info for this glTF file.
	void setAssetInfo( const Json::Value &val );
	//! Verifies whether the glTF File is binary or regular json and loads it.
	void verifyFile( const ci::DataSourceRef &data, std::string &gltfJson );
	//! Recursive function for adding nodes to a parent. Called after all nodes are loaded.
	void setParentForChildren( Node *parent, const std::string &childKey );
	
	//! Appends Accessor Info associated with /a key.
	void addAccessorInfo( const std::string &key, const Json::Value &val );
	//! Appends Animation Info associated with /a key.
	void addAnimationInfo( const std::string &key, const Json::Value &val );
	//! Appends Buffer Info associated with /a key.
	void addBufferInfo( const std::string &key, const Json::Value &val );
	//! Appends BufferView Info associated with /a key.
	void addBufferViewInfo( const std::string &key, const Json::Value &val );
	//! Appends Camera Info associated with /a key.
	void addCameraInfo( const std::string &key, const Json::Value &val );
	//! Appends Image Info associated with /a key.
	void addImageInfo( const std::string &key, const Json::Value &val );
	//! Appends Light Info associated with /a key.
	void addLightInfo( const std::string &key, const Json::Value &val );
	//! Appends Material Info associated with /a key.
	void addMaterialInfo( const std::string &key, const Json::Value &val );
	//! Appends Mesh Info associated with /a key.
	void addMeshInfo( const std::string &key, const Json::Value &val );
	//! Appends Node Info associated with /a key.
	void addNodeInfo( const std::string &key, const Json::Value &val );
	//! Appends Program Info associated with /a key.
	void addProgramInfo( const std::string &key, const Json::Value &val );
	//! Appends Sampler Info associated with /a key.
	void addSamplerInfo( const std::string &key, const Json::Value &val );
	//! Appends Scene Info associated with /a key.
	void addSceneInfo( const std::string &key, const Json::Value &val );
	//! Appends Shader Info associated with /a key.
	void addShaderInfo( const std::string &key, const Json::Value &val );
	//! Appends Skin Info associated with /a key.
	void addSkinInfo( const std::string &key, const Json::Value &val );
	//! Appends Technique Info associated with /a key.
	void addTechniqueInfo( const std::string &key, const Json::Value &val );
	//! Appends Texture Info associated with /a key.
	void addTextureInfo( const std::string &key, const Json::Value &val );
	//! Appends type T associated with /a key.
	template<typename T>
	void				add( const std::string &key, T type );
	
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
	std::string					name, key;
};
	
//! An accessor defines a method for retrieving data as typed arrays from within a bufferView.
struct Accessor {
	//! Possible Data types in the Accessor
	enum class Type { SCALAR, VEC2, VEC3, VEC4, MAT2, MAT3, MAT4 };
	//! Possible data types of each component in the Accessor
	enum class ComponentType {
		BYTE = GL_BYTE,
		UNSIGNED_BYTE = GL_UNSIGNED_BYTE,
		SHORT = GL_SHORT,
		UNSIGNED_SHORT = GL_UNSIGNED_SHORT,
		FLOAT = GL_FLOAT
	};
	//! Returns a void* to the beginning of the data for this accessor
	void*	getDataPtr() const;
	//! Returns the number of components in the data type
	uint8_t getNumComponents() const;
	//! Returns the number of bytes per component of the data type
	uint8_t getNumBytesForComponentType() const;
	
	BufferView*			bufferView{nullptr};
	Type				dataType;
	ComponentType		componentType;
	uint32_t			byteOffset, // Required
						byteStride{0},
						count;		
	std::vector<float>	min, max;
	std::string			name, key;
};

//! Stores key frame data in buffers and references them using accessors.
struct Animation {
	
	//! Connects the output values of the key frame animation to a specific node in the hierarchy.
	struct Channel {
		std::string sampler, path;
		Node		*target{nullptr};
		std::string	targetId;
		Json::Value channelExtras, targetExtras;
	};
	struct Sampler {
		enum class LerpType { LINEAR };
		std::string input, output;
		LerpType type{LerpType::LINEAR};
	};
	struct Parameter {
		struct Data {
			std::string			paramName;
			uint32_t			numComponents;
			std::vector<float>	data;
		};
		std::string parameter;
		Accessor* accessor{nullptr};
	};
	
	//! Returns a vector of Parameter data.
	std::vector<Parameter::Data> getParameters() const;
	//! Returns a TransformClip with the information stored in /a paramData.
	static TransformClip	createTransformClip( const std::vector<Parameter::Data> &paramData );
	//! Returns a Clip<ci::vec3> representing the translation with the information stored in /a paramData.
	static Clip<ci::vec3>	createTranslationClip( const std::vector<Parameter::Data> &paramData );
	//! Returns a Clip<ci::vec3> representing the scale with the information stored in /a paramData.
	static Clip<ci::vec3>	createScaleClip( const std::vector<Parameter::Data> &paramData );
	//! Returns a Clip<ci::quat> representing the rotation with the information stored in /a paramData.
	static Clip<ci::quat>	createRotationClip( const std::vector<Parameter::Data> &paramData );
	
	std::string				target;
	std::vector<Channel>	channels;
	std::vector<Sampler>	samplers;
	Accessor				*timeAccessor{nullptr};
	std::vector<Parameter>	parameters;
	std::string				name, key;
};

struct Buffer {
	
	ci::BufferRef getBuffer() const { return data; }
	
	uint32_t		byteLength{0};
	std::string		uri; // path
	std::string		type{"arrayBuffer"};
	std::string		name, key;
	
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
	
	Buffer			*buffer{nullptr}; // Pointer to buffer
	uint32_t		byteLength{0},
					byteOffset;
	Target			target;
	std::string		name, key;
};

struct Camera {
	
	enum class Type { PERSPECTIVE, ORTHOGRAPHIC };
	ci::CameraPersp		getPerspCameraByName( const ci::mat4 &transformBake = ci::mat4() );
	ci::CameraOrtho		getOrthoCameraByName( const ci::mat4 &transformBack = ci::mat4() );
	
	std::string		name, key;
	Type			type;
	Node			*node;
	float			zfar{0.0f},
					znear{0.0f},
					// only for perspective
					yfov{0.0f},
					aspectRatio{0.0f},
					// only for orthographic
					xmag{0.0f},
					ymag{0.0f};
};

struct Image {
	ci::ImageSourceRef getImage() const;
	
	std::string			name, key;
	std::string			uri; // path
private:
	void cacheData() const;
	
	mutable ci::ImageSourceRef	imageSource;
	friend class File;
};
	
struct Light {
	//! Types of possible lights
	enum class Type { AMBIENT, DIRECTIONAL, POINT, SPOT };
	
	ci::vec4	color{0.0f, 0.0f, 0.0f, 1.0f};
	float		distance{0.0f},
				constantAttenuation{0.0f},
				linearAttenuation{1.0f},
				quadraticAttenuation{1.0f},
				falloffAngle{M_PI / 2.0},
				falloffExponent{0};
	Type		type;
	std::string name, key;
};

struct Material {
	std::string		name, key;
	Technique		*technique = nullptr;
	
	struct Source {
		enum class Type { DIFFUSE, SPECULAR, EMISSION };
		Type		type;
		Texture		*texture{nullptr};
		ci::vec4	color{0.0f, 0.0f, 0.0f, 1.0f};
	};
	
	uint32_t			jointCount = 0;
	ci::vec4			ambient{0.0f, 0.0f, 0.0f, 1.0f};
	std::vector<Source> sources;
	float				shininess{0.0f},
						transparency{1.0f};
	bool				doubleSided{false},
						transparent{false};
	
	Json::Value		values;
};

struct Mesh {
	struct Primitive {
		struct AttribAccessor {
			ci::geom::Attrib	attrib;
			Accessor			*accessor{nullptr};
		};
		std::vector<AttribAccessor>	attributes;
		Accessor			*indices{nullptr}; // Pointer to indices
		Material			*material{nullptr}; // Pointer to material
		GLenum				primitive{GL_TRIANGLES}; // ex. GL_TRIANGLES
		Json::Value			extras;
	};
	
	ci::AxisAlignedBox getPositionAABB();
	static ci::geom::Attrib getAttribEnum( const std::string &attrib );
	static ci::geom::Primitive convertToPrimitive( GLenum primitive );
	
	std::string				name, key;
	std::vector<Primitive>	primitives;
};

struct Node {
	
	size_t getNumChildren() const { return children.size(); }
	const Node* getChild( size_t index ) const;
	const Node* getChild( const std::string &nodeName ) const;
	const Node* getParent() const;
	
	ci::mat4 getHeirarchyTransform() const;
	ci::mat4 getTransformMatrix() const;
	ci::vec3 getTranslation() const;
	ci::quat getRotation() const;
	ci::vec3 getScale() const;
	
	bool isCamera() const { return camera != nullptr; }
	bool isLight() const { return light != nullptr; }
	bool hasMeshes() const { return ! meshes.empty(); }
	bool hasSkeletons() const { return ! skeletons.empty(); }
	bool hasSkin() const { return skin != nullptr; }
	bool isJoint() const { return ! jointName.empty(); }
	bool hasChildren() const { return ! children.empty(); }
	bool isRoot() const { return parent == nullptr; }
	
	void outputToConsole( std::ostream &os, uint8_t tabAmount ) const;
	
	Node					*parent{nullptr};
	Camera					*camera{nullptr};
	Skin					*skin{nullptr};
	Light					*light{nullptr};
	std::vector<Node*>		children, skeletons;
	std::vector<Mesh*>		meshes;
	std::string				jointName;
	std::vector<float>		transformMatrix,	// either 0 or 16
							rotation,			// either 0 or 4
							translation,		// either 0 or 3
							scale;				// either 0 or 3
	
	std::string				name, key;
};

struct Program {
	Shader					*frag{nullptr}, *vert{nullptr};
	std::string				 name, key;
	std::vector<std::string> attributes;
};

struct Sampler {
	std::string				name, key;
	GLenum					magFilter{GL_LINEAR},
							minFilter{GL_NEAREST_MIPMAP_LINEAR},
							wrapS{GL_REPEAT},
							wrapT{GL_REPEAT};
};

struct Shader {
	enum class Type {
		VERTEX = GL_VERTEX_SHADER,
		FRAGMENT = GL_FRAGMENT_SHADER
	};
	
	const std::string& getSource() const;
	
	std::string				name, key;
	std::string				uri; // path
	Type					type;
	
private:
	void cacheData() const;
	
	mutable std::string		source;
	friend class File;
};

struct Skin {
	
	SkeletonRef createSkeleton() const;
	
	ci::mat4			bindShapeMatrix;
	Accessor			*inverseBindMatrices{nullptr};
	std::vector<Node*>	joints;
	std::string			name, key;
};

struct Technique {
	struct Parameter {
		std::string				name;
		Node*					node{nullptr};
		std::string				semantic;
		uint32_t				count{0};
		GLenum					type;
		Json::Value				extras;
	};
	
	struct State {
		struct Functions {
			std::array<float, 4>	blendColor{ 0.0f, 0.0f, 0.0f, 0.0f };
			std::array<uint32_t, 2>	blendEquationSeparate{ 32774, 32774 };
			std::array<int32_t, 4>	blendFuncSeparate{ 1, 1, 0, 0 };
			std::array<bool, 4>		colorMask{ true, true, true, true };
			std::array<float, 2>	depthRange{ 0.0f, 1.0f };
			std::array<float, 2>	polygonOffset{ 0.0f, 0.0f };
			std::array<float, 4>	scissor{ 0.0f, 0.0f, 0.0f, 0.0f };
			
			float		lineWidth{1.0f};
			GLenum		cullFace{GL_BACK};
			GLenum		depthFunc{GL_LESS};
			GLenum		frontFace{GL_CCW};
			bool		depthMask{true};
			Json::Value	extras;
		};
		std::vector<GLenum>	enables;
		Functions			functions;
		Json::Value			extras;
	};
	
	static ci::gl::UniformSemantic getUniformEnum( const std::string &uniform );
	
	Program					*program{nullptr};
	std::string				name, key;
	std::vector<Parameter>	parameters;
	State					states;
	std::vector<std::pair<std::string, std::string>> attributes, uniforms;
};

struct Texture {
	Sampler			*sampler{nullptr};
	Image			*image{nullptr};
	GLenum			format{GL_RGBA},
					internalFormat{GL_RGBA},
					target{GL_TEXTURE_2D},
					type{GL_UNSIGNED_BYTE};
	std::string		name, key;
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
	
template<> const std::map<std::string, Animation>& File::getCollectionOf() const { return mAnimations; }
template<> const std::map<std::string, Accessor>& File::getCollectionOf() const { return mAccessors; }
template<> const std::map<std::string, BufferView>& File::getCollectionOf() const { return mBufferViews; }
template<> const std::map<std::string, Buffer>& File::getCollectionOf() const { return mBuffers; }
template<> const std::map<std::string, Camera>& File::getCollectionOf() const { return mCameras; }
template<> const std::map<std::string, Image>& File::getCollectionOf() const { return mImages; }
template<> const std::map<std::string, Light>& File::getCollectionOf() const { return mLights; }
template<> const std::map<std::string, Material>& File::getCollectionOf() const { return mMaterials; }
template<> const std::map<std::string, Mesh>& File::getCollectionOf() const { return mMeshes; }
template<> const std::map<std::string, Node>& File::getCollectionOf() const { return mNodes; }
template<> const std::map<std::string, Program>& File::getCollectionOf() const { return mPrograms; }
template<> const std::map<std::string, Sampler>& File::getCollectionOf() const { return mSamplers; }
template<> const std::map<std::string, Scene>& File::getCollectionOf() const { return mScenes; }
template<> const std::map<std::string, Shader>& File::getCollectionOf() const { return mShaders; }
template<> const std::map<std::string, Skin>& File::getCollectionOf() const { return mSkins; }
template<> const std::map<std::string, Technique>& File::getCollectionOf() const { return mTechniques; }
template<> const std::map<std::string, Texture>& File::getCollectionOf() const { return mTextures; }

}  // namespace gltf
