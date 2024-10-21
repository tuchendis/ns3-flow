build_example(
        NAME flow-simulation
        SOURCE_FILES
        flow-simulation.cc
        HEADER_FILES
        LIBRARIES_TO_LINK
        ${libpoint-to-point}
        ${libtraffic-control}
        ${libinternet}
        ${libapplications}
)