//
//  Skeleton.cpp
//  Test
//
//  Created by ryan bartley on 3/5/16.
//
//

#include "Skeleton.h"

Skeleton::Skeleton( uint8_t numJoints )
: mJointArray( numJoints ), mJointNames( numJoints ),
	mBindPose( this )
{
}

bool Skeleton::hasJoint( const std::string &name ) const
{
	auto endIt = end( mJointNames );
	return std::find( begin( mJointNames ), endIt, name ) != endIt;
}

const Skeleton::Joint* Skeleton::getJoint( const std::string &name ) const
{
	auto begIt = begin( mJointNames );
	auto endIt = end( mJointNames );
	auto jointIt = std::find( begIt, endIt, name );
	if( jointIt != endIt ) {
		auto dist = std::distance( begIt, jointIt );
		return getJoint( dist );
	}
	else
		return nullptr;
}

const Skeleton::Joint* Skeleton::getJoint( uint8_t id ) const
{
	CI_ASSERT( id < mJointArray.size() );
	return &mJointArray[id];
}

const std::string* Skeleton::getJointName( const Skeleton::Joint &joint ) const
{
	return getJointName( joint.getNameId() );
}

const std::string* Skeleton::getJointName( uint8_t nameId ) const
{
	CI_ASSERT( nameId < mJointNames.size() );
	return &mJointNames[nameId];
}