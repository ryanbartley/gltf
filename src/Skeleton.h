//
//  Skeleton.h
//  Test
//
//  Created by ryan bartley on 3/5/16.
//
//

#pragma once

#include "Transformation.hpp"
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
		Anim( std::vector<Clip<Transform>> jointClips ) : mJointClips( std::move( jointClips ) ) {}
		
		void getLocal( double time, std::vector<Transform> *localJointTransforms ) const;
		void getLoopedLocal( double time, std::vector<Transform> *localJointTransforms  ) const;
		
	private:
		std::vector<Clip<Transform>> mJointClips;
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
	
	void calcMatrixPalette( const std::vector<ci::mat4> &globalCache,
							std::vector<ci::mat4> *offsetMatrices ) const;
	void calcMatrixPalette( const std::vector<Transform> &localJointTransforms,
						    std::vector<ci::mat4> *offsetMatrices ) const;
	void calcGlobalMatrices( const std::vector<Transform> &localJointTransforms,
							 std::vector<ci::mat4> *globalJointTransforms ) const;
	
private:
	std::vector<Joint>			mJointArray;
	std::vector<std::string>	mJointNames;
};

inline void Skeleton::calcMatrixPalette( const std::vector<ci::mat4> &globalCache,
										 std::vector<ci::mat4> *offset ) const
{
	offset->clear();
	// Derive root
	offset->emplace_back( globalCache[0] * mJointArray[0].getInverseBindMatrix() );
	// Derive children
	for( int i = 1, end = mJointArray.size(); i < end; i++ )
		offset->emplace_back( globalCache[i] * mJointArray[i].getInverseBindMatrix() );
}

inline void Skeleton::calcMatrixPalette( const std::vector<Transform> &localJointTransforms,
										 std::vector<ci::mat4> *offsetMatrices ) const
{
	std::vector<ci::mat4> globalJointTransforms;
	globalJointTransforms.reserve( mJointArray.size() );
	calcGlobalMatrices( localJointTransforms, &globalJointTransforms );
	calcMatrixPalette( globalJointTransforms, offsetMatrices );
}

inline void Skeleton::calcGlobalMatrices( const std::vector<Transform> &localJointTransforms,
										 std::vector<ci::mat4> *globalJoint ) const
{
	globalJoint->clear();
	// Derive root
	globalJoint->emplace_back( localJointTransforms[0].getTRS() );
	// Derive children
	for( int i = 1, end = mJointArray.size(); i < end; i++ )
		globalJoint->emplace_back( (*globalJoint)[mJointArray[i].getParentId()] * localJointTransforms[i].getTRS() );
}

inline void Skeleton::Anim::getLocal( double time, std::vector<Transform> *localJointTransforms ) const
{
	localJointTransforms->clear();
	for( auto & jointClip : mJointClips )
		localJointTransforms->emplace_back( jointClip.get( time ) );
}
inline void Skeleton::Anim::getLoopedLocal( double time, std::vector<Transform> *localJointTransforms  ) const
{
	localJointTransforms->clear();
	for( auto & jointClip : mJointClips )
		localJointTransforms->emplace_back( jointClip.getLooped( time ) );
}
