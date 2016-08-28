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

namespace cinder {

template <typename T>
class Clip {
public:
	//! Constructor
	Clip() : mStartTime( std::numeric_limits<double>::max() ), mDuration( 0.0 ) {}
	//! Constructor Taking a vector of pairs representing time/value.
	Clip( std::vector<std::pair<double, T>> keyFrames );
	
	//! Returns value of type T at time /a time.
	T		get( double time ) const;
	//! Returns value of type T at time /a time modulo-ed against the duration of the clip
	//! to derive the position in the key frames.
	T		getLooped( double time ) const;
	//! Adds a keyframe of /a value at /a time.
	void	addKeyFrame( double time, T value );
	//! Returns a pair representing the start and end time of the clip.
	std::pair<double, double>	getTimeBounds() const { return { mStartTime, mStartTime + mDuration }; }
	//! Returns a pair representing the time and value of the index /a keyframeId.
	std::pair<double, T> getKeyFrameValueAt( uint32_t keyframeId ) const;
	
	//! Returns whether there are no keyFrames.
	bool	empty() const;
	//! Returns number of key frames currently
	size_t	numKeyframes() const;
	
	//! Returns double representing start time of the clip.
	double getStartTime() const { return mStartTime; }
	//! Returns double representing duration of the clip.
	double getDuration() const { return mDuration; }
	
private:
	//! Returns a lerped value using /a time, with /a begin to /a end.
	T		lerp( const T &begin, const T &end, double time ) const;
	
	double				mStartTime, mDuration;
	std::vector<double> mKeyFrameTimes;
	std::vector<T>		mKeyFrameValues;
	
	//! Helper friend for output.
	friend std::ostream& operator<<( std::ostream &os, const Clip<T> &clip );
};

class TransformClip {
public:
	//! Constructor
	TransformClip() : mStartTime( std::numeric_limits<double>::max() ), mDuration( 0.0 ) {}
	//! Constructor
	TransformClip( Clip<ci::vec3> translateClip,
				   Clip<ci::quat> rotationClip,
				   Clip<ci::vec3> scaleClip )
	: mTrans( std::move( translateClip ) ),
		mRot( std::move( rotationClip ) ),
		mScale( std::move( scaleClip ) ),
		mStartTime( std::numeric_limits<double>::max() ),
		mDuration( 0.0 )
	{
		deriveStartDuration();
	}
	//! Constructor
	TransformClip( std::vector<std::pair<double, ci::vec3>> translateKeyFrames,
				   std::vector<std::pair<double, ci::quat>> rotationKeyFrames,
				   std::vector<std::pair<double, ci::vec3>> scaleKeyFrames )
	: mTrans( std::move( translateKeyFrames ) ),
		mRot( std::move( rotationKeyFrames ) ),
		mScale( std::move( scaleKeyFrames ) ),
		mStartTime( std::numeric_limits<double>::max() ),
		mDuration( 0.0 )
	{
		deriveStartDuration();
	}
	//! Returns a ci::mat4 representing the transformation at time /a time. The transformation
	//! is built as Translation * Rotation * Scale.
	ci::mat4 getMatrix( double time ) const;
	//! Returns a ci::mat4 representing the transformation at time /a time modulo-ed against
	//! the duration of the underlying clips to derive the transformation. The transformation
	//! is built as Translation * Rotation * Scale.
	ci::mat4 getMatrixLooped( double time ) const;
	
	ci::vec3 getTranslation( double time ) const;
	ci::vec3 getTranslationLooped( double time ) const;
	ci::quat getRotation( double time ) const;
	ci::quat getRotationLooped( double time ) const;
	ci::vec3 getScale( double time ) const;
	ci::vec3 getScaleLooped( double time ) const;
	
	//! Returns a pair representing the start and end time of the clip.
	std::pair<double, double>	getTimeBounds() const { return { mStartTime, mStartTime + mDuration }; }
	
	bool empty() const { return mTrans.empty() && mScale.empty() && mRot.empty(); }
	
