/*
 * Author: kayo
 */

#include "../../registry.h"
#include "../../../objects/Object.h"
#include "../../../objects/Field.h"
#include "../../../interpreter/interpreter.h"
#include "../../../runtime/Frame.h"
#include "../../../runtime/Thread.h"
#include "../../../objects/Array.h"

using namespace utf8;

/*
 * Called after security check for system loader access checks have been made.
 *
 * private static native Class<?> forName0
 *  (String name, boolean initialize, ClassLoader loader, Class<?> caller) throws ClassNotFoundException;
 */
static void forName0(Frame *frame)
{
    auto name = frame->getLocalAsRef(0)->toUtf8(); // 形如 xxx.xx.xx 的形式
    auto initialize = frame->getLocalAsInt(1);
    auto loader = frame->getLocalAsRef(2);

    Class *c = loadClass(loader, dots2SlashDup(name));
    if (c == nullptr) {
        throw ClassNotFoundException(name);
    }

    if (initialize) {
        initClass(c);
    }

    frame->pushr((jref) c);
}

/*
 * Return the Virtual Machine's Class Object for the named primitive type.
 *
 * static native Class<?> getPrimitiveClass(String name);
 */
static void getPrimitiveClass(Frame *frame)
{
    auto so = frame->getLocalAsRef(0);
    const char *className = so->toUtf8(); // 这里得到的 class name 是诸如 "int, float" 之类的 primitive type
    Class *c = loadBootClass(className);
    frame->pushr(c);
}

/*
 * Returns the name of the entity (class, interface, array class,
 * primitive type, or void) represented by this Class object, as a String.
 *
 * Examples:
 * String.class.getName()
 *     returns "java.lang.String"
 * byte.class.getName()
 *     returns "byte"
 * (new Object[3]).getClass().getName()
 *     returns "[Ljava.lang.Object;"
 * (new int[3][4][5][6][7][8][9]).getClass().getName()
 *     returns "[[[[[[[I"
 *
 * private native String getName0();
 */
static void getName0(Frame *frame)
{
    auto _this = frame->getLocalAsRef<Class>(0);
    frame->pushr(newString(slash2DotsDup(_this->className)));
}

/**
 * Returns the assertion status that would be assigned to this
 * class if it were to be initialized at the time this method is invoked.
 * If this class has had its assertion status set, the most recent
 * setting will be returned; otherwise, if any package default assertion
 * status pertains to this class, the most recent setting for the most
 * specific pertinent package default assertion status is returned;
 * otherwise, if this class is not a system class (i.e., it has a
 * class loader) its class loader's default assertion status is returned;
 * otherwise, the system class default assertion status is returned.
 * <p>
 * Few programmers will have any need for this method; it is provided
 * for the benefit of the JRE itself.  (It allows a class to determine at
 * the time that it is initialized whether assertions should be enabled.)
 * Note that this method is not guaranteed to return the actual
 * assertion status that was (or will be) associated with the specified
 * class when it was (or will be) initialized.
 *
 * @return the desired assertion status of the specified class.
 * @see    java.lang.ClassLoader#setClassAssertionStatus
 * @see    java.lang.ClassLoader#setPackageAssertionStatus
 * @see    java.lang.ClassLoader#setDefaultAssertionStatus
 * @since  1.4
 */
//public boolean desiredAssertionStatus() {
//    ClassLoader loader = getClassLoader();
//    // If the loader is null this is a system class, so ask the VM
//    if (loader == null)
//        return desiredAssertionStatus0(this);
//
//    // If the classloader has been initialized with the assertion
//    // directives, ask it. Otherwise, ask the VM.
//    synchronized(loader.assertionLock) {
//        if (loader.classAssertionStatus != null) {
//            return loader.desiredAssertionStatus(getName());
//        }
//    }
//    return desiredAssertionStatus0(this);
//}
//

/*
 * Retrieves the desired assertion status of this class from the VM
 *
 * private static native boolean desiredAssertionStatus0(Class<?> clazz);
 */
static void desiredAssertionStatus0(Frame *frame)
{
    // todo 本vm不讨论断言。desiredAssertionStatus0（）方法把false推入操作数栈顶
    frame->pushi(0);
}

