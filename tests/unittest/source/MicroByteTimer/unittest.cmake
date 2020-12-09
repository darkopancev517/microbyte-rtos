set(unittest-includes ${unittest-includes}
)

set(unittest-sources
    ../../source/CircList.cpp
    ../../source/List.cpp
    ../../source/Cib.cpp
    ../../source/MicroByteCpu.cpp
    ../../source/MicroByteThread.cpp
    ../../source/MicroByteMutex.cpp
    ../../source/MicroByteTimer.cpp
    stubs/MicroByteCpuTest.cpp
    stubs/MicroBytePeriphTimer.cpp
    stubs/periphTimer.cpp
)

set(unittest-test-sources
    source/MicroByteTimer/test_MicroByteTimer.cpp
)
