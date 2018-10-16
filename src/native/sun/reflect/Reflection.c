/*
 * Author: Jia Yang
 */

#include "../../registry.h"
#include "../../../rtda/heap/jclassobj.h"

//// public static native Class<?> getCallerClass(int level)
//static void getCallerClass(StackFrame *frame) {
//    // todo
//}

// public static native Class<?> getCallerClass()
static void getCallerClass(struct stack_frame *frame)
{
    // todo
//    JClassObj *co = frame->method->jclass->getClassObj();
//    frame->operandStack.push(co);



    // top0 is sun/reflect/Reflection
    // top1 is the caller of getCallerClass()
    // top2 is the caller of method
//    callerFrame := frame.Thread().TopFrameN(2)
//    callerClass := callerFrame.Method().Class().JClass()
//    frame.OperandStack().PushRef(callerClass)

#if 0
    // todo
    auto &thread = frame->thread;
    auto top0 = thread->topFrame();
    thread->popFrame();
    auto top1 = thread->topFrame();
    thread->popFrame();
    auto callerFrame = thread->topFrame();
    frame->operandStack.push(callerFrame->method->jclass->getClassObj());

    thread->pushFrame(top1);
    thread->pushFrame(top0);
#endif
}

// public static native int getClassAccessFlags(Class<?> type)
static void getClassAccessFlags(struct stack_frame *frame)
{
    struct jclassobj *type = (struct jclassobj *) slot_getr(frame->local_vars);
    os_pushi(frame->operand_stack, type->obj->jclass->access_flags);
}

void sun_reflect_Reflection_registerNatives(struct stack_frame *frame)
{
//    register_native_method("sun/reflect/Reflection", "getCallerClass", "(I)Ljava/lang/Class;", getCallerClass);
    register_native_method("sun/reflect/Reflection", "getCallerClass", "()Ljava/lang/Class;", getCallerClass);
    register_native_method("sun/reflect/Reflection", "getClassAccessFlags", "(Ljava/lang/Class;)I", getClassAccessFlags);
}