//
//  Skeleton.h
//  Test
//
//  Created by ryan bartley on 3/5/16.
//
//

#pragma once

#include "Animation.h"

using SkeletonRef = std::shared_ptr<class Skeleton>;

class Skeleton {
public:
	struct Joint {
		Joint();
		Joint( uint8_t parentId, uint8_t nameId, ci::mat4 inverseBindMatrix )
		: mInverseBindMatrix( inverseBindMatrix ), mNameId( nameId ), mParentId( parentId ) {}
		
		void setInverseBindMatrix( const glm::mat4 &inverseBind ) { mInverseBindMatrix = inverseBind; }
		glm::mat4& getInverseBindMatrix() { return mInverseBindMatrix; }
		const glm::mat4& getInverseBindMatrix() const { return mInverseBindMatrix; }
		
		uint8_t getNameId() const { return mNameId; }
		void setNameId( uint8_t nameId ) { mNameId = nameId; }
		uint8_t getParentId() const { return mParentId; }
		void setParentId( uint8_t parentId ) { mParentId = parentId; }
		
	private:
		glm::mat4	mInverseBindMatrix;
		uint8_t		mNameId;
		uint8_t		mParentId;
		
		friend class Skeleton;
	};
	
	class Anim {
	public:
		class JointClip {
		public:
			JointClip( const TransformClip &transform );
			
			struct TransKeyframe {
				TransKeyframe( float time, ci::vec3 trans )
				: time( time ), trans( std::move( trans ) ) {}
				float		time;
				ci::vec3	trans;
			};
			
			struct RotKeyframe {
				RotKeyframe( float time, ci::quat rot )
				: time( time ), rot( std::move( rot ) ) {}
				float		time;
				ci::quat	rot;
			};
			
			ci::mat4 get( double absTime ) const;
			ci::mat4 getLooped( double absTime ) const;
			
			std::pair<ci::vec3, ci::quat> getSeparated( double absTime ) const;
			std::pair<ci::vec3, ci::quat> getSeparatedLooped( double absTime ) const;
			
			ci::vec3 getTrans( double absTime ) const;
			ci::vec3 getTransLooped( double absTime ) const;
			
			ci::quat getRot( double absTime ) const;
			ci::quat getRotLooped( double absTime ) const;
			
		private:
			double						mRotStartTime, mRotDuration;
			double						mTransStartTime, mTransDuration;
			std::vector<RotKeyframe>	jointRots;
			std::vector<TransKeyframe>	jointTrans;
		};
		
		Anim( std::vector<TransformClip> transformClips )
		{
			joints.reserve( transformClips.size() );
			for( auto &transformClip : transformClips ) {
				joints.emplace_back( std::move( JointClip( transformClip ) ) );
			}
		}
		
		void getLocal( double time, std::vector<ci::mat4> *localJointTransforms ) const;
		void getLoopedLocal( double time, std::vector<ci::mat4> *localJointTransforms ) const;
		
		void getLocalSeparated( double time, std::vector<std::pair<ci::vec3, ci::quat>> *localSeparatedJointTransforms ) const;
		void getLoopedLocalSeparated( double time, std::vector<std::pair<ci::vec3, ci::quat>> *localSeparatedJointTransforms ) const;
		
	private:
		std::vector<JointClip> joints;
	};
	
	using AnimRef = std::shared_ptr<Anim>;
	
	Skeleton( std::vector<Joint> joints, std::vector<std::string> jointNames );
	
	const Joint*	getRoot() const { return &mJointArray[0]; }
	const Joint*	getJoint( const std::string& name ) const;
	const Joint*	getJoint( uint8_t jointId ) const;
	bool			hasJoint( const std::string& name ) const;
	size_t			getNumJoints() { return mJointArray.size(); }
	
	bool			jointIsChildOf( uint8_t childIndex, uint8_t parentIndex ) const;
	
	const std::string*	getJointName( const Joint &joint ) const;
	const std::string*	getJointName( uint8_t nameId ) const;
	
	const std::vector<Joint>&		getJoints() const { return mJointArray; }
	std::vector<Joint>&				getJoints() { return mJointArray; }
	
