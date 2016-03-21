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

bool Skeleton::hasJoint( const std::string &name ) const
{
	auto endIt = end( jointNames );
	return std::find( begin( jointNames ), endIt, name ) != endIt;
}

const Skeleton::Joint* Skeleton::getJoint( const std::string &name ) const
{
	auto begIt = begin( jointNames );
	auto endIt = end( jointNames );
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
	CI_ASSERT( id < jointArray.size() );
	return &jointArray[id];
}

const std::string* Skeleton::getJointName( const Skeleton::Joint &joint ) const
{
	return getJointName( joint.nameId );
}

const std::string* Skeleton::getJointName( uint8_t nameId ) const
{
	CI_ASSERT( nameId < jointNames.size() );
	return &jointNames[nameId];
}

void SkeletonAnim::get( double time, std::vector<ci::mat4> &offsetMatrices ) const
{
	offsetMatrices.resize( numJoints );
	// get transforms from animations
	std::vector<Transform> localTransforms;
	getPreInverse( time, localTransforms );
	
	std::vector<ci::mat4> globalStack;
	calcGlobalStack( localTransforms, globalStack );
	
	calcMatrixPalette( globalStack, offsetMatrices);
}

void SkeletonAnim::getLooped( double time, std::vector<ci::mat4> &offsetMatrices ) const
{
	offsetMatrices.resize( numJoints );
	// get transforms from animations
	std::vector<Transform> localTransforms;
	getLoopedPreInverse( time, localTransforms );
	
	std::vector<ci::mat4> globalStack( numJoints );
	calcGlobalStack( localTransforms, globalStack );
	
	calcMatrixPalette( globalStack, offsetMatrices);
}

void SkeletonAnim::calcGlobalStack( const std::vector<Transform> &preInverseTransforms,
								   std::vector<ci::mat4> &globalCache ) const
{
	auto &joints = mSkeleton->jointArray;
	// Derive root
	globalCache[0] = preInverseTransforms[0].getTRS();
	// Derive children
	for( int i = 1; i < numJoints; i++ ) {
		globalCache[i] = globalCache[joints[i].parentId] * preInverseTransforms[i].getTRS();
	}
}

void SkeletonAnim::calcMatrixPalette( const std::vector<ci::mat4> &globalCache,
									  std::vector<ci::mat4> &offsetMatrices ) const
{
	auto &joints = mSkeleton->jointArray;
	// Derive root
	offsetMatrices[0] = joints[0].inverseGlobalBindPose * globalCache[0];
	// Derive children
	for( int i = 1; i < numJoints; i++ ) {
		offsetMatrices[i] = joints[i].inverseGlobalBindPose * globalCache[i];
	}
}

void SkeletonAnim::getPreInverse( double time, std::vector<Transform> &preInverseBakedTransforms ) const
{
	preInverseBakedTransforms.resize( numJoints );
	int i = 0;
	for( auto & jointClip : jointClips ) {
		preInverseBakedTransforms[i++] = jointClip.get( time );
	}
}

void SkeletonAnim::getLoopedPreInverse( double time, std::vector<Transform> &preInverseBakedTransforms ) const
{
	preInverseBakedTransforms.resize( numJoints );
	int i = 0;
	for( auto &jointClip : jointClips ) {
		preInverseBakedTransforms[i++] = jointClip.getLooped( time );
	}
}
