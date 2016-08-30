if( NOT TARGET gltf )

	get_filename_component( gltf_SOURCE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../src" ABSOLUTE )
	get_filename_component( CINDER_PATH "${CMAKE_CURRENT_LIST_DIR}/../../../.." ABSOLUTE )

	list( APPEND gltf_SOURCES 
			"${gltf_SOURCE_PATH}/cinder/Skeleton.cpp"
			"${gltf_SOURCE_PATH}/cinder/gltf/Types.cpp"
			"${gltf_SOURCE_PATH}/cinder/gltf/SimpleScene.cpp"
			"${gltf_SOURCE_PATH}/cinder/gltf/MeshLoader.cpp"
			"${gltf_SOURCE_PATH}/cinder/gltf/File.cpp" )

	add_library( gltf "${gltf_SOURCES}" )

	target_include_directories( gltf PUBLIC "${gltf_SOURCE_PATH}" 
											 )
	target_include_directories( gltf PRIVATE BEFORE "${CINDER_PATH}/include" )

	target_compile_options( gltf PRIVATE "-std=c++11" )

	if( NOT TARGET cinder )
		include( "${CINDER_PATH}/proj/cmake/configure.cmake" )
		find_package( cinder REQUIRED PATHS
			"${CINDER_PATH}/${CINDER_LIB_DIRECTORY}"
			"$ENV{CINDER_PATH}/${CINDER_LIB_DIRECTORY}" )
	endif()

	target_link_libraries( gltf PRIVATE cinder )

endif()
