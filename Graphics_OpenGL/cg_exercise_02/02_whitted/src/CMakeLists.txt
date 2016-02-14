foreach (target ${targets})
	add_executable(${target}
		exercise_02.cpp
		main.cpp
	)

	set_target_properties(${target} PROPERTIES COMPILE_FLAGS ${CXXFLAGS_${target}})

	target_link_libraries(${target} ${CGLIB_LINK_LIBRARIES_${target}})
	if(NOT WIN32)
		add_dependencies(${target} AntTweakBar_make)
	endif()

	if (WIN32)
		change_workingdir(${target} ${CMAKE_CURRENT_SOURCE_DIR}/..)

		# TODO: Check if the last line ($<...>) is correct.
		add_custom_command(TARGET ${target} POST_BUILD       
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
			${ANTTWEAKBAR_LIB_DLL}     
			$<TARGET_FILE_DIR:cg>)                
	endif()
endforeach(target)

