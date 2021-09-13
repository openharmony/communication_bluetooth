#cmake compability
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.4)

#allow more human readalbe "if then else" construct
SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)

ADD_SUBDIRECTORY(gtest)
#ADD_SUBDIRECTORY(test_app)