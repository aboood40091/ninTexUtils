#pragma once

#include <misc/rio_Types.h>

#include <cstdint>
#include <cstddef>
#include <vector>
#include <type_traits>

// TODO: Debloating
class Ptr32Registry
{
public:
    // ID tagging: top two bits == 01 means "registry ID"
    static constexpr u32 cTagMask   = 0xC0000000u;
    static constexpr u32 cIdTag     = 0x40000000u; // 01xxxx...
    static constexpr u32 cIndexMask = 0x3FFFFFFFu; // low 30 bits

public:
    static Ptr32Registry& instance() noexcept
    {
        static Ptr32Registry reg;
        return reg;
    }

private:
    Ptr32Registry()
    {
        mTable.reserve(100'000);
        mTable.push_back(nullptr);
    }

public:
    u32 store(void* p)
    {
        if (!p)
            return 0;

        constexpr u32 cMaxIndex = 0x3FFFFFFFu;
        RIO_ASSERT(mTable.size() >= 1); // ensure index 0 exists
        RIO_ASSERT(mTable.size() - 1 < cMaxIndex && "Ptr32Registry overflow (too many stored pointers).");

        mTable.push_back(p);
        u32 index = static_cast<u32>(mTable.size() - 1);
        return encodeID(index);
    }

    void* load(u32 encoded) const noexcept
    {
        if (encoded == 0)
            return nullptr;

        RIO_ASSERT(isEncodedID(encoded) && "Ptr32Registry::load called with non-ID encoding.");
        u32 index = decodeIndex(encoded);
        RIO_ASSERT(index < mTable.size() && "Ptr32Registry::load: invalid ID index.");
        return mTable[index];
    }

    static constexpr bool isEncodedID(u32 v) noexcept
    {
        return v != 0 && ((v & cTagMask) == cIdTag);
    }

    static constexpr u32 encodeID(u32 index) noexcept
    {
        // index must fit in 30 bits
        return cIdTag | (index & cIndexMask);
    }

    static constexpr u32 decodeIndex(u32 encoded) noexcept
    {
        return (encoded & cIndexMask);
    }

    static constexpr bool offsetInRange(s32 off) noexcept
    {
        constexpr s32 cMin = -0x40000000; // -1073741824
        constexpr s32 cMax =  0x3FFFFFFF; // +1073741823
        return off >= cMin && off <= cMax;
    }

private:
    std::vector<void*> mTable;
};

class SerializedPtrBase
{
protected:
    u32 mRaw = 0; // file storage / in-struct storage (always 32 bits)

    friend class ScopedSerializedPtrNullSetter;
};

template <class T>
class SerializedPtr : public SerializedPtrBase
{
private:
    static_assert(std::is_object_v<T> || std::is_void_v<T>, "SerializedPtr<T>: T must be an object type or void.");
    static constexpr bool cNative32 = (sizeof(void*) == 4);

    static constexpr bool isEncodedID_(u32 v) noexcept
    {
        if constexpr (cNative32)
            return false;
        else
            return Ptr32Registry::isEncodedID(v);
    }

    static constexpr bool offsetInRange_(s32 off) noexcept
    {
        if constexpr (cNative32)
            return true;
        else
            return Ptr32Registry::offsetInRange(off);
    }

    constexpr bool holdsID_() const noexcept
    {
        return isEncodedID_(mRaw);
    }

public:
    void set(T* p) noexcept
    {
        if constexpr (cNative32)
            mRaw = static_cast<u32>(reinterpret_cast<uintptr_t>(p));
        else
            mRaw = Ptr32Registry::instance().store(const_cast<std::remove_const_t<T>*>(p));
    }

    T* get() const noexcept
    {
        if (mRaw == 0)
            return nullptr;

        if constexpr (cNative32)
            return reinterpret_cast<T*>(static_cast<uintptr_t>(mRaw));
        else
        {
            RIO_ASSERT(holdsID_() && "SerializedPtr::get(): field is not an ID. Did you forget to relocate offsets?");
            return reinterpret_cast<T*>(Ptr32Registry::instance().load(mRaw));
        }
    }

    T* getIndexed(int i) const noexcept
    {
        return &(get()[i]);
    }

    void resolveRelativePtr(const void* origin, u32 mask = u32(-1)) noexcept
    {
        if (mRaw == 0)
            return;

        if constexpr (!cNative32)
        {
            RIO_ASSERT(!holdsID_() && "SerializedPtr::resolveRelativePtr(): Double resolution of ID not allowed.");
        }

        // Treat as signed offset.
        s32 off = static_cast<s32>(mRaw & mask);
        RIO_ASSERT(offsetInRange_(off) && "SerializedPtr::resolveRelativePtr(): offset out of +-1 GiB range.");
        auto base = reinterpret_cast<const u8*>(origin);
        T* p =  reinterpret_cast<T*>(const_cast<u8*>(base + off));
        set(p);
    }

    s32 getOffset() const noexcept
    {
        // In 64-bit mode, IDs exist and must not be treated as offsets.
        if constexpr (!cNative32)
        {
            RIO_ASSERT(!holdsID_() && "SerializedPtr::getOffset(): field holds an ID, not an offset.");
        }

        return static_cast<s32>(mRaw);
    }
};

static_assert(sizeof(SerializedPtr<void>) == 4, "SerializedPtr must be exactly 4 bytes.");
static_assert(std::is_trivially_copyable_v<SerializedPtr<void>>, "SerializedPtr should be trivially copyable.");

class ScopedSerializedPtrNullSetter
{
public:
    ScopedSerializedPtrNullSetter(SerializedPtrBase* target, bool skip_restore)
        : mTarget(target)
        , mSavedRaw(target->mRaw)
        , mSkipRestore(skip_restore)
    {
        mTarget->mRaw = 0;
    }
    
    ~ScopedSerializedPtrNullSetter()
    {
        if (!mSkipRestore)
            mTarget->mRaw = mSavedRaw;
    }

private:
    u32 mSavedRaw;
    SerializedPtrBase* mTarget;
    bool mSkipRestore;
};
