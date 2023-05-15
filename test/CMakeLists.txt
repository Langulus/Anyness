project(Langulus::Anyness::Test)

add_executable(LangulusAnynessTest 
	Main.cpp
	TestAllocator.cpp
	TestBytes.cpp
	TestDeep.cpp
	TestIncomplete.cpp
	TestIteration.cpp
	TestMaps.cpp
	TestNewDelete.cpp
	TestPointers.cpp
	TestSets.cpp
	TestTAny.cpp
	TestText.cpp
)

target_link_libraries(LangulusAnynessTest
	PRIVATE		LangulusAnyness
				Catch2
)

add_test(
	NAME		Langulus::Anyness::Test 
	COMMAND		LangulusAnynessTest
)

langulus_copy_dlls(LangulusAnynessTest ON LangulusAnynessTest)
