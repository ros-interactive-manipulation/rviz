# Build file for Python bindings via sip and pyqt.

set(rviz_HDRS_DIR ${PROJECT_SOURCE_DIR}/src/rviz)
set(SIP_BUILD_DIR ${PROJECT_SOURCE_DIR}/build/sip/rviz_sip)
set(SIP_SRC_DIR ${PROJECT_SOURCE_DIR}/src/python_bindings/sip)

set(rviz_sip_DEPENDENT_FILES
  rviz.sip
  visualization_frame.sip
  visualization_manager.sip
  display.sip
  display_group.sip
  panel_dock_widget.sip
  property.sip
  bool_property.sip
  view_controller.sip
  view_manager.sip
  tool.sip
  tool_manager.sip
  config.sip
  yaml_config_reader.sip
  yaml_config_writer.sip
  ${rviz_HDRS_DIR}/visualization_frame.h
  ${rviz_HDRS_DIR}/visualization_manager.h
  ${rviz_HDRS_DIR}/display.h
  ${rviz_HDRS_DIR}/display_group.h
  ${rviz_HDRS_DIR}/panel_dock_widget.h
  ${rviz_HDRS_DIR}/properties/property.h
  ${rviz_HDRS_DIR}/properties/bool_property.h
  ${rviz_HDRS_DIR}/view_controller.h
  ${rviz_HDRS_DIR}/view_manager.h
  ${rviz_HDRS_DIR}/tool.h
  ${rviz_HDRS_DIR}/tool_manager.h
  ${rviz_HDRS_DIR}/config.h
  ${rviz_HDRS_DIR}/yaml_config_reader.h
  ${rviz_HDRS_DIR}/yaml_config_writer.h
)

find_package(python_qt_binding REQUIRED)
include(${python_qt_binding_EXTRAS_DIR}/sip_helper.cmake)

# maintain context for different named target
set(rviz_sip_INCLUDE_DIRS ${rviz_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/src)
set(rviz_sip_LIBRARIES ${rviz_LIBRARIES} ${PROJECT_NAME})
set(rviz_sip_LIBRARY_DIRS ${rviz_LIBRARY_DIRS} ${CMAKE_SOURCE_DIR}/lib)
set(rviz_sip_LDFLAGS_OTHER ${rviz_LDFLAGS_OTHER} -Wl,-rpath,${CMAKE_SOURCE_DIR}/lib)

if(sip_helper_FOUND)
  list(APPEND rviz_BINDINGS "sip")
  set(rviz_BINDINGS "${rviz_BINDINGS}" PARENT_SCOPE)
  build_sip_binding(rviz_sip rviz.sip
    SIP_CONFIGURE ${python_qt_binding_EXTRAS_DIR}/sip_configure.py
    SOURCE_DIR ${PROJECT_SOURCE_DIR}/src/python_bindings/sip
    DEPENDS ${rviz_sip_DEPENDENT_FILES}
    DEPENDENCIES rviz
  )
endif()
