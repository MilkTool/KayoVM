/*
 * Author: Jia Yang
 */

#ifndef JVM_SYMREF_H
#define JVM_SYMREF_H

/*
 * 定义及解析类，字段，方法的符号引用
 */

struct field_ref {
    const char *class_name;
    const char *name;
    const char *descriptor;

    struct jclass *resolved_class;
    struct jfield *resolved_field;
};

struct method_ref {
    const char *class_name;
    const char *name;
    const char *descriptor;

    struct jclass *resolved_class;
    struct jmethod *resolved_method;
};

struct jclass* resolve_class(const struct jclass *visitor, const char *class_name);

struct jfield* resolve_static_field_ref(const struct jclass *visitor, struct field_ref *ref);
struct jfield* resolve_non_static_field_ref(const struct jclass *visitor, struct field_ref *ref);

struct jmethod* resolve_static_method_ref(const struct jclass *visitor, struct method_ref *ref);
struct jmethod* resolve_non_static_method_ref(const struct jclass *visitor, struct method_ref *ref);


#endif //JVM_SYMREF_H