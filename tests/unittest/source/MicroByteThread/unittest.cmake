set(unittest-includes ${unittest-includes}
)

set(unittest-sources
    ../../source/CircList.cpp
    ../../source/List.cpp
    ../../source/Cib.cpp
    ../../source/MicroByteCpu.cpp
    ../../source/MicroByteThread.cpp
    ../../source/MicroByteMutex.cpp
    ../../source/MicroByteMsg.cpp
    ../../source/MicroByteTimer.cpp
    stubs/MicroByteCpuTest.cpp
)

set(unittest-test-sources
    source/MicroByteThread/test_MicroByteThread.cpp
)
