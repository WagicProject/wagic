set(WAGIC_APP_NAME wagic)

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
        )

set(MTG_network_src src/NetworkPlayer.cpp)

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
        src/SDLmain.cpp)

set(MTG_iOS_src
        iOS/UI/WagicDownloadProgressViewController.m
        iOS/asi-http-request/ASIAuthenticationDialog.m
        iOS/asi-http-request/ASIDataCompressor.m
        iOS/asi-http-request/ASIDataDecompressor.m
        iOS/asi-http-request/ASIDownloadCache.m
        iOS/asi-http-request/ASIFormDataRequest.m
        iOS/asi-http-request/ASIHTTPRequest.m
        iOS/asi-http-request/ASIInputStream.m
        iOS/asi-http-request/ASINetworkQueue.m
        iOS/SoundManager/SoundManager.m
        iOS/SoundManager/MyOpenALSupport.c
        iOS/Reachability/Reachability/Reachability.m
        iOS/ZipArchive/ZipArchive.mm)


set(MTG_iOS_INCLUDE_DIRS iOS/UI iOS/Reachability/Reachability iOS/asi-http-request iOS/ZipArchive iOS/SoundManager)

set(MTG_INCLUDE_DIRS include include/qt)
set(EXTRA_INCLUDE_DIR extra)

set(MTG_SRC ${MTG_generic_src})

if(NOT WIN32 AND NOT IOS)
    set(MTG_SRC ${MTG_SRC} ${MTG_network_src})
endif()


if(backend_qt_console OR backend_qt_widget)
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
    set(CMAKE_AUTOMOC TRUE)

    if(UNIX AND NOT ANDROID)
        if(backend_qt_console)
            set(MTG_SRC ${MTG_SRC} ${MTG_qt_console_src})
        elseif(backend_qt_widget)
            set(MTG_SRC ${MTG_SRC} ${MTG_qt_graphic_src})
        endif()
    else()
        message(FATAL_ERROR "qt-widget builds are only supported on unix platforms")
    endif()

    add_executable(${WAGIC_APP_NAME} ${MTG_SRC})
    target_link_libraries(${WAGIC_APP_NAME} PUBLIC Wagic::JGE ${CMAKE_THREAD_LIBS_INIT})
elseif(backend_sdl)
    set(MTG_SRC ${MTG_SRC} src/TestSuiteAI.cpp)
    if((UNIX OR EMSCRIPTEN) AND NOT ANDROID)
        add_executable(${WAGIC_APP_NAME} ${MTG_SRC} ${MTG_sdl_src} src/AIPlayerBakaB.cpp)
        target_link_libraries(${WAGIC_APP_NAME} PUBLIC Wagic::JGE ${CMAKE_THREAD_LIBS_INIT})
    elseif(ANDROID)
        #we should try to get rid of this hack
        add_library(${WAGIC_APP_NAME} SHARED ${MTG_SRC} ${MTG_android_sdl_src})
        target_link_libraries(${WAGIC_APP_NAME} PUBLIC Wagic::JGE)
    elseif(WIN32)
        add_executable(${WAGIC_APP_NAME} ${MTG_SRC} ${MTG_sdl_src} src/AIPlayerBakaB.cpp)
        link_directories(${Boost_LIBRARY_DIR} ${SDL2_LIBRARY_DIR})
        target_link_libraries(${WAGIC_APP_NAME} PUBLIC Wagic::JGE ${BOOST_date_time} ${CMAKE_THREAD_LIBS_INIT})
    endif()
elseif(backend_psp)
    if(PSP)
        #${PSPSDK_PATH}/include
        add_executable(${WAGIC_APP_NAME} ${MTG_SRC})
        target_link_libraries(${WAGIC_APP_NAME} PUBLIC Wagic::JGE)

        set(PRXSPECS_FILE "${PSPSDK_PATH}/lib/prxspecs")
        set(LINKFILE_FILE "${PSPSDK_PATH}/lib/linkfile.prx")

        set_target_properties(${WAGIC_APP_NAME} PROPERTIES LINK_FLAGS "-specs=${PRXSPECS_FILE} -Wl,-q,-T${LINKFILE_FILE}")
        
        add_custom_command(TARGET ${WAGIC_APP_NAME} POST_BUILD
            COMMAND ${PSPSDK_MKSFO_COMMAND} ARGS "'Wagic, the Homebrew?!'" "$<TARGET_FILE_DIR:${WAGIC_APP_NAME}>/PARAM.SFO"
            COMMAND ${PSPSDK_FIXUP_IMPORTS_COMMAND} ARGS $<TARGET_FILE:${WAGIC_APP_NAME}>
            COMMAND ${PSPSDK_PRXGEN_COMMAND} ARGS $<TARGET_FILE:${WAGIC_APP_NAME}> "$<TARGET_FILE_DIR:${WAGIC_APP_NAME}>/wagic.prx"
            COMMAND ${PSPSDK_PACK_PBP_COMMAND} ARGS $<TARGET_FILE_DIR:${WAGIC_APP_NAME}>/EBOOT.PBP $<TARGET_FILE_DIR:${WAGIC_APP_NAME}>/PARAM.SFO ${CMAKE_CURRENT_SOURCE_DIR}/icon.png NULL ${CMAKE_CURRENT_SOURCE_DIR}/pic0.png ${CMAKE_CURRENT_SOURCE_DIR}/pic1.png NULL "$<TARGET_FILE_DIR:${WAGIC_APP_NAME}>/wagic.prx" NULL)
    else()
        message(FATAL_ERROR "use the cross-compile toolchain to build as the psp target")
    endif()
endif()

if(IOS)
    set(MTG_generic_src ${MTG_generic_src} ${MTG_iOS_src} PARENT_SCOPE)
    set(MTG_INCLUDE_DIRS ${MTG_INCLUDE_DIRS} ${MTG_iOS_INCLUDE_DIRS} PARENT_SCOPE)
endif()

target_include_directories(${WAGIC_APP_NAME} PUBLIC ${MTG_INCLUDE_DIRS})
#target_link_libraries(${WAGIC_APP_NAME} ${JGE_LINK_LIBRARIES})

set_target_properties(${WAGIC_APP_NAME} PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO )