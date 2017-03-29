CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

include_directories(${COMN_DIR}/include)


if (MSVC)
	include_directories(${THIRD_PARTY}/stdint)
endif()


link_directories(${PROJECT_ROOT}/lib)
link_directories(${COMN_DIR}/lib)



aux_source_directory(. SOURCES)

add_executable(UdpForward ${SOURCES})


if (MSVC)
	SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
endif()

target_link_libraries(UdpForward
	${LIB_PLATFORM}
	debug comnd optimized comn
	)

