project(fetion)
cmake_minimum_required(VERSION 2.6)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules)
set(TARNAME fetion)


# program name, version etc

set(PACKAGE_VERSION "2.2.2")
set(PACKAGE_NAME "fetion")
set(PACKAGE_TARNAME "${TARNAME}")
set(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")
set(PACKAGE_BUGREPORT "http://code.google.com/p/ofetion/issues/list")

find_package(PkgConfig REQUIRED)

##########################################################################
if(NOT DEFINED BIN_INSTALL_DIR)
    set(BIN_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/bin")
endif(NOT DEFINED BIN_INSTALL_DIR)
if(NOT DEFINED DATA_INSTALL_DIR)
    set(DATA_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/share")
endif(NOT DEFINED DATA_INSTALL_DIR)
if(NOT DEFINED MAN_INSTALL_DIR)
    set(MAN_INSTALL_DIR "${DATA_INSTALL_DIR}/man")
endif(NOT DEFINED MAN_INSTALL_DIR)
if(NOT DEFINED RESOURCE_INSTALL_DIR)
    set(RESOURCE_INSTALL_DIR "${DATA_INSTALL_DIR}/clifetion/resource/")
endif(NOT DEFINED RESOURCE_INSTALL_DIR)
if(NOT DEFINED LOCALE_INSTALL_DIR)
    set(LOCALE_INSTALL_DIR "${DATA_INSTALL_DIR}/locale/")
endif(NOT DEFINED LOCALE_INSTALL_DIR)

add_definitions(-DRESOURCE_DIR="${RESOURCE_INSTALL_DIR}")
pkg_check_modules(LIBXML2 REQUIRED libxml-2.0)
pkg_check_modules(OPENSSL REQUIRED openssl)
pkg_check_modules(SQLITE3 REQUIRED sqlite3)

include_directories(
    ${LIBXML2_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIRS}
    ${SQLITE3_INCLUDE_DIRS}
    ${SQLITE3_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
    ${CMAKE_CURRENT_BINARY_DIR}
)

set(fetion_SRC
    lib/fetion_buddylist.c
    lib/fetion_config.c
    lib/fetion_connection.c
    lib/fetion_contact.c
    lib/fetion_conversation.c
    lib/fetion_debug.c
    lib/fetion_directsms.c
    lib/fetion_group.c
    lib/fetion_history.c
    lib/fetion_list.c
    lib/fetion_login.c
    lib/fetion_message.c
    lib/fetion_share.c
    lib/fetion_sip.c
    lib/fetion_user.c

    src/fetion.c
)

add_executable(fetion ${fetion_SRC})
target_link_libraries(fetion ${LIBXML2_LIBRARIES} ${OPENSSL_LIBRARIES} ${SQLITE3_LIBRARIES})
install(TARGETS fetion RUNTIME DESTINATION ${BIN_INSTALL_DIR})

install(FILES fetion.1 DESTINATION ${MAN_INSTALL_DIR}/man1)
install(FILES resource/city.xml DESTINATION ${DATA_INSTALL_DIR}/openfetion/resource)
install(FILES resource/province.xml DESTINATION ${DATA_INSTALL_DIR}/openfetion/resource)
