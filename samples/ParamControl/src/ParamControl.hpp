//
//  ParamControl.hpp
//  ParamControl
//
//  Created by ryan bartley on 12/8/15.
//
//

#pragma once

#include <map>

enum class ParamType {
	FLOAT,
	DOUBLE,
	INT,
	INT64,
	UINT,
	UINT64,
	VEC_2,
	VEC_3,
	VEC_4,
	IVEC_2,
	IVEC_3,
	IVEC_4
};

struct ParamData {
	
	std::string name;
	ParamType	type;
};

class ParamControl {
public:
	
	void beginParam( std::string parentName );
	void addParam( ParamData data );
	void endParam();
private:
	std::map<std::string, std::vector<ParamType>> mParams;
	std::pair<const std::string, ParamType> mCurrentParam;
};
