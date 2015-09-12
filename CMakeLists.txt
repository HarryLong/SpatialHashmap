project(SPATIAL-HASHMAP)
cmake_minimum_required(VERSION 2.8)

find_package(Qt5Core REQUIRED)
#find_package(PlantDB REQUIRED)

#set(LIBS ${LIBS} ${Qt5Core_LIBRARIES} )
set(INCLUDE_DIRECTORIES ${Qt5Core_INCLUDE_DIRS} )

include_directories(${INCLUDE_DIRECTORIES})

add_library(SpatialHashmap spatial_hashmap)
#target_link_libraries(SpatialHashmap ${LIBS})
# Use the Widgets module from Qt 5.

# INSTALL HEADER FILES
install(FILES spatial_hashmap.h
        DESTINATION include/SpatialHashmap)