	//! Returns a const ref to the underlying translation clip.
	const Clip<ci::vec3>&	getTranslationClip() const { return mTrans; }
	//! Returns a ref to the underlying translation clip.
	Clip<ci::vec3>&			getTranslationClip() { return mTrans; }
	//! Returns a const ref to the underlying rotation clip.
	const Clip<ci::quat>&	getRotationClip() const { return mRot; }
	//! Returns a ref to the underlying rotation clip.
	Clip<ci::quat>&			getRotationClip() { return mRot; }
	//! Returns a const ref to the underlying scale clip.
	const Clip<ci::vec3>&	getScaleClip() const { return mScale; }
	//! Returns a ref to the underlying scale clip
	Clip<ci::vec3>&			getScaleClip() { return mScale; }
	
private:
	void deriveStartDuration()
	{
		auto start = mStartTime;
		auto end = 0.0;
		if( ! mTrans.empty() ) {
			auto transStartDur = mTrans.getTimeBounds();
			start = glm::min( start, transStartDur.first );
			end = glm::max( end, transStartDur.second );
		}
		if( ! mRot.empty() ) {
			auto rotStartDur = mRot.getTimeBounds();
			start = glm::min( start, rotStartDur.first );
			end = glm::max( end, rotStartDur.second );
		}
		if( ! mScale.empty() ) {
			auto scaleStartDur = mScale.getTimeBounds();
			start = glm::min( start, scaleStartDur.first );
			end = glm::max( end, scaleStartDur.second );
		}
		mStartTime = start;
		mDuration = end - start;
	}
	
	Clip<ci::vec3>	mTrans;
	Clip<ci::vec3>	mScale;
	Clip<ci::quat>	mRot;
	double			mStartTime, mDuration;
	
	friend std::ostream& operator<<( std::ostream &os, const TransformClip &clip );
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
	auto cyclicTime = glm::mod( time, mDuration ) + mStartTime;
	ci::mat4 ret;
	if( ! mTrans.empty() )
		ret *= ci::translate( mTrans.get( cyclicTime ) );
	if( ! mRot.empty() )
		ret *= ci::toMat4( mRot.get( cyclicTime ) );
	if( ! mScale.empty() )
		ret *= ci::scale( mScale.get( cyclicTime ) );
	return ret;
}
	
inline ci::vec3 TransformClip::getTranslation( double time ) const
{
	if( ! mTrans.empty() )
		return mTrans.get( time );
	
	return ci::vec3();
}
	
inline ci::vec3 TransformClip::getTranslationLooped( double time ) const
{
	if( ! mTrans.empty() ) {
		auto cyclicTime = glm::mod( time, mDuration ) + mStartTime;
		return mTrans.get( cyclicTime );
	}
	return ci::vec3();
}
	
inline ci::quat TransformClip::getRotation( double time ) const
{
	if( ! mRot.empty() )
		return mRot.get( time );
	return ci::quat();
}
	
inline ci::quat TransformClip::getRotationLooped( double time ) const
{
	if( ! mRot.empty() ) {
		auto cyclicTime = glm::mod( time, mDuration ) + mStartTime;
		return mRot.get( cyclicTime );
	}
	return ci::quat();
}
	
inline ci::vec3 TransformClip::getScale( double time ) const
{
	if( ! mScale.empty() )
		return mScale.get( time );
	return ci::vec3( 1.0f );
}
inline ci::vec3 TransformClip::getScaleLooped( double time ) const
{
	if( ! mScale.empty() ) {
		auto cyclicTime = glm::mod( time, mDuration ) + mStartTime;
		return mScale.get( cyclicTime );
	}
	return ci::vec3( 1.0f );
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
	
inline std::ostream& operator<<( std::ostream &os, const TransformClip &transform )
{
	os << std::setprecision( 9 ) << std::fixed;
	os << "Translation: " << std::endl;
	os << transform.getTranslationClip();
	os << "Rotation: " << std::endl;
	os << transform.getRotationClip();
	os << "Scale: " << std::endl;
	return os;
}

using ClipVec3 = Clip<ci::vec3>;
	
} // namespace cinder