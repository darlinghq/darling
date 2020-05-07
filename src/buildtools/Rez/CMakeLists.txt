#   Copyright 2012 Wolfgang Thaller.
#
#   This file is part of Retro68.
#
#   Retro68 is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   Retro68 is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with Retro68.  If not, see <http://www.gnu.org/licenses/>.

# Look for bison.
find_package(BISON)

find_package(Boost REQUIRED COMPONENTS wave filesystem system thread regex program_options)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/ResourceFiles
    ${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
)

add_subdirectory(ResourceFiles)

BISON_TARGET(RezParser RezParser.yy ${CMAKE_CURRENT_BINARY_DIR}/RezParser.generated.cc)

add_executable(Rez
    ${CMAKE_CURRENT_BINARY_DIR}/RezParser.generated.cc

    RezLexer.cc
    RezLexerNextToken.cc

    RezWorld.cc

    ResourceDefinitions.cc

    Expression.cc

    ResourceCompiler.cc
    Diagnostic.cc
       Rez.cc
)

target_link_libraries(Rez $<TARGET_OBJECTS:ResourceFiles> ${Boost_LIBRARIES})

