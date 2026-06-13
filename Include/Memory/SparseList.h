#pragma once

#include <stack>
#include <span>
#include <vector>

#include "Shared/Annotations.h"


#ifndef CONFIG_RELEASE
#include "Shared/Assertion.h"
#endif // CONFIG_RELEASE

template <typename T, bool UseFreeList = true>
class SparseList
{
public:
    using Type = T;

    SparseList() = default;
    SparseList(const SparseList& Other) = delete;
    SparseList(SparseList&& Other) noexcept = delete;
    SparseList& operator=(const SparseList& Other) = delete;
    SparseList& operator=(SparseList&& Other) noexcept = delete;
    ~SparseList();
    
    void Reserve(size_t Size);
    void Resize(size_t Size) requires (std::is_default_constructible_v<T>);
    void Resize(size_t Size) requires (!std::is_default_constructible_v<T>) = delete;
    
    size_t AddEmplace() requires (std::is_default_constructible_v<T>);
    size_t AddEmplace() requires (!std::is_default_constructible_v<T>) = delete;
    size_t AddEmplace(T&& Value);
    template <typename ...Args>
    size_t AddEmplace(Args&&... Params);
    
    void EmplaceBack() requires (std::is_default_constructible_v<T>);
    void EmplaceBack() requires (!std::is_default_constructible_v<T>) = delete;
    void EmplaceBack(T&& Value);
    template <typename ...Args>
    void EmplaceBack(Args&&... Params);
    
    void RemoveAt(size_t Index);
    
    void Clear();
    void ShrinkToFit();
    
    bool IsValid(size_t Index) const;
    T& operator[](size_t Index);
    const T& operator[](size_t Index) const;
    T* At(size_t Index);
    const T* At(size_t Index) const;
    
    size_t Size() const;
    size_t Capacity() const;
    
    INLINE std::span<const T> Data() const {return std::span<const T>(At(0), sizeof(MemType) * m_Data.size());}
    INLINE std::span<T> Data() {return std::span<T>(At(0), sizeof(MemType) * m_Data.size());}
    INLINE const std::vector<bool>& Validity() const {return m_Validity;}
    
    void Swap(size_t Index1, size_t Index2);
    
private:
    struct MemType {uint8_t cell[sizeof(T)];};
    
    std::vector<MemType> m_Data;
    std::vector<bool> m_Validity;
    std::stack<size_t> m_FreeList;
    size_t m_FreeCount = 0;
};

template <typename T, bool UseFreeList>
SparseList<T, UseFreeList>::~SparseList()
{
    for (size_t Index = 0; Index < m_Data.size(); Index++)
    {
        if (m_Validity[Index]) std::destroy_at(At(Index));
    }
}

template <typename T, bool UseFreeList>
void SparseList<T, UseFreeList>::Reserve(size_t Size)
{
    m_Data.reserve(Size);
    m_Validity.reserve(Size);
}

template <typename T, bool UseFreeList>
void SparseList<T, UseFreeList>::Resize(size_t Size) requires (std::is_default_constructible_v<T>)
{
    m_Validity.resize(Size);
    if (m_Data.size() >= Size)
    {
        m_Data.resize(Size);
    }
    else
    {
        bool CleanupFreeList = false;
        for (size_t Index = Size; Index < m_Data.size(); Index++)
        {
            if (m_Validity[Index]) std::destroy_at(At(Index));
            else if (UseFreeList) CleanupFreeList = true;
            else m_FreeCount--;
        }
        
        if (CleanupFreeList && UseFreeList)
        {
            std::stack<size_t> FreeListToKeep;
            
            while (!m_FreeList.empty())
            {
                if (m_FreeList.top() < Size) FreeListToKeep.push(m_FreeList.top());
                m_FreeList.pop();
            }
            
            // while (!FreeListToKeep.empty())
            // {
            //     m_FreeList.push(FreeListToKeep.top());
            //     FreeListToKeep.pop();
            // }
            
            m_FreeList = FreeListToKeep;
        }
    }
}

template <typename T, bool UseFreeList>
size_t SparseList<T, UseFreeList>::AddEmplace() requires (std::is_default_constructible_v<T>)
{
    size_t Index = Size();
    
    if ((UseFreeList && m_FreeList.empty()) || (!UseFreeList && m_FreeCount == 0))
    {
        EmplaceBack();
    }
    else
    {
        if (UseFreeList)
        {
            Index = m_FreeList.top();
            m_FreeList.pop();
        }
        else
        {
            for (size_t i = 0; i < m_Data.size(); i++)
            {
                if (m_Validity[i])
                {
                    Index = i;
                    break;
                }
            }
        }
        
        std::construct_at(At(Index));
        m_FreeCount--;
    }
    
    return Index;
}

template <typename T, bool UseFreeList>
size_t SparseList<T, UseFreeList>::AddEmplace(T&& Value)
{
    size_t Index = Size();
    
    if ((UseFreeList && m_FreeList.empty()) || (!UseFreeList && m_FreeCount == 0))
    {
        EmplaceBack(std::move(Value));
    }
    else
    {
        if (UseFreeList)
        {
            Index = m_FreeList.top();
            m_FreeList.pop();
        }
        else
        {
            for (size_t i = 0; i < m_Data.size(); i++)
            {
                if (m_Validity[i])
                {
                    Index = i;
                    break;
                }
            }
        }
        
        std::construct_at(At(Index), std::move(Value));
    }
    
    return Index;
}

