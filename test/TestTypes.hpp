#pragma once
#include "Main.hpp"


namespace Langulus::Flow
{
   struct Verb {};
   class Construct {};
   class Constructconst {};
   class constConstructconst {};
   class constconst {};
}

/// An empty trivial type                                                     
class ImplicitlyConstructible {};

/// A simple aggregate type                                                   
struct AggregateType {
   int m1, m2, m3, m4;
   bool m5;
};

/// Explicitly deleted destructor                                             
class NonDestructible {
   ~NonDestructible() = delete;
};

/// Has an explicit destructor                                                
class Destructible {
public:
   char* someptr {};

   ~Destructible() {
      if (someptr)
         delete someptr;
   }
};

/// Default-constructible, but only privately                                 
class PrivatelyConstructible {
   LANGULUS(POD) false;
private:
   PrivatelyConstructible() = default;
   PrivatelyConstructible(const PrivatelyConstructible&) = default;
   PrivatelyConstructible(PrivatelyConstructible&&) = default;
};

/// Had no explicit semantic constructors and assigners                       
/// Has only implicit refer & move constructors and assigners                 
class NonSemanticConstructible {
   LANGULUS(POD) false;
public:
   NonSemanticConstructible(CT::NotSemantic auto&&) {}
};

/// Has explicit copy, move, refer, clone, abandon, disown constructors       
/// Has implicit refer & move constructors, too                               
/// Has no explicit semantic assigners, only implicit refer & move            
class PartiallySemanticConstructible {
public:
   template<template<class> class S, class T>
   PartiallySemanticConstructible(S<T>&&) requires CT::Semantic<S<T>> {}
};

/// Has all semantic constructors + implicit refer & move ones                
/// Has no explicit semantic assigners, only implicit refer & move ones       
/// Making constructor explicit makes sure, that no implicit semantic assign  
/// happens                                                                   
class AllSemanticConstructible {
public:
   LANGULUS(POD) false;
   explicit AllSemanticConstructible(CT::Semantic auto&&) {}
};

/// Has all semantic constructors + implicit refer & move ones                
/// Has no explicit semantic assigners, only implicit refer & move ones       
/// Making constructor implicit also allows for semantic assignments          
class AllSemanticConstructibleImplicit {
public:
   LANGULUS(POD) false;
   AllSemanticConstructibleImplicit(CT::Semantic auto&&) {}
};

/// Has all semantic constructors and assigners + implicit refer & move ones  
class AllSemanticConstructibleAndAssignable {
public:
   LANGULUS(POD) false;
   AllSemanticConstructibleAndAssignable(CT::Semantic auto&&) {}
   AllSemanticConstructibleAndAssignable& operator = (CT::Semantic auto&&) { return *this; }
};

/// Has explicit descriptor constructor, and implicit refer & move ones       
/// Has no explicit semantic assigners, only implicit refer & move            
class DescriptorConstructible {
public:
   DescriptorConstructible(Describe&&) {}
};


enum class Pi {
   Number = 314
};

struct IncompleteType;

namespace One::Two::Three {
   struct TypeDeepIntoNamespaces;

   template<class T>
   struct TemplatedTypeDeepIntoNamespaces {
      enum VeryDeeplyTemplatedEnum { YesYouGotThatRight };

      template<class MORE>
      struct Nested;
   };

   template<class T>
   struct VeryComplexTemplate;
}

namespace Verbs
{

   ///                                                                        
   /// A testing verb, similar to the ones used in Langulus::Flow             
   ///                                                                        
   struct Create : public Flow::Verb {
      LANGULUS(POSITIVE_VERB) "Create";
      LANGULUS(NEGATIVE_VERB) "Destroy";
      LANGULUS(POSITIVE_OPERATOR) " + ";
      LANGULUS(NEGATIVE_OPERATOR) " - ";
      LANGULUS(PRECEDENCE) 5;
      LANGULUS(INFO)
         "Used for allocating new elements. "
         "If the type you're creating has	a producer, "
         "you need to execute the verb in a matching producer, "
         "or that producer will be created automatically for you, if possible";

      /// Check if the verb is available in a type, and with given arguments  
      ///   @return true if verb is available in T with arguments A...        
      template<CT::Data T, CT::Data... A>
      static constexpr bool AvailableFor() noexcept {
         if constexpr (sizeof...(A) == 0)
            return requires (T & t, Verb & v) { t.Create(v); };
         else
            return requires (T & t, Verb & v, A... a) { t.Create(v, a...); };
      }

      /// Get the verb functor for the given type and arguments               
      ///   @return the function, or nullptr if not available                 
      template<CT::Data T, CT::Data... A>
      static constexpr auto Of() noexcept {
         if constexpr (!Create::AvailableFor<T, A...>()) {
            return nullptr;
         }
         else if constexpr (CT::Constant<T>) {
            return [](const void* context, Flow::Verb& verb, A... args) {
               auto typedContext = static_cast<const T*>(context);
               typedContext->Create(verb, args...);
            };
         }
         else {
            return [](void* context, Flow::Verb& verb, A... args) {
               auto typedContext = static_cast<T*>(context);
               typedContext->Create(verb, args...);
            };
         }
      }

      template<CT::Data T>
      static bool ExecuteIn(T&, Verb&);

      static bool ExecuteDefault(const Anyness::Block<>&, Verb&) {
         return true;
      }

      static bool ExecuteDefault(Anyness::Block<>&, Verb&) {
         return false;
      }

      static bool ExecuteStateless(Verb&) {
         return false;
      }
   };

}

struct ImplicitlyReflectedData {
   LANGULUS(POD) true;
   LANGULUS(FILES) "ASE";

   enum Named {One, Two, Three};
   LANGULUS_NAMED_VALUES(One, Two, Three);
   LANGULUS(TYPED) Named;

