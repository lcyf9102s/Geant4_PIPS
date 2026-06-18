cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(G4Decay_x)

find_package(Geant4 REQUIRED ui_all vis_all)
find_package(ROOT REQUIRED)

include(${Geant4_USE_FILE})
include_directories(${ROOT_INCLUDE_DIRS})

include_directories("${PROJECT_SOURCE_DIR}/include")

file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB include_headers ${PROJECT_SOURCE_DIR}/include/*.hh)

file(GLOB MACRO_FILES 
    "*.mac"
)

file(COPY ${MACRO_FILES} DESTINATION ${PROJECT_BINARY_DIR})


add_executable(G4Decay g4decay.cc ${sources} ${include_headers})
target_link_libraries(G4Decay ${Geant4_LIBRARIES} ${ROOT_LIBRARIES})

add_custom_target(G4Decay_x DEPENDS G4Decays)
