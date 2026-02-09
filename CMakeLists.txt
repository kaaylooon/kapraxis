cmake_minimum_required(VERSION 3.16)
project(kapraxis)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# NÃO use CMAKE_AUTORCC por enquanto
# set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql)

# SIMPLES - sem qt_ functions
add_executable(kapraxis
    src/main.cpp
    src/app/AppWindow.cpp
    src/app/AppWindow.h
    src/ui/QuestoesPage.cpp
    src/ui/QuestoesPage.h
    src/ui/InicioPage.cpp
    src/ui/InicioPage.h
    src/repo/QuestaoRepoSQLite.cpp
    src/repo/QuestaoRepoSQLite.h


)

target_include_directories(kapraxis PRIVATE src)

target_link_libraries(kapraxis PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Sql
)