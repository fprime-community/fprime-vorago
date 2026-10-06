set(TLM_AC_SCRIPT_PATH "${CMAKE_CURRENT_LIST_DIR}/../scripts/tlm_autocoder.py")

function(telemetry_autocoder_add_global_target TARGET)
endfunction()

function(telemetry_autocoder_add_deployment_target MODULE TARGET SOURCES DEPENDENCIES FULL_DEPENDENCIES)
    # Get the deployment name from the current directory, this is different
    # from MODULE which includes a "Deployments_" prefix
    get_filename_component(DEPLOYMENT "${CMAKE_CURRENT_LIST_DIR}" NAME)
    set(DICT_PATH "${CMAKE_CURRENT_BINARY_DIR}/${DEPLOYMENT}TopologyDictionary.json")
    set(TLM_OUTPUT_CPP "${CMAKE_CURRENT_BINARY_DIR}/${DEPLOYMENT}TlmAc.cpp")

    add_custom_command(
        OUTPUT "${TLM_OUTPUT_CPP}"
        COMMAND "${PYTHON}" "${TLM_AC_SCRIPT_PATH}" "${DEPLOYMENT}" "${DICT_PATH}" "${TLM_OUTPUT_CPP}"
        DEPENDS "${DICT_PATH}"
        VERBATIM
    )
    target_sources("${MODULE}" PRIVATE "${TLM_OUTPUT_CPP}")
endfunction()

function(telemetry_autocoder_add_module_target MODULE_NAME TARGET_NAME SOURCE_FILES DEPENDENCIES)
endfunction()
