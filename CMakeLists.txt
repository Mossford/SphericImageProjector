cmake_minimum_required(VERSION 3.16)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# prevent installing to system directories.
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE INTERNAL "")

set(CMAKE_BUILD_TYPE "Release")

# Declare the project
project(SphericImageProjector)

if (EMSCRIPTEN)
    set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")    # Disable shared builds on platforms where it does not make sense to use them
    set(SDL_SHARED OFF)
    set(BUILD_SHARED_LIBS OFF)
else()
    set(SDL_SHARED ON)
    set(BUILD_SHARED_LIBS ON)
endif()

if(MSVC)
    if(NOT CMAKE_GENERATOR STREQUAL "Ninja")
        add_definitions(/MP)				# parallelize each target, unless Ninja is the generator
    endif()
endif()

# Set the name of the executable
set(EXECUTABLE_NAME ${PROJECT_NAME})

add_executable(${EXECUTABLE_NAME})

# Add your sources to the target
target_sources(${EXECUTABLE_NAME}
PRIVATE
    ${CMAKE_SOURCE_DIR}/src/Core/main.cpp
    ${CMAKE_SOURCE_DIR}/src/Core/mesh.cpp
    ${CMAKE_SOURCE_DIR}/src/Core/shader.cpp
    ${CMAKE_SOURCE_DIR}/src/Core/camera.cpp
    ${CMAKE_SOURCE_DIR}/src/Core/math.cpp
    ${CMAKE_SOURCE_DIR}/src/Core/pipeline.cpp
    ${CMAKE_SOURCE_DIR}/src/Core/texture.cpp
    ${CMAKE_SOURCE_DIR}/src/Core/imguiUi.cpp
    ${CMAKE_SOURCE_DIR}/src/Core/line.cpp
    ${CMAKE_SOURCE_DIR}/src/Core/lineRenderer.cpp

    ${CMAKE_SOURCE_DIR}/src/SIP/sipManager.cpp
    ${CMAKE_SOURCE_DIR}/src/SIP/sipImage.cpp
    ${CMAKE_SOURCE_DIR}/src/SIP/sipCamera.cpp
)

# Set C++ version
target_compile_features(${EXECUTABLE_NAME} PUBLIC cxx_std_17)

# on Web targets, we need CMake to generate a HTML webpage.
if(EMSCRIPTEN)
	set(CMAKE_EXECUTABLE_SUFFIX ".html" CACHE INTERNAL "")
endif()

set(GLM_BUILD_TESTS OFF)
add_subdirectory(glm EXCLUDE_FROM_ALL)

set(GLSLANG_TESTS_DEFAULT OFF)
set(GLSLANG_ENABLE_INSTALL_DEFAULT OFF)
add_subdirectory(glslang EXCLUDE_FROM_ALL)

# Configure SDL by calling its CMake file.
# we use EXCLUDE_FROM_ALL so that its install targets and configs don't
# pollute upwards into our configuration.
add_subdirectory(SDL EXCLUDE_FROM_ALL)

# If you don't want SDL_ttf, then remove this section.
set(SDLTTF_VENDORED ON) # tell SDL_ttf to build its own dependencies
add_subdirectory(SDL_ttf EXCLUDE_FROM_ALL)

# SDL_image (used for loading various image formats)
set(SDLIMAGE_VENDORED ON)
set(SDLIMAGE_AVIF OFF)	# disable formats we don't use to make the build faster and smaller.
set(SDLIMAGE_BMP OFF)
set(SDLIMAGE_JPEG OFF)
set(SDLIMAGE_WEBP OFF)
add_subdirectory(SDL_image EXCLUDE_FROM_ALL)

add_library(Libs STATIC
    ${CMAKE_SOURCE_DIR}/imgui/imgui.cpp
    ${CMAKE_SOURCE_DIR}/imgui/imgui_demo.cpp
    ${CMAKE_SOURCE_DIR}/imgui/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/imgui/imgui_tables.cpp
    ${CMAKE_SOURCE_DIR}/imgui/imgui_widgets.cpp
    ${CMAKE_SOURCE_DIR}/imgui/backends/imgui_impl_sdl3.cpp
    ${CMAKE_SOURCE_DIR}/imgui/backends/imgui_impl_sdlgpu3.cpp
)

