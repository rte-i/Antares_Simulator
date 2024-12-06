# Useful variables definitions
set(src_solver_hydro "${CMAKE_SOURCE_DIR}/solver/hydro")
set(src_libs_antares_study "${CMAKE_SOURCE_DIR}/libs/antares/study")
set(errors "${CMAKE_SOURCE_DIR}/libs/antares/include/antares/antares")



add_executable(test-hydro-inputs-checker test-hydro-inputs-checker.cpp)

target_include_directories(test-hydro-inputs-checker
	PRIVATE
	"${src_libs_antares_study}"
	"${src_solver_hydro}"
	"${errors}"
)
target_link_libraries(test-hydro-inputs-checker
	PRIVATE
	Boost::unit_test_framework
	Antares::study
	antares-solver-hydro
	Antares::array
)

# Linux
if(UNIX AND NOT APPLE)
	target_link_libraries(test-hydro-inputs-checker PRIVATE stdc++fs)
endif()

set_target_properties(test-hydro-inputs-checker PROPERTIES FOLDER Unit-tests)

add_test(NAME hydro-inputs-checker COMMAND test-hydro-inputs-checker)

set_property(TEST hydro-inputs-checker PROPERTY LABELS unit)