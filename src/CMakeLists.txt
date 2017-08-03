cmake_minimum_required(VERSION 2.8)
project(diana)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake")

add_definitions( -DUNICODE -D_UNICODE )

file(GLOB DianaCoreCxx diana_core/diana_core/*.c diana_core/diana_core/diana_patchers/*.c)
file(GLOB DianaCoreH diana_core/diana_core/*.h diana_core/diana_core/diana_patchers/*.h)
set(DianaCoreSrc ${DianaCoreCxx} ${DianaCoreH})

file(GLOB DianaProcessorCxx diana_core/diana_core/diana_processor/*.c diana_core/diana_core/diana_processor/softfloat/*.c)
file(GLOB DianaProcessorH diana_core/diana_core/diana_processor/*.h diana_core/diana_core/diana_processor/softfloat/*.h)
set(DianaProcessorSrc ${DianaProcessorCxx} ${DianaProcessorH})

source_group("Source Files\\diana" FILES ${DianaCoreCxx} ${DianaProcessorCxx})
source_group("Header Files\\diana" FILES ${DianaCoreH} ${DianaProcessorH})

include_directories(diana_core/diana_core)

add_library(diana STATIC ${DianaCoreSrc} ${DianaProcessorSrc})