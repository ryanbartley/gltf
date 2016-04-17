//
//  Animation.hpp
//  Test
//
//  Created by ryan bartley on 3/8/16.
//
//

#pragma once

#include <vector>
#include <algorithm>

#include "cinder/CinderAssert.h"
#include "cinder/Vector.h"
#include "cinder/Matrix.h"
#include "cinder/Quaternion.h"

template <typename T>
class Clip {
public:
	Clip() : mStartTime( std::numeric_limits<double>::max() ), mDuration( 0.0 ) {}
	Clip( std::vector<std::pair<double, T>> keyFrames );
	
	T		get( double time ) const;
	T		getLooped( double time ) const;
	T		lerp( const T &begin, const T &end, double time ) const;
	
	void	addKeyFrame( double time, T value );
	
	std::pair<double, double>	getTimeBounds() const { return { mStartTime, mStartTime + mDuration }; }
	
	std::pair<double, T> getKeyFrameValueAt( uint32_t keyframeId ) const;
	
	bool	empty() const;
	size_t	numKeyframes() const;
	
	double getStartTime() const { return mStartTime; }
	double getDuration() const { return mDuration; }
	
private:
	
	double				mStartTime, mDuration;
	std::vector<double> mKeyFrameTimes;
	std::vector<T>		mKeyFrameValues;
	
	friend std::ostream& operator<<( std::ostream &os, const Clip<T> &clip );
};

class TransformClip {
public:
	TransformClip() = default;
	TransformClip( Clip<ci::vec3> translateClip,
				   Clip<ci::quat> rotationClip,
				   Clip<ci::vec3> scaleClip )
	: mTrans( std::move( translateClip ) ), mRot( std::move( rotationClip ) ),
		mScale( std::move( scaleClip ) )
	{}
	
	TransformClip( std::vector<std::pair<double, ci::vec3>> translateKeyFrames,
				   std::vector<std::pair<double, ci::quat>> rotationKeyFrames,
				   std::vector<std::pair<double, ci::vec3>> scaleKeyFrames )
	: mTrans( std::move( translateKeyFrames ) ), mRot( std::move( rotationKeyFrames ) ),
		mScale( std::move( scaleKeyFrames ) )
	{}
	
	ci::mat4 getMatrix( double time ) const;
	ci::mat4 getMatrixLooped( double time ) const;
	
	ci::vec3 getTranslation( double time ) const { return mTrans.get( time ); }
	ci::vec3 getScale( double time ) const { return mScale.get( time ); }
	ci::quat getRotation( double time ) const { return mRot.get( time ); }
	
	const Clip<ci::vec3>&	getTranslationClip() const { return mTrans; }
	Clip<ci::vec3>&			getTranslationClip() { return mTrans; }
	const Clip<ci::quat>&	getRotationClip() const { return mRot; }
	Clip<ci::quat>&			getRotationClip() { return mRot; }
	const Clip<ci::vec3>&	getScaleClip() const { return mScale; }
	Clip<ci::vec3>&			getScaleClip() { return mScale; }
	
private:
	Clip<ci::vec3>	mTrans;
	Clip<ci::vec3>	mScale;
	Clip<ci::quat>	mRot;
};


template <typename T>
Clip<T>::Clip( std::vector<std::pair<double, T>> keyFrames )
{
	CI_ASSERT( keyFrames.size() >= 2 );
	auto begIt = begin( keyFrames );
	auto endIt = end( keyFrames );
	std::sort( begIt, endIt,
	[]( const std::pair<double, T> &lhs, const std::pair<double, T> &rhs ){
		return lhs.first < rhs.first;
	});
	begIt = begin( keyFrames );
	endIt = end( keyFrames );
	
	mStartTime = begIt->first;
	mDuration = (endIt - 1)->first - mStartTime;
	
	auto numKeyFrames = keyFrames.size();
	mKeyFrameTimes.reserve( numKeyFrames );
	mKeyFrameValues.reserve( numKeyFrames );
	
	while( begIt != endIt ) {
		mKeyFrameTimes.push_back( begIt->first );
		mKeyFrameValues.push_back( begIt->second );
		++begIt;
	}
}

template <typename T>
inline void Clip<T>::addKeyFrame( double time, T value )
{
	mStartTime = glm::min( time, mStartTime );
	mDuration = glm::max( time - mStartTime, mDuration );
	
	auto begIt = begin( mKeyFrameTimes );
	auto endIt = end( mKeyFrameTimes );
	auto found = std::lower_bound( begIt, endIt, time );
	mKeyFrameTimes.emplace( found, time );
	
	auto dist = std::distance( begIt, found );
	mKeyFrameValues.emplace( mKeyFrameValues.begin() + dist, value );
}

