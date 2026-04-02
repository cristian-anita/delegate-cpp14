cmake_minimum_required(VERSION 3.21) # for Visual Studio 17 2022

project(Delegate
	DESCRIPTION "A non-owning, const-correct and equality comparable callable wrapper."
	HOMEPAGE_URL "https://github.com/cristian-anita/delegate-cpp14"
	LANGUAGES CXX
)


# for clang-cl use ClangCL toolset; ex: cmake -G "Visual Studio 17 2022" -T ClangCL...
message(STATUS "CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "CMAKE_CXX_SIMULATE_ID: ${CMAKE_CXX_SIMULATE_ID}")
message(STATUS "CMAKE_CXX_COMPILER_FRONTEND_VARIANT: ${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}")


# /Zc:__cplusplus; please see:
# - MSVC now correctly reports __cplusplus: https://devblogs.microsoft.com/cppblog/msvc-now-correctly-reports-__cplusplus
# - /Zc:__cplusplus (Enable updated __cplusplus macro): https://learn.microsoft.com/en-us/cpp/build/reference/zc-cplusplus
if(MSVC)
	if(MSVC_VERSION LESS 1914)
		message(FATAL_ERROR "msvc >= 2017 v15.7 required!!! (for /Zc:__cplusplus)")
	endif()
	
	add_compile_options(/Zc:__cplusplus)
endif()

# C++ std
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# warning levels
if(MSVC)
	# cl Warning level: https://learn.microsoft.com/en-us/cpp/build/reference/compiler-option-warning-level
	# clang-cl Command-Line Options: https://clang.llvm.org/docs/UsersManual.html#id13
	add_compile_options(/Wall)
else()
	# gcc Options to Request or Suppress Warnings: https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html
	add_compile_options(-Wall -Wextra)
endif()

add_compile_definitions("$<$<NOT:$<CONFIG:Debug>>:NDEBUG>")

# disable warnings
if(MSVC)
	# C4710: 'function' : function not inlined
	# C4711: function 'function' selected for inline expansion
	add_compile_options(/wd4710 /wd4711)
endif()
if ((CMAKE_CXX_COMPILER_ID STREQUAL "Clang") AND (CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC"))
	add_compile_options(-Wno-c++98-compat)
endif()


add_executable(delegate)

target_sources(delegate
	PRIVATE
		main.cpp
		delegate.hpp
)
