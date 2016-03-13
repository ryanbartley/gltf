//
//  Animation.hpp
//  Test
//
//  Created by ryan bartley on 3/8/16.
//
//

#pragma once



template <typename T>
class Clip {
public:
	Clip()
	: mStartTime( std::numeric_limits<double>::max() ), mDuration( 0.0 ) {}
	Clip( const std::vector<std::pair<double, T>> &keyFrames );
	
	void addKeyFrame( double time, T value );
	T get( double time );
	T getLooped( double time );
	double getCyclicTime( double time ) { return glm::mod( time, mDuration ); }
	T lerp( const T &begin, const T &end, double time );
	std::pair<double, double> getBounds() const { return { mStartTime, mStartTime + mDuration }; }
	
private:
	
	double				mStartTime, mDuration;
	std::vector<double> mKeyFrameTimes;
	std::vector<T>		mKeyFrameValues;
	
	friend std::ostream& operator<<( std::ostream &os, const Clip<T> &clip );
};

template <typename T>
Clip<T>::Clip( const std::vector<std::pair<double, T>> &keyFrames )
{
	auto begIt = begin( keyFrames );
	auto endIt = end( keyFrames );
//	std::sort( begIt, endIt,
//			  []( const std::pair<double, T> &lhs, const std::pair<double, T> &rhs ){
//				  return lhs.first < rhs.first;
//			  });
//	begIt = begin( keyFrames );
//	endIt = end( keyFrames );
	
	mStartTime = begIt->first;
	mDuration = (endIt - 1)->first - mStartTime;
	std::cout << mDuration << std::endl;
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
inline T Clip<T>::get( double absTime )
{
	T ret;
	auto clamped = glm::clamp( absTime, mStartTime, mStartTime + mDuration );
	auto begIt = begin( mKeyFrameTimes );
	auto nextIt = std::upper_bound( begIt, end( mKeyFrameTimes ) - 1, clamped );
	auto dist = std::distance( begIt, nextIt );
	
	auto prevTime = nextIt - 1;
	auto perTime = (clamped - *(prevTime)) / ((*nextIt) - *(prevTime));
	return lerp( mKeyFrameValues[dist - 1], mKeyFrameValues[dist], perTime );
}

template <typename T>
inline T Clip<T>::getLooped( double absTime )
{
	T ret;
	auto cyclicTime = getCyclicTime( absTime ) + mStartTime;
	auto begIt = begin( mKeyFrameTimes );
	auto nextIt = std::upper_bound( begIt, end( mKeyFrameTimes ) - 1, cyclicTime );
	auto dist = std::distance( begIt, nextIt );
	
	// could be last or first, probably not first.
	auto prevTime = nextIt - 1;
	auto perTime = (cyclicTime - *(prevTime)) / ((*nextIt) - *(prevTime));
	return lerp( mKeyFrameValues[dist - 1], mKeyFrameValues[dist], perTime );
}

template<>
inline float Clip<float>::lerp( const float &begin, const float &end, double time )
{
	return glm::mix( begin, end, time );
}

template<>
inline ci::vec2 Clip<ci::vec2>::lerp( const ci::vec2 &begin, const ci::vec2 &end, double time )
{
	return glm::mix( begin, end, time );
}

template<>
inline ci::vec3 Clip<ci::vec3>::lerp( const ci::vec3 &begin, const ci::vec3 &end, double time )
{
	return glm::mix( begin, end, time );
}

template<>
inline ci::vec4 Clip<ci::vec4>::lerp( const ci::vec4 &begin, const ci::vec4 &end, double time )
{
	return glm::mix( begin, end, time );
}

template<>
inline ci::quat Clip<ci::quat>::lerp( const ci::quat &begin, const ci::quat &end, double time )
{
	return glm::slerp( begin, end, static_cast<float>( time ) );
}

template<>
inline ci::dquat Clip<ci::dquat>::lerp( const ci::dquat &begin, const ci::dquat &end, double time )
{
	return glm::slerp( begin, end, time );
}

template<>
inline Transform Clip<Transform>::lerp( const Transform &begin, const Transform &end, double time )
{
	Transform ret;
	ret.trans = glm::mix( begin.trans, end.trans, time );
	ret.scale = glm::mix( begin.scale, end.scale, time );
	ret.rot = glm::slerp( begin.rot, end.rot, static_cast<float>( time ) );
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