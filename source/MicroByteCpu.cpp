#include "MicroByteCpu.h"

namespace microbyte {

MicroByteCpu *uByteCpu = NULL;

MicroByteCpu::MicroByteCpu()
{
    uByteCpu = this;
}

} // namespace microbyte
