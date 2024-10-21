build_example(
        NAME dumbell
        SOURCE_FILES dumbell.cc
        LIBRARIES_TO_LINK
        ${libnetwork}
        ${libnetanim}
        ${libmobility}
        ${libpoint-to-point}
        ${libinternet}
        ${libapplications}
        ${libflow-monitor}
)
