find_library(URING_LIBRARIES NAMES uring)

if (NOT TARGET uring)
    add_library(uring SHARED IMPORTED)
    add_library(uring::uring ALIAS uring)
    set_target_properties(uring PROPERTIES IMPORTED_LOCATION ${URING_LIBRARIES})
endif ()
