#include "Node.h"
#include "Scene.h"
#include "cinder/gltf/MeshLoader.h"

using namespace std;

namespace cinder {
namespace gltf {
namespace simple {

Node::Node( const gltf::Node *node, simple::Node *parent, Scene *scene )
	: mScene( scene ), mParent( parent ), mAnimationIndex( -1 ), mKey( node->key ), mName( node->name )
{
	// Cache current node local model matrix
	//cout << "name: " << node->key << std::endl;
	ci::mat4 modelMatrix;
	// if it's a transform matrix
	if( !node->transformMatrix.empty() ) {
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
		if( !transformClip.empty() )
			mAnimationIndex = mScene->addTransformClip( move( transformClip ) );
	}
	//cout << "matrix: " << modelMatrix << endl;

	// get the parent index if there's a parent
	uint32_t parentIndex = std::numeric_limits<uint32_t>::max();
	if( mParent )
		parentIndex = mParent->getTransformIndex();
	// cache the transform
	mTransformIndex = mScene->setupTransform( parentIndex, modelMatrix );

	// cache the children
	for( auto &children : node->children )
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
			if( !sources.empty() ) {
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

Node* Node::findNodeByKey( const std::string &key )
{
	if( key == mKey )
		return this;

	for( auto &child : mChildren ) {
		auto found = child->findNodeByKey( key );
		if( found )
			return found;
	}

	return nullptr;
}

const ci::mat4& Node::getLocalTransform() const
{
	return mScene->getLocalTransform( mTransformIndex );
}

const ci::mat4& Node::getWorldTransform() const
{
	return mScene->getWorldTransform( mTransformIndex );
}

ci::mat4 Node::getParentWorldTransform() const
{
	return mScene->getParentWorldTransform( mTransformIndex );
}

UniqueNode Node::create( const gltf::Node *node, simple::Node *parent, Scene *scene )
{
	return std::unique_ptr<simple::Node>( new Node( node, parent, scene ) );
}

void Node::update( float globalTime )
{
	for( auto &child : mChildren )
		child->update( globalTime );

	if( mAnimationIndex < 0 )
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
	// update the scene's modelmatrix
	mScene->updateTransform( mTransformIndex, modelMatrix );
}

} // simple
} // gltf
} // cinder