 # This Source Code Form is subject to the terms of the Mozilla Public
 # License, v. 2.0. If a copy of the MPL was not distributed with this
 # file, You can obtain one at https://mozilla.org/MPL/2.0/.
 #
 # Copyright 2021 Dominik Meyer <dmeyer@federationhq.de>
 # This file is part of the EventManager distribution hosted at https://gitea.federationhq.de/byterazor/EventManager.git


find_program(COMPDB_PATH
      NAME compdb
      PATHS ~/.local/bin/
            /bin
            /sbin
            /usr/bin
            /usr/sbin
            /usr/local/bin
            /usr/local/sbin
  )



if (COMPDB_PATH)
  IF(NOT TARGET COMPD)
    add_custom_target(COMPD
      ALL
      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      COMMAND ${COMPDB_PATH} -p ${CMAKE_CURRENT_BINARY_DIR} list >compile_commands.json
    )
  endif()
endif()
