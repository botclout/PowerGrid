#ifndef ITEMPILE_H
#define ITEMPILE_H

#include "groundentity.h"

namespace world{

class ItemPile : public GroundEntity {
private:
    Q_DISABLE_COPY(ItemPile)
    jint topid;
    jint topstacksize;
    jint middleid;
    jint middlestacksize;
    jint bottomid;
    jint bottomstacksize;
public:
    ItemPile(jobject obj) : GroundEntity(obj){}
    jint getTopId(bool useCache = true);
    jint getTopStackSize(bool useCache = true);
    jint getMiddleId(bool useCache = true);
    jint getMiddleStackSize(bool useCache = true);
    jint getBottomId(bool useCache = true);
    jint getBottomStackSize(bool useCache = true);
};

}
#endif // ITEMPILE_H
