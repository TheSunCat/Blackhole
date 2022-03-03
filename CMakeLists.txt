cmake_minimum_required(VERSION 3.22)
project(Blackhole)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
# Instruct CMake to create code from Qt designer ui files
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6Widgets CONFIG REQUIRED)
find_package(Qt6 COMPONENTS OpenGL REQUIRED)
find_package(Qt6 COMPONENTS OpenGLWidgets REQUIRED)
find_package(Qt6 COMPONENTS Core5Compat REQUIRED)

add_subdirectory(glm)

set(blackhole_SRC
  include/ui/Blackhole.h
  include/ui/AboutForm.h
  include/ui/GalaxyEditorForm.h

  src/main.cpp
  src/Util.cpp

  src/ui/Blackhole.cpp
  src/ui/AboutForm.cpp
  src/ui/GalaxyEditorForm.cpp

  src/rendering/GalaxyRenderer.cpp
  src/rendering/Camera.cpp
  src/rendering/GX.cpp
  src/rendering/Texture.cpp
  src/rendering/ObjectRenderer.cpp
  src/rendering/Material.cpp

  src/io/BaseFile.cpp
  src/io/ExternalFile.cpp
  src/io/MemoryFile.cpp
  src/io/RarcFile.cpp
  src/io/InRarcFile.cpp
  src/io/BcsvFile.cpp
  src/io/Yaz0File.cpp
  src/io/BmdFile.cpp

  src/smg/Galaxy.cpp
  src/smg/Zone.cpp

  src/smg/BaseObject.cpp
  src/smg/ZoneObject.cpp
  src/smg/MapPartObject.cpp
  src/smg/ChildObject.cpp
  src/smg/LevelObject.cpp
  src/smg/StartObject.cpp
  src/smg/GravityObject.cpp
  src/smg/SoundObject.cpp
  src/smg/AreaObject.cpp
  src/smg/CameraObject.cpp
  src/smg/CutsceneObject.cpp
  src/smg/PositionObject.cpp
  src/smg/DebugObject.cpp
  src/smg/ChangeObject.cpp
)

add_executable(blackhole ${blackhole_SRC})
target_link_libraries(blackhole Qt6::Widgets Qt6::OpenGL Qt6::OpenGLWidgets Qt6::Core5Compat Threads::Threads)

target_include_directories(blackhole PRIVATE include libs glm)
set_property(TARGET blackhole PROPERTY CXX_STANDARD 20)

# Install the executable
install(TARGETS blackhole DESTINATION bin)