template <typename T, bool UseFreeList>
template <typename ... Args>
size_t SparseList<T, UseFreeList>::AddEmplace(Args&&... Params)
{
    size_t Index = Size();
    
    if ((UseFreeList && m_FreeList.empty()) || (!UseFreeList && m_FreeCount == 0))
    {
        EmplaceBack(std::forward<Args>(Params)...);
    }
    else
    {
        if (UseFreeList)
        {
            Index = m_FreeList.top();
            m_FreeList.pop();
        }
        else
        {
            for (size_t i = 0; i < m_Data.size(); i++)
            {
                if (m_Validity[i])
                {
                    Index = i;
                    break;
                }
            }
        }
        
        std::construct_at(At(Index), std::forward<Args>(Params)...);
    }
    
    return Index;
}

template <typename T, bool UseFreeList>
void SparseList<T, UseFreeList>::EmplaceBack() requires (std::is_default_constructible_v<T>)
{
    m_Data.emplace_back();
    std::construct_at(At(Size() - 1));
    
    m_Validity.emplace_back(true);
}

template <typename T, bool UseFreeList>
void SparseList<T, UseFreeList>::EmplaceBack(T&& Value)
{
    m_Data.emplace_back();
    std::construct_at(At(Size() - 1), std::move(Value));
    
    m_Validity.emplace_back(true);
}

template <typename T, bool UseFreeList>
template <typename ... Args>
void SparseList<T, UseFreeList>::EmplaceBack(Args&&... Params)
{
    m_Data.emplace_back();
    std::construct_at(At(Size() - 1), std::forward<Args>(Params)...);
    
    m_Validity.emplace_back(true);
}

template <typename T, bool UseFreeList>
void SparseList<T, UseFreeList>::RemoveAt(size_t Index)
{
#ifndef CONFIG_RELEASE
    AssertOrError(Index < m_Data.size(), "Index out of bounds")
    AssertOrError(IsValid(Index), "Object at index is invalid")
#endif // CONFIG_RELEASE
    
    std::destroy_at(At(Index));
    m_Validity[Index] = false;
    if (UseFreeList)
    {
        m_FreeList.push(Index);
    }
    else
    {
        m_FreeCount++;
    }
}

template <typename T, bool UseFreeList>
void SparseList<T, UseFreeList>::Clear()
{
    for (size_t Index = 0; Index < m_Data.size(); Index++)
    {
        if (m_Validity[Index]) std::destroy_at(At(Index));
    }
    
    m_Data.clear();
    m_Validity.clear();
    if (UseFreeList)
    {
        while (!m_FreeList.empty()) m_FreeList.pop();
    }
    else
    {
        m_FreeCount = 0;
    }
}

template <typename T, bool UseFreeList>
void SparseList<T, UseFreeList>::ShrinkToFit()
{
    if (Capacity() == Size()) return;

    m_Data.shrink_to_fit();
    m_Validity.shrink_to_fit();
}

template <typename T, bool UseFreeList>
bool SparseList<T, UseFreeList>::IsValid(size_t Index) const
{
    return Index < m_Validity.size() && m_Validity[Index];
}

template <typename T, bool UseFreeList>
T& SparseList<T, UseFreeList>::operator[](size_t Index)
{
#ifndef CONFIG_RELEASE
    AssertOrError(Index < m_Data.size(), "Index out of bounds")
    AssertOrError(IsValid(Index), "Object at index is invalid")
#endif // CONFIG_RELEASE
    
    return *At(Index);
}

template <typename T, bool UseFreeList>
const T& SparseList<T, UseFreeList>::operator[](size_t Index) const
{
#ifndef CONFIG_RELEASE
    AssertOrError(Index < m_Data.size(), "Index out of bounds")
    AssertOrError(IsValid(Index), "Object at index is invalid")
#endif // CONFIG_RELEASE
    
    return *At(Index);
}

template <typename T, bool UseFreeList>
T* SparseList<T, UseFreeList>::At(size_t Index)
{
    return reinterpret_cast<T*>(&m_Data[Index]);
}

template <typename T, bool UseFreeList>
const T* SparseList<T, UseFreeList>::At(size_t Index) const
{
    return reinterpret_cast<const T*>(&m_Data[Index]);
}

template <typename T, bool UseFreeList>
size_t SparseList<T, UseFreeList>::Size() const
{
    return m_Data.size();
}

template <typename T, bool UseFreeList>
size_t SparseList<T, UseFreeList>::Capacity() const
{
    return m_Data.capacity();
}

template <typename T, bool UseFreeList>
void SparseList<T, UseFreeList>::Swap(size_t Index1, size_t Index2)
{
#ifndef CONFIG_RELEASE
    if (UseFreeList)
    {
        AssertOrError(IsValid(Index1), "Object at index is invalid")
        AssertOrError(IsValid(Index2), "Object at index is invalid")
    }
#endif // CONFIG_RELEASE
    
    std::swap(*At(Index1), *At(Index2));
    
    bool Temp = IsValid(Index1);
    m_Validity[Index1] = IsValid(Index2);
    m_Validity[Index2] = Temp;
}
