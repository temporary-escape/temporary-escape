include(FindPackageHandleStandardArgs)

if(NOT TARGET Xml2)
  find_library(XML2_LIBRARIES NAMES xml2 libxml2)
  find_path(XML2_INCLUDE_DIR NAMES libxml/xmlreader.h)
  add_library(Xml2 STATIC IMPORTED)
  set_target_properties(Xml2 PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${XML2_INCLUDE_DIR})
  set_target_properties(Xml2 PROPERTIES IMPORTED_LOCATION ${XML2_LIBRARIES})
endif()

find_package_handle_standard_args(Xml2 DEFAULT_MSG XML2_INCLUDE_DIR XML2_LIBRARIES)
