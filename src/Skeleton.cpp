//
//  Skeleton.cpp
//  Test
//
//  Created by ryan bartley on 3/5/16.
//
//

#include "Skeleton.h"

Skeleton::Skeleton( std::vector<Joint> joints, std::vector<std::string> jointNames )
: mJointArray( std::move( joints ) ), mJointNames( std::move( jointNames ) )
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

bool Skeleton::jointIsChildOf( uint8_t childIndex, uint8_t parentIndex ) const
{
	uint8_t currentParent = mJointArray[childIndex].getParentId();
	while ( currentParent != parentIndex && currentParent != 0xFF ) {
		currentParent = mJointArray[currentParent].getParentId();
	}
	return currentParent == parentIndex;
}

Skeleton::AnimCombined::JointClip::JointClip( const TransformClip &transform )
{
	auto &transClip = transform.getTranslationClip();
	auto &rotClip = transform.getRotationClip();
	mStartTime = transClip.getStartTime();
	mDuration = transClip.getDuration();
	for( int i = 0, end = transClip.numKeyframes(); i < end; i++ ) {
		auto translation = transClip.getKeyFrameValueAt( i );
		auto rotation = rotClip.getKeyFrameValueAt( i );
		jointAnim.emplace_back( translation.first, translation.second, rotation.second );
	}
}

ci::mat4 Skeleton::AnimCombined::JointClip::get( double absTime ) const
{
	CI_ASSERT( jointAnim.size() >= 2 && jointAnim.size() >= 2 );
	auto clamped = glm::clamp( absTime, mStartTime, mStartTime + mDuration );
	auto begIt = begin( jointAnim );
	auto nextIt = std::upper_bound( begIt, end( jointAnim ) - 1, clamped,
								   []( float time, const Value &val ){
									   return time < val.time;
								   });
	
	auto prevIt = nextIt - 1;
	auto lerped = lerp( *prevIt, *nextIt, clamped );
	return getMatrix( lerped.first, lerped.second );
}

ci::mat4 Skeleton::AnimCombined::JointClip::getLooped( double absTime ) const
{
	CI_ASSERT( jointAnim.size() >= 2 && jointAnim.size() >= 2 );
	auto cyclicTime = glm::mod( absTime, mDuration ) + mStartTime;
	auto begIt = begin( jointAnim );
	auto nextIt = std::upper_bound( begIt, end( jointAnim ) - 1, cyclicTime,
								   []( float time, const Value &val ){
									   return time < val.time;
								   });
	
	auto prevIt = nextIt - 1;
	auto lerped = lerp( *prevIt, *nextIt, cyclicTime );
	return getMatrix( lerped.first, lerped.second );
};

std::pair<ci::vec3, ci::quat> Skeleton::AnimCombined::JointClip::lerp( const Value &prev, const Value &next, float clampedTime ) const
{
	std::pair<ci::vec3, ci::quat> ret;
	auto perTime = (clampedTime - prev.time) / (next.time - prev.time);
	ret.first = glm::mix( prev.trans, next.trans, perTime );
	ret.second = glm::slerp( prev.rot, next.rot, perTime );
	return ret;
}

ci::mat4 Skeleton::AnimCombined::JointClip::getMatrix( const ci::vec3 &trans, const ci::quat &rot ) const
{
	glm::mat4 ret;
	ret *= glm::translate( trans );
	ret *= glm::toMat4( rot );
	return ret;
}

void Skeleton::AnimCombined::getLoopedLocal( double time, std::vector<ci::mat4> *localJointTransforms ) const
{
	localJointTransforms->clear();
	for( auto & jointClip : joints )
		localJointTransforms->emplace_back( jointClip.getLooped( time ) );
}

void Skeleton::AnimCombined::getLocal( double time, std::vector<ci::mat4> *localJointTransforms ) const
{
	localJointTransforms->clear();
	for( auto & jointClip : joints )
		localJointTransforms->emplace_back( jointClip.get( time ) );
}

Skeleton::AnimSeparated::JointClip::JointClip( const TransformClip &transform )
{
	auto &transClip = transform.getTranslationClip();
	auto &rotClip = transform.getRotationClip();
	
	mTransStartTime = transClip.getStartTime();
	mTransDuration = transClip.getDuration();
	for( int i = 0, end = transClip.numKeyframes(); i < end; i++ ) {
		auto translation = transClip.getKeyFrameValueAt( i );
		jointTrans.emplace_back( translation.first, translation.second );
	}
	
	mRotStartTime = rotClip.getStartTime();
	mRotDuration = rotClip.getDuration();
	for( int i = 0, end = rotClip.numKeyframes(); i < end; i++ ) {
		auto translation = rotClip.getKeyFrameValueAt( i );
		jointRots.emplace_back( translation.first, translation.second );
	}
}

