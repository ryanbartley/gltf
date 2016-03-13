//
//  Animation.hpp
//  Test
//
//  Created by ryan bartley on 3/8/16.
//
//

#pragma once

#include "gltf.h"

template< typename T >
class Clip {
public:
	Clip( const std::vector<std::pair<double, T>> &keyframes );
	
	void addKeyframe( double time, T value );
	T getValue( double time ) const;
	
	inline bool	empty() const { return mKeyframes.empty(); }
	std::pair<double, double>	getBounds() const { return { mStartTime, mStartTime + mDuration }; }
	double	getDuration() const { return mDuration; }
	static inline T	lerp( const T& start, const T& end, double time );
	
private:
	Clip();
	
	static inline bool	isFinite( const glm::vec3& vec );
	inline double		getCyclicTime( double time ) const;
	
	std::vector<double>		mTimes;
	std::vector<T>			mKeyframes;
	
	double				mStartTime;
	double				mDuration;
};

template< typename T >
Clip<T>::Clip()
: mStartTime( std::numeric_limits<double>::max() )
, mDuration( std::numeric_limits<double>::lowest() )
{
}

template< typename T>
Clip<T>::Clip( const std::vector<std::pair<double, T>> &keyframe )
: mStartTime( std::numeric_limits<double>::max() )
, mDuration( std::numeric_limits<double>::lowest() )
{
	// insert in;
	if( ! mKeyframes.empty() ) {
		mStartTime = mKeyframes.begin().time;
		mDuration = mKeyframes.rbegin().time - mStartTime;
	}
}

template< typename T >
void Clip<T>::addKeyframe( double time, T value )
{
	auto it = std::lower_bound( begin( mKeyframes ), end( mKeyframes ), time );
	mKeyframes.emplace( it, { time, value } );
	
	
	mStartTime = glm::min( time, mStartTime );
	mDuration = mKeyframes.rbegin().time - mStartTime;
}

template< typename T >
T Clip<T>::getValue( double relativeTime ) const
{
	CI_ASSERT( ! mKeyframes.empty() );
	
	auto begin = std::begin( mKeyframes );
	auto end = std::end( mKeyframes );
	auto next = std::upper_bound( begin, end, relativeTime );
	auto dist = std::distance( begin, next );
	
//	CI_ASSERT( 0.0f < normalizedTime && 1.0f >= normalizedTime);
//	return lerp( itprev->second, itnext->second, normalizedTime );
}

template< typename T >
inline double Clip<T>::getCyclicTime( double time ) const
{
	return glm::mod( time, getDuration() );
}

template< typename T >
inline T Clip<T>::lerp( const T& start, const T& end, double time )
{
	return glm::mix( start, end, time );
}

template<>
inline ci::quat Clip<ci::quat>::lerp( const ci::quat& start, const ci::quat& end, double time )
{
	return glm::slerp( start, end, static_cast<float>( time ) );
}

template<>
inline ci::dquat Clip<ci::dquat>::lerp( const ci::dquat& start, const ci::dquat& end, double time )
{
	return glm::slerp( start, end, time );
}

using Vec3Clip = Clip<ci::vec3>;
using Vec4Clip = Clip<ci::vec4>;
using QuatClip = Clip<ci::quat>;