template <typename T>
inline T Clip<T>::get( double absTime ) const
{
	CI_ASSERT( mKeyFrameTimes.size() >= 2 && mKeyFrameValues.size() >= 2 );
	auto clamped = glm::clamp( absTime, mStartTime, mStartTime + mDuration );
	auto begIt = begin( mKeyFrameTimes );
	auto nextIt = std::upper_bound( begIt, end( mKeyFrameTimes ) - 1, clamped );
	auto dist = std::distance( begIt, nextIt );
	
	auto prevTime = nextIt - 1;
	auto perTime = (clamped - *(prevTime)) / ((*nextIt) - *(prevTime));
	return lerp( mKeyFrameValues[dist - 1], mKeyFrameValues[dist], perTime );
}

template <typename T>
inline T Clip<T>::getLooped( double absTime ) const
{
	CI_ASSERT( mKeyFrameTimes.size() >= 2 && mKeyFrameValues.size() >= 2 );
	auto cyclicTime = glm::mod( absTime, mDuration ) + mStartTime;
	auto begIt = begin( mKeyFrameTimes );
	auto nextIt = std::upper_bound( begIt, end( mKeyFrameTimes ) - 1, cyclicTime );
	auto dist = std::distance( begIt, nextIt );
	
	// could be last or first, probably not first.
	auto prevTime = nextIt - 1;
	auto perTime = (cyclicTime - *(prevTime)) / ((*nextIt) - *(prevTime));
	return lerp( mKeyFrameValues[dist - 1], mKeyFrameValues[dist], perTime );
}

template<typename T>
inline std::pair<double, T> Clip<T>::getKeyFrameValueAt( uint32_t keyframeId ) const
{
	CI_ASSERT( keyframeId < mKeyFrameValues.size() );
	return { mKeyFrameTimes[keyframeId], mKeyFrameValues[keyframeId] };
}

template<typename T>
inline bool Clip<T>::empty() const
{
	CI_ASSERT( mKeyFrameTimes.size() == mKeyFrameValues.size() );
	return mKeyFrameTimes.empty();
}

template<typename T>
inline size_t Clip<T>::numKeyframes() const
{
	CI_ASSERT( mKeyFrameTimes.size() == mKeyFrameValues.size() );
	return mKeyFrameTimes.size();
}

template<>
inline float Clip<float>::lerp( const float &begin, const float &end, double time ) const
{
	return glm::mix( begin, end, time );
}

template<>
inline ci::vec2 Clip<ci::vec2>::lerp( const ci::vec2 &begin, const ci::vec2 &end, double time ) const
{
	return glm::mix( begin, end, time );
}

template<>
inline ci::vec3 Clip<ci::vec3>::lerp( const ci::vec3 &begin, const ci::vec3 &end, double time ) const
{
	return glm::mix( begin, end, time );
}

template<>
inline ci::vec4 Clip<ci::vec4>::lerp( const ci::vec4 &begin, const ci::vec4 &end, double time ) const
{
	return glm::mix( begin, end, time );
}

template<>
inline ci::quat Clip<ci::quat>::lerp( const ci::quat &begin, const ci::quat &end, double time ) const
{
	return glm::slerp( begin, end, static_cast<float>( time ) );
}

template<>
inline ci::dquat Clip<ci::dquat>::lerp( const ci::dquat &begin, const ci::dquat &end, double time ) const
{
	return glm::slerp( begin, end, time );
}

inline ci::mat4 TransformClip::getMatrix( double time ) const
{
	ci::mat4 ret;
	if( ! mTrans.empty() )
		ret *= ci::translate( mTrans.get( time ) );
	if( ! mRot.empty() )
		ret *= ci::toMat4( mRot.get( time ) );
	if( ! mScale.empty() )
		ret *= ci::scale( mScale.get( time ) );
	return ret;
}

inline ci::mat4 TransformClip::getMatrixLooped( double time ) const
{
	ci::mat4 ret;
	if( ! mTrans.empty() )
		ret *= ci::translate( mTrans.getLooped( time ) );
	if( ! mRot.empty() )
		ret *= ci::toMat4( mRot.getLooped( time ) );
	if( ! mScale.empty() )
		ret *= ci::scale( mScale.getLooped( time ) );
	return ret;
}

inline std::ostream& operator<<( std::ostream &os, const Clip<ci::vec3> &clip )
{
	auto numKeyFrames = clip.mKeyFrameTimes.size();
	for( int i = 0; i < numKeyFrames; i++ ) {
		os << clip.mKeyFrameTimes[i] << " = " << clip.mKeyFrameValues[i] << std::endl;
	}
	return os;
}

inline std::ostream& operator<<( std::ostream &os, const Clip<ci::quat> &clip )
{
	auto numKeyFrames = clip.mKeyFrameTimes.size();
	for( int i = 0; i < numKeyFrames; i++ ) {
		os << clip.mKeyFrameTimes[i] << " = " << clip.mKeyFrameValues[i] << std::endl;
	}
	return os;
}

using ClipVec3 = Clip<ci::vec3>;