ci::mat4 Skeleton::AnimSeparated::JointClip::get( double absTime ) const
{
	CI_ASSERT( jointTrans.size() >= 2 && jointRots.size() >= 2 );
	auto transClamped = glm::clamp( absTime, mTransStartTime, mTransStartTime + mTransDuration );
	auto rotClamped = glm::clamp( absTime, mRotStartTime, mRotStartTime + mRotDuration );
	auto begItTrans = begin( jointTrans );
	auto nextItTrans = std::upper_bound( begItTrans, end( jointTrans ) - 1, transClamped,
										[]( float time, const TransKeyframe &val ){
											return time < val.time;
										});
	
	auto prevItTrans = nextItTrans - 1;
	auto perTimeTrans = ( transClamped - prevItTrans->time) /
	( nextItTrans->time - prevItTrans->time);
	auto lerpTrans = glm::mix( prevItTrans->trans, nextItTrans->trans, perTimeTrans );
	
	auto begItRot = begin( jointRots );
	auto nextItRot = std::upper_bound( begItRot, end( jointRots ) - 1, transClamped,
									  []( float time, const RotKeyframe &val ){
										  return time < val.time;
									  });
	
	auto prevItRot = nextItRot - 1;
	auto perTimeRot = ( rotClamped - prevItRot->time) /
	( nextItRot->time - prevItRot->time);
	auto lerpRot = glm::slerp( prevItRot->rot, nextItRot->rot, (float)perTimeRot );
	
	return getMatrix( lerpTrans, lerpRot );
}

ci::mat4 Skeleton::AnimSeparated::JointClip::getLooped( double absTime ) const
{
	CI_ASSERT( jointTrans.size() >= 2 && jointRots.size() >= 2 );
	auto cyclicTimeTrans = glm::mod( absTime, mTransDuration ) + mTransStartTime;
	auto cyclicTimeRot = glm::mod( absTime, mRotDuration ) + mRotStartTime;
	auto begItTrans = begin( jointTrans );
	auto nextItTrans = std::upper_bound( begItTrans, end( jointTrans ) - 1, cyclicTimeTrans,
										[]( float time, const TransKeyframe &val ){
											return time < val.time;
										});
	
	auto prevItTrans = nextItTrans - 1;
	auto perTimeTrans = ( cyclicTimeTrans - prevItTrans->time) /
	( nextItTrans->time - prevItTrans->time);
	auto lerpTrans = glm::mix( prevItTrans->trans, nextItTrans->trans, perTimeTrans );
	
	auto begItRot = begin( jointRots );
	auto nextItRot = std::upper_bound( begItRot, end( jointRots ) - 1, cyclicTimeRot,
									  []( float time, const RotKeyframe &val ){
										  return time < val.time;
									  });
	
	auto prevItRot = nextItRot - 1;
	auto perTimeRot = ( cyclicTimeRot - prevItRot->time) /
	( nextItRot->time - prevItRot->time);
	auto lerpRot = glm::slerp( prevItRot->rot, nextItRot->rot, (float)perTimeRot );
	
	auto ret = getMatrix( lerpTrans, lerpRot );
	return ret;
};

ci::mat4 Skeleton::AnimSeparated::JointClip::getMatrix( const ci::vec3 &trans, const ci::quat &rot ) const
{
	glm::mat4 ret;
	ret *= glm::translate( trans );
	ret *= glm::toMat4( rot );
	return ret;
}

void Skeleton::AnimSeparated::getLocal( double time, std::vector<ci::mat4> *localJointTransforms ) const
{
	localJointTransforms->clear();
	for( auto & jointClip : joints )
		localJointTransforms->emplace_back( jointClip.get( time ) );
}
void Skeleton::AnimSeparated::getLoopedLocal( double time, std::vector<ci::mat4> *localJointTransforms ) const
{
	localJointTransforms->clear();
	for( auto & jointClip : joints ) {
		auto ret = jointClip.getLooped( time );
		localJointTransforms->emplace_back( ret );
	}
}

void Skeleton::Anim::getLocal( double time, std::vector<ci::mat4> *localJointTransforms ) const
{
	localJointTransforms->clear();
	for( auto & jointClip : mJointClips )
		localJointTransforms->emplace_back( jointClip.getMatrix( time ) );
}

void Skeleton::Anim::getLoopedLocal( double time, std::vector<ci::mat4> *localJointTransforms  ) const
{
	localJointTransforms->clear();
	for( auto & jointClip : mJointClips )
		localJointTransforms->emplace_back( jointClip.getMatrixLooped( time ) );
}