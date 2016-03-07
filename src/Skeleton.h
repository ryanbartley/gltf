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
	glm::mat4x3 inverseBindPose;
	std::string name;
	uint8_t		parentId;
};

struct Transform {
	glm::quat rot;
	glm::vec3 scale;
	glm::vec3 trans;
};

struct JointPose {
	glm::quat	rot;
	ci::vec4	trans;
	ci::vec4	scale;
};

struct Skeleton {
	std::vector<Joint>	jointArray;
};

struct SkeletonPose {
	std::shared_ptr<Skeleton>	skeleton;
	std::vector<JointPose>		localPoses;
};

struct TransformationManager {
	
	std::vector<uint32_t>	mUnusedTransforms;
	std::vector<Transform>	mTransforms;
};

struct Node {
	
	uint32_t transformIndex;
};