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
#include "cinder/Skeleton.h"

#include "cinder/gltf/Types.h"

namespace cinder {
namespace gltf {
	
using FileRef = std::shared_ptr<class File>;
	
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
	Json::Value			getExtrasFrom( const std::string &key );
	
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
	
	TransformClip collectTransformClipFor( const Node *node ) const;
	
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
	
template<> inline const std::map<std::string, Animation>& File::getCollectionOf() const { return mAnimations; }
template<> inline const std::map<std::string, Accessor>& File::getCollectionOf() const { return mAccessors; }
template<> inline const std::map<std::string, BufferView>& File::getCollectionOf() const { return mBufferViews; }
template<> inline const std::map<std::string, Buffer>& File::getCollectionOf() const { return mBuffers; }
template<> inline const std::map<std::string, Camera>& File::getCollectionOf() const { return mCameras; }
template<> inline const std::map<std::string, Image>& File::getCollectionOf() const { return mImages; }
template<> inline const std::map<std::string, Light>& File::getCollectionOf() const { return mLights; }
template<> inline const std::map<std::string, Material>& File::getCollectionOf() const { return mMaterials; }
template<> inline const std::map<std::string, Mesh>& File::getCollectionOf() const { return mMeshes; }
template<> inline const std::map<std::string, Node>& File::getCollectionOf() const { return mNodes; }
template<> inline const std::map<std::string, Program>& File::getCollectionOf() const { return mPrograms; }
template<> inline const std::map<std::string, Sampler>& File::getCollectionOf() const { return mSamplers; }
template<> inline const std::map<std::string, Scene>& File::getCollectionOf() const { return mScenes; }
template<> inline const std::map<std::string, Shader>& File::getCollectionOf() const { return mShaders; }
template<> inline const std::map<std::string, Skin>& File::getCollectionOf() const { return mSkins; }
template<> inline const std::map<std::string, Technique>& File::getCollectionOf() const { return mTechniques; }
template<> inline const std::map<std::string, Texture>& File::getCollectionOf() const { return mTextures; }

}  // namespace gltf
}

std::ostream& operator<<( std::ostream &lhs, const ci::gltf::File &rhs );