/**
 * Determines if the specified {@code Object} is assignment-compatible
 * with the object represented by this {@code Class}.  This method is
 * the dynamic equivalent of the Java language {@code instanceof}
 * operator. The method returns {@code true} if the specified
 * {@code Object} argument is non-null and can be cast to the
 * reference type represented by this {@code Class} object without
 * raising a {@code ClassCastException.} It returns {@code false}
 * otherwise.
 *
 * <p> Specifically, if this {@code Class} object represents a
 * declared class, this method returns {@code true} if the specified
 * {@code Object} argument is an instance of the represented class (or
 * of any of its subclasses); it returns {@code false} otherwise. If
 * this {@code Class} object represents an array class, this method
 * returns {@code true} if the specified {@code Object} argument
 * can be converted to an object of the array class by an identity
 * conversion or by a widening reference conversion; it returns
 * {@code false} otherwise. If this {@code Class} object
 * represents an interface, this method returns {@code true} if the
 * class or any superclass of the specified {@code Object} argument
 * implements this interface; it returns {@code false} otherwise. If
 * this {@code Class} object represents a primitive type, this method
 * returns {@code false}.
 *
 * @param   obj the object to check
 * @return  true if {@code obj} is an instance of this class
 *
 * @since JDK1.1
 *
 * public native boolean isInstance(Object obj);
 */
static void isInstance(Frame *frame)
{
    auto _this = frame->getLocalAsRef<Class>(0);

    jref obj = frame->getLocalAsRef(1);
    frame->pushi((obj != nullptr && obj->isInstanceOf(_this)) ? 1 : 0);
}

/**
 * Determines if the class or interface represented by this
 * {@code Class} object is either the same as, or is a superclass or
 * superinterface of, the class or interface represented by the specified
 * {@code Class} parameter. It returns {@code true} if so;
 * otherwise it returns {@code false}. If this {@code Class}
 * object represents a primitive type, this method returns
 * {@code true} if the specified {@code Class} parameter is
 * exactly this {@code Class} object; otherwise it returns
 * {@code false}.
 *
 * <p> Specifically, this method tests whether the type represented by the
 * specified {@code Class} parameter can be converted to the type
 * represented by this {@code Class} object via an identity conversion
 * or via a widening reference conversion. See <em>The Java Language
 * Specification</em>, sections 5.1.1 and 5.1.4 , for details.
 *
 * @param cls the {@code Class} object to be checked
 * @return the {@code boolean} value indicating whether objects of the
 * type {@code cls} can be assigned to objects of this class
 * @exception NullPointerException if the specified Class parameter is null.
 * @since JDK1.1
 *
 * public native boolean isAssignableFrom(Class<?> cls);
 */
static void isAssignableFrom(Frame *frame)
{
    auto _this = frame->getLocalAsRef<Class>(0);
    auto cls = frame->getLocalAsRef<Class>(1);
    if (cls == nullptr) {
        thread_throw(new NullPointerException);
    }

    bool b = cls->isSubclassOf(_this);
    frame->pushi(b ? 1 : 0);
}

/*
 * Determines if the specified class Object represents an interface type.
 *
 * public native boolean isInterface();
 */
static void isInterface(Frame *frame)
{
    auto _this = frame->getLocalAsRef<Class>(0);
    frame->pushi(_this->isInterface() ? 1 : 0);
}

/*
 * Determines if this class Object represents an array class.
 *
 * public native boolean isArray();
 */
static void isArray(Frame *frame)
{
    auto _this = frame->getLocalAsRef<Class>(0);
    frame->pushi(_this->isArrayClass() ? 1 : 0);  // todo
}

// public native boolean isPrimitive();
static void isPrimitive(Frame *frame)
{
    auto _this = frame->getLocalAsRef<Class>(0);
    bool b = _this->isPrimClass();
    frame->pushi(b ? 1 : 0);
}

/**
 * Returns the {@code Class} representing the superclass of the entity
 * (class, interface, primitive type or void) represented by this
 * {@code Class}.  If this {@code Class} represents either the
 * {@code Object} class, an interface, a primitive type, or void, then  todo  interface 和 primitive type 的父类是不是 Object
 * null is returned.  If this object represents an array class then the
 * {@code Class} object representing the {@code Object} class is
 * returned.
 *
 * @return the superclass of the class represented by this object.
 *
 * public native Class<? super T> getSuperclass();
 */
