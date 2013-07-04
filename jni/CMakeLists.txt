project(banknote_desktop)

find_package(OpenCV)

add_executable(match matcher.cpp test_matcher.cpp)
target_link_libraries(match ${OpenCV_LIBS})

add_executable(train train_samples.cpp)
target_link_libraries(train ${OpenCV_LIBS})
