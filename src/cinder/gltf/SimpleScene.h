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
	
	class Node {
	public:
		Node( const gltf::Node *node, simple::Scene::Node *parent, Scene *scene );
		
		void update( float globalTime );
		void render();
		
		uint32_t getTransformIndex() { return mTransformIndex; }
		int32_t getAnimationId() { return mAnimationIndex; }
		
	private:
		Scene				*mScene;
		Node				*mParent;
		std::vector<Node>	mChildren;
		
		gl::BatchRef		mBatch;
		gl::Texture2dRef	mDiffuseTex;
		ColorA				mDiffuseColor;
		
		uint32_t	mTransformIndex;
		int32_t		mAnimationIndex;
		
		ci::vec3	mTranslation, mScale;
		ci::quat	mRotation;
		
		std::string mKey, mName;
	};
	
	void toggleAnimation() { mAnimate = !mAnimate; }
	
private:
	uint32_t	setupTransform( uint32_t parentTransId, ci::mat4 localTransform );
	void		updateTransform( uint32_t transId, ci::mat4 localTransform );
	ci::mat4	getWorldTransform( uint32_t transId );
	
	int32_t		addTransformClip( TransformClip clip );
	void		getClipComponentsAtTime( int32_t animationId, float globalTime,
										 ci::vec3 *translation, ci::quat *rotation, ci::vec3 *scale );
	
	gltf::FileRef			mFile;
	std::vector<Node>		mNodes;
	
	struct Transform {
		uint32_t	parentId;
		bool		dirty;
		ci::mat4	localTransform;
		ci::mat4	worldTransform;
	};
	
	std::vector<Transform>		mTransforms;
	std::vector<TransformClip>	mTransformClips;
	double						mStartTime, mDuration;
	bool						mAnimate;
	
	friend class Node;
};
	
	
}}}
