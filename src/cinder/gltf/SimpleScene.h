//
//  SimpleScene.hpp
//  BasicAnimation
//
//  Created by Ryan Bartley on 8/27/16.
//
//

#pragma once

#include "cinder/gltf/Types.h"
#include "cinder/gltf/File.h"

namespace cinder { namespace gltf { namespace simple {

class Scene {
public:
	Scene( const gltf::FileRef &file, const gltf::Scene *scene );
	
	void update();
	void renderScene();
	void toggleAnimation() { mAnimate = !mAnimate; }
	
	
	class Node {
	public:
		Node( const gltf::Node *node, simple::Scene::Node *parent, Scene *scene );
		static std::unique_ptr<Node> create( const gltf::Node *node,
											 simple::Scene::Node *parent,
											 simple::Scene *scene );
		
		void update( float globalTime );
		
		const ci::vec3& getLocalTranslation() const { return mTranslation; }
		const ci::vec3& getLocalScale() const { return mScale; }
		const ci::quat& getLocalRotation() const { return mRotation; }
		
		uint32_t		getTransformIndex() { return mTransformIndex; }
		int32_t			getAnimationId() { return mAnimationIndex; }
		
		enum class Type {
			NODE,
			MESH,
			CAMERA
		};
		
		Type getNodeType() const { return mType; }
		
	private:
		Scene				*mScene;
		Node				*mParent;
		std::vector<std::unique_ptr<Node>>	mChildren;
		
		Type		mType;
		uint32_t	mTypeId;
		
		uint32_t	mTransformIndex;
		int32_t		mAnimationIndex;
		
		ci::vec3	mTranslation, mScale;
		ci::quat	mRotation;
		
		std::string mKey, mName;
	};
	
	using UniqueNode = std::unique_ptr<Node>;
	
private:
	uint32_t	setupTransform( uint32_t parentTransId, ci::mat4 localTransform );
	void		updateTransform( uint32_t transId, ci::mat4 localTransform );
	ci::mat4	getWorldTransform( uint32_t transId );
	
	int32_t		addTransformClip( TransformClip clip );
	void		getClipComponentsAtTime( int32_t animationId, float globalTime,
										 ci::vec3 *translation, ci::quat *rotation, ci::vec3 *scale );
	
	gltf::FileRef			mFile;
	std::vector<UniqueNode>	mNodes;
	
	struct Transform {
		uint32_t	parentId;
		bool		dirty;
		ci::mat4	localTransform;
		ci::mat4	worldTransform;
	};
	
	struct Mesh {
		Mesh( gl::BatchRef batch, gl::Texture2dRef difTex, ColorA difColor, Node *node );
		Mesh( const Mesh &mesh );
		Mesh& operator=( const Mesh &mesh );
		Mesh( Mesh &&mesh ) noexcept;
		Mesh& operator=( Mesh &&mesh ) noexcept;
		
		gl::BatchRef		mBatch;
		gl::Texture2dRef	mDiffuseTex;
		ColorA				mDiffuseColor;
		Node				*node;
	};
	
	struct CameraInfo {
		CameraInfo( float aspectRatio, float yfov, float znear, float zfar, Node *node );
		CameraInfo( const CameraInfo &info );
		CameraInfo& operator=( const CameraInfo &info );
		CameraInfo( CameraInfo &&info ) noexcept;
		CameraInfo& operator=( CameraInfo &&info ) noexcept;
		
		float	zfar,
				znear,
				yfov,
				aspectRatio;
		Node	*node;
	};
	
	ci::CameraPersp				mCamera;
	uint32_t					mCurrentCameraInfoId;
	
	std::vector<Mesh>			mMeshes;
	std::vector<CameraInfo>		mCameras;
	std::vector<Transform>		mTransforms;
	std::vector<TransformClip>	mTransformClips;
	double						mStartTime, mDuration;
	bool						mAnimate;
	
	friend class Node;
};
	
	
}}}
