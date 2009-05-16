add_executable(drone main.cc simplehandlers.cc pythonlangsupport.cc cclangsupport.cc importhandler.cc)
target_link_libraries(drone common)