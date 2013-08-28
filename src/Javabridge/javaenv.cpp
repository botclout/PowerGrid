/*
 * Copyright 2012-2013 Patrick Kramer, Vincent Wassenaar
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
#include "javaenv.h"
#include <stdexcept>
#include "jniexception.h"
#include "jniclass.h"
#include "jnistring.h"
#include <QDir>
#include <QLibrary>
#include <QProcessEnvironment>
#include <iostream>

namespace jni {
  using namespace std;

  class JNIMethod;

  JavaEnv* JavaEnv::theEnvironment = NULL;

  JavaEnv::JavaEnv(QObject* parent) : QObject(parent) {
    running = JNI_FALSE;
    setObjectName("JavaEnvironment");
  }

  JNIEnv* JavaEnv::GetEnv() {
    QThread* current = QThread::currentThread();
    QMap<QThread*,JNIEnv*>::const_iterator it = environments.find(current);
    QMap<QThread*,JNIEnv*>::const_iterator end = environments.cend();
    if (it == end) {
      JNIEnv* env;
      if (jvm == NULL) {
        throw jni_error("JVM not started");
      }
      jvm->AttachCurrentThread(reinterpret_cast<void**>(&env), NULL);
      environments.insert(current, env);
      qDebug() << "Created JNIEnv for thread:" << current->objectName();
      return env;
    } else {
      return it.value();
    }
  }

  // Intialize and set up the environment
  void JavaEnv::Start() {
    if (running) {
      throw logic_error("JVM already running.");
    }

    QFile javaLib; // QFile denoting the jvm.dll file.

#ifdef Q_OS_WIN
    // Try to find the JVM library using Windows' where command
    // first we find the location(s) of the Java executable.
    QProcess where, whereJVM;

    where.start("where", QStringList() << "java.exe");
    uint entriesFound = 0;
    where.waitForFinished();
    // The output of the program consists of a list of paths,
    // separated by '\n', on which the java command was found.
    // we then look for the jvm library in each of these directories.
    while (where.canReadLine()) {
      QByteArray lineData = where.readLine(256);
      QString line = QString::fromLocal8Bit(lineData);

      if (!line.isEmpty()) {
        entriesFound ++;
        line.chop(line.size() - line.lastIndexOf('\\'));
        QString directory = line;
        if (!directory.isEmpty()) {
          qDebug() << "Searching " << directory << " for jvm.dll";

          // Look recursively for jvm.dll in the directory of the java executable.
          // This is guaranteed to work for normal Java installations on Windows.
          whereJVM.start("where", QStringList() << "/R" << directory << "jvm.dll");
          whereJVM.waitForFinished();

          int exitCode = whereJVM.exitCode();
          if (exitCode == 0) {
            // We matched at least one entry, so we simply take the first one.
            lineData = whereJVM.readLine(256);
            line = QString::fromLatin1(lineData);
            line.chop(1); // remove the unwanted \n char at the end.
            qDebug() << "detected jvm.dll at" << line;
            javaLib.setFileName(line.replace('\\', '/')); // Qt does not offically support Windows-style path separators ('\\').
            break;
          } else {
            qDebug() << "Failed to find jvm.dll: exit code =" << exitCode;
          }
        }
      }
    }
    qDebug() << "Searched" << entriesFound << "entries for java.exe";
#elif defined Q_OS_MACX
    //TODO Find the jvm library in the JavaVM framework
    javaLib.setFileName("/Frameworks/JavaVM.framework/...");
#else // any Unix OS (except Mac OS)
    //TODO Find the jvm library by scanning possible locations
    javaLib.setFileName("/usr/lib/...");
#endif

    qDebug() << "Identified jvm library at" << javaLib.fileName();

    JavaVMInitArgs vmargs;
    vmargs.version            = JNI_VERSION_1_6;
    vmargs.ignoreUnrecognized = JNI_FALSE;

#ifdef DEBUG
    int nOptions = 4;
#else
    int nOptions = 3;
#endif
    // These arguments are practically the same as for the official Runescape loader.
    // As such, the max heap size should be sufficient.
    JavaVMOption options[nOptions];
    options[0].optionString  = const_cast<char*>("-Djava.class.path=PowerGridLoader.jar");
    options[1].optionString  = const_cast<char*>("-Xmx256m");
    options[2].optionString  = const_cast<char*>("-Xss2m");;
#ifdef DEBUG
    // JNI calls are checked while debugging, in order to help tracing problems within the JNI.
    // In release builds they are disabled for speed.
    options[3].optionString  = const_cast<char*>("-Xcheck:jni");
#endif

    vmargs.nOptions = nOptions;
    vmargs.options = options;
    JNIEnv* env = NULL;

    JNI_CreateJavaVM(&jvm, (void**)&env, &vmargs);
    if (env != NULL) {
      QThread* current = QThread::currentThread();
      environments.insert(current, env);
    } else {
      throw jni_error(tr("Failed to create environment"));
    }

    // We redirect log traffic from System.err to a logfile instead, to prevent the usual Runescape Exception
    // stack traces from showing up in the console window.
    JNIClass* system = GetClass("java/lang/System");
    JNIClass* printstream = GetClass("java/io/PrintStream");
    if (system != NULL) {
      jclass s = system->GetJNIObject();
      jfieldID err = env->GetStaticFieldID(s, "err", "Ljava/io/PrintStream;");

      if (err != NULL && printstream != NULL) {
        // We replace the System.err field with another PrintStream that outputs to the file 'runescape.log'
        jclass ps = printstream->GetJNIObject();
        jmethodID initPrintStream = env->GetMethodID(ps, "<init>", "(Ljava/lang/String;)V");
        jstring file = env->NewStringUTF("runescape.log");
        jobject newout = env->NewObject(ps, initPrintStream, file);
        env->SetStaticObjectField(s, err, newout);
      } else {
        qDebug() << tr("Failed to redirect log messages");
      }
    }
    // Now we start the Jagex Applet Viewer's main class.
    jclass c = env->FindClass("net/pgrid/loader/AppletLoader");
    if (c == NULL) qFatal("Cannot start client: AppletLoader class not found");
    jmethodID main = env->GetStaticMethodID(c, "main", "([Ljava/lang/String;)V");
    if (main == NULL) qFatal("Cannot start client: main method not found");

    env->CallStaticVoidMethod(c, main, NULL);
    try {
      exceptionCheck();
    } catch (java_exception e) {
      qDebug() << "Exception occurred in main:" << e.what();
    }

    env->DeleteLocalRef(c);

    running = JNI_TRUE;

    emit started();
  }

  JNIClass* JavaEnv::GetClass(QString n) {
    QMap<QString,JNIClass*>::iterator it = classes.find(n);
    QMap<QString,JNIClass*>::iterator end = classes.end();
    JNIClass* jnic = NULL;
    if (it == end) {
      JNIEnv* env = GetEnv();
      jclass cls = env->FindClass(qPrintable(n));
      if (cls == NULL) {
        qWarning() << "Request for non-existing class:" << n;
        return NULL;
      }
      jnic = new JNIClass(cls);
      classes.insert(n, jnic);
      qDebug() << "Registered JNIClass with name:" << n;
    } else {
      jnic = it.value();
    }
    return jnic;
  }

  JNIClass* JavaEnv::GetClass(jclass c) {
    if (c == NULL) {
      return NULL;
    }

    QMap<QString,JNIClass*>::iterator it = classes.begin();
    QMap<QString,JNIClass*>::iterator end = classes.end();
    JNIClass* jnic;

    while (it != end) {
      jnic = it.value();
      if (jnic->GetJNIObject() == c) {
        return jnic;
      }
      ++it;
    }

    jnic = new JNIClass(c);
    jmethodID getName = GetMethodID(jnic, "getName", "()Ljava/lang/String;");
    JNIString result (GetEnv()->CallObjectMethod(jnic->GetJNIObject(), getName));
    classes.insert(result.GetStringValue(), jnic);
    return jnic;
  }

  JNIClass* JavaEnv::GetClassForObject(jobject obj) {
    if (obj == NULL) {
      return NULL;
    }
    JNIEnv* env = GetEnv();
    jclass cls = env->GetObjectClass(obj);
    QMap<QString,JNIClass*>::iterator it;
    for (it = classes.begin(); it != classes.end(); ++it) {
      if ((*it) != NULL) {
        JNIClass* c = (*it);
        if (c->GetJNIObject() == cls) {
          return c;
        }
      }
    }
    JNIClass* c = new JNIClass(cls);
    jmethodID getName = GetMethodID(c, "getName", "()Ljava/lang/String;");
    JNIString result (env->CallObjectMethod(c->GetJNIObject(), getName));
    classes.insert(result.GetStringValue(),c);
    return c;
  }

  jmethodID JavaEnv::GetMethodID(JNIClass* c, QString name, QString signature) {
    JNIEnv* env = GetEnv();

    jmethodID meth = env->GetMethodID(static_cast<jclass>(c->GetJNIObject()), qPrintable(name), qPrintable(signature));
    return meth;
  }

  jmethodID JavaEnv::GetStaticMethodID(JNIClass* c, const char* name, const char* signature) {
    JNIEnv* env = GetEnv();
    jmethodID meth = env->GetStaticMethodID(static_cast<jclass>(c->GetJNIObject()), name, signature);
    return meth;
  }

  QString JavaEnv::GetEnvironmentVersion() {
    if (running) {
      JNIEnv* env = GetEnv();
      jclass system = env->FindClass("java/lang/System");
      if (system == NULL) {
        throw jni_error("java.lang.System class not found");
      }
      jmethodID getProperty = env->GetStaticMethodID(system, "getProperty", "(Ljava/lang/String;)Ljava/lang/String;");
      if (getProperty == NULL) {
        throw jni_error("java.lang.System.getProperty method not found");
      }

      jstring infoStr = (jstring) env->CallStaticObjectMethod(system, getProperty, env->NewStringUTF("java.version"));
      QString info (env->GetStringUTFChars(infoStr, NULL));
      return info;
    } else {
      return QStringLiteral("Java VM not running");
    }
  }

  // Non-static Method Invocations

  jobject JavaEnv::CallObjectMethod(jobject obj, jmethodID method, uint n_args, ...) {
    va_list args;
    va_start(args, n_args);
    JNIEnv* env = GetEnv();
    jobject result = env->CallObjectMethodV(obj, method, args);
    va_end(args);
    exceptionCheck();
    return result;
  }

  jint JavaEnv::CallIntMethod(jobject obj, jmethodID method, uint n_args, ...) {
    va_list args;
    va_start(args, n_args);
    JNIEnv* env = GetEnv();
    jint result = env->CallIntMethodV(obj, method, args);
    va_end(args);
    exceptionCheck();
    return result;
  }

  jdouble JavaEnv::CallDoubleMethod(jobject obj, jmethodID method, uint n_args, ...) {
    va_list args;
    va_start(args, n_args);
    JNIEnv* env = GetEnv();
    jdouble result = env->CallDoubleMethodV(obj, method, args);
    va_end(args);
    exceptionCheck();
    return result;
  }

  jboolean JavaEnv::CallBooleanMethod(jobject obj, jmethodID method, uint n_args, ...) {
    va_list args;
    va_start(args, n_args);
    JNIEnv* env = GetEnv();
    jboolean result = env->CallBooleanMethodV(obj, method, args);
    va_end(args);
    exceptionCheck();
    return result;
  }

  jfloat JavaEnv::CallFloatMethod(jobject obj, jmethodID method, uint n_args, ...) {
    va_list args;
    va_start(args, n_args);
    JNIEnv* env = GetEnv();
    jfloat result = env->CallFloatMethodV(obj, method, args);
    va_end(args);
    exceptionCheck();
    return result;
  }

  jbyte JavaEnv::CallByteMethod(jobject obj, jmethodID method, uint n_args, ...) {
    va_list args;
    va_start(args, n_args);
    JNIEnv* env = GetEnv();
    jbyte result = env->CallByteMethodV(obj, method, args);
    va_end(args);
    exceptionCheck();
    return result;
  }

  jchar JavaEnv::CallCharMethod(jobject obj, jmethodID method, uint n_args, ...) {
    va_list args;
    va_start(args, n_args);
    JNIEnv* env = GetEnv();
    jchar result = env->CallCharMethodV(obj, method, args);
    va_end(args);
    exceptionCheck();
    return result;
  }

  jlong JavaEnv::CallLongMethod(jobject obj, jmethodID method, uint n_args, ...) {
    va_list args;
    va_start(args, n_args);
    JNIEnv* env = GetEnv();
    jlong result = env->CallLongMethodV(obj, method, args);
    va_end(args);
    exceptionCheck();
    return result;
  }

  jshort JavaEnv::CallShortMethod(jobject obj, jmethodID method, uint n_args, ...) {
    va_list args;
    va_start(args, n_args);
    JNIEnv* env = GetEnv();
    jshort result = env->CallShortMethodV(obj, method, args);
    va_end(args);
    exceptionCheck();
    return result;
  }


  // Static Method Invocations

  jobject JavaEnv::CallStaticObjectMethod(jclass c, jmethodID method, uint n_args, ...) {
    va_list args;
    va_start(args, n_args);
    JNIEnv* env = GetEnv();
    jobject result = env->CallStaticObjectMethodV(c, method, args);
    va_end(args);
    exceptionCheck();
    return result;
  }

  jint JavaEnv::CallStaticIntMethod(jclass c, jmethodID method, uint n_args, ...) {
    va_list args;
    va_start(args, n_args);
    JNIEnv* env = GetEnv();
    jint result = env->CallStaticIntMethodV(c, method, args);
    va_end(args);
    exceptionCheck();
    return result;
  }

  jdouble JavaEnv::CallStaticDoubleMethod(jclass c, jmethodID method, uint n_args, ...) {
    va_list args;
    va_start(args, n_args);
    JNIEnv* env = GetEnv();
    jdouble result = env->CallStaticDoubleMethodV(c, method, args);
    va_end(args);
    exceptionCheck();
    return result;
  }

  jboolean JavaEnv::CallStaticBooleanMethod(jclass c, jmethodID method, uint n_args, ...) {
    va_list args;
    va_start(args, n_args);
    JNIEnv* env = GetEnv();
    jboolean result = env->CallStaticBooleanMethodV(c, method, args);
    va_end(args);
    exceptionCheck();
    return result;
  }

  jfloat JavaEnv::CallStaticFloatMethod(jclass c, jmethodID method, uint n_args, ...) {
    va_list args;
    va_start(args, n_args);
    JNIEnv* env = GetEnv();
    jfloat result = env->CallStaticFloatMethodV(c, method, args);
    va_end(args);
    exceptionCheck();
    return result;
  }

  jbyte JavaEnv::CallStaticByteMethod(jclass c, jmethodID method, uint n_args, ...) {
    va_list args;
    va_start(args, n_args);
    JNIEnv* env = GetEnv();
    jbyte result = env->CallStaticByteMethodV(c, method, args);
    va_end(args);
    exceptionCheck();
    return result;
  }

  jchar JavaEnv::CallStaticCharMethod(jclass c, jmethodID method, uint n_args, ...) {
    va_list args;
    va_start(args, n_args);
    JNIEnv* env = GetEnv();
    jchar result = env->CallStaticCharMethodV(c, method, args);
    va_end(args);
    exceptionCheck();
    return result;
  }

  jlong JavaEnv::CallStaticLongMethod(jclass c, jmethodID method, uint n_args, ...) {
    va_list args;
    va_start(args, n_args);
    JNIEnv* env = GetEnv();
    jlong result = env->CallStaticLongMethodV(c, method, args);
    va_end(args);
    exceptionCheck();
    return result;
  }

  jshort JavaEnv::CallStaticShortMethod(jclass c, jmethodID method, uint n_args, ...) {
    va_list args;
    va_start(args, n_args);
    JNIEnv* env = GetEnv();
    jshort result = env->CallStaticShortMethodV(c, method, args);
    va_end(args);
    exceptionCheck();
    return result;
  }

  JNIMethod* JavaEnv::GetMethod(QString className, QString name, QString signature, bool stat) {
    JNIClass* cls = GetClass(className);

    JNIMethod* m = new JNIMethod(cls, name, signature, stat);

    return m;
  }

  JNIMethod* JavaEnv::GetMethod(JNIClass* cls, QString name, QString signature, bool stat) {
    return new JNIMethod(cls, name, signature, stat);
  }

  jvalue_type JavaEnv::ParseReturnValueFromSignature(QString signature) {
    std::string sig = signature.toStdString();
    uint index = sig.find_last_of(')') + 1;
    if (index < sig.length()) {
      char c = sig.at(index);
      switch (c) {
        case 'Z': return JBOOLEAN;
        case 'S': return JSHORT;
        case 'C': return JCHAR;
        case 'V': return JVOID;
        case 'B': return JBYTE;
        case 'L': return JOBJECT;
        case 'I': return JINT;
        case 'J': return JLONG;
        case 'D': return JDOUBLE;
        case 'F': return JFLOAT;
      }
    }
    throw runtime_error("Invalid signature");
  }

  vector<jvalue_type> JavaEnv::ParseArgumentTypesFromSignature(QString signature) {
    std::string sig = signature.toStdString();
    vector<jvalue_type> arguments;
    uint index = 0;
    uint end = sig.find_last_of(')');
    if (end >= sig.length()) {
        throw runtime_error("Invalid signature");
    }
    while (index < end) {
        char c = sig.at(index++);
        switch(c) {
        case 'Z': arguments.push_back(JBOOLEAN); break;
        case 'S': arguments.push_back(JSHORT);   break;
        case 'C': arguments.push_back(JCHAR);   break;
        case 'B': arguments.push_back(JBYTE);   break;
        case 'I': arguments.push_back(JINT); break;
        case 'J': arguments.push_back(JLONG); break;
        case 'D': arguments.push_back(JDOUBLE); break;
        case 'F': arguments.push_back(JFLOAT); break;
        case 'L': {
              arguments.push_back(JOBJECT);
              // For object types, the substring denoting the class should be skipped before continuing.
              index = sig.find_first_of(';', index)+1;
              break;
            }
        default:
            qWarning() << "unrecognized type: " << c;
        }
    }
    return arguments;
  }

  bool JavaEnv::isAttached(QThread* thread) {
    if(thread->isRunning()) {
      return environments.contains(thread);
    } else {
      environments.remove(thread);
      return false;
    }
  }

  void JavaEnv::exceptionCheck() {
    JNIEnv* env = GetEnv();
    jthrowable thrown = env->ExceptionOccurred();
    if (thrown != NULL) {
#ifdef DEBUG
      env->ExceptionDescribe();
#endif
      env->ExceptionClear();
      throw java_exception(thrown, "Exception in method call");
    }
  }

  void JavaEnv::deleteLocalReference(jobject obj) {
    if (obj != NULL) {
      JNIEnv* env = GetEnv();
      if (env != NULL) {
        env->DeleteLocalRef(obj);
      }
    }
  }

  void JavaEnv::syncJavaVMThreads() {
    QList<QThread*> threads = environments.keys();
    for (QList<QThread*>::iterator it = threads.begin(); it != threads.end(); ++it) {
      QThread* key = (*it);
      if (key->isFinished()) {
        environments.remove(key);
      }
    }
  }

  void JavaEnv::unlinkThread() {
    QThread* current = QThread::currentThread();
    if ( environments.remove(current) != 0) {
      jvm->DetachCurrentThread();
    }
  }
}