static void getSuperclass(Frame *frame)
{
    auto _this = frame->getLocalAsRef<Class>(0);

    Class *c = loadClass(frame->method->clazz->loader, _this->className);
    frame->pushr(c->superClass != nullptr ? c->superClass : nullptr);
}

/**
 * Determines the interfaces implemented by the class or interface
 * represented by this object.
 *
 * <p> If this object represents a class, the return value is an array
 * containing objects representing all interfaces implemented by the
 * class. The order of the interface objects in the array corresponds to
 * the order of the interface names in the {@code implements} clause
 * of the declaration of the class represented by this object. For
 * example, given the declaration:
 * <blockquote>
 * {@code class Shimmer implements FloorWax, DessertTopping { ... }}
 * </blockquote>
 * suppose the value of {@code s} is an instance of
 * {@code Shimmer}; the value of the expression:
 * <blockquote>
 * {@code s.getClass().getInterfaces()[0]}
 * </blockquote>
 * is the {@code Class} object that represents interface
 * {@code FloorWax}; and the value of:
 * <blockquote>
 * {@code s.getClass().getInterfaces()[1]}
 * </blockquote>
 * is the {@code Class} object that represents interface
 * {@code DessertTopping}.
 *
 * <p> If this object represents an interface, the array contains objects
 * representing all interfaces extended by the interface. The order of the
 * interface objects in the array corresponds to the order of the interface
 * names in the {@code extends} clause of the declaration of the
 * interface represented by this object.
 *
 * <p> If this object represents a class or interface that implements no
 * interfaces, the method returns an array of length 0.
 *
 * <p> If this object represents a primitive type or void, the method
 * returns an array of length 0.
 *
 * <p> If this {@code Class} object represents an array type, the
 * interfaces {@code Cloneable} and {@code java.io.Serializable} are
 * returned in that order.
 *
 * @return an array of interfaces implemented by this class.
 */
//private native Class<?>[] getInterfaces0();
static void getInterfaces0(Frame *frame)
{
    auto _this = frame->getLocalAsRef<Class>(0);

    auto interfaces = newArray(loadBootClass(S(array_java_lang_Class)), _this->interfaces.size());
    for (size_t i = 0; i < _this->interfaces.size(); i++) {
        assert(_this->interfaces[i] != nullptr);
        interfaces->set(i, _this->interfaces[i]);
    }

    frame->pushr(interfaces);
}

/*
 * Returns the representing the component type of an array.
 * If this class does not represent an array class this method returns null.
 *
 * like:
 * [[I -> [I -> int -> null
 *
 * public native Class<?> getComponentType();
 */
static void getComponentType(Frame *frame)
{
    auto _this = frame->getLocalAsRef<Class>(0);

    if (_this->isArrayClass()) {
        frame->pushr(_this->componentClass());
    } else {
        frame->pushr(nullptr);
    }
}

/**
 * Returns the Java language modifiers for this class or interface, encoded
 * in an integer. The modifiers consist of the Java Virtual Machine's
 * constants for {@code public}, {@code protected},
 * {@code private}, {@code final}, {@code static},
 * {@code abstract} and {@code interface}; they should be decoded
 * using the methods of class {@code Modifier}.
 *
 * <p> If the underlying class is an array class, then its
 * {@code public}, {@code private} and {@code protected}
 * modifiers are the same as those of its component type.  If this
 * {@code Class} represents a primitive type or void, its
 * {@code public} modifier is always {@code true}, and its
 * {@code protected} and {@code private} modifiers are always
 * {@code false}. If this object represents an array class, a
 * primitive type or void, then its {@code final} modifier is always
 * {@code true} and its interface modifier is always
 * {@code false}. The values of its other modifiers are not determined
 * by this specification.
 *
 * <p> The modifier encodings are defined in <em>The Java Virtual Machine
 * Specification</em>, table 4.1.
 *
 * @return the {@code int} representing the modifiers for this class
 * @see     java.lang.reflect.Modifier
 * @since JDK1.1
 */
//public native int getModifiers();
static void getModifiers(Frame *frame)
{
    auto _this = frame->getLocalAsRef<Class>(0);
    frame->pushi(_this->modifiers);
}

