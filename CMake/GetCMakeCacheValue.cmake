function( GetCMakeCacheValue BINARY_PATH )

  file( READ "${BINARY_PATH}/CMakeCache.txt" CMAKE_CACHE_CONTENTS )

  foreach( VAR ${ARGN} )
    string( FIND "${CMAKE_CACHE_CONTENTS}" "${VAR}:" start )
    if ( NOT ${start} EQUAL -1 )
      string( SUBSTRING "${CMAKE_CACHE_CONTENTS}" ${start} -1 contents_tail )
      string( FIND "${contents_tail}" ":" type_start_pos )
      string( FIND "${contents_tail}" "=" type_end_pos )
      math( EXPR type_start_pos "${type_start_pos} + 1" )
      math( EXPR type_length "${type_end_pos} - ${type_start_pos}" )
      string( SUBSTRING "${contents_tail}" ${type_start_pos} ${type_length} var_type )

      string( FIND "${contents_tail}" "=" val_start_pos )
      string( FIND "${contents_tail}" "\n" val_end_pos )
      math( EXPR val_start_pos "${val_start_pos} + 1" )
      math( EXPR val_length "${val_end_pos} - ${val_start_pos}" )
      string( SUBSTRING "${contents_tail}" ${val_start_pos} ${val_length} var_value )

      set( ${VAR} "${var_value}" PARENT_SCOPE )
      set( ${VAR}_TYPE "${var_type}" PARENT_SCOPE )
    else()
      set( ${VAR} "${VAR}-NOT_FOUND" PARENT_SCOPE )
      set( ${VAR}_TYPE "${VAR}_TYPE-NOT_FOUND" PARENT_SCOPE )
    endif()
  endforeach()

endfunction()
