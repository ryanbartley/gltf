//
//  Skeleton.h
//  Test
//
//  Created by ryan bartley on 3/5/16.
//
//

#pragma once

#include "cinder/Matrix.h"
#include "cinder/Vector.h"
#include <string>
#include <vector>


struct Joint {
	glm::mat4	inverseBindPose;
	uint8_t		nameId;
	uint8_t		parentId;
};

struct JointPose {
	glm::quat	rot;
	ci::vec4	trans;
	ci::vec4	scale;
};

using SkeletonRef = std::shared_ptr<class Skeleton>;

struct Skeleton {
	Skeleton( std::vector<std::string> names )
	: jointNames( move( names ) ), jointArray( jointNames.size() )
	{}
	
	std::vector<std::string>	jointNames;
	std::vector<JointPose>		jointBindPoses;
	std::vector<Joint>			jointArray;
};

struct SkeletonPose {
	std::shared_ptr<Skeleton>	skeleton;
	std::vector<JointPose>		localPoses;
};