/*
 * Gets the signers of this class.
 *
 * the signers of this class, or null if there are no signers.
 * In particular, this method returns null
 * if this object represents a primitive type or void.
 *
 * public native Object[] getSigners();
 */
static void getSigners(Frame *frame)
{
    auto _this = frame->getLocalAsRef<Class>(0);

    jvm_abort("getSigners");
}


/**
 * Set the signers of this class.
 */
// native void setSigners(Object[] signers);
static void setSigners(Frame *frame)
{
    jvm_abort("setSigners");
}

// private native Object[] getEnclosingMethod0();
static void getEnclosingMethod0(Frame *frame)
{
    auto _this = frame->getLocalAsRef<Class>(0);

    if (_this->enclosing.clazz == nullptr) {
        frame->pushr(nullptr);
        return;
    }

    auto result = newArray(loadBootClass(S(array_java_lang_Object)), 3);
    result->set(0, _this->enclosing.clazz);
    result->set(1, _this->enclosing.name);
    result->set(2, _this->enclosing.descriptor);

    frame->pushr(result);
}

/*
 * Returns the ProtectionDomain of this class.
 */
//private native java.security.ProtectionDomain getProtectionDomain0();
static void getProtectionDomain0(Frame *frame)
{
    jvm_abort("getProtectionDomain0");
}

// Generic signature handling
//private native String getGenericSignature0();
static void getGenericSignature0(Frame *frame)
{
    jvm_abort("getGenericSignature0");
}

// Annotations handling
//native byte[] getRawAnnotations();
static void getRawAnnotations(Frame *frame)
{
    jvm_abort("getRawAnnotations");
}

// native byte[] getRawTypeAnnotations();
static void getRawTypeAnnotations(Frame *frame)
{
    jvm_abort("getRawTypeAnnotations");
}

// native ConstantPool getConstantPool();
static void getConstantPool(Frame *frame)
{
    auto _this = frame->getLocalAsRef<Class>(0);

    Class *cpClass = loadBootClass("sun/reflect/ConstantPool");
    jref cp = newObject(cpClass);
    cp->setFieldValue("constantPoolOop", "Ljava/lang/Object;", (slot_t) &_this->cp);

    frame->pushr(cp);
}

// private native Field[] getDeclaredFields0(boolean publicOnly);
static void getDeclaredFields0(Frame *frame)
{
    auto _this = frame->getLocalAsRef<Class>(0);
    bool publicOnly = frame->getLocalAsBool(1);

    Class *cls = loadClass(frame->method->clazz->loader, _this->className);
    jint fieldsCount = publicOnly ? cls->publicFieldsCount : cls->fields.size();

    Class *fieldClass = loadBootClass(S(java_lang_reflect_Field));
    auto fieldArr = newArray(fieldClass->arrayClass(), fieldsCount);
    frame->pushr(fieldArr);

    /*
     * Field(Class<?> declaringClass, String name, Class<?> type,
     *      int modifiers, int slot, String signature, byte[] annotations)
     */
    Method *constructor = fieldClass->getConstructor(
            "(Ljava/lang/Class;" "Ljava/lang/String;" "Ljava/lang/Class;" "II" "Ljava/lang/String;" "[B)V");

    // invoke constructor of class java/lang/reflect/Field
    for (int i = 0; i < fieldsCount; i++) {
        Object *o = newObject(fieldClass);
        fieldArr->set(i, o);

        execJavaFunc(constructor, {
                (slot_t) o, // this
                (slot_t) _this, // declaring class
                // name must be interned.
                // 参见 java/lang/reflect/Field 的说明
                (slot_t) stringClass->intern(cls->fields[i]->name), // name
                (slot_t) cls->fields[i]->getType(), // type
                cls->fields[i]->modifiers, /* modifiers */
                (slot_t) cls->fields[i]->id, /* slot   todo */
                (slot_t) nullptr, /* signature  todo */
                (slot_t) nullptr, /* annotations  todo */
        });
    }
}

/*
 * 注意 getDeclaredMethods 和 getMethods 方法的不同。
 * getDeclaredMethods(),该方法是获取本类中的所有方法，包括私有的(private、protected、默认以及public)的方法。
 * getMethods(),该方法是获取本类以及父类或者父接口中所有的公共方法(public修饰符修饰的)
 *
 * getDeclaredMethods 强调的是本类中定义的方法，不包括继承而来的。
 *
 * private native Method[] getDeclaredMethods0(boolean publicOnly);
 */
