/*
 * Copyright 2014 Patrick Kramer, Vincent Wassenaar
 *
 * This file is part of PowerGrid.
 *
 * PowerGrid is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PowerGrid is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PowerGrid.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "string.h"

namespace java {
namespace lang {

/*!
 * \class java::lang::String
 * \brief Proxy class for \c{java.lang.String}
 *
 * This class provides access to java String objects, and provides
 * additional operation to convert java Strings to \c std::string and
 * \c QString objects.
 */

String::String() : Object() {
}

String::String(const String &obj) :
    JObject(NoOp()), Object(obj) {
}

const jace::JClass* String::staticGetJavaJniClass() throw (jace::JNIException) {
    static jace::JClassImpl cls ("java.lang.String");
    return &cls;
}

const jace::JClass* String::getJavaJniClass() const throw (jace::JNIException) {
    return String::staticGetJavaJniClass();
}

String::String(jobject object) :
    Object(object) {}

String::String(jvalue value) :
    Object(value) {}

String::String(jstring string) :
    Object(string) {}

String& String::operator =(const String &obj) {
    setJavaJniObject(obj.getJavaJniObject());
    return *this;
}
/*!
 * \brief Coverts this String into an \c std::string
 * \return an \c std::string with the contents of this java String
 */
std::string String::toStdString() const throw (jace::JNIException){
    JNIEnv* env = jace::helper::attach();
    jstring string = static_cast<jstring>(getJavaJniObject());
    const char* chars = env->GetStringUTFChars(string, NULL);
    return chars;
}
/*!
 * \brief Coverts this String into an \c QString
 * \return an \c QString with the contents of this java String
 */
QString String::toQString() const throw (jace::JNIException) {
    JNIEnv* env = jace::helper::attach();
    jstring string = static_cast<jstring>(getJavaJniObject());
    const char* chars = env->GetStringUTFChars(string, NULL);
    return QString::fromLocal8Bit(chars);
}

}
}
