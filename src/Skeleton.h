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
		
		const std::vector<Transform>&	getLocalPoses() const { return mLocalPoseTransforms; }
		std::vector<Transform>&			getLocalPoses() { return mLocalPoseTransforms; }
		const std::vector<ci::mat4>&	getGlobalMatrices() const { return mGlobalPose; }
		std::vector<ci::mat4>&			getGlobalMatrices() { return mGlobalPose; }
		bool							needsGlobalCache() const { return mNeedsGlobalCache; }
		
	private:
		Skeleton				*mSkeleton = nullptr;
		std::vector<Transform>	mLocalPoseTransforms;
		std::vector<ci::mat4>	mGlobalPose;
		bool					mNeedsGlobalCache;
	};
	
	struct Anim {
		Anim( const std::vector<Clip<Transform>> &jointClips );
		
		void get( double time, Pose &localJointTransforms ) const;
		void getLooped( double time, Pose &localJointTransforms  ) const;
		
	public:
		std::vector<Clip<Transform>> mJointClips;
	};
	
	const Joint*	getRoot() const { return &mJointArray[0]; }
	const Joint*	getJoint( const std::string& name ) const;
	const Joint*	getJoint( uint8_t jointId ) const;
	bool			hasJoint( const std::string& name ) const;
	size_t			getNumJoints() { return mJointArray.size(); }
	
	const std::string*	getJointName( const Joint &joint ) const;
	const std::string*	getJointName( uint8_t nameId ) const;
	
	const std::vector<Joint>&		getJoints() const { return mJointArray; }
	const std::vector<std::string>& getJointNames() const { return mJointNames; }
	
	ci::AxisAlignedBox calcBoundingBox() const;
	
	void calcGlobalStack( const Pose &jointLocalTransforms,
						  std::vector<ci::mat4> &globalStack );
	void calcMatrixPalette( const std::vector<ci::mat4> &globalCache,
						   std::vector<ci::mat4> &offsetMatrices ) const;
	
	const Pose&		getBindPose() { return mBindPose; }
	
private:
	std::vector<Joint>			mJointArray;
	Pose						mBindPose;
	std::vector<std::string>	mJointNames;
};

using SkeletonAnimRef = std::shared_ptr<class SkeletonAnim>;

class SkeletonAnim {
public:
	SkeletonAnim( SkeletonRef skeleton, std::vector<Clip<Transform>> animations )
	: mSkeleton( skeleton ), jointClips( std::move( animations ) ), numJoints( jointClips.size() ) {}
	
	void get( double time, std::vector<ci::mat4> &offsetMatrices ) const;
	void getLooped( double time, std::vector<ci::mat4> &offsetMatrices ) const;
	void getPreInverse( double time, std::vector<Transform> &preInverseBakedTransforms ) const;
	void getLoopedPreInverse( double time, std::vector<Transform> &preInverseBakedTransforms ) const;

	void			traverse( std::function<void( const Skeleton::Joint& )> visit ) const;
	static void		traverse( const Skeleton::Joint& node, std::function<void( const Skeleton::Joint& )> visit );
	
private:
	void calcGlobalStack( const std::vector<Transform> &preInverseTransforms,
						  std::vector<ci::mat4> &globalCache ) const;
	void calcMatrixPalette( const std::vector<ci::mat4> &globalCache,
						    std::vector<ci::mat4> &offsetMatrices ) const;
	
	SkeletonRef						mSkeleton;
	std::vector<Clip<Transform>>	jointClips;
	uint32_t						numJoints;
	std::function<void()>			postProcessor;
};


