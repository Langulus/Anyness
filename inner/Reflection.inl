#pragma once
#include "Reflection.hpp"

namespace Langulus::Anyness
{
   
   template<ReflectedData T>
   constexpr bool Member::Is() const noexcept {
      return mType->Is<T>();
   }
   
   template<ReflectedData T>
   const T& Member::As(const Byte* instance) const noexcept {
      return *reinterpret_cast<const T*>(Get(instance));
   }
   
   template<ReflectedData T>
   T& Member::As(Byte* instance) const noexcept {
      return *reinterpret_cast<T*>(Get(instance));
   }
   
   constexpr const Byte* Member::Get(const Byte* instance) const noexcept {
      return instance + mOffset;
   }
   
   constexpr Byte* Member::Get(Byte* instance) const noexcept {
      return instance + mOffset;
   }
   
} // namespace Langulus::Anyness

