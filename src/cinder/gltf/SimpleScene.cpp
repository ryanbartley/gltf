//
//  SimpleScene.cpp
//  BasicAnimation
//
//  Created by Ryan Bartley on 8/27/16.
//
//

#include "SimpleScene.h"
#include "cinder/gltf/MeshLoader.h"
#include "cinder/app/App.h"

using namespace std;

namespace cinder { namespace gltf { namespace simple {
	
Scene::Scene( const gltf::FileRef &file, const gltf::Scene *scene )
: mFile( file ), mAnimate( false ), mCurrentCameraInfoId( 0 ), mUsingDebugCamera( false )
{
	mMeshes.reserve( 100 );
	for ( auto &node : scene->nodes ) {
		mNodes.emplace_back( Scene::Node::create( node, nullptr, this ) );
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
	
void Scene::update()
{
	if ( mTransformClips.empty() )
		return;
	
	auto time = ci::app::getElapsedSeconds();
	auto cyclicTime = glm::mod( time, mDuration ) + mStartTime;
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
	if( ! mCameras.empty() ) {
//		auto &localTrans = getLocalTransform( node->getTransformIndex() );
//		cout << "model of cam: " << localTrans << endl;
//		cout << "inverse of model: " << glm::inverse( localTrans ) << endl;
//		cout << "world matrix: " << worldTrans << endl;
//		cout << "inverse of world: " << glm::inverse( worldTrans ) << endl;
		
//		auto trans = node->getLocalTranslation();
//		auto rot = node->getLocalRotation();
//		mCamera.setEyePoint( trans );
//		mCamera.setOrientation( rot );
		
//		gl::ScopedGlslProg scope( gl::getStockShader( gl::ShaderDef().color() ) );
//		gl::color( 1, 1, 1 );
//		gl::drawFrustum( mCamera );
		
		auto node = mCameras[mCurrentCameraInfoId].node;
		auto &worldTrans = getWorldTransform( node->getTransformIndex() );
		auto parentTrans = getParentWorldTransform( node->getTransformIndex() );
		gl::setMatrices( mCamera );
		gl::setViewMatrix( glm::inverse( worldTrans ) );
	}

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
	
void Scene::toggleDebugCamera()
{
	
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
}
	
using Node = Scene::Node;

Node::Node( const gltf::Node *node, simple::Scene::Node *parent, Scene *scene )
: mScene( scene ), mParent( parent ), mAnimationIndex( -1 ), mKey( node->key ), mName( node->name )
{
	// Cache current node local model matrix
	cout << "name: " << node->key << std::endl;
	ci::mat4 modelMatrix;
	// if it's a transform matrix
	if( ! node->transformMatrix.empty() ) {
		// grab it
		modelMatrix = node->getTransformMatrix();
	}
	// otherwise, it's broken up into components
	else {
		// grab the components
		mOriginalTranslation = node->getTranslation();
		mOriginalRotation = node->getRotation();
		mOriginalScale = node->getScale();
		// create the placeholder matrix
		modelMatrix *= glm::translate( mOriginalTranslation );
		modelMatrix *= glm::toMat4( mOriginalRotation );
		modelMatrix *= glm::scale( mOriginalScale );
		// usually when it's broken up like this that means it's animated
		auto transformClip = mScene->mFile->collectTransformClipFor( node );
		if( ! transformClip.empty() )
			mAnimationIndex = mScene->addTransformClip( move( transformClip ) );
	}
	cout << "matrix: " << modelMatrix << endl;
	
	// get the parent index if there's a parent
	uint32_t parentIndex = std::numeric_limits<uint32_t>::max();
	if ( mParent )
		parentIndex = mParent->getTransformIndex();
	// cache the transform
	mTransformIndex = mScene->setupTransform( parentIndex, modelMatrix );
	
	// cache the children
	for ( auto &children : node->children )
		mChildren.emplace_back( Node::create( children, this, scene ) );
	
	// check if there's meshes
	if( node->hasMeshes() ) {
		geom::SourceMods meshCombo;
		// there may be multiple meshes, so combine them for now
		gl::Texture2dRef diffuseTex;
		ci::ColorA diffuseCol;
		for( auto mesh : node->meshes ) {
			meshCombo &= gltf::MeshLoader( mesh );
			// this is rough.
			auto &sources = mesh->primitives[0].material->sources;
			if( ! sources.empty() ) {
				if( sources[0].texture ) {
					auto image = sources[0].texture->image->getImage();
					diffuseTex = gl::Texture2d::create( image, gl::Texture2d::Format().loadTopDown() );
				}
				else
					diffuseCol = sources[0].color;
			}
		}
		// quick rendering decision
		gl::GlslProgRef glsl;
		if( diffuseTex )
			glsl = gl::getStockShader( gl::ShaderDef().lambert().texture() );
		else
			glsl = gl::getStockShader( gl::ShaderDef().color().lambert() );
		// finally create the batch
		auto batch = gl::Batch::create( meshCombo, glsl );
		mType = Type::MESH;
		mTypeId = mScene->mMeshes.size();
		mScene->mMeshes.emplace_back( move( batch ), move( diffuseTex ), diffuseCol, this );
	}
	else if( node->isCamera() ) {
		mType = Type::CAMERA;
		mTypeId = mScene->mCameras.size();
		mScene->mCameras.emplace_back( node->camera->aspectRatio, node->camera->yfov, node->camera->znear, node->camera->zfar, this );
	}
}
	
Scene::UniqueNode Node::create( const gltf::Node *node, simple::Scene::Node *parent, Scene *scene )
{
	return std::unique_ptr<simple::Scene::Node>( new Node( node, parent, scene ) );
}
	
void Node::update( float globalTime )
{
	for( auto &child : mChildren )
		child->update( globalTime );
	
	if ( mAnimationIndex < 0 )
		return;
	
	// setup the defaults
	mCurrentTrans = mOriginalTranslation;
	mCurrentRot = mOriginalRotation;
	mCurrentScale = mOriginalScale;
	// get the clips animation values, if a certain component isn't animated than it's defaults remain
	mScene->getClipComponentsAtTime( mAnimationIndex, globalTime, &mCurrentTrans, &mCurrentRot, &mCurrentScale );
	// create the modelMatrix
	ci::mat4 modelMatrix;
	modelMatrix *= glm::translate( mCurrentTrans );
	modelMatrix *= glm::toMat4( mCurrentRot );
	modelMatrix *= glm::scale( mCurrentScale );
	cout << "trans: " << mCurrentTrans << " rot: " << mCurrentRot << " scale: " << mCurrentScale << std::endl;
	// update the scene's modelmatrix
	mScene->updateTransform( mTransformIndex, modelMatrix );
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

Scene::Mesh::Mesh( Mesh &&mesh ) noexcept
: mBatch( move( mesh.mBatch ) ), mDiffuseTex( move(mesh.mDiffuseTex ) ),
mDiffuseColor( move(mesh.mDiffuseColor) ), node( mesh.node )
{
}

Scene::Mesh& Scene::Mesh::operator=( Mesh &&mesh ) noexcept
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

Scene::CameraInfo::CameraInfo( CameraInfo &&info ) noexcept
: aspectRatio( info.aspectRatio ), yfov( info.yfov ), znear( info.znear ),
	zfar( info.zfar ), node( info.node )
{
}

Scene::CameraInfo& Scene::CameraInfo::operator=( CameraInfo &&info ) noexcept
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
