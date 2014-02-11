#include "client.h"
#include "RSClassMapper.h"
#include "jace/JField.h"

namespace api {
namespace native {

Client::Client(const Client &obj) {
    setJavaJniValue(obj.getJavaJniValue());
}

Client::Client(jobject obj) {
    setJavaJniObject(obj);
}

Client::Client(jvalue val) {
    setJavaJniValue(val);
}

RSClass* Client::staticGetJavaJniClass() {
    static RSClass* cls = RSClassMapper::DefaultInstance()->getRSClass("Client");
    return cls;
}

RSClass* Client::getJavaJniClass() const {
    return Client::staticGetJavaJniClass();
}

// Example 1: static object-type field.
// static fields map to static member functions in a proxy class.
// We collect the value for the field (using the class reference)
// and return it directly, since RS does not mess with object references.

// Furthermore, we get the contents of the field using JField::getReadOnly,
// since this returns the field contents directly instead of a JFieldProxy.
// By doing this, we effectively prevent external code from writing to the
// Runescape client directly.
Client Client::getClient() {
    RSClass* rsc = Client::staticGetJavaJniClass();
    return jace::JField<Client>(rsc->getFieldName("getClient")).getReadOnly(rsc);
}

// Example 2: virtual primitive-type method..
// virtual fields are practically collected the same way, only here the object
// reference is used for the actual collection.
// Additionally, since it's a primitive type, we may need to apply a modifier
// value to the result as well.

// A possible improvement on this technique involves moving the field getter
// and the modifier application to RSClass, since this makes the proxy classes even
// smaller and easier to understand. The content of any getter method would then
// just be "return getJavaJniClass().getField<Type>(name);".
JInt Client::getFPS() {
    RSClass* rsc = getJavaJniClass();
    JInt result = jace::JField<JInt>(rsc->getFieldName("getFPS")).getReadOnly(*this);
    return result * rsc->getFieldModifier("getFPS");
}


} // end namespace native
} // end namespace api