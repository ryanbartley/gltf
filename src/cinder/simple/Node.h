#pragma once

#include "cinder/Vector.h"
#include "cinder/Quaternion.h"
#include <vector>
#include <memory>

namespace cinder {
namespace gltf {
class Node;
namespace simple {

class Scene;

class Node {
public:
	Node( const gltf::Node *node, simple::Node *parent, Scene *scene );
	Node* getParent() { return mParent; }
	void update( float globalTime );

	const ci::vec3& getLocalTranslation() const { return mCurrentTrans; }
	const ci::vec3& getLocalScale() const { return mCurrentScale; }
	const ci::quat& getLocalRotation() const { return mCurrentRot; }

	uint32_t		getTransformIndex() { return mTransformIndex; }
	int32_t			getAnimationId() { return mAnimationIndex; }

	enum class Type {
		NODE,
		MESH,
		CAMERA
	};

	Type getNodeType() const { return mType; }

	ci::vec3	mCurrentTrans, mCurrentScale;
	ci::quat	mCurrentRot;

private:
	Scene				*mScene;
	Node				*mParent;
	std::vector<std::unique_ptr<Node>>	mChildren;

	Type		mType;
	uint32_t	mTypeId;

	uint32_t	mTransformIndex;
	int32_t		mAnimationIndex;

	ci::vec3	mOriginalTranslation, mOriginalScale;
	ci::quat	mOriginalRotation;

	std::string mKey, mName;
};

using UniqueNode = std::unique_ptr<Node>;

} // simple
} // gltf
} // cinder