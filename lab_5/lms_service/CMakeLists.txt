cmake_minimum_required(VERSION 3.12...3.31)
project(lms_service CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(DownloadUserver)
include(FetchContent)

find_package(
    userver
    COMPONENTS core postgresql mongo redis
    QUIET
)

if(NOT userver_FOUND)
    download_userver(TRY_DIR third_party/userver)
endif()

# jwt-cpp
FetchContent_Declare(
    jwt-cpp
    GIT_REPOSITORY https://github.com/Thalhammer/jwt-cpp.git
    GIT_TAG v0.7.2
)

FetchContent_MakeAvailable(jwt-cpp)

userver_setup_environment()

include_directories(src)

add_library(
    ${PROJECT_NAME}_objs OBJECT
    src/auth/jwt_auth_checker.cpp
    src/storage/in_memory_storage.cpp
    src/storage/postgres_storage.cpp
    src/storage/mongo_storage.cpp
    src/auth/jwt_auth_factory.cpp
    src/views/auth/register_handler.cpp
    src/views/auth/login_handler.cpp
    src/views/users/get_user_by_login_handler.cpp
    src/views/users/search_users_handler.cpp
    src/views/courses/create_course_handler.cpp
    src/views/courses/list_courses_handler.cpp
    src/views/lessons/add_lesson_handler.cpp
    src/views/lessons/list_lessons_handler.cpp
    src/views/enrollments/enroll_user_handler.cpp
    src/views/enrollments/get_user_courses_handler.cpp
    src/views/progress/mark_lesson_completed_handler.cpp
    src/components/db_cleaner.cpp    
)

target_link_libraries(
    ${PROJECT_NAME}_objs
    PUBLIC
    userver::core
    userver::postgresql
    userver::mongo
    userver::redis
    jwt-cpp
)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_objs)

userver_testsuite_add_simple()

include(GNUInstallDirs)

if(DEFINED ENV{PREFIX})
    message(STATUS "Set install prefix: $ENV{PREFIX}")
    file(TO_CMAKE_PATH "$ENV{PREFIX}" PREFIX_PATH)
    set(CMAKE_INSTALL_PREFIX "${PREFIX_PATH}")
endif()

file(
    GLOB CONFIGS_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/configs/*.yaml
    ${CMAKE_CURRENT_SOURCE_DIR}/configs/*.json
)

install(
    TARGETS ${PROJECT_NAME}
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT ${PROJECT_NAME}
)

install(
    FILES ${CONFIGS_FILES}
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/${PROJECT_NAME}
    COMPONENT ${PROJECT_NAME}
)