#include "lookuptable.h"

namespace api {
namespace bridge {

IMPL_JACE_CONSTRUCTORS(LookupTable)
IMPL_RSCLASS_GET(LookupTable)

IMPL_ARRAY_METHOD(LookupTable, getIdentityTable, JInt)

} // namespace bridge
} // namespace api
