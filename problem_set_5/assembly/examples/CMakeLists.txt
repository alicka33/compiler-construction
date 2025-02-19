cmake_minimum_required(VERSION 3.0)
project(AssemblyExamples)

enable_language(ASM)

file(GLOB ASSEMBLY_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.S")

foreach(ASSEMBLY_SOURCE ${ASSEMBLY_SOURCES})
    get_filename_component(ASSEMBLY_TARGET ${ASSEMBLY_SOURCE} NAME_WE)
    add_executable(${ASSEMBLY_TARGET} ${ASSEMBLY_SOURCE})
endforeach()