   Named v = One;

   inline bool operator == (const ImplicitlyReflectedData&) const noexcept = default;
};

class alignas(128) ImplicitlyReflectedDataWithTraits : public ImplicitlyReflectedData {
public:
   int member {664};
   RTTI::Tag<bool, Traits::Name> anotherMember {};
   int anotherMemberArray [12] {};
   int* sparseMember {};

   inline operator int() const noexcept {
      return member;
   }

   void Create(Flow::Verb&) const {
      //++member;
   }

   void Create(Flow::Verb&) {
      ++member;
   }

   ImplicitlyReflectedDataWithTraits() = default;
   explicit ImplicitlyReflectedDataWithTraits(Pi)
      : member {314} {}

   LANGULUS(NAME) "MyType";
   LANGULUS(INFO) "Info about MyType";
   LANGULUS(FILES) "txt, pdf";
   LANGULUS(VERSION_MAJOR) 2;
   LANGULUS(VERSION_MINOR) 1;
   LANGULUS(DEEP) true;
   LANGULUS(POD) true;
   LANGULUS(NULLIFIABLE) true;
   IF_LANGULUS_MANAGED_MEMORY(LANGULUS(POOL_TACTIC) RTTI::PoolTactic::Size);
   LANGULUS(CONCRETE) ImplicitlyReflectedData;
   LANGULUS(UNINSERTABLE) true;
   LANGULUS(ALLOCATION_PAGE) 250;
   LANGULUS(ABSTRACT) true;
   LANGULUS_BASES(ImplicitlyReflectedData);
   LANGULUS_VERBS(Verbs::Create);
   LANGULUS_CONVERTS_TO(int);
   LANGULUS_CONVERTS_FROM(Pi);
   LANGULUS_NAMED_VALUES();

   using Self = ImplicitlyReflectedDataWithTraits;
   LANGULUS_MEMBERS(
      &Self::member,
      &Self::anotherMember,
      &Self::anotherMemberArray,
      &Self::sparseMember
   );
};

/// Doesn't have implicit copy/move, so it is abandon-makable by explicit move
/// but not abandon-assignable                                                
class alignas(128) Complex {
public:
   int member;
   bool anotherMember {};
   int anotherMemberArray [12] {};
   int* sparseMember {};

   LANGULUS(NAME) "ComplexType";
   LANGULUS(INFO) "Info about ComplexType";
   LANGULUS(VERSION_MAJOR) 2;
   LANGULUS(VERSION_MINOR) 1;
   IF_LANGULUS_MANAGED_MEMORY(LANGULUS(POOL_TACTIC) RTTI::PoolTactic::Size);
   LANGULUS(ALLOCATION_PAGE) 250;

   using Self = Complex;
   LANGULUS_MEMBERS(
      &Self::member,
      &Self::anotherMember,
      &Self::anotherMemberArray,
      &Self::sparseMember
   );

   Complex(const Complex& s)
      : member(s.member) {}
   Complex(Complex&& s)
      : member(s.member) {}
   Complex(int stuff)
      : member(stuff) {}

   ~Complex() {
      if (sparseMember)
         delete sparseMember;
   }
};

struct AnotherTypeWithSimilarilyNamedValues {
   enum Named {One = 501, Two, Three};
   LANGULUS_NAMED_VALUES(One, Two, Three);
   LANGULUS(NAME) "YetAnotherNamedType";

   int v = One;

   inline bool operator == (const AnotherTypeWithSimilarilyNamedValues&) const noexcept = default;
};

struct CheckingWhatGetsInherited : ImplicitlyReflectedDataWithTraits {
   LANGULUS(NAME) "CheckingWhatGetsInherited";

   using ImplicitlyReflectedDataWithTraits::ImplicitlyReflectedDataWithTraits;
};

class ContainsComplex {
   Complex mData;
};

/// A complex aggregate type                                                  
struct AggregateTypeComplex {
   int m1, m2, m3, m4;
   bool m5;
   Complex mData;
};

/// A complex aggregate type                                                  
struct AggregateThatCanBeConfusedWithDescriptorMakable {
   DescriptorConstructible mConfusable;
   int m1, m2, m3, m4;
};

class ForcefullyPod {
   LANGULUS(POD) true;
   Complex mData;
};

struct Type {};

struct TypeErasedContainer {
   LANGULUS(TYPED) void;
};

namespace N1 {
   struct Type {};
   struct Create {};
}

namespace N2 {
   struct Type {};
}

namespace N3 {
   struct type {};
}

enum class TypedEnum : int16_t {
   E1, E2, E3
};

/// Simple type for testing Referenced types                                  
struct RT : Referenced {
   int data;
   const char* t;
   bool destroyed = false;
   bool copied_in = false;
   bool moved_in = false;
   bool moved_out = false;

   RT() : data {0}, t {nullptr} {}
   RT(int a) : data {a}, t {nullptr} {}
   RT(const char* tt) : data(0), t {tt} {}
   RT(const RT& rhs) : data(rhs.data), t {rhs.t}, copied_in {true} {}
   RT(RT&& rhs) : data(rhs.data), t {rhs.t}, moved_in {true} { rhs.moved_out = true; }
   ~RT() {
      destroyed = true;
   }

   RT& operator = (const RT& rhs) {
      data = rhs.data;
      t = rhs.t;
      copied_in = true;
      moved_in = moved_out = false;
      return *this;
   }

   RT& operator = (RT&& rhs) {
      data = rhs.data;
      t = rhs.t;
      copied_in = false;
      moved_in = true;
      moved_out = false;
      rhs.copied_in = false;
      rhs.moved_in = false;
      rhs.moved_out = true;
      return *this;
   }

   operator const int& () const noexcept {
      return data;
   }
};