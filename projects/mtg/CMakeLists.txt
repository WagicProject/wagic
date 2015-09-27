#MTGs common sources
set(MTG_generic_src         
        src/AbilityParser.cpp
        src/ActionElement.cpp
        src/ActionLayer.cpp
        src/ActionStack.cpp
        src/AIHints.cpp
        src/AIMomirPlayer.cpp
        src/AIPlayer.cpp
        src/AIPlayerBaka.cpp
        src/AIStats.cpp
        src/AllAbilities.cpp
        src/CardDescriptor.cpp
        src/CardDisplay.cpp
        src/CardGui.cpp
        src/CardPrimitive.cpp
        src/CardSelector.cpp
        src/CardSelectorSingleton.cpp
        src/CarouselDeckView.cpp
        src/Closest.cpp
        src/Counters.cpp
        src/Credits.cpp
        src/Damage.cpp
        src/DamagerDamaged.cpp
        src/DeckDataWrapper.cpp
        src/DeckEditorMenu.cpp
        src/DeckManager.cpp
        src/DeckMenu.cpp
        src/DeckMenuItem.cpp
        src/DeckMetaData.cpp
        src/DeckStats.cpp
        src/DeckView.cpp
        src/DuelLayers.cpp
        src/ExtraCost.cpp
        src/GameApp.cpp
        src/GameLauncher.cpp
        src/GameObserver.cpp
        src/GameOptions.cpp
        src/GameStateAwards.cpp
        src/GameState.cpp
        src/GameStateDeckViewer.cpp
        src/GameStateDuel.cpp
        src/GameStateMenu.cpp
        src/GameStateOptions.cpp
        src/GameStateShop.cpp
        src/GameStateStory.cpp
        src/GameStateTransitions.cpp
        src/GridDeckView.cpp
        src/GuiAvatars.cpp
        src/GuiBackground.cpp
        src/GuiCardsController.cpp
        src/GuiCombat.cpp
        src/GuiFrame.cpp
        src/GuiHand.cpp
        src/GuiLayers.cpp
        src/GuiMana.cpp
        src/GuiPhaseBar.cpp
        src/GuiPlay.cpp
        src/GuiStatic.cpp
        src/IconButton.cpp
        src/InteractiveButton.cpp
        src/ManaCost.cpp
        src/ManaCostHybrid.cpp
        src/MenuItem.cpp
        src/ModRules.cpp
        src/MTGAbility.cpp
        src/MTGCard.cpp
        src/MTGCardInstance.cpp
        src/MTGDeck.cpp
        src/MTGDefinitions.cpp
        src/MTGGamePhase.cpp
        src/MTGGameZones.cpp
        src/MTGPack.cpp
        src/MTGRules.cpp
        src/ObjectAnalytics.cpp
        src/OptionItem.cpp
        src/PhaseRing.cpp
        src/Player.cpp
        src/PlayerData.cpp
        src/PlayGuiObject.cpp
        src/PlayGuiObjectController.cpp
        src/PlayRestrictions.cpp
        src/Pos.cpp
        src/PriceList.cpp
        src/ReplacementEffects.cpp
        src/Rules.cpp
        src/SimpleMenu.cpp
        src/SimpleMenuItem.cpp
        src/SimpleButton.cpp
        src/SimplePad.cpp
        src/SimplePopup.cpp
        src/StoryFlow.cpp
        src/Subtypes.cpp
        src/StyleManager.cpp
        src/TargetChooser.cpp
        src/TargetsList.cpp
        src/Tasks.cpp
        src/TextScroller.cpp
        src/ThisDescriptor.cpp
        src/Token.cpp
        src/Translate.cpp
        src/TranslateKeys.cpp
        src/Trash.cpp
        src/utils.cpp
        src/WCachedResource.cpp
        src/WDataSrc.cpp
        src/WEvent.cpp
        src/WFilter.cpp
        src/WFont.cpp
        src/WGui.cpp
        src/WResourceManager.cpp
        src/NetworkPlayer.cpp
        )

#the sources we need if we compile a graphical qt version
#TODO: add declarative version since this only works with QWidget right now
set(MTG_qt_graphic_src
        src/qt/filedownloader.cpp
        src/qt/corewrapper.cpp
        include/qt/corewrapper.h #so automoc finds it
        include/qt/filedownloader.h
        src/Qtmain.cpp)

