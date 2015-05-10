//
//  GltfContainer.h
//  GLTFWork
//
//  Created by Ryan Bartley on 6/24/14.
//
//

#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Vbo.h"
#include "cinder/params/Params.h"

#pragma once

typedef std::function<void(const ci::JsonTree&)> JsonHandler;
typedef std::pair<std::string, JsonHandler> NameHandlerPair;
typedef std::pair<std::string, ci::gl::TextureRef> NameTexturePair;
typedef std::pair<std::string, std::vector<float>> NameValuePair;
typedef std::map<std::string, GLenum> NameEnumMap;
typedef std::vector<NameHandlerPair> CategoryHandlers;

typedef std::shared_ptr<ci::Surface>				SurfaceRef;
typedef std::shared_ptr<ci::Buffer>					BufferRef;
typedef std::map<std::string, std::string>			KeyValue;
typedef std::map<std::string, ci::geom::Attrib>		AccessorAttribMap;

namespace gltf {
	
struct InstanceTechnique {
	std::map<std::string, ci::ColorAf> colors;
	std::map<std::string, std::string> textures;
	std::string				technique;
	float					shininess;
};
	
struct Material {
	std::string				name;
	std::string				key;
	InstanceTechnique		instanceTechnique;
};
	
struct Buffer {
	BufferRef				data;
	uint32_t				byteLength;
	std::string				uri; // path
	std::string				type;
};
	
struct BufferView {
	
	Buffer getBuffer();
	
	std::string				name;
	std::string				buffer; // Pointer to buffer
	uint32_t				byteLength;
	uint32_t				byteOffset;
	uint32_t				target;
};

struct Accessor {
	
	BufferView getBufferView();
	
	std::string				bufferView;		// Pointer to bufferView
	std::string				name;			// for use in lookup
	uint32_t				componentType;
	uint32_t				byteOffset;
	uint32_t				byteStride;
	uint32_t				count;
	ci::AxisAlignedBox3f	box;
	bool					boxSet = false;
	std::string				type; // type of data in string form Aka VEC4
};
	
struct Mesh {
	
	Material getMaterial();
	Accessor getAccessors( ci::geom::Attrib attrib );
	Accessor getIndices();
	
	
	std::string				key;
	std::string				name;
	AccessorAttribMap		attributes;
	std::string				indices; // Pointer to indices
	std::string				material; // Pointer to material
	GLenum					primitive; // ex. GL_TRIANGLES
};

struct Shader {
	std::string				name;
	std::string				uri; // path
	uint32_t				type;
	ci::DataSourceRef		source;
};

struct Program {
	std::string				name;
	std::vector<std::string> attributes;
	std::string				frag; // Pointer to frag
	std::string				vert; // Pointer to vert
};

struct Image {
	
	void loadSurface();
	
	std::string				name;
	std::string				uri; // path
	bool					generateMipMap = false;
	SurfaceRef				surface;
};

struct Sampler {
	GLenum					magFilter,
							minFilter,
							wrapS,
							wrapT;
	std::string				name;
};

struct Texture {
	
	Sampler getSampler();
	Image getImage();
	
	GLenum					format,
							internalFormat,
							target,
							type;
	std::string				sampler;
	std::string				source;
	std::string				name;
};

struct Camera {
	std::string				name;
	std::string				type;
	float					zfar = 0.0f,
							znear = 0.0f,
							// only for perspective
							yfov = 0.0f,
							aspectRatio = 0.0f,
							// only for orthographic
							xmag = 0.0f,
							ymag = 0.0f;
};
	
struct Parameter {
	std::string				name;
	GLenum					type;
	std::string				semantic;
	std::vector<float>		values;
	std::string				source;
};
	
struct Details {
	std::string				type;
	std::string				lightingModel;
	std::vector<std::string> parameters;
	KeyValue				texcoordBindings;
};
	
struct InstanceProgram {
	std::string				program;
	KeyValue				attributes;
	KeyValue				uniforms;
};
	
struct State {
	bool					depthMask;
	std::vector<GLenum>		enables;
};
	
struct Pass {
	
	std::string				name;
	Details					detail;
	InstanceProgram			instanceProgram;
	State					states;
};
	
struct Technique {
	std::string				name;
	std::map<std::string, Parameter>	parameters;
	//TODO: figure out if this should be a vector
	std::string				defaultPass;
	std::vector<Pass>		passes;
};
	
struct Node {
	std::vector<std::string> children;
	std::vector<std::string> meshes;
	std::string				camera;
	std::string				light;
	std::string				joint;
	std::string				instanceSkin;
	ci::quat				rotation;
	ci::vec3				translation;
	ci::vec3				scale = ci::vec3( 1.0f );
	ci::mat4				transformMatrix;
	std::string				name;
};
	
struct Scene {
	std::vector<std::string> nodes;
	std::string				name;
};

}