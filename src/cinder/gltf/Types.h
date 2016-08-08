//
//  Types.h
//  Test
//
//  Created by Ryan Bartley on 8/8/16.
//
//

#pragma once

#include <memory>
#include <array>
// need this to define the GL constants, possibly figure out something else
#include "cinder/gl/gl.h"
// need this because of materials, possibly figure out something else
#include "jsoncpp/json.h"

namespace cinder {
	
class TransformClip;
template<typename T>
class Clip;
using SkeletonRef = std::shared_ptr<class Skeleton>;
	
namespace gltf {
	
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
	bool			premultipliedAlpha{false};
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
	};
	//! Animation type. Linear is the only defined type
	struct Sampler {
		enum class LerpType { LINEAR };
		std::string input, output;
		LerpType type{LerpType::LINEAR};
	};
	//! Parameter data with associated accessor.
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
	std::string		type = "arrayBuffer";
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
	ci::ImageSourceRef getImage() const { return imageSource; }
	
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
				falloffAngle{float( M_PI ) / 2.0f},
				falloffExponent{0.0f};
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
	Json::Value			values;
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
	};
	
	struct State {
		struct Functions {
			Functions();
			std::array<float, 4>	blendColor;
			std::array<uint32_t, 2>	blendEquationSeparate;
			std::array<int32_t, 4>	blendFuncSeparate;
			std::array<bool, 4>		colorMask;
			std::array<float, 2>	depthRange;
			std::array<float, 2>	polygonOffset;
			std::array<float, 4>	scissor;
			
			float		lineWidth = 1.0f;
			GLenum		cullFace = GL_BACK;
			GLenum		depthFunc = GL_LESS;
			GLenum		frontFace = GL_CCW;
			bool		depthMask = true;
		};
		std::vector<GLenum>	enables;
		Functions			functions;
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
	
} // gltf
} // cinder

std::ostream& operator<<( std::ostream &lhs, const ci::gltf::Accessor &rhs );
std::ostream& operator<<( std::ostream &lhs, const ci::gltf::Animation &rhs );
std::ostream& operator<<( std::ostream &lhs, const ci::gltf::Asset &rhs );
std::ostream& operator<<( std::ostream &lhs, const ci::gltf::BufferView &rhs );
std::ostream& operator<<( std::ostream &lhs, const ci::gltf::Buffer &rhs );
std::ostream& operator<<( std::ostream &lhs, const ci::gltf::Camera &rhs );
std::ostream& operator<<( std::ostream &lhs, const ci::gltf::Image &rhs );
std::ostream& operator<<( std::ostream &lhs, const ci::gltf::Material &rhs );
std::ostream& operator<<( std::ostream &lhs, const ci::gltf::Mesh &rhs );
std::ostream& operator<<( std::ostream &lhs, const ci::gltf::Node &rhs );
std::ostream& operator<<( std::ostream &lhs, const ci::gltf::Program &rhs );
std::ostream& operator<<( std::ostream &lhs, const ci::gltf::Sampler &rhs );
std::ostream& operator<<( std::ostream &lhs, const ci::gltf::Scene &rhs );
std::ostream& operator<<( std::ostream &lhs, const ci::gltf::Shader &rhs );
std::ostream& operator<<( std::ostream &lhs, const ci::gltf::Skin &rhs );
std::ostream& operator<<( std::ostream &lhs, const ci::gltf::Technique &rhs );
std::ostream& operator<<( std::ostream &lhs, const ci::gltf::Texture &rhs );

