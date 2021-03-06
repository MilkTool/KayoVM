#include "../../../registry.h"
#include "../../../../kayo.h"
#include "../../../../runtime/Frame.h"
#include "../../../../objects/Class.h"
#include "../../../../objects/Array.h"
#include "../../../../objects/invoke.h"

/*
 * Author: kayo
 */

using namespace utf8;
using namespace method_type;
using namespace member_name;

// static native int getConstant(int which);
static void getConstant(Frame *frame)
{
    jint which = frame->getLocalAsInt(0);
    // todo 啥意思
    if (which == 4)
        frame->pushi(1);
    else
        frame->pushi(0);
}

// static native void init(MemberName self, Object ref);
static void init(Frame *frame)
{
    jvm_abort("init");

    jref self = frame->getLocalAsRef(0);
    jref ref = frame->getLocalAsRef(1);
    /*
	 * in fact, `ref` will be one of these three:
	 * 1. java/lang/reflect/Field.
	 * 2. java/lang/reflect/Constructor.
	 * 3. java/lang/reflect/Method.
	 */
    // private Class<?> clazz;
    auto clazz = ref->getInstFieldValue<jclsref>(S(clazz), S(sig_java_lang_Class));
    // private int modifiers;
    auto modifiers = ref->getInstFieldValue<jint>("modifiers", "I");

    if (equals(ref->clazz->className, "java/lang/reflect/Field")) {

    } else if (equals(ref->clazz->className, "java/lang/reflect/Constructor")) {

    } else if (equals(ref->clazz->className, "java/lang/reflect/Method")) {

    } else {
        jvm_abort("never get here.");
    }
}

/*
 * todo 说明这函数是干嘛的。。。。。。。。。
 * // There are 4 entities in play here:
//   * LC: lookupClass
//   * REFC: symbolic reference class (MN.clazz before resolution);
//   * DEFC: resolved method holder (MN.clazz after resolution);
//   * PTYPES: parameter types (MN.type)
//
// What we care about when resolving a MemberName is consistency between DEFC and PTYPES.
// We do type alias (TA) checks on DEFC to ensure that. DEFC is not known until the JVM
// finishes the resolution, so do TA checks right after MHN.resolve() is over.
//
// All parameters passed by a caller are checked against MH type (PTYPES) on every invocation,
// so it is safe to call a MH from any context.
//
// REFC view on PTYPES doesn't matter, since it is used only as a starting point for resolution and doesn't
// participate in method selection.
 *
 * static native MemberName resolve(MemberName self, Class<?> caller) throws LinkageError, ClassNotFoundException;
 */
static void resolve(Frame *frame)
{
    jref mn = frame->getLocalAsRef(0);
    jref caller = frame->getLocalAsRef(1);

    // todo
    // private Class<?> clazz;       // class in which the method is defined
    // private String   name;        // may be null if not yet materialized
    // private Object   type;        // may be null if not yet materialized
    // private int      flags;       // modifier bits; see reflect.Modifier
    // private Object   resolution;  // if null, this guy is resolved
    auto clazz = mn->getInstFieldValue<jclsref>("clazz", "Ljava/lang/Class;");
    auto name = mn->getInstFieldValue<jstrref>("name", "Ljava/lang/String;");
    // type maybe a String or an Object[] or a MethodType
    // Object[]: (Class<?>) Object[0] is return type
    //           (Class<?>[]) Object[1] is parameter types
    auto type = mn->getInstFieldValue<jref>("type", "Ljava/lang/Object;");
    auto flags = mn->getInstFieldValue<jint>("flags", "I");
    auto resolution = mn->getInstFieldValue<jref>("resolution", "Ljava/lang/Object;");

    auto refKind = getRefKind(mn);

    if (isMethod(mn)) {
        jstrref descriptor = nullptr;

        // TODO "java/lang/invoke/MethodHandle" 及其子类暂时特殊处理，因为取到的type一直是错的，我也不知道为什么？？？？
        if (equals(clazz->className, "java/lang/invoke/MethodHandle") and
                (equals(name->toUtf8(), "invoke")
                 || equals(name->toUtf8(), "invokeBasic")
                 || equals(name->toUtf8(), "invokeExact")
                 || equals(name->toUtf8(), "invokeWithArguments")
                 || equals(name->toUtf8(), "linkToSpecial")
                 || equals(name->toUtf8(), "linkToStatic")
                 || equals(name->toUtf8(), "linkToVirtual")
                 || equals(name->toUtf8(), "linkToInterface"))) {
            descriptor = newString("([Ljava/lang/Object;)Ljava/lang/Object;");
        } else if (equals(clazz->className, "java/lang/invoke/BoundMethodHandle$Species_L") and equals(name->toUtf8(), "make")) {
            descriptor = newString("(Ljava/lang/invoke/MethodType;Ljava/lang/invoke/LambdaForm;Ljava/lang/Object;)Ljava/lang/invoke/BoundMethodHandle;");
        } else if (equals(type->clazz->className, S(java_lang_String))) {
            descriptor = type;
        } else if (equals(type->clazz->className, "java/lang/invoke/MethodType")) {
            descriptor = toMethodDescriptor(type);
        } else if (type->isArrayObject()) {
            auto arr = (jarrref) (type);

            auto rtype = arr->get<jclsref>(0);
            auto ptypes = arr->get<jarrref>(1);
            descriptor = toMethodDescriptor(methodType(rtype, ptypes));
        } else {
            NEVER_GO_HERE_ERROR("");
        }
        assert(descriptor != nullptr);

        jref resolvedMemberName = nullptr;
        if (refKind == REF_invokeStatic) {
//            assert(isStatic(mn));
            Method *m = clazz->getDeclaredStaticMethod(name->toUtf8(), descriptor->toUtf8());
            assert(m->isStatic());
            resolvedMemberName = memberName(m, refKind);
            auto newFlags = flags | Modifier::MOD_STATIC; // todo
            resolvedMemberName->setFieldValue("flags", "I", newFlags); // todo
            assert(isStatic(resolvedMemberName));
        } else {
            jvm_abort("not support!");
        }

        frame->pushr(resolvedMemberName);
        return;
    } else {
        jvm_abort("not support!");
    }

    jvm_abort("not support!");
}

void java_lang_invoke_MethodHandleNatives_registerNatives()
{
#undef C
#define C "java/lang/invoke/MethodHandleNatives"
    registerNative(C, "getConstant", "(I)I", getConstant);
    registerNative(C, "init", "(Ljava/lang/invoke/MemberName;Ljava/lang/Object;)V", init);
    registerNative(C, "resolve",
                   "(Ljava/lang/invoke/MemberName;Ljava/lang/Class;)Ljava/lang/invoke/MemberName;", resolve);
}
