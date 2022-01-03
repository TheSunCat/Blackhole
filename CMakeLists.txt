cmake_minimum_required(VERSION 3.22)
project(Blackhole)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
# Instruct CMake to create code from Qt designer ui files
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find the QtWidgets library
find_package(Qt5Widgets CONFIG REQUIRED)

set(blackhole_SRC
  include/ui/Blackhole.h
  include/ui/AboutForm.h
  include/ui/GalaxyEditorForm.h

  src/main.cpp
  src/ui/Blackhole.cpp
  src/ui/AboutForm.cpp
  src/ui/GalaxyEditorForm.cpp
  src/filetypes/Yaz0.cpp
  src/io/FileBase.cpp
  src/io/ExternalFile.cpp
  src/io/MemoryFile.cpp
  src/io/RarcFile.cpp
  src/io/BcsvFile.cpp
  src/io/Yaz0File.cpp
  src/smg/Galaxy.cpp
)

# Tell CMake to create the helloworld executable
add_executable(blackhole ${blackhole_SRC})

# Use the Widgets module from Qt 5.
target_link_libraries(blackhole Qt5::Widgets)

# Add include directory
target_include_directories(blackhole PRIVATE include)

# Install the executable
install(TARGETS blackhole DESTINATION bin)
