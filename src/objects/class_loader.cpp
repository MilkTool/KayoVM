/*
 * Author: kayo
 */

#include <memory>
#include <sstream>
#include <unordered_set>
#include <unordered_map>
#include "class_loader.h"
#include "../symbol.h"
#include "Class.h"
#include "Array.h"
#include "../interpreter/interpreter.h"
#include "../../zlib/minizip/unzip.h"
#include "../runtime/Thread.h"
#include "Prims.h"

using namespace std;
using namespace utf8;


#if TRACE_LOAD_CLASS
#define TRACE PRINT_TRACE
#else
#define TRACE(...)
#endif

using class_map = unordered_map<const utf8_t *, Class *, utf8::Hash, utf8::Comparator>;

static utf8_set bootPackages;
static class_map bootClasses;

// 保存每个class loader（不包括 boot class loader）的 classes
// pair.first: class loader
// pair.second: the classes loaded by @key class loader.
static vector<pair<Object *, class_map>> classes;

static unique_ptr<pair<u1 *, size_t>> read_class_from_jar(const char *jar_path, const char *class_name)
{
    unzFile jar_file = unzOpen64(jar_path);
    if (jar_file == nullptr) {
        thread_throw(new IOException(NEW_MSG("unzOpen64 failed: %s\n", jar_path)));
    }

    if (unzGoToFirstFile(jar_file) != UNZ_OK) {
        unzClose(jar_file);
        thread_throw(new IOException(NEW_MSG("unzGoToFirstFile failed: %s\n", jar_path)));
    }

    char buf[strlen(class_name) + 8];
    strcat(strcpy(buf, class_name), ".class");

    int k = unzLocateFile(jar_file, buf, 1);
    if (k != UNZ_OK) {
        // not found
        unzClose(jar_file);
        return nullptr;
    }

    // find out!
    if (unzOpenCurrentFile(jar_file) != UNZ_OK) {
        unzClose(jar_file);
        thread_throw(new IOException(NEW_MSG("unzOpenCurrentFile failed: %s\n", jar_path)));
    }

    unz_file_info64 file_info;
    unzGetCurrentFileInfo64(jar_file, &file_info, buf, sizeof(buf), nullptr, 0, nullptr, 0);

    auto bytecode = (u1 *) g_heap.allocBytecode(file_info.uncompressed_size); //new u1[file_info.uncompressed_size];
    int size = unzReadCurrentFile(jar_file, bytecode, (unsigned int)(file_info.uncompressed_size));
    unzCloseCurrentFile(jar_file);
    unzClose(jar_file);
    if (size != file_info.uncompressed_size) {
        thread_throw(new IOException(NEW_MSG("unzReadCurrentFile failed: %s.\n", jar_path)));
    }
    return make_unique<pair<u1 *, size_t>>(bytecode, file_info.uncompressed_size);
}

void addClassToClassLoader(Object *classLoader, Class *c)
{
    if (classLoader == bootClassLoader) {
        bootClasses.insert(make_pair(c->className, c));
        return;
    }

    // Invoked by the VM to record every loaded class with this loader.
    // void addClass(Class<?> c);
//    Method *m = classLoader->clazz->getDeclaredInstMethod("addClass", "Ljava/lang/Class;");
//    assert(m != nullptr);
//    execJavaFunc(m, { (slot_t) classLoader, (slot_t) c });

    for (auto &p : classes) {
        if (p.first == classLoader) {
            p.second.insert(make_pair(c->className, c));
            return;
        }
    }

    // not find this classLoader, add classLoader first.
    classes.emplace_back(make_pair(classLoader, class_map()));
    classes.back().second.insert(make_pair(c->className, c));
}

Class *loadBootClass(const utf8_t *name)
{
    assert(name != nullptr);
    //assert(isSlashName(name));

    auto iter = bootClasses.find(name);
    if (iter != bootClasses.end()) {
        TRACE("find loaded class (%s) from pool.", className);
        return iter->second;
    }

    Class *c = nullptr;
    if (name[0] == '[' or Prims::isPrimClassName(name)) {
        c = Class::newClass(name);
    } else {
        for (auto &jar : jreLibJars) {
            auto content = read_class_from_jar(jar.c_str(), name);
            if (content) { // find out
                c = defineClass(bootClassLoader, content->first, content->second);
            }
        }
    }

    if (c != nullptr) {
        bootPackages.insert(c->pkgName);
        addClassToClassLoader(bootClassLoader, c);
    }
    return c;
}

