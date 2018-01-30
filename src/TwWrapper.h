#ifndef __TWWRAPPER_H__
#define __TWWRAPPER_H__

#include <cstdint>
#include <cassert>
#include <functional>
#include <AntTweakBar.h>

template <typename T>
class TwWrapper
{
    typedef std::function<void(T)> WrapperCb;

private:
    WrapperCb m_callback;
    T m_value;

public:
    TwWrapper(T value): m_value(value) {}
    // TODO destructor - note that TwRemoveVar requires name!

    T get() const {return m_value;}
    void set(T value) {m_value = value;}

    void init(TwBar* tweakBar, const char* name, const char* desc, WrapperCb callback)
    {
        TwAddVarCB(tweakBar, name, type(), SetCallback, GetCallback, this, desc);
        m_callback = callback;
    }

    static constexpr TwType type();

private:
    static void TW_CALL SetCallback(const void *valuePtr, void *wrapperPtr)
    {
        assert(wrapperPtr != nullptr);
        TwWrapper* wrapper = static_cast<TwWrapper*>(wrapperPtr);
        T value = *static_cast<const T*>(valuePtr);
        wrapper->m_value = value;
        if (wrapper->m_callback)
            wrapper->m_callback(value);
    }

    static void TW_CALL GetCallback(void *valuePtr, void *wrapperPtr)
    {
        assert(wrapperPtr != nullptr);
        TwWrapper* wrapper = static_cast<TwWrapper*>(wrapperPtr);
        *static_cast<T*>(valuePtr) = wrapper->m_value;
    }
};

// TODO handle enum, color, quat, dir - maybe define struct wrappers
template <> constexpr TwType TwWrapper<bool>::type() {return TW_TYPE_BOOLCPP;}
template <> constexpr TwType TwWrapper<char>::type() {return TW_TYPE_CHAR;}
template <> constexpr TwType TwWrapper<int8_t>::type() {return TW_TYPE_INT8;}
template <> constexpr TwType TwWrapper<uint8_t>::type() {return TW_TYPE_UINT8;}
template <> constexpr TwType TwWrapper<int16_t>::type() {return TW_TYPE_INT16;}
template <> constexpr TwType TwWrapper<uint16_t>::type() {return TW_TYPE_UINT16;}
template <> constexpr TwType TwWrapper<int32_t>::type() {return TW_TYPE_INT32;}
template <> constexpr TwType TwWrapper<uint32_t>::type() {return TW_TYPE_UINT32;}
template <> constexpr TwType TwWrapper<float>::type() {return TW_TYPE_FLOAT;}
template <> constexpr TwType TwWrapper<double>::type() {return TW_TYPE_DOUBLE;}

#endif //__TWWRAPPER_H__
