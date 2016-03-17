//
//  Skeleton.cpp
//  Test
//
//  Created by ryan bartley on 3/5/16.
//
//

#include "Skeleton.h"

void Skeleton::resolveGlobalBindPose()
{
	bindPose.globalPoses[0] = bindPose.localPoses[0].getTRS();
	for( int i = 1; i < jointArray.size(); i++ ) {
		bindPose.globalPoses[i] = getParentsLocalTransform( jointArray[i].parentId ) * bindPose.localPoses[i].getTRS();
	}
}

ci::mat4 Skeleton::getParentsLocalTransform( uint8_t parentId ) const
{
	if( parentId != 0xFF ) {
		auto &parentJoint = jointArray[parentId];
		auto &localPose = bindPose.localPoses[parentId];
		return getParentsLocalTransform( parentJoint.parentId ) * localPose.getTRS();
	}
	else {
		return ci::mat4();
	}
}