target_include_directories(Libs PUBLIC
    ${CMAKE_SOURCE_DIR}/src/includes/Core
    ${CMAKE_SOURCE_DIR}/src/includes/SIP
    ${CMAKE_SOURCE_DIR}/imgui
    $<TARGET_PROPERTY:SDL3::SDL3,INTERFACE_INCLUDE_DIRECTORIES>
)

# Link SDL to our executable. This also makes its include directory available to us.
target_link_libraries(${EXECUTABLE_NAME} PUBLIC
    glm::glm
    glslang::glslang
    glslang::SPIRV
    glslang::glslang-default-resource-limits
    Libs
    SDL3_ttf::SDL3_ttf      # remove if you are not using SDL_ttf
	SDL3_image::SDL3_image	# remove if you are not using SDL_image
    SDL3::SDL3              # If using satelite libraries, SDL must be the last item in the list.
)

target_compile_definitions(${EXECUTABLE_NAME} PUBLIC SDL_MAIN_USE_CALLBACKS)

# Dealing with assets
# We have some non-code resources that our application needs in order to work. How we deal with those differs per platform.
if(EMSCRIPTEN)
    # on the web, we have to put the files inside of the webassembly
    # somewhat unintuitively, this is done via a linker argument.
    target_link_libraries(${EXECUTABLE_NAME} PRIVATE
        "--preload-file \"${CMAKE_CURRENT_LIST_DIR}/src/Res/Inter-VariableFont.ttf@/\""
        "--preload-file \"${CMAKE_CURRENT_LIST_DIR}/src/Res/M51.png@/\""
        "--preload-file \"${CMAKE_CURRENT_LIST_DIR}/src/Res/uvCheck.jpg@/\""
        "--preload-file \"${CMAKE_CURRENT_LIST_DIR}/src/Res/M101.jpg@/\""

        "--preload-file \"${CMAKE_CURRENT_LIST_DIR}/src/Res/default.frag@/\""
        "--preload-file \"${CMAKE_CURRENT_LIST_DIR}/src/Res/default.vert@/\""
        "--preload-file \"${CMAKE_CURRENT_LIST_DIR}/src/Res/SIPImage.frag@/\""
        "--preload-file \"${CMAKE_CURRENT_LIST_DIR}/src/Res/SIPImage.vert@/\""
        "--preload-file \"${CMAKE_CURRENT_LIST_DIR}/src/Res/line.frag@/\""
        "--preload-file \"${CMAKE_CURRENT_LIST_DIR}/src/Res/line.vert@/\""
    )
else()

    macro(copy_helper filename)
        set(outname "${CMAKE_BINARY_DIR}/$<CONFIGURATION>/${filename}")
        add_custom_command(POST_BUILD
            TARGET "${EXECUTABLE_NAME}"
		    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_LIST_DIR}/src/Res/${filename}" "${outname}"
		    DEPENDS "${filename}"
	    )
    endmacro()

    copy_helper("Inter-VariableFont.ttf")
    copy_helper("M101.jpg")
    copy_helper("M51.png")
    copy_helper("uvCheck.jpg")

    copy_helper("default.frag")
    copy_helper("default.vert")
    copy_helper("SIPImage.frag")
    copy_helper("SIPImage.vert")
    copy_helper("line.frag")
    copy_helper("line.vert")
    copy_helper("SIPCamera.frag")
    copy_helper("SIPCamera.vert")

endif()

# set some extra configs for each platform
set(RESOURCE_FILES "src/logo.png")

set_target_properties(${EXECUTABLE_NAME} PROPERTIES
    RESOURCE "${RESOURCE_FILES}"
)

# on Visual Studio, set our app as the default project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "${EXECUTABLE_NAME}")
