//
//  GLTFLoader.h
//  GLTFWork
//
//  Created by Ryan Bartley on 11/22/14.
//
//

#include "cinder/GeomIo.h"
#include "cinder/CinderAssert.h"
#include "cinder/gltf/Types.h"

namespace cinder {
namespace gltf {

//! Represents a Mesh Geom for gltf::Mesh(s). Note: gltf::File associated with Mesh used to build
//! this, needs to outlive this loader.
class MeshLoader : public ci::geom::Source {
public:
	//! Constructor taking a gltf Mesh pointer.
	MeshLoader( const Mesh *mesh );
	~MeshLoader() = default;
	
	//! Returns the number of vertices contained within the Mesh.
	virtual size_t	getNumVertices() const { return mNumVertices; }
	//! Returns the number of indices contained within the Mesh.
	virtual size_t	getNumIndices() const { return mNumIndices; }
	//! Returns the geom::Primitive that this mesh will be represented as.
	virtual ci::geom::Primitive	getPrimitive() const { return mPrimitive; }
	//! Returns the number of dimensions contained in this /a attr.
	virtual uint8_t	getAttribDims( ci::geom::Attrib attr ) const;
	//! Loads attibutes into /a target.
	virtual void	loadInto( ci::geom::Target *target, const ci::geom::AttribSet &requestedAttribs ) const;
	//! Returns the set of available attributes available in this mesh.
	virtual ci::geom::AttribSet	getAvailableAttribs() const { return mAvailableAttribs; }
	//! Clones this source and returns a copy of this MeshLodader.
	virtual Source*		clone() const { return new MeshLoader( *this ); }
	
	//! Represents a beginning and ending vert or index and the Material associated with this.
	struct MeshInstance {
		MeshInstance( Material *material, uint32_t first, uint32_t count )
		: material( material ), first( first ), count( count ) {}
		Material *material;
		uint32_t first;
		uint32_t count;
	};
	//! Returns Mesh Instance for this mesh.
	const std::vector<MeshInstance>& getMeshInstances() { return mMeshInstances; }
	
private:
	template<typename T>
	void copyIndices( std::vector<uint32_t> &indices, const T* data, uint32_t count ) const;
	
	const Mesh			*mMesh;
	ci::geom::AttribSet mAvailableAttribs;
	size_t				mNumVertices, mNumIndices;
	ci::geom::Primitive mPrimitive;
	
	std::map<ci::geom::Attrib, const Accessor*> mAttribAccessors;
	std::vector<const Accessor*>	mIndexAccessors;
	std::vector<MeshInstance>		mMeshInstances;
	
};

} // namespace gltf
} // namespace cinder