cmake_minimum_required(VERSION 3.22)
project(Blackhole)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
# Instruct CMake to create code from Qt designer ui files
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5Widgets CONFIG REQUIRED)

set(blackhole_SRC
  include/ui/Blackhole.h
  include/ui/AboutForm.h
  include/ui/GalaxyEditorForm.h

  src/main.cpp
  src/Util.cpp

  src/ui/Blackhole.cpp
  src/ui/AboutForm.cpp
  src/ui/GalaxyEditorForm.cpp
  src/ui/GalaxyRenderer.cpp

  src/filetypes/Yaz0.cpp

  src/io/BaseFile.cpp
  src/io/ExternalFile.cpp
  src/io/MemoryFile.cpp
  src/io/RarcFile.cpp
  src/io/InRarcFile.cpp
  src/io/BcsvFile.cpp
  src/io/Yaz0File.cpp

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
target_link_libraries(blackhole Qt5::Widgets)

target_include_directories(blackhole PRIVATE include libs)
set_property(TARGET blackhole PROPERTY CXX_STANDARD 20)

# Install the executable
install(TARGETS blackhole DESTINATION bin)