static void getDeclaredMethods0(Frame *frame)
{
    auto _this = frame->getLocalAsRef<Class>(0);
    bool publicOnly = frame->getLocalAsBool(1);

    Class *cls = loadClass(frame->method->clazz->loader, _this->className);
    jint methodsCount = publicOnly ? cls->publicMethodsCount : cls->methods.size();

    Class *methodClass = loadBootClass(S(java_lang_reflect_Method));
    auto methodArr = newArray(methodClass->arrayClass(), methodsCount);
    frame->pushr(methodArr);

    /*
     * Method(Class<?> declaringClass, String name, Class<?>[] parameterTypes, Class<?> returnType,
     *      Class<?>[] checkedExceptions, int modifiers, int slot, String signature,
     *      byte[] annotations, byte[] parameterAnnotations, byte[] annotationDefault)
     */
    Method *constructor = methodClass->getConstructor(
                "(Ljava/lang/Class;" "Ljava/lang/String;" "[Ljava/lang/Class;" "Ljava/lang/Class;"
                "[Ljava/lang/Class;" "II" "Ljava/lang/String;" "[B[B[B)V");

    for (int i = 0; i < methodsCount; i++) {
        auto method = cls->methods[i];
        Object *o = newObject(methodClass);
        methodArr->set(i, o);

        execJavaFunc(constructor, {
                (slot_t) o,        /* this  */
                (slot_t) _this, /* declaring class */
                // name must be interned.
                // 参见 java/lang/reflect/Method 的说明
                (slot_t) stringClass->intern(method->name), /* name */
                (slot_t) method->getParameterTypes(), /* parameter types */
                (slot_t) method->getReturnType(),     /* return type */
                (slot_t) method->getExceptionTypes(), /* checked exceptions */
                method->modifiers, /* modifiers*/
                0, /* slot   todo */
                (slot_t) nullptr, /* signature  todo */
                (slot_t) nullptr, /* annotations  todo */
                (slot_t) nullptr, /* parameter annotations  todo */
                (slot_t) nullptr, /* annotation default  todo */
        });
    }
}

// private native Constructor<T>[] getDeclaredConstructors0(boolean publicOnly);
static void getDeclaredConstructors0(Frame *frame)
{
    auto _this = frame->getLocalAsRef<Class>(0);
    bool publicOnly = frame->getLocalAsBool(1);

    Class *cls = loadClass(frame->method->clazz->loader, _this->className);

    std::vector<Method *> constructors = cls->getConstructors(publicOnly);
    int constructorsCount = constructors.size();

    Class *constructorClass = loadBootClass("java/lang/reflect/Constructor");
    auto constructorArr = newArray(constructorClass->arrayClass(), constructorsCount);
    frame->pushr(constructorArr);

    /*
     * Constructor(Class<T> declaringClass, Class<?>[] parameterTypes,
     *      Class<?>[] checkedExceptions, int modifiers, int slot,
     *      String signature, byte[] annotations, byte[] parameterAnnotations)
     */
    Method *constructor_constructor = constructorClass->getConstructor(
                "(Ljava/lang/Class;" "[Ljava/lang/Class;" "[Ljava/lang/Class;" "II" "Ljava/lang/String;" "[B[B)V");

    // invoke constructor of class java/lang/reflect/Constructor
    for (int i = 0; i < constructorsCount; i++) {
        auto constructor = constructors[i];
        struct Object *o = newObject(constructorClass);
        constructorArr->set(i, o);

        execJavaFunc(constructor_constructor, {
                (slot_t) o, // this
                (slot_t) _this, // declaring class
                (slot_t) constructor->getParameterTypes(),  // parameter types
                (slot_t) constructor->getExceptionTypes(),  // checked exceptions
                constructor->modifiers, // modifiers
                0, // slot   todo
                (slot_t) nullptr, // signature  todo
                (slot_t) nullptr, // annotations  todo
                (slot_t) nullptr, // parameter annotations  todo
        });
    }
}

/*
 * getClasses和getDeclaredClasses的区别：
 * getClasses得到该类及其父类所有的public的内部类。
 * getDeclaredClasses得到该类所有的内部类，除去父类的。
 *
 * private native Class<?>[] getDeclaredClasses0();
 */
