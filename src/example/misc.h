#ifndef __MISC_H__
#define __MISC_H__

/**
 * Function helpers to generate FREE_ON_EXIT for C-style functions
 */
template <typename T>
struct finally_s
{
    T t;
    finally_s(T t) : t(t) {}
    ~finally_s() { t(); }
};

template <typename T>
finally_s<T> finally(T t) { return finally_s<T>{t}; }

#endif /* __MISC_H__ */