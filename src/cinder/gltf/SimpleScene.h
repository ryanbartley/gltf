//
//  SimpleScene.hpp
//  BasicAnimation
//
//  Created by Ryan Bartley on 8/27/16.
//
//

#pragma once

#include "cinder/CameraUi.h"

#include "cinder/gltf/Types.h"
#include "cinder/gltf/File.h"

namespace cinder { namespace gltf { namespace simple {

class Scene {
public:
	Scene( const gltf::FileRef &file, const gltf::Scene *scene );
	
	void update( double globalTime );
	void renderScene();
	void setAnimate( bool animate ) { mAnimate = animate; }
	void toggleAnimation() { mAnimate = !mAnimate; }
	
	class Node {
	public:
		Node( const gltf::Node *node, simple::Scene::Node *parent, Scene *scene );
		static std::unique_ptr<Node> create( const gltf::Node *node,
											 simple::Scene::Node *parent,
											 simple::Scene *scene );
		Node* getParent() { return mParent; }
		void update( float globalTime );
		
		const ci::vec3& getLocalTranslation() const { return mCurrentTrans; }
		const ci::vec3& getLocalScale() const { return mCurrentScale; }
		const ci::quat& getLocalRotation() const { return mCurrentRot; }
		
		uint32_t		getTransformIndex() { return mTransformIndex; }
		int32_t			getAnimationId() { return mAnimationIndex; }
		
		const ci::mat4& getLocalTransform() const;
		const ci::mat4& getWorldTransform() const;
		ci::mat4		getParentWorldTransform() const;
		
		enum class Type {
			NODE,
			MESH,
			CAMERA
		};
		
		Type getNodeType() const { return mType; }
		
		Node* findNodeByKey( const std::string &key );
		
	private:
		Scene	*mScene;
		Node	*mParent;
		std::vector<std::unique_ptr<Node>>	mChildren;
		
		Type		mType;
		uint32_t	mTypeId;
		
		uint32_t	mTransformIndex;
		int32_t		mAnimationIndex;
		
		ci::vec3	mOriginalTranslation, mOriginalScale;
		ci::quat	mOriginalRotation;
		
		ci::vec3	mCurrentTrans, mCurrentScale;
		ci::quat	mCurrentRot;
		
		std::string mKey, mName;
	};
	
	Node* findNodeByKey( const std::string &key );
	
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
	
	std::vector<Mesh>&			getMeshes() { return mMeshes; }
	const std::vector<Mesh>&	getMeshes() const { return mMeshes; }
	
	struct CameraInfo {
		CameraInfo( float aspectRatio, float yfov, float znear,
				    float zfar, Node *node );
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
	
	void		selectCamera( uint32_t selection );
	uint32_t	numCameras() const { return mCameras.size(); }
	std::vector<CameraInfo>&		getCameras() { return mCameras; }
	const std::vector<CameraInfo>&	getCameras() const { return mCameras; }
	const ci::CameraPersp&			getCamera() const { return mCamera; }
	
	struct Transform {
		uint32_t	parentId;
		bool		dirty;
		ci::mat4	localTransform;
		ci::mat4	worldTransform;
	};
	
	using UniqueNode = std::unique_ptr<Node>;
	
private:
	uint32_t	setupTransform( uint32_t parentTransId, ci::mat4 localTransform );
	void		updateTransform( uint32_t transId, ci::mat4 localTransform );
	ci::mat4&	getWorldTransform( uint32_t transId );
	ci::mat4&	getLocalTransform( uint32_t transId );
	ci::mat4	getParentWorldTransform( uint32_t transId );
	
	int32_t		addTransformClip( TransformClip clip );
	void		getClipComponentsAtTime( int32_t animationId, float globalTime,
										 ci::vec3 *translation, ci::quat *rotation, ci::vec3 *scale );
	
	gltf::FileRef			mFile;
	std::vector<UniqueNode>	mNodes;

	ci::CameraPersp				mCamera;
	ci::CameraUi				mDebugCamera;
	bool						mUsingDebugCamera;
	
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
