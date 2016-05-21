//
//  box_animated.h
//  Test
//
//  Created by ryan bartley on 3/14/16.
//
//

#pragma once

class BoxAnimated {
public:
	BoxAnimated();
	void setup();
	void draw();
	
	struct Renderable {
		std::string nodeName;
		ci::mat4	modelMatrix;
		ci::gl::BatchRef batch;
	};
	Clip<ci::vec3> animTrans;
	Clip<ci::quat> animRot;
	
	std::vector<Renderable> mRenderables;
};

BoxAnimated::BoxAnimated()
{
	auto file = gltf::File::create( ci::app::loadAsset( ci::fs::path( "boxAnimated" ) / "glTF" / "glTF.gltf" ) );
	const auto &defaultScene = file->getDefaultScene();
	for( auto & node : defaultScene.nodes ) {
		if( node->hasMeshes() ) {
			auto mesh = node->meshes[0];
			gltf::MeshLoader meshLoader( mesh );
			auto batch = ci::gl::Batch::create( meshLoader, ci::gl::getStockShader( ci::gl::ShaderDef().color().lambert() ) );
			Renderable rend{ node->name, ci::mat4(), batch };
			rend.modelMatrix = node->getTransformMatrix();
			
			mRenderables.emplace_back( std::move( rend ) );
		}
	}
	const auto &animations = file->getCollectionOf<gltf::Animation>();
	for( const auto &animationKV : animations ) {
		auto &animation = animationKV.second;
		
		auto paramData = animation.getParameters();
		if( paramData[1].paramName == "rotation" ) {
			animRot = gltf::Animation::createRotationClip( paramData );
		}
		else if( paramData[1].paramName == "translation" ) {
			animTrans = gltf::Animation::createTranslationClip( paramData );
		}
	}
}

void BoxAnimated::draw()
{
	for( auto & rend : mRenderables ) {
		ci::gl::ScopedModelMatrix scopeModel;
		ci::gl::setModelMatrix( rend.modelMatrix );
		if( rend.nodeName == "inner_box" ) {
//			auto elapsedSeconds = ci::app::getElapsedFrames() / 60.0;
//			Transform trans;
//			trans.setTranslation( animTrans.get( elapsedSeconds ) );
//			trans.setRotation( animRot.get( elapsedSeconds ) );
//			ci::gl::multModelMatrix( trans.getTRS() );
		}
		rend.batch->draw();
	}
}