add_library(
  udp-traffic-cache-cp-lib
  lib/udp-cache-server.cc
  lib/udp-content-provider.cc
  lib/udp-traffic-generator.cc
  lib/udp-traffic-cache-cp-helper.cc
)

build_exec(
  EXECNAME udp-complete
  SOURCE_FILES udp-complete.cc
  LIBRARIES_TO_LINK udp-traffic-cache-cp-lib ${libcore} ${ns3-libs}
  EXECUTABLE_DIRECTORY_PATH ${CMAKE_OUTPUT_DIRECTORY}/scratch/tesi
)

build_exec(
  EXECNAME udp-no-cache
  SOURCE_FILES udp-no-cache.cc
  LIBRARIES_TO_LINK udp-traffic-cache-cp-lib ${libcore} ${ns3-libs}
  EXECUTABLE_DIRECTORY_PATH ${CMAKE_OUTPUT_DIRECTORY}/scratch/tesi
)
