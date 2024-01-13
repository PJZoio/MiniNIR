cmake_minimum_required(VERSION 3.16)

project(spec-res VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#find_package(Qt6 REQUIRED COMPONENTS Core)

add_executable(spec-res spec-res.cpp)

target_link_libraries(spec-res ${CMAKE_SOURCE_DIR}/libsarspec.a ftd2xx)

#SET(CPACK_GENERATOR "TGZ;DEB")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Sarspec spec res+ Linux app")
SET(CPACK_PACKAGE_VENDOR "IPFN-IST")
SET(CPACK_PACKAGE_CONTACT "Bernardo Brotas Carvalho <bernardo.carvalho@tecnico.ulisboa.pt>")
SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "0")
SET(CPACK_PACKAGE_VERSION_PATCH "1")

#INCLUDE(CPack)

