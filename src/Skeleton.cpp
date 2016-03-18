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
		bindPose.globalPoses[i] = getParentsWorldTransform( jointArray[i].parentId ) * bindPose.localPoses[i].getTRS();
	}
}

const ci::mat4& Skeleton::getParentsWorldTransform( uint8_t parentId ) const
{
	return bindPose.globalPoses[parentId];
}