	const std::vector<std::string>& getJointNames() const { return mJointNames; }
	std::vector<std::string>&		getJointNames() { return mJointNames; }
	
	void calcMatrixPaletteFromGlobal( const std::vector<ci::mat4> &globalCache,
							std::vector<ci::mat4> *offsetMatrices ) const;
	void calcMatrixPaletteFromLocal( const std::vector<ci::mat4> &localJointTransforms,
						    std::vector<ci::mat4> *offsetMatrices ) const;
	void calcGlobalMatrices( const std::vector<ci::mat4> &localJointTransforms,
							 std::vector<ci::mat4> *globalJointTransforms ) const;
	
private:
	std::vector<Joint>			mJointArray;
	std::vector<std::string>	mJointNames;
};

inline void Skeleton::calcMatrixPaletteFromGlobal( const std::vector<ci::mat4> &globalCache,
												  std::vector<ci::mat4> *offset ) const
{
	offset->clear();
	// Derive root
	offset->emplace_back( globalCache[0] * mJointArray[0].getInverseBindMatrix() );
	// Derive children
	for( int i = 1, end = mJointArray.size(); i < end; i++ )
		offset->emplace_back( globalCache[i] * mJointArray[i].getInverseBindMatrix() );
}

inline void Skeleton::calcMatrixPaletteFromLocal( const std::vector<ci::mat4> &localJointTransforms,
												 std::vector<ci::mat4> *offsetMatrices ) const
{
	std::vector<ci::mat4> globalJointTransforms;
	globalJointTransforms.reserve( mJointArray.size() );
	calcGlobalMatrices( localJointTransforms, &globalJointTransforms );
	calcMatrixPaletteFromGlobal( globalJointTransforms, offsetMatrices );
}

inline void Skeleton::calcGlobalMatrices( const std::vector<ci::mat4> &localJointTransforms,
										 std::vector<ci::mat4> *globalJoint ) const
{
	globalJoint->clear();
	// Derive root
	globalJoint->emplace_back( localJointTransforms[0] );
	// Derive children
	for( int i = 1, end = mJointArray.size(); i < end; i++ )
		globalJoint->emplace_back( (*globalJoint)[mJointArray[i].getParentId()] * localJointTransforms[i] );
}

inline Skeleton::Anim::JointClip::JointClip( const TransformClip &transform )
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

inline ci::mat4 Skeleton::Anim::JointClip::get( double absTime ) const
{
	CI_ASSERT( jointTrans.size() >= 2 && jointRots.size() >= 2 );
	auto lerpTrans = getTrans( absTime );
	auto lerpRot = getRot( absTime );
	glm::mat4 ret;
	ret *= glm::translate( lerpTrans );
	ret *= glm::toMat4( lerpRot );
	return ret;
}

inline std::pair<ci::vec3, ci::quat> Skeleton::Anim::JointClip::getSeparated( double absTime ) const
{
	CI_ASSERT( jointTrans.size() >= 2 && jointRots.size() >= 2 );
	auto lerpTrans = getTrans( absTime );
	auto lerpRot = getRot( absTime );
	
	return { lerpTrans, lerpRot };
}

inline ci::vec3 Skeleton::Anim::JointClip::getTrans( double absTime ) const
{
	auto transClamped = glm::clamp( absTime, mTransStartTime, mTransStartTime + mTransDuration );
	auto begItTrans = begin( jointTrans );
	auto nextItTrans = std::upper_bound( begItTrans, end( jointTrans ) - 1, transClamped,
	[]( float time, const TransKeyframe &val ){ return time < val.time; });
	
	auto prevItTrans = nextItTrans - 1;
	auto perTimeTrans = ( transClamped - prevItTrans->time) /
	( nextItTrans->time - prevItTrans->time);
	return glm::mix( prevItTrans->trans, nextItTrans->trans, perTimeTrans );
}

