#include <stdexcept>
#include "ssre.h"
#include "internal/ssre_internal.h"

namespace ssre
{
    namespace internal
    {
        SSREBuffer buffer;

        void SSREBuffer::reset()
        {
            lighting_source_count = 0;
        }
    }
}
