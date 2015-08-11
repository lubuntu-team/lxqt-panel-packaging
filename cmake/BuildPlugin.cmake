MACRO (BUILD_LXQT_PLUGIN NAME)
    set(PROGRAM "lxqt-panel")
    project(${PROGRAM}_${NAME})

    set(PROG_SHARE_DIR ${CMAKE_INSTALL_FULL_DATAROOTDIR}/lxqt/${PROGRAM})
    set(PLUGIN_SHARE_DIR ${PROG_SHARE_DIR}/${NAME})

    # Translations **********************************
    lxqt_translate_ts(${PROJECT_NAME}_QM_FILES
        UPDATE_TRANSLATIONS ${UPDATE_TRANSLATIONS}
        SOURCES
            ${HEADERS}
            ${SOURCES}
            ${MOCS}
            ${UIS}
        TEMPLATE
            ${NAME}
        INSTALL_DIR
            ${LXQT_TRANSLATIONS_DIR}/${PROGRAM}/${NAME}
    )

    #lxqt_translate_to(QM_FILES ${CMAKE_INSTALL_FULL_DATAROOTDIR}/lxqt/${PROGRAM}/${PROJECT_NAME})
    file (GLOB ${PROJECT_NAME}_DESKTOP_FILES_IN resources/*.desktop.in)
    lxqt_translate_desktop(DESKTOP_FILES
        SOURCES
            ${${PROJECT_NAME}_DESKTOP_FILES_IN}
    )
    #************************************************

    file (GLOB CONFIG_FILES resources/*.conf)

    if (NOT DEFINED PLUGIN_DIR)
        set (PLUGIN_DIR ${CMAKE_INSTALL_FULL_LIBDIR}/${PROGRAM})
    endif (NOT DEFINED PLUGIN_DIR)

    qt5_wrap_cpp(MOC_SOURCES ${MOCS})
    qt5_add_resources(QRC_SOURCES ${RESOURCES})
    qt5_wrap_ui(UI_SOURCES ${UIS})
    set(QTX_LIBRARIES Qt5::Widgets)
    if(QT_USE_QTXML)
        set(QTX_LIBRARIES ${QTX_LIBRARIES} Qt5::Xml)
    endif()
    if(QT_USE_QTDBUS)
        set(QTX_LIBRARIES ${QTX_LIBRARIES} Qt5::DBus)
    endif()

    add_library(${NAME} MODULE ${HEADERS} ${SOURCES} ${MOC_SOURCES} ${${PROJECT_NAME}_QM_FILES} ${QRC_SOURCES} ${UIS} ${DESKTOP_FILES})
    target_link_libraries(${NAME} ${QTX_LIBRARIES} ${LXQT_LIBRARIES} ${LIBRARIES} KF5::WindowSystem)

    install(TARGETS ${NAME} DESTINATION ${PLUGIN_DIR})
    install(FILES ${CONFIG_FILES}  DESTINATION ${PLUGIN_SHARE_DIR})
    install(FILES ${DESKTOP_FILES} DESTINATION ${PROG_SHARE_DIR})

ENDMACRO(BUILD_LXQT_PLUGIN)
