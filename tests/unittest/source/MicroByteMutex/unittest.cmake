set(unittest-includes ${unittest-includes}
)

set(unittest-sources
    ../../source/CircList.cpp
    ../../source/List.cpp
    ../../source/Cib.cpp
    ../../source/MicroByteCpu.cpp
    ../../source/MicroByteThread.cpp
    ../../source/MicroByteMutex.cpp
    stubs/MicroByteCpuTest.cpp
)

set(unittest-test-sources
    source/MicroByteMutex/test_MicroByteMutex.cpp
)
