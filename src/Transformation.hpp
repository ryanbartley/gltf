//
//  Transformation.hpp
//  Test
//
//  Created by ryan bartley on 3/7/16.
//
//

#pragma once

#include "cinder/Vector.h"
#include "dqconv.h"

struct Transform {
	Transform() : scale( 1.0f ), trans( 0.0f, 0.0f, 0.0f ) {}
	Transform( const ci::vec4 &translation, const ci::quat &rotation, const ci::vec3 &scale )
	: trans( translation ), rot( rotation ), scale( scale )
	{}
	
	ci::mat4 getTRS() const {
		ci::mat4 ret;
		ret *= ci::translate( trans );
		ret *= ci::toMat4( rot );
		ret *= ci::scale( scale );
		return ret;
	}
	
	const ci::vec3& getTranslation() const { return trans; }
	const ci::vec3& getScale() const { return scale; }
	const ci::quat& getRotation() const { return rot; }
	
	void setTranslation( const ci::vec3 &translation ) { trans = translation; }
	void setRotation( const ci::quat &rotation ) { rot = rotation; }
	void setScale( const ci::vec3 &scale ) { this->scale = scale; }
	
	void translateBy( const ci::vec3 &translation ) { trans+= translation; }
	void rotateBy( const ci::quat &rotation ) { rot *= rotation; }
	void scaleBy( const ci::vec3 &scale ) { this->scale *= scale; }
	
	Transform translateToParentSpace( const Transform &parentTransform ) const
	{
		Transform ret;
		const glm::quat& parentRotation = parentTransform.getRotation();
		const glm::vec3& parentScale = parentTransform.getScale();
		
		ret.rot = parentRotation * rot;
		ret.scale = parentScale * scale;
		
		// change position vector based on parent's rotation & scale
		ret.trans = parentRotation * ( parentScale * trans );
		// add altered position vector to parent's
		ret.trans += parentTransform.getTranslation();
		return ret;
	}
	
	ci::vec3	trans;
	ci::vec3	scale;
	ci::quat	rot;
};

std::ostream& operator<<( std::ostream &os, const Transform &trans )
{
	os << "Translation: " << trans.trans << " Rotation: " << trans.rot << " Scale: " << trans.scale << std::endl;
	return os;
}

namespace dq {
	struct DQTransform {
		
	};
}