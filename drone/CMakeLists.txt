add_executable(proxy_g++ proxy_g++.cc)
add_executable(proxy_python proxy_python)
add_executable(drone main.cc simplehandlers.cc pythonlangsupport.cc cclangsupport.cc importhandler.cc globals.cc)
target_link_libraries(drone common ${Boost_LIBRARIES})
