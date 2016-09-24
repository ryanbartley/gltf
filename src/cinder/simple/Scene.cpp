//
//  SimpleScene.cpp
//  BasicAnimation
//
//  Created by Ryan Bartley on 8/27/16.
//
//

#include "Scene.h"
#include "cinder/gltf/MeshLoader.h"
#include "cinder/app/App.h"

using namespace std;

namespace cinder { namespace gltf { namespace simple {
	
Scene::Scene( const gltf::FileRef &file, const gltf::Scene *scene )
: mFile( file ), mAnimate( false ), mCurrentCameraInfoId( 0 ), mUsingDebugCamera( false )
{
	mMeshes.reserve( 100 );
	for ( auto &node : scene->nodes ) {
		mNodes.emplace_back( Node::create( node, nullptr, this ) );
	}
	
	// setup camera
	if ( ! mCameras.empty() ) {
		auto &camInfo = mCameras[mCurrentCameraInfoId];
		
		mCamera.setPerspective( toDegrees( camInfo.yfov ), camInfo.aspectRatio, 0.01f, 100000.0f );
	}
	else {
		mCamera.setPerspective( 45.0f, ci::app::getWindowAspectRatio(), 0.01f, 10000.0f );
		mCamera.lookAt( vec3( 0, 10, -5 ), vec3( 0 ) );
	}
	
	double  begin = std::numeric_limits<double>::max(),
			end = std::numeric_limits<double>::min();
	for ( auto & transformClip : mTransformClips ) {
		auto timeBounds = transformClip.getTimeBounds();
		begin = glm::min( begin, timeBounds.first );
		end = glm::max( end, timeBounds.second );
	}
	mStartTime = begin;
	mDuration = end - begin;
	
	for( auto i = 0; i < mTransforms.size(); i++ ) {
		auto &trans = mTransforms[i];
		if( trans.parentId != std::numeric_limits<uint32_t>::max() )
			trans.worldTransform = mTransforms[trans.parentId].worldTransform * trans.localTransform;
		else
			trans.worldTransform = trans.localTransform;
	}
}
	
void Scene::update( double globalTime )
{
	if ( mTransformClips.empty() )
		return;
	
	auto cyclicTime = glm::mod( globalTime, mDuration ) + mStartTime;
	for ( auto &node : mNodes )
		node->update( cyclicTime );
	
	for( auto i = 0; i < mTransforms.size(); i++ ) {
		auto &trans = mTransforms[i];
		if( trans.parentId != std::numeric_limits<uint32_t>::max() )
			trans.worldTransform = mTransforms[trans.parentId].worldTransform * trans.localTransform;
		else
			trans.worldTransform = trans.localTransform;
	}
}

void Scene::renderScene()
{
	gl::ScopedMatrices scopeMat;
	gl::ScopedDepth scope(true);
	if( ! mCameras.empty() ) {
		auto node = mCameras[mCurrentCameraInfoId].node;
		auto &worldTrans = getWorldTransform( node->getTransformIndex() );
		gl::setMatrices( mCamera );
		gl::setViewMatrix( glm::inverse( worldTrans ) );
	}

	gl::ScopedDepth scopeDepth( true );
	for( auto &mesh : mMeshes ) {
		auto ctx = gl::context();
		auto &difTex = mesh.mDiffuseTex;
		if( difTex ) {
			gl::color( 1, 1, 1 );
			ctx->pushTextureBinding( difTex->getTarget(), difTex->getId(), 0 );
		}
		else
			gl::color( 1, 1, 1, 1 );
		
		gl::setModelMatrix( getWorldTransform( mesh.node->getTransformIndex() ) );
		mesh.mBatch->draw();
		
		if( difTex )
			ctx->popTextureBinding( difTex->getTarget(), 0 );
	}
}
	
uint32_t Scene::setupTransform( uint32_t parentTransId, ci::mat4 localTransform )
{
	auto ret = mTransforms.size();
	Transform trans;
	trans.parentId = parentTransId;
	trans.localTransform = localTransform;
	trans.worldTransform = localTransform;
	trans.dirty = false;
	mTransforms.emplace_back( trans );
	return ret;
}
	
void Scene::selectCamera( uint32_t selection )
{
	mCurrentCameraInfoId = glm::clamp( selection, (uint32_t)0, numCameras() - 1 );
}

void Scene::updateTransform( uint32_t transId, ci::mat4 localTransform )
{
	CI_ASSERT( transId < mTransforms.size() );
	mTransforms[transId].localTransform = localTransform;
}

ci::mat4& Scene::getWorldTransform( uint32_t transId )
{
	CI_ASSERT( transId < mTransforms.size() );
	return mTransforms[transId].worldTransform;
}
	
ci::mat4& Scene::getLocalTransform( uint32_t transId )
{
	CI_ASSERT( transId < mTransforms.size() );
	return mTransforms[transId].localTransform;
}
	
ci::mat4 Scene::getParentWorldTransform( uint32_t transId )
{
	CI_ASSERT( transId < mTransforms.size() );
	auto &trans = mTransforms[transId];
	if( trans.parentId != std::numeric_limits<uint32_t>::max() )
		return mTransforms[trans.parentId].worldTransform;
	
	return ci::mat4();
}
	
simple::Node* Scene::findNodeByKey( const std::string &key )
{
	for ( auto &node : mNodes ) {
		auto found = node->findNodeByKey( key );
		if( found )
			return found;
	}
	
	return nullptr;
}

int32_t	Scene::addTransformClip( TransformClip clip )
{
	auto ret = mTransformClips.size();
	mTransformClips.emplace_back( std::move( clip ) );
	return ret;
}

void Scene::getClipComponentsAtTime( int32_t animationId, float globalTime,
									ci::vec3 *translation, ci::quat *rotation, ci::vec3 *scale )
{
	CI_ASSERT( animationId < mTransformClips.size() );
	if ( ! mAnimate )
		return;
	
	auto &transClip = mTransformClips[animationId];
	if( ! transClip.getTranslationClip().empty() )
		*translation = transClip.getTranslation( globalTime );
	if( ! transClip.getRotationClip().empty() )
		*rotation = transClip.getRotation( globalTime );
	if( ! transClip.getScaleClip().empty() )
		*scale = transClip.getScale( globalTime );

	//ci::app::console() << transClip << std::endl;
}
	
Scene::Mesh::Mesh( gl::BatchRef batch, gl::Texture2dRef difTex, ColorA difColor, Node *node )
: mBatch( std::move( batch ) ), mDiffuseTex( std::move( difTex ) ),
	mDiffuseColor( difColor ), node( node )
{
}

Scene::Mesh::Mesh( const Mesh & mesh )
: mBatch( mesh.mBatch ), mDiffuseTex( mesh.mDiffuseTex ),
	mDiffuseColor( mesh.mDiffuseColor ), node( mesh.node )
{
}

Scene::Mesh& Scene::Mesh::operator=( const Mesh & mesh )
{
	if( this != &mesh ) {
		mBatch = mesh.mBatch;
		mDiffuseTex = mesh.mDiffuseTex;
		mDiffuseColor = mesh.mDiffuseColor;
		node = mesh.node;
	}
	return *this;
}

Scene::Mesh::Mesh( Mesh &&mesh ) NOEXCEPT
: mBatch( move( mesh.mBatch ) ), mDiffuseTex( move(mesh.mDiffuseTex ) ),
mDiffuseColor( move(mesh.mDiffuseColor) ), node( mesh.node )
{
}

Scene::Mesh& Scene::Mesh::operator=( Mesh &&mesh ) NOEXCEPT
{
	if( this != &mesh ) {
		mBatch = move(mesh.mBatch);
		mDiffuseTex = move(mesh.mDiffuseTex);
		mDiffuseColor = mesh.mDiffuseColor;
		node = mesh.node;
	}
	return *this;
}
	
Scene::CameraInfo::CameraInfo( float aspectRatio, float yfov, float znear, float zfar, Node *node )
: aspectRatio( aspectRatio ), yfov( yfov ), znear( znear ), zfar( zfar ), node( node )
{
}

Scene::CameraInfo::CameraInfo( const CameraInfo & info )
: aspectRatio( info.aspectRatio ), yfov( info.yfov ), znear( info.znear ),
	zfar( info.zfar ), node( info.node )
{
}

Scene::CameraInfo& Scene::CameraInfo::operator=( const CameraInfo &info )
{
	if( this != &info ) {
		aspectRatio = info.aspectRatio;
		yfov = info.yfov;
		znear = info.znear;
		zfar = info.zfar;
		node = info.node;
	}
	return *this;
}

Scene::CameraInfo::CameraInfo( CameraInfo &&info ) NOEXCEPT
: aspectRatio( info.aspectRatio ), yfov( info.yfov ), znear( info.znear ),
	zfar( info.zfar ), node( info.node )
{
}

Scene::CameraInfo& Scene::CameraInfo::operator=( CameraInfo &&info ) NOEXCEPT
{
	if( this != &info ) {
		aspectRatio = info.aspectRatio;
		yfov = info.yfov;
		znear = info.znear;
		zfar = info.zfar;
		node = info.node;
	}
	return *this;
}
	
	
}}}
