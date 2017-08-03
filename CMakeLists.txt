cmake_minimum_required(VERSION 2.8)
project(diana)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake")

add_definitions( -DUNICODE -D_UNICODE )

file(GLOB DianaCoreCxx src/diana_core/diana_core/*.c src/diana_core/diana_core/diana_patchers/*.c)
file(GLOB DianaCoreHxx src/diana_core/diana_core/*.h src/diana_core/diana_core/diana_patchers/*.h)
set(DianaCoreSrc ${DianaCoreCxx} ${DianaCoreHxx})

file(GLOB DianaProcessorCxx src/diana_core/diana_core/diana_processor/*.c src/diana_core/diana_core/diana_processor/softfloat/*.c)
file(GLOB DianaProcessorHxx src/diana_core/diana_core/diana_processor/*.h src/diana_core/diana_core/diana_processor/softfloat/*.h)
set(DianaProcessorSrc ${DianaProcessorCxx} ${DianaProcessorHxx})

include_directories(src/diana_core/diana_core)

add_library(diana STATIC ${DianaCoreSrc} ${DianaProcessorSrc})