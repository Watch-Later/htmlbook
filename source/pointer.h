#ifndef POINTER_H
#define POINTER_H

#include <memory_resource>
#include <algorithm>
#include <cstdint>
#include <cassert>

namespace htmlbook {

using Heap = std::pmr::monotonic_buffer_resource;

class HeapMember {
public:
    static void* operator new(size_t size, Heap* heap) { return heap->allocate(size); }
    static void operator delete(void* data) {}
};

template<typename T>
class RefCounted {
public:
    RefCounted() = default;

    void ref() { ++m_refCount; }
    void deref() {
        if(--m_refCount == 0) {
            delete static_cast<T*>(this);
        }
    }

    uint32_t refCount() const { return m_refCount; }
    bool hasOneRefCount() const { return m_refCount == 1; }

private:
    uint32_t m_refCount{1};
};

template<typename T>
inline void refIfNotNull(T* ptr)
{
    if(ptr)
        ptr->ref();
}

template<typename T>
inline void derefIfNotNull(T* ptr)
{
    if(ptr)
        ptr->deref();
}

template<typename T> class RefPtr;
template<typename T> RefPtr<T> adoptPtr(T*);

template<typename T>
class RefPtr {
public:
    RefPtr() = default;
    RefPtr(std::nullptr_t) : m_ptr(nullptr) {}
    RefPtr(T* ptr) : m_ptr(ptr) { refIfNotNull(m_ptr); }
    RefPtr(T& ref) : m_ptr(&ref) { m_ptr->ref(); }
    RefPtr(const RefPtr<T>& p) : m_ptr(p.get()) { refIfNotNull(m_ptr); }
    RefPtr(RefPtr<T>&& p) : m_ptr(p.release()) {}

    template<typename U>
    RefPtr(const RefPtr<U>& p) : m_ptr(p.get()) { refIfNotNull(m_ptr); }

    template<typename U>
    RefPtr(RefPtr<U>&& p) : m_ptr(p.release()) {}

    ~RefPtr() { derefIfNotNull(m_ptr); }

    T* get() const { return m_ptr; }
    T& operator*() const { return *m_ptr; }
    T* operator->() const { return m_ptr; }

    bool empty() const { return !m_ptr; }
    bool operator!() const { return !m_ptr; }
    operator bool() const { return !!m_ptr; }

    RefPtr<T>& operator=(std::nullptr_t)
    {
        clear();
        return *this;
    }

    RefPtr<T>& operator=(T* o)
    {
        RefPtr<T> p = o;
        swap(p);
        return *this;
    }

    RefPtr<T>& operator=(T& o)
    {
        RefPtr<T> p = o;
        swap(p);
        return *this;
    }

    RefPtr<T>& operator=(const RefPtr<T>& o)
    {
        RefPtr<T> p = o;
        swap(p);
        return *this;
    }

    RefPtr<T>& operator=(RefPtr<T>&& o)
    {
        RefPtr<T> p = std::move(o);
        swap(p);
        return *this;
    }

    template<typename U>
    RefPtr<T>& operator=(const RefPtr<U>& o)
    {
        RefPtr<T> p = o;
        swap(p);
        return *this;
    }

    template<typename U>
    RefPtr<T>& operator=(RefPtr<U>&& o)
    {
        RefPtr<T> p = std::move(o);
        swap(p);
        return *this;
    }

    void swap(RefPtr<T>& o)
    {
        std::swap(m_ptr, o.m_ptr);
    }

    T* release()
    {
        T* ptr = m_ptr;
        m_ptr = nullptr;
        return ptr;
    }

    void clear()
    {
        derefIfNotNull(m_ptr);
        m_ptr = nullptr;
    }

    friend RefPtr<T> adoptPtr<T>(T*);

private:
    RefPtr(T* ptr, std::nullptr_t) : m_ptr(ptr) {}
    T* m_ptr{nullptr};
};

template<typename T>
inline RefPtr<T> adoptPtr(T* ptr)
{
    return RefPtr<T>(ptr, nullptr);
}

template<class T>
inline void swap(RefPtr<T>& a, RefPtr<T>& b)
{
    a.swap(b);
}

template<typename T, typename U>
inline bool operator==(const RefPtr<T>& a, const RefPtr<U>& b)
{
    return a.get() == b.get();
}

template<typename T, typename U>
inline bool operator==(const RefPtr<T>& a, const U* b)
{
    return a.get() == b;
}

template<typename T, typename U>
inline bool operator==(const T* a, const RefPtr<U>& b)
{
    return a == b.get();
}

template<typename T>
inline bool operator==(const RefPtr<T>& a, std::nullptr_t)
{
    return a.get() == nullptr;
}

template<typename T, typename U>
inline bool operator!=(const RefPtr<T>& a, const RefPtr<U>& b)
{
    return a.get() != b.get();
}

template<typename T, typename U>
inline bool operator!=(const RefPtr<T>& a, const U* b)
{
    return a.get() != b;
}

template<typename T, typename U>
inline bool operator!=(const T* a, const RefPtr<U>& b)
{
    return a != b.get();
}

template<typename T>
inline bool operator!=(const RefPtr<T>& a, std::nullptr_t)
{
    return a.get() != nullptr;
}

template<typename T>
struct is_a {
    template<typename U>
    static bool check(const U& value);
};

template<typename T, typename U>
constexpr bool is(U& value) {
    return is_a<T>::check(value);
}

template<typename T, typename U>
constexpr bool is(const U& value) {
    return is_a<T>::check(value);
}

template<typename T, typename U>
constexpr bool is(U* value) {
    return value && is_a<T>::check(*value);
}

template<typename T, typename U>
constexpr bool is(const U* value) {
    return value && is_a<T>::check(*value);
}

template<typename T, typename U>
constexpr bool is(const RefPtr<U>& value) {
    return value && is_a<T>::check(*value);
}

template<typename T, typename U>
constexpr bool is(RefPtr<U>& value) {
    return value && is_a<T>::check(*value);
}

template<typename T, typename U>
constexpr T& to(U& value) {
    assert(is<T>(value));
    return static_cast<T&>(value);
}

template<typename T, typename U>
constexpr const T& to(const U& value) {
    assert(is<T>(value));
    return static_cast<const T&>(value);
}

template<typename T, typename U>
constexpr T* to(U* value) {
    if(!is<T>(value))
        return nullptr;
    return static_cast<T*>(value);
}

template<typename T, typename U>
constexpr const T* to(const U* value) {
    if(!is<T>(value))
        return nullptr;
    return static_cast<const T*>(value);
}

template<typename T, typename U>
constexpr RefPtr<T> to(const RefPtr<U>& value) {
    if(!is<T>(value))
        return nullptr;
    return static_cast<T*>(value.get());
}

template<typename T, typename U>
constexpr RefPtr<T> to(RefPtr<U>& value) {
    if(!is<T>(value))
        return nullptr;
    return static_cast<T*>(value.get());
}

} // namespace htmlbook

#endif // POINTER_H
