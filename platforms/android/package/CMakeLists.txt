if(NOT ANDROID_PACKAGE_RELEASE)
  set(ANDROID_PACKAGE_RELEASE 1)
endif()

if(NOT ANDROID_PACKAGE_PLATFORM)
  if(ARMEABI_V7A)
    if(NEON)
      set(ANDROID_PACKAGE_PLATFORM armv7a_neon)
    else()
      set(ANDROID_PACKAGE_PLATFORM armv7a)
    endif()
  elseif(ARMEABI_V6)
    set(ANDROID_PACKAGE_PLATFORM armv6)
  elseif(ARMEABI)
    set(ANDROID_PACKAGE_PLATFORM armv5)
  elseif(X86)
    set(ANDROID_PACKAGE_PLATFORM x86)
  elseif(MIPS)
    set(ANDROID_PACKAGE_PLATFORM mips)
  else()
    message(ERROR "Can not automatically determine the value for ANDROID_PACKAGE_PLATFORM")
  endif()
endif()

if(NOT ANDROID_PACKAGE_PLATFORM_NAME)
  if(ARMEABI_V7A)
    if(NEON)
      set(ANDROID_PACKAGE_PLATFORM_NAME "armeabi-v7a with NEON")
    else()
      set(ANDROID_PACKAGE_PLATFORM_NAME "armeabi-v7a")
    endif()
  elseif(ARMEABI_V6)
    set(ANDROID_PACKAGE_PLATFORM_NAME "armeabi-v6")
  elseif(ARMEABI)
    set(ANDROID_PACKAGE_PLATFORM_NAME "armeabi")
  elseif(X86)
    set(ANDROID_PACKAGE_PLATFORM_NAME "x86")
  elseif(MIPS)
    set(ANDROID_PACKAGE_PLATFORM_NAME "mips")
  else()
    message(ERROR "Can not automatically determine the value for ANDROID_PACKAGE_PLATFORM_NAME")
  endif()
endif()

if("${ANDROID_NATIVE_API_LEVEL}" MATCHES "[1-9][0-9]*$")
  set(ANDROID_SDK_VERSION ${CMAKE_MATCH_0})
endif()

if(NOT ANDROID_SDK_VERSION GREATER 7)
  set(ANDROID_SDK_VERSION 8)
endif()

set(PACKAGE_DIR "${CMAKE_BINARY_DIR}/package")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/AndroidManifest.xml" 
    "${PACKAGE_DIR}/AndroidManifest.xml"     @ONLY)
#configure_file("${CMAKE_CURRENT_SOURCE_DIR}/build.xml" 
#    "${PACKAGE_DIR}/build.xml"     COPYONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/res/values/strings.xml"   
    "${PACKAGE_DIR}/res/values/strings.xml"       COPYONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/res/drawable-hdpi/icon.png"   
    "${PACKAGE_DIR}/res/drawable-hdpi/icon.png"   COPYONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/res/drawable-ldpi/icon.png"   
    "${PACKAGE_DIR}/res/drawable-ldpi/icon.png"   COPYONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/res/drawable-mdpi/icon.png"   
    "${PACKAGE_DIR}/res/drawable-mdpi/icon.png"   COPYONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/res/layout/main.xml"   
    "${PACKAGE_DIR}/res/layout/main.xml"   COPYONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/assets/_keystore/wagic-release.keystore"   
    "${PACKAGE_DIR}/assets/_keystore/wagic-release.keystore"   COPYONLY)
    
string( REGEX MATCH "[0-9]+" ANDROID_NATIVE_API_LEVEL_NUMBER "${ANDROID_NATIVE_API_LEVEL}" )

set(target_name "Wagic_${WAGIC_VERSION}_binary_pack_${ANDROID_PACKAGE_PLATFORM}")
get_target_property(wagic_location wagic LOCATION)

string(TOLOWER ${CMAKE_BUILD_TYPE} android_build_type)

set(android_proj_target_files ${ANDROID_PROJECT_FILES})
list_add_prefix(android_proj_target_files "${PACKAGE_DIR}/")
set(APK_NAME "${PACKAGE_DIR}/bin/${target_name}-${android_build_type}-unsigned.apk")

file(GLOB camera_wrappers "${OpenCV_SOURCE_DIR}/3rdparty/lib/${ANDROID_NDK_ABI_NAME}/libnative_camera_r*.so")
set(CAMERA_LIB_COMMANDS "")

foreach(wrapper ${camera_wrappers})
  list(APPEND CAMERA_LIB_COMMANDS COMMAND ${CMAKE_COMMAND} -E copy "${wrapper}" "${PACKAGE_DIR}/libs/${ANDROID_NDK_ABI_NAME}/")
endforeach()

add_custom_command(
         OUTPUT "${APK_NAME}"
         COMMAND ${CMAKE_COMMAND} -E remove_directory "${PACKAGE_DIR}/libs"
         COMMAND ${CMAKE_COMMAND} -E remove_directory "${PACKAGE_DIR}/bin"
         COMMAND ${CMAKE_COMMAND} -E remove_directory "${PACKAGE_DIR}/gen"
         COMMAND ${CMAKE_COMMAND} -E remove ${android_proj_target_files}
         COMMAND ${CMAKE_COMMAND} -E make_directory "${PACKAGE_DIR}/src"
         COMMAND ${CMAKE_COMMAND} -E make_directory "${PACKAGE_DIR}/libs/${ANDROID_NDK_ABI_NAME}/"
         COMMAND ${CMAKE_COMMAND} -E copy "${SDL2_LIBRARY}" "${PACKAGE_DIR}/libs/${ANDROID_NDK_ABI_NAME}/"
         COMMAND ${CMAKE_COMMAND} -E copy "${ZLIB_LIBRARY}" "${PACKAGE_DIR}/libs/${ANDROID_NDK_ABI_NAME}/"
         COMMAND ${CMAKE_COMMAND} -E copy "${OPENSL_LIBRARY}" "${PACKAGE_DIR}/libs/${ANDROID_NDK_ABI_NAME}/"
         COMMAND ${CMAKE_COMMAND} -E copy "${wagic_location}" "${PACKAGE_DIR}/libs/${ANDROID_NDK_ABI_NAME}/"
         COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/src" "${PACKAGE_DIR}/src"
         COMMAND ${ANDROID_EXECUTABLE} --silent update project --path "${PACKAGE_DIR}" --target "android-${ANDROID_NATIVE_API_LEVEL}" --name "${target_name}"
         COMMAND ${ANT_EXECUTABLE} -noinput -k ${android_build_type}
         COMMAND ${CMAKE_COMMAND} -E touch "${APK_NAME}"
         WORKING_DIRECTORY "${PACKAGE_DIR}"
         MAIN_DEPENDENCY "${PACKAGE_DIR}/${ANDROID_MANIFEST_FILE}"
         DEPENDS
         "${PACKAGE_DIR}/res/values/strings.xml" 
         "${PACKAGE_DIR}/res/layout/main.xml"
         "${PACKAGE_DIR}/res/drawable-mdpi/icon.png" 
         "${PACKAGE_DIR}/res/drawable-ldpi/icon.png"
         "${PACKAGE_DIR}/res/drawable-hdpi/icon.png"
         "${PACKAGE_DIR}/${ANDROID_MANIFEST_FILE}"
         wagic
       )

install(FILES "${APK_NAME}" DESTINATION "apk/" COMPONENT main)
add_custom_target(android_package ALL SOURCES "${APK_NAME}" )
add_dependencies(android_package wagic)