inline ci::quat Skeleton::Anim::JointClip::getRot( double absTime ) const
{
	auto rotClamped = glm::clamp( absTime, mRotStartTime, mRotStartTime + mRotDuration );
	auto begItRot = begin( jointRots );
	auto nextItRot = std::upper_bound( begItRot, end( jointRots ) - 1, rotClamped,
	[]( float time, const RotKeyframe &val ){ return time < val.time; });
	
	auto prevItRot = nextItRot - 1;
	auto perTimeRot = ( rotClamped - prevItRot->time) /
	( nextItRot->time - prevItRot->time);
	return glm::slerp( prevItRot->rot, nextItRot->rot, (float)perTimeRot );
}

inline ci::mat4 Skeleton::Anim::JointClip::getLooped( double absTime ) const
{
	CI_ASSERT( jointTrans.size() >= 2 && jointRots.size() >= 2 );
	auto lerpTrans = getTransLooped( absTime );
	auto lerpRot = getRotLooped( absTime );
	glm::mat4 ret;
	ret *= glm::translate( lerpTrans );
	ret *= glm::toMat4( lerpRot );
	return ret;
};

inline std::pair<ci::vec3, ci::quat> Skeleton::Anim::JointClip::getSeparatedLooped( double absTime ) const
{
	CI_ASSERT( jointTrans.size() >= 2 && jointRots.size() >= 2 );
	auto lerpTrans = getTransLooped( absTime );
	auto lerpRot = getRotLooped( absTime );
	return { lerpTrans, lerpRot };
}

inline ci::vec3 Skeleton::Anim::JointClip::getTransLooped( double absTime ) const
{
	auto cyclicTimeTrans = glm::mod( absTime, mTransDuration ) + mTransStartTime;
	auto begItTrans = begin( jointTrans );
	auto nextItTrans = std::upper_bound( begItTrans, end( jointTrans ) - 1, cyclicTimeTrans,
	[]( float time, const TransKeyframe &val ){ return time < val.time; });
	
	auto prevItTrans = nextItTrans - 1;
	auto perTimeTrans = ( cyclicTimeTrans - prevItTrans->time) /
	( nextItTrans->time - prevItTrans->time);
	return glm::mix( prevItTrans->trans, nextItTrans->trans, perTimeTrans );
}

inline ci::quat Skeleton::Anim::JointClip::getRotLooped( double absTime ) const
{
	auto cyclicTimeRot = glm::mod( absTime, mRotDuration ) + mRotStartTime;
	auto begItRot = begin( jointRots );
	auto nextItRot = std::upper_bound( begItRot, end( jointRots ) - 1, cyclicTimeRot,
									  []( float time, const RotKeyframe &val ){ return time < val.time; });
	
	auto prevItRot = nextItRot - 1;
	auto perTimeRot = ( cyclicTimeRot - prevItRot->time) /
	( nextItRot->time - prevItRot->time);
	return glm::slerp( prevItRot->rot, nextItRot->rot, (float)perTimeRot );
}

inline void Skeleton::Anim::getLocal( double time, std::vector<ci::mat4> *localJointTransforms ) const
{
	localJointTransforms->clear();
	for( auto & jointClip : joints )
		localJointTransforms->emplace_back( jointClip.get( time ) );
}

inline void Skeleton::Anim::getLoopedLocal( double time, std::vector<ci::mat4> *localJointTransforms ) const
{
	localJointTransforms->clear();
	for( auto & jointClip : joints )
		localJointTransforms->emplace_back( jointClip.getLooped( time ) );
}

void Skeleton::Anim::getLocalSeparated( double time, std::vector<std::pair<ci::vec3, ci::quat>> *localSeparatedJointTransforms ) const
{
	localSeparatedJointTransforms->clear();
	for( auto & jointClip : joints )
		localSeparatedJointTransforms->emplace_back( jointClip.getSeparated( time ) );
}

void Skeleton::Anim::getLoopedLocalSeparated( double time, std::vector<std::pair<ci::vec3, ci::quat>> *localSeparatedJointTransforms ) const
{
	localSeparatedJointTransforms->clear();
	for( auto & jointClip : joints )
		localSeparatedJointTransforms->emplace_back( jointClip.getSeparatedLooped( time ) );
}
