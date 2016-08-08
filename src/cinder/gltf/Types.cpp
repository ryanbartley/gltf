//
//  Types.cpp
//  Test
//
//  Created by Ryan Bartley on 8/8/16.
//
//

#include "cinder/gltf/Types.h"
#include "cinder/Log.h"
#include "cinder/Skeleton.h"

using namespace ci;
using namespace std;

namespace cinder {
namespace gltf {
	
CameraOrtho Camera::getOrthoCameraByName( const ci::mat4 &transformMatrix )
{
	if( type != Camera::Type::ORTHOGRAPHIC ) throw "This should be orthographic but it's not";
	
	//TODO: This is most likely wrong need to change it.
	CameraOrtho ret( -xmag, xmag, -ymag, ymag, znear, zfar);
	ret.setOrientation( glm::toQuat( transformMatrix ) );
	ret.setEyePoint( ci::vec3( transformMatrix[3] ) );
	return ret;
}

CameraPersp Camera::getPerspCameraByName( const ci::mat4 &transformMatrix )
{
	if( type != Camera::Type::PERSPECTIVE ) throw "This should be perspective but it's not";
	
	CameraPersp ret; //( app->getWindowWidth(), app->getWindowHeight(), yfov, znear, zfar );
	ret.setPerspective( aspectRatio, yfov, znear, zfar );
	ret.setOrientation( glm::toQuat( transformMatrix ) );
	ret.setEyePoint( ci::vec3( transformMatrix[3] ) );
	return ret;
}

ci::geom::Primitive Mesh::convertToPrimitive( GLenum primitive )
{
	switch (primitive) {
		case GL_LINES: return ci::geom::LINES; break;
		case GL_LINE_STRIP:	return ci::geom::LINE_STRIP; break;
		case GL_TRIANGLES: return ci::geom::TRIANGLES; break;
		case GL_TRIANGLE_STRIP: return ci::geom::TRIANGLE_STRIP; break;
		case GL_TRIANGLE_FAN: return ci::geom::TRIANGLE_FAN; break;
		default: CI_LOG_E("Don't know this primitive"); return (ci::geom::Primitive)-1; break;
	}
}

ci::geom::Attrib Mesh::getAttribEnum( const std::string &attrib )
{
	using namespace ci::geom;
	if( attrib == "POSITION" )			return Attrib::POSITION;
	else if( attrib == "NORMAL" )		return Attrib::NORMAL;
	else if( attrib == "TEXCOORD_0" )	return Attrib::TEX_COORD_0;
	else if( attrib == "TEXCOORD_1" )	return Attrib::TEX_COORD_1;
	else if( attrib == "TEXCOORD_2" )	return Attrib::TEX_COORD_2;
	else if( attrib == "TEXCOORD_3" )	return Attrib::TEX_COORD_3;
	else if( attrib == "COLOR" )		return Attrib::COLOR;
	else if( attrib == "JOINT" )		return Attrib::BONE_INDEX;
	else if( attrib == "JOINTMATRIX" ) {
		CI_LOG_W( "UNDEFINED Attib JOINTMATRIX Using CUSTOM_0" );
		return Attrib::CUSTOM_0;
	}
	else if( attrib == "WEIGHT" )		return Attrib::BONE_WEIGHT;
	else								return Attrib::NUM_ATTRIBS;
}

ci::gl::UniformSemantic Technique::getUniformEnum( const std::string &uniform )
{
	auto & u = uniform;
	using namespace ci::gl;
	if( u == "MODEL" )								return UniformSemantic::UNIFORM_MODEL_MATRIX;
	else if( u == "VIEW" )							return UniformSemantic::UNIFORM_VIEW_MATRIX;
	else if( u == "PROJECTION" )					return UniformSemantic::UNIFORM_PROJECTION_MATRIX;
	else if( u == "MODELVIEW" )						return UniformSemantic::UNIFORM_MODEL_VIEW;
	else if( u == "MODELVIEWPROJECTION" )			return UniformSemantic::UNIFORM_MODEL_VIEW_PROJECTION;
	else if( u == "MODELINVERSE" )					return UniformSemantic::UNIFORM_MODEL_MATRIX_INVERSE;
	else if( u == "VIEWINVERSE" )					return UniformSemantic::UNIFORM_VIEW_MATRIX_INVERSE;
	else if( u == "PROJECTIONINVERSE" )				return UniformSemantic::UNIFORM_PROJECTION_MATRIX_INVERSE;
	else if( u == "MODELVIEWINVERSE" )				return UniformSemantic::UNIFORM_MODEL_VIEW;
	else if( u == "MODELVIEWPROJECTIONINVERSE" )	return UniformSemantic::UNIFORM_MODEL_VIEW_PROJECTION;
	else if( u == "MODELINVERSETRANSPOSE" )			return UniformSemantic::UNIFORM_MODEL_MATRIX_INVERSE;
	else if( u == "MODELVIEWINVERSETRANSPOSE" )		return UniformSemantic::UNIFORM_MODEL_VIEW_INVERSE_TRANSPOSE;
	else if( u == "VIEWPORT" )						return UniformSemantic::UNIFORM_VIEWPORT_MATRIX;
	else return (UniformSemantic)-1;
}

uint8_t Accessor::getNumComponents() const
{
	switch( dataType ) {
		case Accessor::Type::SCALAR: return 1;
		case Accessor::Type::VEC2: return 2;
		case Accessor::Type::VEC3: return 3;
		case Accessor::Type::VEC4:
		case Accessor::Type::MAT2: return 4;
		case Accessor::Type::MAT3: return 9;
		case Accessor::Type::MAT4: return 12;
	}
}

uint8_t Accessor::getNumBytesForComponentType() const
{
	switch (componentType) {
		case Accessor::ComponentType::BYTE:
		case Accessor::ComponentType::UNSIGNED_BYTE:
			return 1;
			break;
		case Accessor::ComponentType::SHORT: // SHORT
		case Accessor::ComponentType::UNSIGNED_SHORT: // UNSIGNED_SHORT
			return 2;
			break;
		case Accessor::ComponentType::FLOAT: // FLOAT
			return 4;
			break;
		default: {
			CI_LOG_E("ERROR: That enum doesn't have a dimmension/size.");
			return 0;
		}
			break;
	}
}

void* Accessor::getDataPtr() const
{
	const auto &buffer = bufferView->buffer;
	return reinterpret_cast<uint8_t*>(buffer->getBuffer()->getData()) + bufferView->byteOffset + byteOffset;
}

ci::AxisAlignedBox Mesh::getPositionAABB()
{
	ci::AxisAlignedBox ret;
	for ( auto &prim : primitives ) {
		auto end = prim.attributes.end();
		auto attrib = find_if( prim.attributes.begin(), end,
							  []( const Primitive::AttribAccessor &a ) { return a.attrib == geom::Attrib::POSITION; });
		if( attrib != end ) {
			auto accessor = attrib->accessor;
			vec3 min, max;
			int i = 0;
			for ( auto val : accessor->min )
				min[i++] = val;
			ret.include( min );
			i = 0;
			for ( auto val : accessor->max )
				max[i++] = val;
			ret.include( max );
		}
	}
	return ret;
}

const Node* Node::getChild( size_t index ) const
{
	return children[index];
}

const Node* Node::getChild( const std::string &nodeName ) const
{
	const Node* ret = nullptr;
	auto endIt = end( children );
	auto found = std::find_if( begin( children ), endIt,
							  [nodeName]( const Node *node ){
								  return nodeName == node->name;
							  });
	if( found != endIt ) {
		ret = *found;
	}
	return ret;
}

ci::mat4 Node::getHeirarchyTransform() const
{
	ci::mat4 ret = getTransformMatrix();
	Node* tempParent = parent;
	
	while( tempParent != nullptr ) {
		ret = tempParent->getHeirarchyTransform() * ret;
		tempParent = tempParent->parent;
	}
	
	return ret;
}

SkeletonRef Skin::createSkeleton() const
{
	auto matricesPtr = reinterpret_cast<ci::mat4*>( inverseBindMatrices->getDataPtr() );
	auto numJoints = joints.size();
	std::vector<std::string> jointNames;
	jointNames.reserve( numJoints );
	
	std::vector<Skeleton::Joint> jointsContainer;
	jointsContainer.reserve( numJoints );
	for( int i = 0; i < numJoints; i++ ) {
		uint8_t parentId;
		// if this joint is the root.
		if( i == 0 )
			parentId = 0xFF;
		else {
			auto begIt = begin( jointNames );
			auto foundIt = std::find( begIt, end( jointNames ), joints[i]->parent->name );
			auto distance = std::distance( begIt, foundIt );
			parentId = distance;
		}
		CI_ASSERT( !  joints[i]->jointName.empty() );
		jointNames.emplace_back(  joints[i]->jointName );
		jointsContainer.emplace_back( parentId, jointNames.size() - 1, *matricesPtr++ );
	}
	auto ret = std::make_shared<Skeleton>( std::move( jointsContainer ), std::move( jointNames ), bindShapeMatrix );
	
	// TODO: possibly faster way of holding jointNames, a little more complicated to keep clean
	
	//	// Transfers joint's names: First computes name's buffer size, then allocate
	//	// and do the copy.
	//	size_t buffer_size = numJoints * sizeof(char*);
	//	for (int i = 0; i < numJoints; ++i) {
	//		buffer_size += (joints[i]->jointName.size() + 1) * sizeof(char);
	//	}
	//	ret->joint_names_ = reinterpret_cast<char**>( new char[buffer_size] );
	//	memset( ret->joint_names_, 0, buffer_size );
	//	char* cursor = reinterpret_cast<char*>(ret->joint_names_ + numJoints);
	//	for (int i = 0; i < numJoints; ++i) {
	//		auto& jointName = joints[i]->jointName;
	//		ret->joint_names_[i] = cursor;
	//		strcpy(cursor, jointName.c_str());
	//		cursor += (jointName.size() + 1) * sizeof(char);
	//	}
	
	return ret;
}

std::vector<Animation::Parameter::Data> Animation::getParameters() const
{
	std::vector<Animation::Parameter::Data> ret;
	ret.reserve( parameters.size() + 1 );
	
	// Initialize times for keyframes
	CI_ASSERT( timeAccessor->dataType == Accessor::Type::SCALAR );
	auto totalKeyFrames = timeAccessor->count;
	auto dataPtr = timeAccessor->getDataPtr();
	
	Animation::Parameter::Data time{ "TIME", 1, std::vector<float>( totalKeyFrames ) };
	memcpy( time.data.data(), dataPtr, totalKeyFrames * sizeof( float ) );
	ret.emplace_back( move( time ) );
	
	for( auto & param : parameters ) {
		const auto accessor = param.accessor;
		auto numComponents = accessor->getNumComponents();
		
		CI_ASSERT( totalKeyFrames == accessor->count );
		auto dataPtr = accessor->getDataPtr();
		
		Animation::Parameter::Data parameter{ param.parameter, numComponents, std::vector<float>( totalKeyFrames * numComponents ) };
		memcpy( parameter.data.data(), dataPtr, accessor->count * numComponents * sizeof( float ) );
		ret.emplace_back( move( parameter ) );
	}
	
	return ret;
}

TransformClip Animation::createTransformClip( const std::vector<Parameter::Data> &paramData )
{
	const std::vector<float> *timeData = nullptr, *scaleData = nullptr,
	*transData = nullptr, *rotData = nullptr;
	for( auto &param : paramData )
		if( param.paramName == "TIME" )
			timeData = &param.data;
		else if( param.paramName == "rotation" )
			rotData = &param.data;
		else if( param.paramName == "scale" )
			scaleData = &param.data;
		else if( param.paramName == "translation" )
			transData = &param.data;
	
	auto totalKeyFrames = timeData->size();
	
	std::vector<std::pair<double, ci::quat>> rotationKeyframes( totalKeyFrames );
	std::vector<std::pair<double, ci::vec3>> translationKeyframes( totalKeyFrames ),
	scaleKeyframes( totalKeyFrames, { 0.0, vec3( 1 ) } );
	
	for( int i = 0, end = totalKeyFrames; i < end; i++ ) {
		auto time = (*timeData)[i];
		translationKeyframes[i].first = time;
		rotationKeyframes[i].first = time;
		scaleKeyframes[i].first = time;
		if( transData != nullptr ) {
			auto translation = *reinterpret_cast<const ci::vec3*>( &(*transData)[i*3] );
			translationKeyframes[i].second = translation;
		}
		if( rotData != nullptr ) {
			auto rotation = *reinterpret_cast<const ci::quat*>( &(*rotData)[i*4] );
			rotationKeyframes[i].second = rotation;
		}
		if( scaleData != nullptr ) {
			auto scale = *reinterpret_cast<const ci::vec3*>( &(*scaleData)[i*3] );
			scaleKeyframes[i].second = scale;
		}
	}
	
	TransformClip ret( std::move( translationKeyframes ),
				   std::move( rotationKeyframes ),
				   std::move( scaleKeyframes ) );
	return ret;
}

Clip<ci::vec3>	Animation::createTranslationClip( const std::vector<Parameter::Data> &paramData )
{
	const std::vector<float> *timeData = nullptr, *transData = nullptr;
	for( auto &param : paramData )
		if( param.paramName == "TIME" )
			timeData = &param.data;
		else if( param.paramName == "translation" )
			transData = &param.data;
	
	std::vector<std::pair<double, ci::vec3>> transformKeyFrames( timeData->size() );
	
	int i = 0;
	for( auto & transformKeyFrame : transformKeyFrames ) {
		transformKeyFrame.first = (*timeData)[i];
		auto translation = *reinterpret_cast<const ci::vec3*>( &(*transData)[i*3] );
		transformKeyFrame.second = translation;
		i++;
	}
	
	Clip<ci::vec3> ret( move( transformKeyFrames ) );
	return ret;
}

Clip<ci::vec3>	Animation::createScaleClip( const std::vector<Parameter::Data> &paramData )
{
	const std::vector<float> *timeData = nullptr, *scaleData = nullptr;
	for( auto &param : paramData )
		if( param.paramName == "TIME" )
			timeData = &param.data;
		else if( param.paramName == "scale" )
			scaleData = &param.data;
	
	std::vector<std::pair<double, ci::vec3>> transformKeyFrames( timeData->size() );
	
	int i = 0;
	for( auto & transformKeyFrame : transformKeyFrames ) {
		transformKeyFrame.first = (*timeData)[i];
		auto scale = *reinterpret_cast<const ci::vec3*>( &(*scaleData)[i*3] );
		transformKeyFrame.second = scale;
		i++;
	}
	
	Clip<ci::vec3> ret( move( transformKeyFrames ) );
	return ret;
}

Clip<ci::quat>	Animation::createRotationClip( const std::vector<Parameter::Data> &paramData )
{
	const std::vector<float> *timeData = nullptr, *rotData = nullptr;
	for( auto &param : paramData )
		if( param.paramName == "TIME" )
			timeData = &param.data;
		else if( param.paramName == "rotation" )
			rotData = &param.data;
	
	std::vector<std::pair<double, ci::quat>> transformKeyFrames( timeData->size() );
	
	int i = 0;
	for( auto & transformKeyFrame : transformKeyFrames ) {
		transformKeyFrame.first = (*timeData)[i];
		auto rotation = *reinterpret_cast<const ci::quat*>( &(*rotData)[i*4] );
		transformKeyFrame.second = rotation;
		i++;
	}
	
	Clip<ci::quat> ret( move( transformKeyFrames ) );
	return ret;
}

ci::mat4 Node::getTransformMatrix() const
{
	ci::mat4 ret;
	if( ! transformMatrix.empty() )
		ret = glm::make_mat4( transformMatrix.data() );
	return ret;
}

ci::vec3 Node::getTranslation() const
{
	ci::vec3 ret;
	if( ! translation.empty() )
		ret = glm::make_vec3( translation.data() );
	return ret;
}

ci::quat Node::getRotation() const
{
	ci::quat ret;
	if( ! rotation.empty() )
		ret = glm::make_quat( rotation.data() );
	return ret;
}

ci::vec3 Node::getScale() const
{
	ci::vec3 ret( 1.0f );
	if( ! scale.empty() )
		ret = glm::make_vec3( scale.data() );
	return ret;
}

void Node::outputToConsole( std::ostream &os, uint8_t tabAmount ) const
{
	using std::endl;
	os << "Name: " << name << endl;
	if( camera != nullptr )
		os << "Camera: " << camera->name << endl;
	else if( light != nullptr )
		os << "Light: " << light->name << endl;
	else if( ! jointName.empty() )
		os << "JointName: " << jointName << endl;
	else {
		if( ! meshes.empty() ) {
			os << "Meshes: " << endl;
			for( auto &mesh : meshes )
				os << "\t" << mesh->name << endl;
		}
		if( skin != nullptr )
			os << "Skin: " << skin->name << endl;
		if( ! skeletons.empty() ) {
			os << "Skeletons: " << endl;
			for( auto &skeleton : skeletons )
				os << "\t" << skeleton->name << endl;
		}
	}
	os << "Transform:" << endl;
	if( ! transformMatrix.empty() ) {
		os << "\tMatrix: [";
		int i = 0;
		for( auto &val : transformMatrix )
			os << " " << val << (i++ < 15 ? "," : " ]");
		os << endl;
	}
	else {
		if( ! translation.empty() ) {
			os << "\tTranslation: [";
			int i = 0;
			for( auto & val : translation )
				os << " " << val << (i++ < 2 ? "," : " ]");
			os << endl;
		}
		if( ! rotation.empty() ) {
			os << "\tRotation: [";
			int i = 0;
			for( auto & val : rotation )
				os << " " << val << (i++ < 3 ? "," : " ]");
			os << endl;
		}
		if( ! scale.empty() ) {
			os << "\tScale: [";
			int i = 0;
			for( auto & val : scale )
				os << " " << val << (i++ < 2 ? "," : " ]");
			os << endl;
		}
	}
	os << endl;
}

} // namespace gltf
} // namespace cinder