static void getDeclaredClasses0(Frame *frame)
{
    jvm_abort("getDeclaredClasses0");
}

/**
 * If the class or interface represented by this {@code Class} object
 * is a member of another class, returns the {@code Class} object
 * representing the class in which it was declared.  This method returns
 * null if this class or interface is not a member of any other class.  If
 * this {@code Class} object represents an array class, a primitive
 * type, or void,then this method returns null.
 *
 * 如果此类为内部类，返回其外部类
 *
 * @return the declaring class for this class
 * @throws SecurityException
 *         If a security manager, <i>s</i>, is present and the caller's
 *         class loader is not the same as or an ancestor of the class
 *         loader for the declaring class and invocation of {@link
 *         SecurityManager#checkPackageAccess s.checkPackageAccess()}
 *         denies access to the package of the declaring class
 * @since JDK1.1
 *
 * private native Class<?> getDeclaringClass0();
 */
static void getDeclaringClass0(Frame *frame)
{
    Class *_this = (frame->getLocalAsRef<Class>(0));
    if (_this->isArrayClass()) {
        frame->pushr(nullptr);
        return;
    }

    char buf[strlen(_this->className) + 1];
    strcpy(buf, _this->className);
    char *last_dollar = strrchr(buf, '$'); // 内部类标识：out_class_name$inner_class_name
    if (last_dollar == nullptr) {
        frame->pushr(nullptr);
    } else {
        *last_dollar = 0;
        frame->pushr(loadClass(frame->method->clazz->loader, buf));
    }
}

void java_lang_Class_registerNatives()
{
#undef C
#define C "java/lang/Class",
    registerNative(C "getPrimitiveClass", "(Ljava/lang/String;)Ljava/lang/Class;", getPrimitiveClass);
    registerNative(C "getName0", "()Ljava/lang/String;", getName0);
    registerNative(C "forName0",
                   "(Ljava/lang/String;ZLjava/lang/ClassLoader;Ljava/lang/Class;)Ljava/lang/Class;", forName0);
    registerNative(C "desiredAssertionStatus0", "(Ljava/lang/Class;)Z", desiredAssertionStatus0);

    registerNative(C "isInstance", "(Ljava/lang/Object;)Z", isInstance);
    registerNative(C "isAssignableFrom", "(Ljava/lang/Class;)Z", isAssignableFrom);
    registerNative(C "isInterface", "()Z", isInterface);
    registerNative(C "isArray", "()Z", isArray);
    registerNative(C "isPrimitive", "()Z", isPrimitive);

    registerNative(C "getSuperclass", "()" LCLS, getSuperclass);
    registerNative(C "getInterfaces0", "()[Ljava/lang/Class;", getInterfaces0);
    registerNative(C "getComponentType", "()Ljava/lang/Class;", getComponentType);
    registerNative(C "getModifiers", "()I", getModifiers);
    registerNative(C "getEnclosingMethod0", "()[Ljava/lang/Object;", getEnclosingMethod0);
    registerNative(C "getDeclaringClass0", "()" LCLS, getDeclaringClass0);
    registerNative(C "getGenericSignature0", "()" LSTR, getGenericSignature0);
    registerNative(C "getProtectionDomain0", "()Ljava/security/ProtectionDomain;", getProtectionDomain0);
    registerNative(C "getConstantPool", "()Lsun/reflect/ConstantPool;", getConstantPool);

    registerNative(C "getSigners", "()[Ljava/lang/Object;", getSigners);
    registerNative(C "setSigners", "([Ljava/lang/Object;)V", setSigners);

    registerNative(C "getRawAnnotations", "()[B", getRawAnnotations);
    registerNative(C "getRawTypeAnnotations", "()[B", getRawTypeAnnotations);

    registerNative(C "getDeclaredFields0", "(Z)[Ljava/lang/reflect/Field;", getDeclaredFields0);
    registerNative(C "getDeclaredMethods0", "(Z)[Ljava/lang/reflect/Method;", getDeclaredMethods0);
    registerNative(C "getDeclaredConstructors0", "(Z)[Ljava/lang/reflect/Constructor;", getDeclaredConstructors0);
    registerNative(C "getDeclaredClasses0", "()[Ljava/lang/Class;", getDeclaredClasses0);
}
