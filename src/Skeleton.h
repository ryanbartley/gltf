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

struct Skeleton {
	Skeleton( uint32_t numJoints )
	: jointNames( numJoints ), jointArray( jointNames.size() )
	{
		bindPose.skeleton = this;
		bindPose.localPoses.resize( numJoints );
		bindPose.globalPoses.resize( numJoints );
	}
	
	struct Joint {
		glm::mat4	inverseGlobalBindPose;
		uint8_t		nameId;
		uint8_t		parentId;
	};
	
	struct TreeJoint {
		Joint*				parent;
		std::vector<Joint*> children;
		std::string			*name;
	};
	
	const Joint*	getRoot() const { return &jointArray[0]; }
	
	void			traverse( std::function<void( const Joint& )> visit ) const;
	static void		traverse( const Joint& node, std::function<void( const Joint& )> visit );
	
	bool			hasJoint( const std::string& name ) const;
	const Joint*	getJoint( const std::string& name ) const;
	const Joint*	getJoint( uint8_t jointId ) const;
	
	const std::string* getJointName( const Joint &joint ) const;
	const std::string* getJointName( uint8_t nameId ) const;
	
	size_t			getNumJoints() { return jointArray.size(); }
	const std::vector<Joint>&	getJoints() const { return jointArray; }
	
	ci::AxisAlignedBox calcBoundingBox() const;
	
	struct Pose {
		Skeleton					*skeleton = nullptr;
		std::vector<Transform>		localPoses;
		std::vector<ci::mat4>		globalPoses;
	};
	
	void resolveGlobalBindPose();
	const ci::mat4& getParentsWorldTransform( uint8_t parentId ) const;
	
	std::vector<std::string>	jointNames;
	std::vector<Joint>			jointArray;
	Pose						bindPose;
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