using namespace ci;
using namespace ci::gltf;

std::ostream& operator<<( std::ostream &lhs, const Accessor &rhs )
{
	return lhs;
}

std::ostream& operator<<( std::ostream &lhs, const Animation &rhs )
{
	return lhs;
}

std::ostream& operator<<( std::ostream &lhs, const Asset &rhs )
{
	return lhs;
}

std::ostream& operator<<( std::ostream &lhs, const BufferView &rhs )
{
	return lhs;
}

std::ostream& operator<<( std::ostream &lhs, const gltf::Buffer &rhs )
{
	return lhs;
}

std::ostream& operator<<( std::ostream &lhs, const gltf::Camera &rhs )
{
	return lhs;
}

std::ostream& operator<<( std::ostream &lhs, const Image &rhs )
{
	return lhs;
}

std::ostream& operator<<( std::ostream &lhs, const Material &rhs )
{
	return lhs;
}

std::ostream& operator<<( std::ostream &lhs, const Mesh &rhs )
{
	return lhs;
}

std::ostream& operator<<( std::ostream &lhs, const Node &rhs )
{
	rhs.outputToConsole( lhs, 0 );
	return lhs;
}

std::ostream& operator<<( std::ostream &lhs, const Program &rhs )
{
	return lhs;
}

std::ostream& operator<<( std::ostream &lhs, const Sampler &rhs )
{
	return lhs;
}

std::ostream& operator<<( std::ostream &lhs, const Scene &rhs )
{
	return lhs;
}

std::ostream& operator<<( std::ostream &lhs, const Shader &rhs )
{
	return lhs;
}

std::ostream& operator<<( std::ostream &lhs, const Skin &rhs )
{
	return lhs;
}

std::ostream& operator<<( std::ostream &lhs, const Technique &rhs )
{
	return lhs;
}

std::ostream& operator<<( std::ostream &lhs, const Texture &rhs )
{
	return lhs;
}