//
//  SimpleScene.cpp
//  BasicAnimation
//
//  Created by Ryan Bartley on 8/27/16.
//
//

#include "SimpleScene.h"
#include "cinder/gltf/MeshLoader.h"

using namespace std;

namespace cinder { namespace gltf { namespace simple {
	
Scene::Scene( const gltf::FileRef &file, const gltf::Scene *scene )
: mFile( file ), mAnimate( false )
{
	for ( auto &node : scene->nodes ) {
		mNodes.emplace_back( node, nullptr, this );
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
}
	
void Scene::update()
{
	if ( mTransformClips.empty() )
		return;
	
	auto time = ci::app::getElapsedSeconds();
	auto cyclicTime = glm::mod( time, mDuration ) + mStartTime;
	for ( auto &node : mNodes )
		node.update( cyclicTime );
	
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
	for ( auto &node : mNodes )
		node.render();
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

void Scene::updateTransform( uint32_t transId, ci::mat4 localTransform )
{
	CI_ASSERT( transId < mTransforms.size() );
	mTransforms[transId].localTransform = localTransform;
}

ci::mat4 Scene::getWorldTransform( uint32_t transId )
{
	CI_ASSERT( transId < mTransforms.size() );
	return mTransforms[transId].worldTransform;
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
	ci::mat4 modelMatrix;
	// if it's a transform matrix
	if( ! node->transformMatrix.empty() ) {
		// grab it
		modelMatrix = node->getTransformMatrix();
	}
	// otherwise, it's broken up into components
	else {
		// grab the components
		mTranslation = node->getTranslation();
		mRotation = node->getRotation();
		mScale = node->getScale();
		// create the placeholder matrix
		modelMatrix *= glm::translate( mTranslation );
		modelMatrix *= glm::toMat4( mRotation );
		modelMatrix *= glm::scale( mScale );
		// usually when it's broken up like this that means it's animated
		auto transformClip = mScene->mFile->collectTransformClipFor( node );
		if( ! transformClip.empty() )
			mAnimationIndex = mScene->addTransformClip( move( transformClip ) );
	}
	
	// get the parent index if there's a parent
	uint32_t parentIndex = std::numeric_limits<uint32_t>::max();
	if ( mParent )
		parentIndex = mParent->getTransformIndex();
	// cache the transform
	mTransformIndex = mScene->setupTransform( parentIndex, modelMatrix );
	
	// cache the children
	for ( auto &node : node->children ) {
		mChildren.emplace_back( node, this, scene );
	}
	
	// check if there's meshes
	if( ! node->meshes.empty() ) {
		geom::SourceMods meshCombo;
		// there may be multiple meshes, so combine them for now
		for( auto mesh : node->meshes ) {
			meshCombo &= gltf::MeshLoader( mesh );
			// this is rough.
			auto &sources = mesh->primitives[0].material->sources;
			if( ! sources.empty() ) {
				if( sources[0].texture ) {
					auto image = sources[0].texture->image->getImage();
					mDiffuseTex = gl::Texture2d::create( image, gl::Texture2d::Format().loadTopDown() );
				}
				else
					mDiffuseColor = sources[0].color;
			}
		}
		// quick rendering decision
		gl::GlslProgRef glsl;
		if( mDiffuseTex )
			glsl = gl::getStockShader( gl::ShaderDef().lambert().texture() );
		else
			glsl = gl::getStockShader( gl::ShaderDef().color().lambert() );
		// finally create the batch
		mBatch = gl::Batch::create( meshCombo, glsl );
	}
}
	
void Node::update( float globalTime )
{
	for( auto &child : mChildren ) {
		child.update( globalTime );
	}
	
	if ( mAnimationIndex < 0 )
		return;
	
	// setup the defaults
	ci::vec3 translation = mTranslation;
	ci::quat rotation = mRotation;
	ci::vec3 scale = mScale;
	// get the clips animation values, if a certain component isn't animated than it's defaults remain
	mScene->getClipComponentsAtTime( mAnimationIndex, globalTime, &translation, &rotation, &scale );
	// create the modelMatrix
	ci::mat4 modelMatrix;
	modelMatrix *= glm::translate( translation );
	modelMatrix *= glm::toMat4( rotation );
	modelMatrix *= glm::scale( scale );
	// update the scene's modelmatrix
	mScene->updateTransform( mTransformIndex, modelMatrix );
}

void Node::render()
{
	for ( auto &node : mChildren ) {
		node.render();
	}
	
	if ( ! mBatch )
		return;
	
	auto ctx = gl::context();
	if( mDiffuseTex ) {
		gl::color( 1, 1, 1 );
		ctx->pushTextureBinding( mDiffuseTex->getTarget(), mDiffuseTex->getId(), 0 );
	}
	else {
		gl::color( mDiffuseColor );
	}
	
	gl::setModelMatrix( mScene->getWorldTransform( mTransformIndex ) );
	mBatch->draw();
	
	if( mDiffuseTex )
		ctx->popTextureBinding( mDiffuseTex->getTarget(), 0 );
}
	
}}}