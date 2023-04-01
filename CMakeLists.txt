cmake_minimum_required(VERSION 3.13)
project(compiler)


if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    message("Mac OS Build")



    # settings
    # set to OFF to enable C mode
    set(CPP_MODE ON)
    if (CPP_MODE)
        set(FB_EXT ".cpp")
    else ()
        set(FB_EXT ".c")
    endif ()
    message(STATUS "Flex/Bison generated source file extension: ${FB_EXT}")

    set(CMAKE_CXX_STANDARD 17)


    message(STATUS "Flex/Bison generated source file extension: ${FB_EXT}")

    add_compile_options(-Wall -Wno-register)


    # options about libraries and includes
    set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)
    set(INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
    message(STATUS "Library directory: ${LIB_DIR}")
    message(STATUS "Include directory: ${INC_DIR}")


    # On macOS, search Homebrew for keg-only versions of Bison and Flex. Xcode does
    # not provide new enough versions for us to use.
    if (CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
        execute_process(
                COMMAND brew --prefix bison
                RESULT_VARIABLE BREW_BISON
                OUTPUT_VARIABLE BREW_BISON_PREFIX
                OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if (BREW_BISON EQUAL 0 AND EXISTS "${BREW_BISON_PREFIX}")
            message(STATUS "Found Bison keg installed by Homebrew at ${BREW_BISON_PREFIX}")
            set(BISON_EXECUTABLE "${BREW_BISON_PREFIX}/bin/bison")
        endif ()

        execute_process(
                COMMAND brew --prefix flex
                RESULT_VARIABLE BREW_FLEX
                OUTPUT_VARIABLE BREW_FLEX_PREFIX
                OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if (BREW_FLEX EQUAL 0 AND EXISTS "${BREW_FLEX_PREFIX}")
            message(STATUS "Found Flex keg installed by Homebrew at ${BREW_FLEX_PREFIX}")
            set(FLEX_EXECUTABLE "${BREW_FLEX_PREFIX}/bin/flex")
        endif ()
    endif ()

    # find Flex/Bison
    find_package(FLEX REQUIRED)
    find_package(BISON 3.5 REQUIRED)

    # generate lexer/parser
    file(GLOB_RECURSE L_SOURCES "src/*.l")
    file(GLOB_RECURSE Y_SOURCES "src/*.y")
    #message(STATUS "${L_SOURCES}")
    if (NOT (L_SOURCES STREQUAL "" AND Y_SOURCES STREQUAL ""))
        string(REGEX REPLACE ".*/(.*)\\.l" "${CMAKE_CURRENT_BINARY_DIR}/\\1.lex${FB_EXT}" L_OUTPUTS "${L_SOURCES}")
        string(REGEX REPLACE ".*/(.*)\\.y" "${CMAKE_CURRENT_BINARY_DIR}/\\1.tab${FB_EXT}" Y_OUTPUTS "${Y_SOURCES}")
        flex_target(Lexer ${L_SOURCES} ${L_OUTPUTS})
        bison_target(Parser ${Y_SOURCES} ${Y_OUTPUTS} VERBOSE)
        add_flex_bison_dependency(Lexer Parser)
    endif ()

    message(${LIB_DIR})
    # project link directories
    link_directories(${LIB_DIR})

    # project include directories
    include_directories(src)
    include_directories(${CMAKE_CURRENT_BINARY_DIR})
    include_directories(${INC_DIR})

    # all of C/C++ source files
    file(GLOB_RECURSE CXX_SOURCES "src/*.h")
    file(GLOB_RECURSE CXX_SOURCES "src/*.cpp")

    # SET(CMAKE_BUILD_TYPE Debug)
    # SET(CMAKE_C_FLAGS_DEBUG "-D_DEBUG")

    set(SOURCES ${CXX_SOURCES} ${FLEX_Lexer_OUTPUTS} ${BISON_Parser_OUTPUT_SOURCE})


    # executable
    add_executable(compiler ${SOURCES})
    set_target_properties(compiler PROPERTIES C_STANDARD 11 CXX_STANDARD 17)
    target_link_libraries(compiler koopa pthread dl)


endif ()


if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    message("Linux Build")

    # settings
    # set to OFF to enable C mode
    set(CPP_MODE ON)
    if (CPP_MODE)
        set(FB_EXT ".cpp")
    else ()
        set(FB_EXT ".c")
    endif ()
    message(STATUS "Flex/Bison generated source file extension: ${FB_EXT}")

    # enable all warnings
    if (MSVC)
        add_compile_options(/W3)
    else ()
        # disable warnings caused by old version of Flex
        add_compile_options(-Wall -Wno-register)
    endif ()

    # options about libraries and includes
    set(LIB_DIR "$ENV{CDE_LIBRARY_PATH}/native" CACHE STRING "directory of libraries")
    set(INC_DIR "$ENV{CDE_INCLUDE_PATH}" CACHE STRING "directory of includes")
    message(STATUS "Library directory: ${LIB_DIR}")
    message(STATUS "Include directory: ${INC_DIR}")

    # find Flex/Bison
    find_package(FLEX REQUIRED)
    find_package(BISON REQUIRED)

    # generate lexer/parser
    file(GLOB_RECURSE L_SOURCES "src/*.l")
    file(GLOB_RECURSE Y_SOURCES "src/*.y")
    if (NOT (L_SOURCES STREQUAL "" AND Y_SOURCES STREQUAL ""))
        string(REGEX REPLACE ".*/(.*)\\.l" "${CMAKE_CURRENT_BINARY_DIR}/\\1.lex${FB_EXT}" L_OUTPUTS "${L_SOURCES}")
        string(REGEX REPLACE ".*/(.*)\\.y" "${CMAKE_CURRENT_BINARY_DIR}/\\1.tab${FB_EXT}" Y_OUTPUTS "${Y_SOURCES}")
        flex_target(Lexer ${L_SOURCES} ${L_OUTPUTS})
        bison_target(Parser ${Y_SOURCES} ${Y_OUTPUTS})
        add_flex_bison_dependency(Lexer Parser)
    endif ()

    # project link directories
    link_directories(${LIB_DIR})

    # project include directories
    include_directories(src)
    include_directories(${CMAKE_CURRENT_BINARY_DIR})
    include_directories(${INC_DIR})

    # all of C/C++ source files
    file(GLOB_RECURSE C_SOURCES "src/*.c")
    file(GLOB_RECURSE CXX_SOURCES "src/*.cpp")
    file(GLOB_RECURSE CC_SOURCES "src/*.cc")
    set(SOURCES ${C_SOURCES} ${CXX_SOURCES} ${CC_SOURCES}
            ${FLEX_Lexer_OUTPUTS} ${BISON_Parser_OUTPUT_SOURCE})

    # executable
    add_executable(compiler ${SOURCES})
    set_target_properties(compiler PROPERTIES C_STANDARD 11 CXX_STANDARD 17)
    target_link_libraries(compiler koopa pthread dl)

endif ()