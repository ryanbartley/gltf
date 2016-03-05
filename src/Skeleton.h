//
//  Skeleton.h
//  Test
//
//  Created by ryan bartley on 3/5/16.
//
//

#pragma once


struct Joint {
	glm::mat4x3 inverseBindPose;
	std::string name;
	uint8_t		parentId;
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