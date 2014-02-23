#ifndef API_BRIDGE_NPCDEF_H
#define API_BRIDGE_NPCDEF_H

#include "MethodHelper.h"
#include "java/lang/object.h"
using namespace java::lang;

// Forward declarations of Java API classes
namespace java {
namespace lang {
  class String;
}
}

namespace api {
namespace bridge {

class HashTable;
class NPCDef : public Object {
public:
    RS_OBJECT(NPCDef)

    JACE_PROXY_API QList<String> getActions();
    JACE_PROXY_API JInt getID();
    JACE_PROXY_API JInt getModelBoundRadius();
    JACE_PROXY_API String getName();
    JACE_PROXY_API HashTable getNodeTable();
    JACE_PROXY_API JInt getPrayerHeadIconIndex();
    JACE_PROXY_API JInt getRenderEmote();
private:
    DECLARE_FRIENDS
};

} // namespace bridge
} // namespace api

#endif // API_BRIDGE_NPCDEF_H