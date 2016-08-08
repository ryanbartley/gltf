//
//  Skeleton.h
//  Test
//
//  Created by ryan bartley on 3/5/16.
//
//

#pragma once

#include "Animation.h"

namespace cinder {

using SkeletonRef = std::shared_ptr<class Skeleton>;

class Skeleton {
public:
	class Joint;
	//! Constructor
	Skeleton( std::vector<Joint> joints,
			  std::vector<std::string> jointNames,
			  const ci::mat4 &bindShapeMatrix );
	
	//! Returns a const ptr to the root joint.
	const Joint*	getRoot() const { return &mJointArray[0]; }
	//! Returns a const ptr to joint with name /a name.
	const Joint*	getJoint( const std::string& name ) const;
	//! Returns a const ptr to joint with id /a jointId.
	const Joint*	getJoint( uint8_t jointId ) const;
	//! Returns whether this skeleton has joint with name /a name.
	bool			hasJoint( const std::string& name ) const;
	//! Returns the number of joints in this skeleton.
	size_t			getNumJoints() const { return mJointArray.size(); }
	//! Returns whether joint at /a childIndex is a child of joint at /a parentIndex.
	bool			jointIsChildOf( uint8_t childIndex, uint8_t parentIndex ) const;
	//! Returns a const ptr of the name of /a joint as a string.
	const std::string*	getJointName( const Joint &joint ) const;
	//! Returns a const ptr of the name of /a nameId as a string.
	const std::string*	getJointName( uint8_t nameId ) const;
	
	//! Returns a const ref of the Joint vector.
	const std::vector<Joint>&		getJoints() const { return mJointArray; }
	//! Returns a ref of the Joint vector.
	std::vector<Joint>&				getJoints() { return mJointArray; }
	//! Returns a const ref of the Joint name vector.
	const std::vector<std::string>& getJointNames() const { return mJointNames; }
	//! Returns a ref of the Joint name vector.
	std::vector<std::string>&		getJointNames() { return mJointNames; }
	
	//! Returns the bind shape matrix of this skeleton.
	ci::mat4				getBindShapeMatrix() const { return mBindShapeMatrix; }
	
	//! Returns the Matrix palette of the current pose of the skeleton in /a offsetMatrices.
	//! /a globalCache should represent the global joint transforms (parent * local) of the
	//! skeleton. /a offsetMatrices is calculated as...
	//! the joints global matrix * the joints inverse bind matrix * the bind shape matrix
	void calcMatrixPaletteFromGlobal( const std::vector<ci::mat4> &globalCache,
									 std::vector<ci::mat4> *offsetMatrices ) const;
	//! Returns the global joint transforms of the skeleto in /a globalJointTransforms.
	//! /a localJointMatrices should represent the local joint transforms of the skeleton derived
	//! from either Skeleton::Anim or TransformClip. /a offsetMatrices is calculated as
	//! parentJointTransforms * localJointTransforms
	void calcGlobalMatrices( const std::vector<ci::mat4> &localJointTransforms,
							std::vector<ci::mat4> *globalJointTransforms ) const;
	//! Returns the Matrix Palette of the current pose of the skeleton in /a offsetMatrices.
	//! /a localJointTransforms should represent the local joint transforms of the skeleton,
	//! derived from either Skeleton::Anim or TransformClip. /a offsetMatrices is calculated as...
	//! the joints global matrix (parent * local) * the joints inverse bind matrix * the bind shape matrix
	void calcMatrixPaletteFromLocal( const std::vector<ci::mat4> &localJointTransforms,
									std::vector<ci::mat4> *offsetMatrices ) const;
	
	//! Represents the inverse bind matrix of a skeletal node, along with parentalId if present.
	struct Joint {
		//! Constructor.
		Joint();
		//! Constructor.
		Joint( uint8_t parentId, uint8_t nameId, ci::mat4 inverseBindMatrix )
		: mInverseBindMatrix( inverseBindMatrix ), mNameId( nameId ), mParentId( parentId ) {}
		
		//! Sets the inverseBindMatrix of this joint to /a inverseBindMatrix.
		void setInverseBindMatrix( const glm::mat4 &inverseBind ) { mInverseBindMatrix = inverseBind; }
		//! Returns a ref to the inverse bind matrix of this joint.
		glm::mat4& getInverseBindMatrix() { return mInverseBindMatrix; }
		//! Returns a const ref to the inverse bind matrix of this joint.
		const glm::mat4& getInverseBindMatrix() const { return mInverseBindMatrix; }
		
		//! Returns the name id for use in retrieving the name from the skeleton.
		uint8_t getNameId() const { return mNameId; }
		//! Sets the name id to /a nameId.
		void setNameId( uint8_t nameId ) { mNameId = nameId; }
		//! Returns the parent id for use in retreiving the parent joint from the skeleton.
		uint8_t getParentId() const { return mParentId; }
		//! Sets the parent id to /a parentId.
		void setParentId( uint8_t parentId ) { mParentId = parentId; }
		
	private:
		glm::mat4	mInverseBindMatrix;
		uint8_t		mNameId;
		uint8_t		mParentId;
		
