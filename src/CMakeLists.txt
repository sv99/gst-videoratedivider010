# config.h
ADD_DEFINITIONS(-DHAVE_CONFIG_H)

set(LIB_SOURCE_FILES
        config.h
        gstvideoratedivider.h
        gstvideoratedivider.c
        )

add_library(gstvideoratedivider SHARED ${LIB_SOURCE_FILES})
set_target_properties(gstvideoratedivider PROPERTIES SUFFIX .so)
target_link_libraries(gstvideoratedivider
        ${GLIB2_LIBRARIES} ${GSTRM_LIBRARIES} ${GSTRMAPP_LIBRARIES})

install(TARGETS gstvideoratedivider
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        )
