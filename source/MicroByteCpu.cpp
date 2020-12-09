#include "MicroByteCpu.h"

namespace microbyte {

static MicroByteCpu *microByteCpu = NULL;

void cpuSet(MicroByteCpu *cpu)
{
    microByteCpu = cpu;
}

MicroByteCpu *cpuGet()
{
    return microByteCpu;
}

} // namespace microbyte
