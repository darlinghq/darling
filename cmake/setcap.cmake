include(FindSetcap)

function(setcap file caps)
install(CODE "execute_process(
		COMMAND
				${SETCAP_EXECUTABLE}
				cap_sys_rawio+ep
				${CMAKE_INSTALL_PREFIX}/${file}
		RESULT_VARIABLE
				_SETCAP_RESULT
		)
		if( _SETCAP_RESULT )
				message( WARNING \"setcap failed (${_SETCAP_RESULT}).\")
		endif()"
)

endfunction(setcap)

