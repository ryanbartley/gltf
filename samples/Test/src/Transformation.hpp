//
//  Transformation.hpp
//  Test
//
//  Created by ryan bartley on 3/7/16.
//
//

#pragma once

#include "cinder/Vector.h"

struct Transform {
	enum UpdatedType {
		SCALE = 1 << 0,
		ROTATION = 1 << 1,
		TRANSLATION = 1 << 2
	};
	
	Transform() : mUpdated( 0 ), mScale( 1.0f ), mTranslation( 0.0f, 0.0f, 0.0f, 1.0f ) {}
	Transform( const ci::vec4 &translation, const ci::quat &rotation, const ci::vec3 &scale )
	: mUpdated( SCALE | ROTATION | TRANSLATION ), mTranslation( translation ),
		mRotation( rotation ), mScale( scale )
	{}
	Transform( const ci::mat4 &modelMatrix )
	: mUpdated( 0 ), mModelMatrix( modelMatrix ), mScale( 1.0f )
	{}
	inline void checkUpdated() {
		if( mUpdated == 0 ) return;
		
		// TODO: Changed this alot figure out if it still works
		mModelMatrix = ci::mat4();
		mModelMatrix *= ci::translate( ci::vec3( mTranslation ) );
		mModelMatrix *= ci::toMat4( mRotation );
		mModelMatrix *= ci::scale( mScale );
		mUpdated = 0;
	}
	
	inline const ci::mat4& getModelMatrix() const { return mModelMatrix; }
	inline const ci::vec4& getTranslation() const { return mTranslation; }
	inline const ci::vec3& getScale() const { return mScale; }
	inline const ci::quat& getRotation() const { return mRotation; }
	
	inline void setTranslation( const ci::vec4 &translation ) {
		mUpdated |= UpdatedType::TRANSLATION;
		mTranslation = translation;
	}
	inline void translate( const ci::vec4 &translation ) {
		mUpdated |= UpdatedType::TRANSLATION;
		mTranslation += translation;
	}
	inline void setScale( const ci::vec3 &scale ) {
		mUpdated |= UpdatedType::SCALE;
		mScale = scale;
	}
	inline void setRotation( const ci::quat &rotation ) {
		mUpdated |= UpdatedType::ROTATION;
		mRotation = rotation;
	}
	inline void rotate( const ci::quat &rotation ) {
		mUpdated |= UpdatedType::ROTATION;
		mRotation *= rotation;
	}
	inline void setMatrix( const ci::mat4 &mat ) {
		mUpdated = 0;
		mModelMatrix = mat;
	}
	
private:
	ci::mat4	mModelMatrix;
	ci::vec4	mTranslation;
	ci::quat	mRotation;
	ci::vec3	mScale;
	uint32_t	mUpdated;
	
};
