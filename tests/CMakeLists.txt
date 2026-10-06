# Qt6::Test ships with the Qt6 base package the project already requires, so
# this adds no new third-party dependency.
find_package(Qt6 REQUIRED COMPONENTS Test)

# Each test's QObject is declared inside its .cpp and pulls in its own .moc.
add_executable(tst_playback_controller tst_playback_controller.cpp)
target_link_libraries(tst_playback_controller Qt6::Core Qt6::Test controllers)

add_executable(tst_image_processor tst_image_processor.cpp)
target_link_libraries(tst_image_processor Qt6::Core Qt6::Gui Qt6::Test processors)

add_executable(tst_ui_controller tst_ui_controller.cpp)
target_link_libraries(tst_ui_controller Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Test ui)

foreach(test tst_playback_controller tst_image_processor tst_ui_controller)
    add_test(NAME ${test} COMMAND ${test})
    # The UI test needs a QApplication; offscreen keeps it headless.
    set_tests_properties(${test} PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endforeach()
