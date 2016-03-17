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
		glm::mat4	inverseBindPose;
		uint8_t		nameId;
		uint8_t		parentId;
	};
	
	struct Pose {
		Skeleton					*skeleton = nullptr;
		std::vector<Transform>		localPoses;
		std::vector<ci::mat4>		globalPoses;
	};
	
	struct AnimationSample {
		std::vector<Clip<Transform>> jointPoses;
	};
	
	void resolveGlobalBindPose();
	ci::mat4 getParentsLocalTransform( uint8_t parentId ) const;
	
	std::vector<std::string>	jointNames;
	std::vector<Joint>			jointArray;
	Pose						bindPose;
};


