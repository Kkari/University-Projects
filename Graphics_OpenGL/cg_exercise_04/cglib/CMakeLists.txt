include(${CGLIB_DIR}/CMakeExerciseConfig.txt)

include_directories(SYSTEM
	lib/glm
)

include_directories(
	lib/glew/include
	lib/glfw/include
	lib/AntTweakBar/include
	include
)

include(${CGLIB_DIR}/CMakeCGLibSource.txt)

foreach (target ${targets})
	add_library (cglib_${target}_${EXERCISE_NUMBER} ${CGLIB_SOURCE_FILES})

	set_target_properties(cglib_${target}_${EXERCISE_NUMBER} PROPERTIES COMPILE_FLAGS ${CXXFLAGS_${target}})

	if   (MSVC)
		target_link_libraries(cglib_${target}_${EXERCISE_NUMBER} ${GLEW_LIBRARIES})
	else ()
        if (APPLE)
            target_link_libraries(cglib_${target}_${EXERCISE_NUMBER} ${GLEW_LIBRARIES} m stdc++ anttweakbar)
        else()
            target_link_libraries(cglib_${target}_${EXERCISE_NUMBER} ${GLEW_LIBRARIES} m stdc++ pthread)
        endif()
	endif()
endforeach(target)

if(BUILD_TEST_FRAMEWORK)
	add_library (cglib_TestFramework
		include/cglib/TestFramework/CmdParser.h
		include/cglib/TestFramework/TestFramework.h
		include/cglib/TestFramework/TestGroup.h
		include/cglib/TestFramework/TestLog.h
		include/cglib/TestFramework/TestSuite.h
		include/cglib/TestFramework/TestUtil.h
		src/TestFramework/CmdParser.cpp
		src/TestFramework/TestFramework.cpp
		src/TestFramework/TestGroup.cpp
		src/TestFramework/TestSuite.cpp
		src/TestFramework/TestUtil.cpp

		# seccomp-bpf helper files
		include/cglib/TestFramework/seccomp-bpf/seccomp.h
		include/cglib/TestFramework/seccomp-bpf/seccomp-bpf.h
	)
endif(BUILD_TEST_FRAMEWORK)
