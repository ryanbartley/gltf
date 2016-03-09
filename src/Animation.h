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
	Clip();
	Clip( const std::map<double, T> curve );
	
	struct KeyFrame {
		double	time;
		T		value;
	};
	
	void	addKeyframe( double time, T value );
	
	T		getValue( double time ) const;
	
	inline bool	empty() const { return mKeyframes.empty(); }
	std::pair<double, double>	getBounds() const { return { mStartTime, mEndTime }; }
	double	getDuration() const { return mEndTime - mStartTime; }
	
	
	static inline T		lerp( const T& start, const T& end, double time );
private:
	static inline bool	isFinite( const glm::vec3& vec );
	inline double		getCyclicTime( double time ) const;
	
	std::vector<KeyFrame>	mKeyframes;
	
	double				mStartTime;
	double				mEndTime;
};

template< typename T >
Clip<T>::Clip()
: mStartTime( std::numeric_limits<double>::max() )
, mEndTime( std::numeric_limits<double>::lowest() )
{
	
}

template< typename T>
Clip<T>::Clip( const std::map<double, T> curve )
: mKeyframes( curve )
, mStartTime( std::numeric_limits<double>::max() )
, mEndTime( std::numeric_limits<double>::lowest() )
{
	if( ! mKeyframes.empty() ) {
		mStartTime = mKeyframes.begin()->first;
		mEndTime = mKeyframes.rbegin()->first;
	}
}

template< typename T >
void Clip<T>::addKeyframe( double time, T value )
{
	mKeyframes[time] = value;
	
	mStartTime = glm::min( time, mStartTime );
	mEndTime = glm::max( time, mEndTime );
}

template< typename T >
T Clip<T>::getValue( double time ) const
{
	CI_ASSERT( ! mKeyframes.empty() );
	
	if( mKeyframes.size() == 1 )
		return mKeyframes.begin()->second;
	
	double cyclicTime = getCyclicTime( time );
	
	auto itnext =  mKeyframes.upper_bound( cyclicTime );
	auto itprev = ( itnext == mKeyframes.begin() ) ? mKeyframes.end() : itnext;
	itprev--;
	
	// no interpolation needed, we are right on the 'prev' keyframe
	if( cyclicTime == 0.0f || itprev->first == cyclicTime )
		return itprev->second;
	
	double normalizedTime;
	if( itnext == mKeyframes.begin() )
		normalizedTime = cyclicTime / itnext->first;
	else
		normalizedTime = (cyclicTime - itprev->first) / (itnext->first - itprev->first);
	
	CI_ASSERT( 0.0f < normalizedTime && 1.0f >= normalizedTime);
	return lerp( itprev->second, itnext->second, normalizedTime );
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

//class Keyframe {
//public:
//	Keyframe( uint32_t index, uint8_t count, float time )
//	: mIndex( index ), mCount( count ), mAtTime( time ) {}
//	
//	float	 getTime() { return mAtTime; }
//	uint32_t getIndex() { return mIndex; }
//	uint8_t	 getCount() { return mCount; }
//	
//private:
//	uint32_t	mIndex;
//	uint8_t		mCount;
//	float		mAtTime;
//};
//
//class Clip {
//public:
//	Clip( const gltf::FileRef &file, const gltf::Animation &param );
//	
//	void updateGlobalTime( float globalTime, float elapsedSeconds );
//	
//private:
//	
//	std::map<std::string, std::vector<Keyframe>> mKeyFrames;
//	std::vector<float>				mData;
//};