const utf8_t *getBootPackage(const utf8_t *name)
{
    auto iter = bootPackages.find(name);
    return iter != bootPackages.end() ? *iter : nullptr;
}

utf8_set &getBootPackages()
{
    return bootPackages;
}

Class *findLoadedClass(Object *classLoader, const utf8_t *name)
{
    assert(name != nullptr);
    //assert(isSlashName(name));

    if (classLoader == nullptr) {
        auto iter = bootClasses.find(name);
        return iter != bootClasses.end() ? iter->second : nullptr;
    }

    // is not boot classLoader
    for (auto &p : classes) {
        if (p.first == classLoader) {
            auto iter = p.second.find(name);
            return iter != p.second.end() ? iter->second : nullptr;
        }
    }

    // not find
    return nullptr;
}

Class *loadClass(Object *classLoader, const utf8_t *name)
{
    assert(name != nullptr);
    //assert(isSlashName(name));

    auto slashName = dots2SlashDup(name);
    Class *c = findLoadedClass(classLoader, slashName);
    if (c != nullptr)
        return c;

    // 先尝试用boot class loader load the class
    c = loadBootClass(slashName);
    if (c != nullptr || classLoader == nullptr)
        return c;

    // todo 再尝试用扩展classLoader load the class

    // public Class<?> loadClass(String name) throws ClassNotFoundException
    Method *m = classLoader->clazz->lookupInstMethod("loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
    assert(m != nullptr);

    auto dotName = slash2DotsDup(name);
    slot_t *slot = execJavaFunc(m, classLoader, newString(dotName));
    assert(slot != nullptr);
    c =  *(Class **) slot;
    addClassToClassLoader(classLoader, c);
    return c;
}

Class *defineClass(jref classLoader, u1 *bytecode, size_t len)
{
    return Class::newClass(classLoader, bytecode, len);
}

Class *defineClass(jref classLoader, jstrref name,
                   jarrref bytecode, jint off, jint len, jref protectionDomain, jstrref source)
{
    auto data = (u1 *) bytecode->data;
    return defineClass(classLoader, data + off, len);
}

Class *initClass(Class *c)
{
    assert(c != nullptr);

    // todo
    c->clinit();

    c->state = Class::INITED;
    return c;
}

Class *linkClass(Class *c)
{
    assert(c != nullptr);

    // todo

    c->state = Class::LINKED;
    return c;
}

Object *getSystemClassLoader()
{
    Class *scl = loadBootClass(S(java_lang_ClassLoader));
    assert(scl != nullptr);

    // public static ClassLoader getSystemClassLoader();
    Method *get = scl->getDeclaredStaticMethod(S(getSystemClassLoader), S(___java_lang_ClassLoader));
    return (Object *) *execJavaFunc(get);
}

Class *objectClass;
Class *classClass = nullptr;
Class *stringClass;

void initClassLoader()
{
    objectClass = loadBootClass(S(java_lang_Object));
    classClass = loadBootClass(S(java_lang_Class));

    if (classClass->instFieldsCount != CLASS_CLASS_INST_FIELDS_COUNT) {
        jvm_abort("What the fuck! java/lang/Class.java 文件有变动？！"); // todo
    }

    objectClass->clazz = classClass->clazz = classClass;

    stringClass = loadBootClass(S(java_lang_String));
    stringClass->buildStrPool();
}

void printBootClassLoader()
{
    printvm("boot class loader.\n");
    for (auto iter : bootClasses) {
        printvm("%s\n", iter.first);
    }
    printvm("\n");
}

void printClassLoader(Object *classLoader)
{
    if (classLoader == nullptr) {
        printBootClassLoader();
        return;
    }

    for (auto iter : classes) {
        if (iter.first == classLoader) {
            printvm("class loader %p.\n", classLoader);
            for (auto k : iter.second) {
                printvm("%s\n", k.first);
            }
            printvm("\n");
        }
    }
}

void printAllClassLoaders()
{
    printBootClassLoader();

    for (auto iter : classes) {
        printvm("class loader %p.\n", iter.first);
        for (auto k : iter.second) {
            printvm("%s\n", k.first);
        }
        printvm("\n");
    }
}

