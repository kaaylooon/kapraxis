cmake_minimum_required(VERSION 3.16)
project(study_app)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# NÃO use CMAKE_AUTORCC por enquanto
# set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql)

# SIMPLES - sem qt_ functions
add_executable(study_app
    src/main.cpp
    src/app/AppWindow.cpp
    src/ui/QuestoesPage.cpp
    src/ui/InicioPage.cpp
    src/repo/QuestaoRepoSQLite.cpp

)

target_include_directories(study_app PRIVATE src)

target_link_libraries(study_app PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Sql
)