		friend class Skeleton;
	};
	
	class Anim {
	public:
		//! Constructor
		Anim( std::vector<TransformClip> transformClips );
		//! Returns the lerped/transformed joints in local space in /a localJointTransforms at time /a time.
		//! The transforms are built as Translation * Rotation. Scale is dismissed as not usually needed
		//! for optimization purposes.
		void getLocal( double time, std::vector<ci::mat4> *localJointTransforms ) const;
		//! Returns the transformed joints in local space in /a localJointTransfroms at time /a time
		//! modulo-ed against the duration of the underlying clips to derive the transformation. The
		//! transforms are built as Translation * Rotation. Scale is dismissed as not usually needed
		//! for optimization purposes.
		void getLoopedLocal( double time, std::vector<ci::mat4> *localJointTransforms ) const;
		
		//! Returns the lerped separated components of the local-space joint info in /a
		//! localSeparatedJointTransforms at time /a time. Scale is dismissed as not usually needed
		//! for optimization purposes.
		void getLocalSeparated( double time, std::vector<std::pair<ci::vec3, ci::quat>> *localSeparatedJointTransforms ) const;
		//! Returns the lerped separated components of the local-space joint info in /a
		//! localSeparatedJointTransforms at time /a time modulo-ed against the duration of the
		//! underlying clips to derive the transformation. Scale is dismissed as not usually
		//! needed for optimization purposes.
		void getLoopedLocalSeparated( double time, std::vector<std::pair<ci::vec3, ci::quat>> *localSeparatedJointTransforms ) const;
		
		//! Modified/optimized animation clip that disregards scale component, which isn't used often.
		class JointClip {
		public:
			//! Constructor
			JointClip( const TransformClip &transform );
			
			//! Represents a translational key frame.
			struct TransKeyframe {
				TransKeyframe( float time, ci::vec3 trans )
				: time( time ), trans( std::move( trans ) ) {}
				float		time;
				ci::vec3	trans;
			};
			
			//! Represents a rotational key frame.
			struct RotKeyframe {
				RotKeyframe( float time, ci::quat rot )
				: time( time ), rot( std::move( rot ) ) {}
				float		time;
				ci::quat	rot;
			};
			
			//! Returns a ci::mat4 representing the transformation at time /a time. The transformation
			//! is built as Translation * Rotation.
			ci::mat4 get( double absTime ) const;
			//! Returns a ci::mat4 representing the transformation at time /a time modulo-ed against
			//! the duration of the underlying clips to derive the transformation. The transformation
			//! is built as Translation * Rotation.
			ci::mat4 getLooped( double absTime ) const;
			
			//! Returns a pair of vec3(Trans) and quat(Trans) representing the transformation at
			//! time /a time. The transformation should be built as Translation * Rotation.
			std::pair<ci::vec3, ci::quat> getSeparated( double absTime ) const;
			//! Returns a pair of vec3(Trans) and quat(Rot) representing the transformation at
			//! time /a time modulo-ed against the duration of the underlying clips to derive the
			//! transformation. The transformation should be built as Translation * Rotation.
			std::pair<ci::vec3, ci::quat> getSeparatedLooped( double absTime ) const;
			
			//! Returns value of translation at time /a time.
			ci::vec3 getTrans( double absTime ) const;
			//! Returns value of translation at time /a time modulo-ed against the duration of the clip
			//! to derive the position in the key frames.
			ci::vec3 getTransLooped( double absTime ) const;
			//! Returns value of rotation at time /a time.
			ci::quat getRot( double absTime ) const;
			//! Returns value of rotation at time /a time modulo-ed against the duration of the clip
			//! to derive the position in the key frames.
			ci::quat getRotLooped( double absTime ) const;
			
		private:
			double						mRotStartTime, mRotDuration;
			double						mTransStartTime, mTransDuration;
			std::vector<RotKeyframe>	jointRots;
			std::vector<TransKeyframe>	jointTrans;
		};
		
	private:
		std::vector<JointClip> joints;
	};
	
	using AnimRef = std::shared_ptr<Anim>;
	
private:
	std::vector<Joint>			mJointArray;
	std::vector<std::string>	mJointNames;
	ci::mat4					mBindShapeMatrix;
};

class SkeletonRenderer {
public:
	SkeletonRenderer();

	void draw( const Skeleton &skeleton, const std::vector<ci::mat4> &finalPose );
	void draw( const Skeleton &skeleton );
private:
	std::array<ci::gl::BatchRef, 2> boneJointBatches;
	ci::gl::VboRef					matrixPaletteVbo;
};

inline void Skeleton::calcMatrixPaletteFromGlobal( const std::vector<ci::mat4> &globalCache,
												  std::vector<ci::mat4> *offset ) const
{
	offset->clear();
	// Derive root
	offset->emplace_back( globalCache[0] * mJointArray[0].getInverseBindMatrix() * mBindShapeMatrix );
	// Derive children
	for( int i = 1, end = mJointArray.size(); i < end; i++ )
		offset->emplace_back( globalCache[i] * mJointArray[i].getInverseBindMatrix() * mBindShapeMatrix );
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
	
} // namespace cinder
