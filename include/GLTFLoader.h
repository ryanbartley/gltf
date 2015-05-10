//
//  GLTFLoader.h
//  GLTFWork
//
//  Created by Ryan Bartley on 11/22/14.
//
//

#include "cinder/GeomIo.h"
#include "cinder/ObjLoader.h"
#include "cinder/Json.h"
#include "GLTF.h"

namespace gltf {

class GltfLoader : public ci::geom::Source {
public:
	GltfLoader( const ci::JsonTree &tree, const std::string &meshId );
	GltfLoader( const gltf::Gltf &gltf, const std::string &meshId );
	GltfLoader( const ci::DataSourceRef &dataSource, const std::string &meshId );
	
	~GltfLoader();
	
	virtual size_t		getNumVertices() const;
	virtual size_t		getNumIndices() const;
	virtual ci::geom::Primitive	getPrimitive() const;
	virtual uint8_t		getAttribDims( ci::geom::Attrib attr ) const;
	virtual void		loadInto( ci::geom::Target *target, const ci::geom::AttribSet &requestedAttribs ) const;
	
	virtual ci::geom::AttribSet	getAvailableAttribs() const { return mAvailableAttribs; }
	
	struct Mesh {
		ci::geom::BufferLayout mBufferLayout;
		uint32_t mNumVertices = 0;
		std::map<ci::geom::Attrib, std::vector<uint8_t>> mAttributes;
		
		std::vector<uint8_t> mIndices;
		uint32_t mNumIndices = 0;
		uint8_t mBytesPerIndex = 0;
		ci::geom::Primitive mPrimitive;
	};
	
private:
	
	ci::geom::AttribSet mAvailableAttribs;
	std::vector<Mesh> mMeshes;
	std::map<ci::geom::Attrib, uint32_t> mAttribVerticesSize;
};

}