#the sources we need to compile the testsuit
set(MTG_qt_console_src
        src/Qtconsole.cpp
        src/TestSuiteAI.cpp)
        
set(MTG_sdl_src
        src/SDLmain.cpp)
        
set(MTG_android_sdl_src
        src/SDLmain.cpp
        src/SDL_android_main.cpp)


set(MTG_INCLUDE_DIRS include include/qt)
set(JGE_INCLUDE_DIRS ../../JGE/include ../../JGE/include/hge)
set(JGE_LIBRARY jge)
    
#turn moc on
if(backend_qt_console OR backend_qt_widget)
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
    set(CMAKE_AUTOMOC TRUE)
endif()

if(backend_qt_console)
    if(UNIX AND NOT ANDROID)
        include_directories(${MTG_INCLUDE_DIRS} ${JGE_INCLUDE_DIRS} ${ZIPFS_INCLUDE_DIR} ${Qt5Core_INCLUDE_DIRS} ${Qt5Multimedia_INCLUDE_DIRS}
            ${UNZIP_INCLUDE_DIR} ${HGE_INCLUDE_DIR} ${TINYXML_INCLUDE_DIR} ${QT_MKSPECS_DIR}/default)
        add_executable(wagic ${MTG_generic_src} ${MTG_qt_console_src})
        target_link_libraries(wagic  ${JGE_LIBRARY} ${Qt5Core_INCLUDE_DIRS} ${Qt5Multimedia_INCLUDE_DIRS} ${TINYXML_LIBRARIES}
            ${HGE_LIBRARY})
    else()
        message(FATAL_ERROR "qt-console bilds are only supported on unix platforms")
    endif()
endif()

if(backend_qt_widget)
    if(UNIX AND NOT ANDROID)
        include_directories(${MTG_INCLUDE_DIRS} ${JGE_INCLUDE_DIRS} ${ZIPFS_INCLUDE_DIR} ${Qt5Core_INCLUDE_DIRS} ${Qt5Multimedia_INCLUDE_DIRS}
            ${UNZIP_INCLUDE_DIR} ${HGE_INCLUDE_DIR} ${TINYXML_INCLUDE_DIR} ${QT_MKSPECS_DIR}/default)
        add_executable(wagic ${MTG_generic_src} ${MTG_qt_graphic_src})
        target_link_libraries(wagic  ${JGE_LIBRARY} ${Qt5Core_INCLUDE_DIRS} ${Qt5Multimedia_INCLUDE_DIRS} ${OPENGL_LIBRARIES} ${TINYXML_LIBRARIES}
            ${HGE_LIBRARY})# ${X11_LIBRARIES})
    else()
        message(FATAL_ERROR "qt-widget bilds are only supported on unix platforms")
    endif()
endif()

