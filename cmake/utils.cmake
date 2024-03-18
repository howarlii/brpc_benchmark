# ~~~
# get all targets under directory (including interface targets)
# _dir is the input directory
# the target list is output to _result
function(_get_all_targets _result _dir)
  message(DEBUG "_get_all_targets DIRECTORY ${_dir}")
  get_property(
    _subdirs
    DIRECTORY "${_dir}"
    PROPERTY SUBDIRECTORIES
  )
  foreach(_subdir IN LISTS _subdirs)
    _get_all_targets(${_result} "${_subdir}")
  endforeach()

  get_directory_property(_sub_targets DIRECTORY "${_dir}" BUILDSYSTEM_TARGETS)
  set(${_result}
      ${${_result}} ${_sub_targets}
      PARENT_SCOPE
  )
endfunction()

# ~~~
# get all non-interface targets under directory
# _dir is the input directory
# the target list is output to _result
function(get_all_targets _result _dir)
  _get_all_targets(_tmp_result ${_dir})
  foreach(_target IN LISTS _tmp_result)
    get_target_property(_type ${_target} TYPE)
    if(NOT
       ${_type}
       STREQUAL
       "INTERFACE_LIBRARY"
    )
      set(${_result} ${${_result}} ${_target})
    endif()
  endforeach()
  # set to the scope of the caller function
  set(${_result}
      ${${_result}}
      PARENT_SCOPE
  )
endfunction()
