/*
 * Author: Jia Yang
 */

#include "../../registry.h"
#include "../../../rtda/heap/Object.h"
#include "../../../interpreter/interpreter.h"
#include "../../../symbol.h"

// @CallerSensitive
// public static native <T> T doPrivileged(PrivilegedAction<T> action);
static void doPrivileged(Frame *frame)
{
    // todo 这个函数干什么用的。。。。
    jref thisObj = frame_locals_getr(frame, 0);

    /*
     * run 函数返回 T类型 的对象
     *
     * public interface PrivilegedAction<T> {
     *     T run();
     * }
     */
    Method *m = thisObj->clazz->getDeclaredMethod(S(run), S(___java_lang_Object));
    slot_t *s = exec_java_func(m, (slot_t *) &thisObj);
    frame_stack_pushr(frame, RSLOT(s));
}

// @CallerSensitive
// public static native <T> T doPrivileged(PrivilegedAction<T> action, AccessControlContext context);
static void doPrivileged1(Frame *frame)
{
    // todo
    doPrivileged(frame);
}

// @CallerSensitive
// public static native <T> T doPrivileged(PrivilegedExceptionAction<T> action) throws PrivilegedActionException;
static void doPrivileged2(Frame *frame)
{
    // todo
    doPrivileged(frame);
}

// @CallerSensitive
// public static native <T> T doPrivileged(PrivilegedExceptionAction<T> action, AccessControlContext context)
//      throws PrivilegedActionException;
static void doPrivileged3(Frame *frame)
{
    // todo
    doPrivileged(frame);
}

// private static native AccessControlContext getStackAccessControlContext();
static void getStackAccessControlContext(Frame *frame)
{
    // todo
    frame_stack_pushr(frame, nullptr);
}

// static native AccessControlContext getInheritedAccessControlContext();
static void getInheritedAccessControlContext(Frame *frame)
{
    // todo
    jvm_abort("");
}

void java_security_AccessController_registerNatives()
{
#undef C
#define C "java/security/AccessController",
    register_native_method(C"getStackAccessControlContext", "()Ljava/security/AccessControlContext;",
                           getStackAccessControlContext);

    register_native_method(C"getInheritedAccessControlContext", "()Ljava/security/AccessControlContext;",
                           getInheritedAccessControlContext);

    register_native_method(C"doPrivileged", "(Ljava/security/PrivilegedAction;)"LOBJ, doPrivileged);

    register_native_method(C"doPrivileged",
             "(Ljava/security/PrivilegedAction;Ljava/security/AccessControlContext;)"LOBJ,
                         doPrivileged1);

    register_native_method(C"doPrivileged", "(Ljava/security/PrivilegedExceptionAction;)"LOBJ,
                         doPrivileged2);

    register_native_method(C"doPrivileged",
             "(Ljava/security/PrivilegedExceptionAction;Ljava/security/AccessControlContext;)"LOBJ,
                           doPrivileged3);
}
