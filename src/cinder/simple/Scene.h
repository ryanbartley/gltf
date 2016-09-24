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
#include "cinder/simple/Node.h"

#if ! defined( _MSC_VER ) || _MSC_VER >= 1900
#define NOEXCEPT noexcept
#else 
#define NOEXCEPT
#endif

namespace cinder { namespace gltf { namespace simple {

class Scene {
public:
	Scene( const gltf::FileRef &file, const gltf::Scene *scene );

	struct Mesh {
		Mesh( gl::BatchRef batch, gl::Texture2dRef difTex, ColorA difColor, Node *node );
		Mesh( const Mesh &mesh );
		Mesh& operator=( const Mesh &mesh );
		Mesh( Mesh &&mesh ) NOEXCEPT;
		Mesh& operator=( Mesh &&mesh ) NOEXCEPT;

		gl::BatchRef		mBatch;
		gl::Texture2dRef	mDiffuseTex;
		ColorA				mDiffuseColor;
		Node				*node;
	};


	struct CameraInfo {
		CameraInfo( float aspectRatio, float yfov, float znear,
					float zfar, Node *node );
		CameraInfo( const CameraInfo &info );
		CameraInfo& operator=( const CameraInfo &info );
		CameraInfo( CameraInfo &&info ) NOEXCEPT;
		CameraInfo& operator=( CameraInfo &&info ) NOEXCEPT;

		float	zfar,
			znear,
			yfov,
			aspectRatio;
		Node	*node;
	};

	struct Transform {
		uint32_t	parentId;
		bool		dirty;
		ci::mat4	localTransform;
		ci::mat4	worldTransform;
	};
	
	void update( double globalTime );
	void renderScene();

	void setAnimate( bool animate ) { mAnimate = animate; }
	void toggleAnimation() { mAnimate = !mAnimate; }

	simple::Node*				findNodeByKey( const std::string &key );

	std::vector<Mesh>&			getMeshes() { return mMeshes; }
	const std::vector<Mesh>&	getMeshes() const { return mMeshes; }

	void							selectCamera( uint32_t selection );
	uint32_t						numCameras() const { return mCameras.size(); }
	std::vector<CameraInfo>&		getCameras() { return mCameras; }
	const std::vector<CameraInfo>&	getCameras() const { return mCameras; }
	const ci::CameraPersp&			getCamera() const { return mCamera; }
	
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
