//
//  Skeleton.h
//  Test
//
//  Created by ryan bartley on 3/5/16.
//
//

#pragma once

#include "Transformation.hpp"
#include "Animation.h"

using SkeletonRef = std::shared_ptr<class Skeleton>;

class Skeleton {
public:
	Skeleton( uint8_t numJoints );
	
	struct Joint {
		Joint();
		Joint( uint8_t parentId, uint8_t nameId );
		
		void setInverseBindMatrix( const glm::mat4 &inverseBind ) { mInverseBindMatrix = inverseBind; }
		glm::mat4& getInverseBindMatrix() { return mInverseBindMatrix; }
		const glm::mat4& getInverseBindMatrix() const { return mInverseBindMatrix; }
		
		uint8_t getNameId() const { return mNameId; }
		uint8_t getParentId() const { return mParentId; }
		
	private:
		void setNameId( uint8_t nameId ) { mNameId = nameId; }
		void setParentId( uint8_t parentId ) { mParentId = parentId; }
		
		glm::mat4	mInverseBindMatrix;
		uint8_t		mNameId;
		uint8_t		mParentId;
		
		friend class Skeleton;
	};
	
	class Pose {
	public:
		Pose( Skeleton *skeleton );
		
		void calcMatrixPalette( std::vector<ci::mat4> &offsetMatrices );
		void calcGlobalStack();
		
		const std::vector<Transform>&	getLocalJointTransforms() const { return mLocalJointTransforms; }
		std::vector<Transform>&			getLocalJointTransforms() { return mLocalJointTransforms; }
		const std::vector<ci::mat4>&	getGlobalJointMatrices() const { return mGlobalJointMatrices; }
		std::vector<ci::mat4>&			getGlobalJointMatrices() { return mGlobalJointMatrices; }
		bool							needsGlobalCache() const { return mNeedsGlobalCache; }
		void setNeedsGlobalCache( bool needsCache ) { mNeedsGlobalCache = needsCache; }
		
	private:
		Skeleton				*mSkeleton = nullptr;
		std::vector<Transform>	mLocalJointTransforms;
		std::vector<ci::mat4>	mGlobalJointMatrices;
		bool					mNeedsGlobalCache;
	};
	
	class PartialPose {
		PartialPose( Skeleton *skeleton, const std::string &rootName );
		PartialPose( Skeleton *skeleton, const Joint &rootJoint );
		
	};
	
	using AnimRef = std::shared_ptr<class Anim>;
	
	class Anim {
	public:
		Anim( const std::vector<Clip<Transform>> &jointClips );
		
		void get( double time, Pose &localJointTransforms ) const;
		void getLooped( double time, Pose &localJointTransforms  ) const;
		
	private:
		std::vector<Clip<Transform>> mJointClips;
	};
	
	const Joint*	getRoot() const { return &mJointArray[0]; }
	const Joint*	getJoint( const std::string& name ) const;
	const Joint*	getJoint( uint8_t jointId ) const;
	bool			hasJoint( const std::string& name ) const;
	size_t			getNumJoints() { return mJointArray.size(); }
	
	bool			jointIsChildOf( uint8_t childIndex, uint8_t parentIndex ) const;
	
	const Pose&		getBindPose() const { return mBindPose; }
	Pose&			getBindPose() { return mBindPose; }
	
	const std::string*	getJointName( const Joint &joint ) const;
	const std::string*	getJointName( uint8_t nameId ) const;
	
	const std::vector<Joint>&		getJoints() const { return mJointArray; }
	std::vector<Joint>&				getJoints() { return mJointArray; }
	
	const std::vector<std::string>& getJointNames() const { return mJointNames; }
	
	void calcGlobalStack( const Pose &jointLocalTransforms,
						  std::vector<ci::mat4> &globalStack );
	void calcMatrixPalette( const std::vector<ci::mat4> &globalCache,
						   std::vector<ci::mat4> &offsetMatrices ) const;
	
	ci::AxisAlignedBox calcBoundingBox() const;
	
private:
	std::vector<Joint>			mJointArray;
	Pose						mBindPose;
	std::vector<std::string>	mJointNames;
};

inline void Skeleton::Pose::calcMatrixPalette( std::vector<ci::mat4> &offsetMatrices )
{
	if( mNeedsGlobalCache )
		calcGlobalStack();
	
	auto numJoints = mGlobalJointMatrices.size();
	offsetMatrices.resize( numJoints );
	
	auto &joints = mSkeleton->getJoints();
	// Derive root
	offsetMatrices[0] = joints[0].getInverseBindMatrix() * mGlobalJointMatrices[0];
	// Derive children
	for( int i = 1; i < numJoints; i++ ) {
		offsetMatrices[i] = joints[i].getInverseBindMatrix() * mGlobalJointMatrices[i];
	}
}

inline void Skeleton::Pose::calcGlobalStack()
{
	auto numJoints = mGlobalJointMatrices.size();
	auto &joints = mSkeleton->getJoints();
	// Derive root
	mGlobalJointMatrices[0] = mLocalJointTransforms[0].getTRS();
	// Derive children
	for( int i = 1; i < numJoints; i++ ) {
		mGlobalJointMatrices[i] = mGlobalJointMatrices[joints[i].getParentId()] * mLocalJointTransforms[i].getTRS();
	}
}

inline void Skeleton::Anim::get( double time, Skeleton::Pose &localJointTransforms ) const
{
	auto &jointTransforms = localJointTransforms.getLocalJointTransforms();
	CI_ASSERT( jointTransforms.size() == mJointClips.size() );
	int i = 0;
	for( auto & jointClip : mJointClips ) {
		jointTransforms[i++] = jointClip.get( time );
	}
	localJointTransforms.setNeedsGlobalCache( true );
}

inline void Skeleton::Anim::getLooped( double time, Skeleton::Pose &localJointTransforms ) const
{
	auto &jointTransforms = localJointTransforms.getLocalJointTransforms();
	CI_ASSERT( jointTransforms.size() == mJointClips.size() );
	int i = 0;
	for( auto &jointClip : mJointClips ) {
		jointTransforms[i++] = jointClip.getLooped( time );
	}
	localJointTransforms.setNeedsGlobalCache( true );
}