if(backend_sdl)
    if(UNIX AND NOT ANDROID)
        include_directories(${MTG_INCLUDE_DIRS} ${JGE_INCLUDE_DIRS} ${SDL2_INCLUDE_DIR} ${ZIPFS_INCLUDE_DIR} 
            ${BOOST_INCLUDE_DIRS} ${UNZIP_INCLUDE_DIR} ${HGE_INCLUDE_DIR} ${TINYXML_INCLUDE_DIR})
        add_executable(wagic ${MTG_generic_src} ${MTG_sdl_src})
        target_link_libraries(wagic  ${JGE_LIBRARY} ${SDL2_LIBRARY} ${OPENGL_LIBRARIES} ${TINYXML_LIBRARIES} 
            ${Boost_LIBRARIES} ${HGE_LIBRARY})
    elseif(ANDROID)
        include_directories(${MTG_INCLUDE_DIRS} ${JGE_INCLUDE_DIRS} ${SDL2_INCLUDE_DIR} ${ZIPFS_INCLUDE_DIR} 
            ${BOOST_INCLUDE_DIRS} ${UNZIP_INCLUDE_DIR} ${HGE_INCLUDE_DIR} ${TINYXML_INCLUDE_DIR})
        #we should try to get rid of this hack
        add_library(wagic SHARED ${MTG_generic_src} ${MTG_android_sdl_src}
                    ${ANDROID_BOOST_PTHREAD_SRC_DIR}/thread.cpp
                    ${ANDROID_BOOST_PTHREAD_SRC_DIR}/once.cpp
                    ${ANDROID_BOOST_SYSTEM_SRC_DIR}/error_code.cpp
                    src/TestSuiteAI.cpp)
        target_link_libraries(wagic  ${JGE_LIBRARY} ${SDL2_LIBRARY} ${OPENGL_LIBRARIES} ${TINYXML_LIBRARIES} 
            ${HGE_LIBRARY})
    elseif(WIN32)
        include_directories(${MTG_INCLUDE_DIRS} ${JGE_INCLUDE_DIRS} ${SDL2_INCLUDE_DIR} ${ZIPFS_INCLUDE_DIR} 
            ${Boost_INCLUDE_DIRS} ${UNZIP_INCLUDE_DIR} ${HGE_INCLUDE_DIR} ${TINYXML_INCLUDE_DIR}
            ${ZLIB_INCLUDE_DIR})
        add_executable(wagic ${MTG_generic_src} ${MTG_sdl_src} src/TestSuiteAI.cpp src/AIPlayerBakaB.cpp)
        link_directories(${Boost_LIBRARY_DIR} ${SDL2_LIBRARY_DIR})
        target_link_libraries(wagic  ${JGE_LIBRARY} ${SDL2_LIBRARY} ${OPENGL_LIBRARIES} ${TINYXML_LIBRARIES} 
            ${BOOST_date_time} ${HGE_LIBRARY} ${CMAKE_SOURCE_DIR}/thirdparty/binary/win/lib/fmodvc.lib)
    endif()
endif()

if(backend_psp)
    if(PSP)
        #${PSPSDK_PATH}/include
        include_directories(${MTG_INCLUDE_DIRS} ${JGE_INCLUDE_DIRS} ${TINYXML_INCLUDE_DIR} ${ZIPFS_INCLUDE_DIR} 
            ${UNZIP_INCLUDE_DIR} ${BOOST_INCLUDE_DIRS} ${GIF_INCLUDE_DIR} ${HGETOOLS_INCLUDE_DIRS})
        add_executable(wagic ${MTG_generic_src})
        target_link_libraries(wagic ${JGE_LIBRARY} ${PSPSDK_LIB} ${HGETOOLS_LIBRARY} ${FREETYPE_LIB} ${JPEG_LIBRARY} 
            ${GIF_LIBRARIES} ${PNG_LIBRARIES} z m ${MIKMOD_LIBRARY} ${TINYXML_LIBRARIES} 
            stdc++ ${PSPSDK_REQUIRED_LIB})

        set(PRXSPECS_FILE "${PSPSDK_PATH}/lib/prxspecs")
        set(LINKFILE_FILE "${PSPSDK_PATH}/lib/linkfile.prx")

        get_property(wagic_elf_location TARGET wagic PROPERTY LOCATION)
        get_filename_component(wagic_elf_directory ${wagic_elf_location} PATH)

        set_target_properties(wagic PROPERTIES LINK_FLAGS "-specs=${PRXSPECS_FILE} -Wl,-q,-T${LINKFILE_FILE}")
        
        add_custom_command(TARGET wagic POST_BUILD
            COMMAND ${PSPSDK_MKSFO_COMMAND} ARGS "'Wagic, the Homebrew?!'" "${wagic_elf_directory}/PARAM.SFO"
            COMMAND ${PSPSDK_FIXUP_IMPORTS_COMMAND} ARGS ${wagic_elf_location}
            COMMAND ${PSPSDK_PRXGEN_COMMAND} ARGS ${wagic_elf_location} "${wagic_elf_directory}/wagic.prx"
            COMMAND ${PSPSDK_PACK_PBP_COMMAND} ARGS ${wagic_elf_directory}/EBOOT.PBP ${wagic_elf_directory}/PARAM.SFO ${CMAKE_CURRENT_SOURCE_DIR}/icon.png NULL ${CMAKE_CURRENT_SOURCE_DIR}/pic0.png ${CMAKE_CURRENT_SOURCE_DIR}/pic1.png NULL "${wagic_elf_directory}/wagic.prx" NULL)
    else()
        message(FATAL_ERROR "use the cross-compile toolchain to build as the psp target")
    endif()
endif()
