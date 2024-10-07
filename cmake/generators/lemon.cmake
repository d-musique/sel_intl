# The SEL extension library
# Free software published under the MIT license.

find_program(LEMON_PROGRAM "lemon" REQUIRED)

function(generate_lemon input suffix)
  if(NOT suffix)
    message(FATAL_ERROR "No suffix given")
  endif()
  if("${input}" IS_NEWER_THAN "${input}${suffix}")
    message(STATUS "Generating: ${input}${suffix}")
    string(TIMESTAMP uniqueRunID "%s%f")
    get_filename_component(Base "${input}" NAME_WLE)
    file(MAKE_DIRECTORY "lemon.${uniqueRunID}")
    execute_process(
      COMMAND "${LEMON_PROGRAM}" "-dlemon.${uniqueRunID}" "-q" "-l" "${input}"
      RESULT_VARIABLE Result)
    if(NOT Result EQUAL 0)
      file(REMOVE_RECURSE "lemon.${uniqueRunID}")
      message(FATAL_ERROR "Command has failed")
    endif()
    file(COPY_FILE "lemon.${uniqueRunID}/${Base}.c" "${input}${suffix}")
    file(REMOVE_RECURSE "lemon.${uniqueRunID}")
  else()
    message(STATUS "Up-to-date: ${input}${suffix}")
  endif()
endfunction()
