///                                                                           
/// Langulus::RTTI                                                            
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#include "TestManyCommon.hpp"


///                                                                           
///   Refer semantics                                                         
///                                                                           
TEMPLATE_TEST_CASE("Testing refer-makable types", "[semantics]",
   TMany<AggregateType>,
   TMany<ImplicitlyConstructible>,
   TMany<Destructible>,
   TMany<NonSemanticConstructible>,
   TMany<AllSemanticConstructible>,
   TMany<AllSemanticConstructibleAndAssignable>,
   TMany<PartiallySemanticConstructible>,
   TMany<DescriptorConstructible>,
   TMany<Complex>, TMany<ContainsComplex>,
   TMany<bool>, TMany<uint32_t>, TMany<float>, TMany<char>, TMany<wchar_t>, TMany<char8_t>, TMany<Langulus::Byte>,
   TMany<AMeta>, TMany<TMeta>, TMany<CMeta>, TMany<DMeta>, TMany<VMeta>,
   TMany<ForcefullyPod>,
   TMany<NonDestructible>,
   TMany<PrivatelyConstructible>
) {
   using T = TestType;
   static_assert(CT::ReferMakable<T>);
   static_assert(CT::ReferMakable<T*>);
   static_assert(CT::SemanticMakable<Referred, T>);
   static_assert(CT::SemanticMakable<Referred, T*>);
   static_assert(CT::SemanticMakableAlt<Referred<T>>);
   static_assert(CT::SemanticMakableAlt<Referred<T*>>);

   auto meta1 = MetaDataOf<T>();
   REQUIRE(meta1);
   REQUIRE(meta1->mReferConstructor);

   auto meta2 = MetaDataOf<T*>();
   REQUIRE(meta2);
   REQUIRE(meta2->mReferConstructor);
}

/*TEMPLATE_TEST_CASE("Testing non-refer-makable types", "[semantics]",
   //TMany<IncompleteType>,
   TMany<NonDestructible>,
   TMany<PrivatelyConstructible>
) {
   using T = TestType;
   static_assert(not CT::ReferMakable<T>);
   static_assert(    CT::ReferMakable<T*>);
   static_assert(not CT::SemanticMakable<Referred, T>);
   static_assert(    CT::SemanticMakable<Referred, T*>);
   static_assert(not CT::SemanticMakableAlt<Referred<T>>);
   static_assert(    CT::SemanticMakableAlt<Referred<T*>>);

   auto meta = MetaDataOf<Conditional<CT::Complete<T>, T, T*>>();
   REQUIRE(meta);
   REQUIRE_FALSE(meta->mReferConstructor);
}*/

TEMPLATE_TEST_CASE("Testing refer-assignable types", "[semantics]",
   TMany<AggregateType>,
   TMany<ImplicitlyConstructible>,
   TMany<NonDestructible>,
   TMany<Destructible>,
   TMany<NonSemanticConstructible>,
   TMany<AllSemanticConstructible>,
   TMany<AllSemanticConstructibleAndAssignable>,
   TMany<PartiallySemanticConstructible>,
   TMany<DescriptorConstructible>,
   TMany<bool>, TMany<uint32_t>, TMany<float>, TMany<char>, TMany<wchar_t>, TMany<char8_t>, TMany<Langulus::Byte>,
   TMany<AMeta>, TMany<TMeta>, TMany<CMeta>, TMany<DMeta>, TMany<VMeta>,
   TMany<ForcefullyPod>,
   TMany<Complex>,
   TMany<ContainsComplex>,
   TMany<PrivatelyConstructible>
) {
   using T = TestType;
   static_assert(    CT::ReferAssignable<T>);
   static_assert(not CT::ReferAssignable<const T>);
   static_assert(    CT::ReferAssignable<T*>);
   static_assert(    CT::ReferAssignable<const T*>);
   static_assert(    CT::SemanticAssignable<Referred, T>);
   static_assert(not CT::SemanticAssignable<Referred, const T>);
   static_assert(    CT::SemanticAssignable<Referred, T*>);
   static_assert(    CT::SemanticAssignable<Referred, const T*>);
   static_assert(    CT::SemanticAssignableAlt<Referred<T>>);
   static_assert(not CT::SemanticAssignableAlt<Referred<const T>>);
   static_assert(    CT::SemanticAssignableAlt<Referred<T*>>);
   static_assert(    CT::SemanticAssignableAlt<Referred<const T*>>);

   auto meta1 = MetaDataOf<T>();
   REQUIRE(meta1);
   REQUIRE(meta1->mReferAssigner);

   auto meta2 = MetaDataOf<T*>();
   REQUIRE(meta2);
   REQUIRE(meta2->mReferAssigner);

   auto meta3 = MetaDataOf<const T>();
   REQUIRE(meta3);
   REQUIRE(meta3->mReferAssigner);
}

/*TEMPLATE_TEST_CASE("Testing non-refer-assignable types", "[semantics]",
   //TMany<IncompleteType>,
   TMany<Complex>,
   TMany<ContainsComplex>,
   TMany<PrivatelyConstructible>
) {
   using T = TestType;
   static_assert(not CT::ReferAssignable<T>);
   static_assert(not CT::ReferAssignable<const T>);
   static_assert(    CT::ReferAssignable<T*>);
   static_assert(    CT::ReferAssignable<const T*>);
   static_assert(not CT::SemanticAssignable<Referred, T>);
   static_assert(not CT::SemanticAssignable<Referred, const T>);
   static_assert(    CT::SemanticAssignable<Referred, T*>);
   static_assert(    CT::SemanticAssignable<Referred, const T*>);
   static_assert(not CT::SemanticAssignableAlt<Referred<T>>);
   static_assert(not CT::SemanticAssignableAlt<Referred<const T>>);
   static_assert(    CT::SemanticAssignableAlt<Referred<T*>>);
   static_assert(    CT::SemanticAssignableAlt<Referred<const T*>>);

   auto meta = MetaDataOf<Conditional<CT::Complete<T>, T, T*>>();
   REQUIRE(meta);
   REQUIRE_FALSE(meta->mReferAssigner);
}*/


///                                                                           
///   Move semantics                                                          
///                                                                           
TEMPLATE_TEST_CASE("Testing move-makable types", "[semantics]",
   TMany<AggregateType>,
   TMany<ImplicitlyConstructible>,
   TMany<Destructible>,
   TMany<NonSemanticConstructible>,
   TMany<AllSemanticConstructible>,
   TMany<AllSemanticConstructibleAndAssignable>,
   TMany<PartiallySemanticConstructible>,
   TMany<DescriptorConstructible>,
   TMany<Complex>,
   TMany<ContainsComplex>,
   TMany<bool>, TMany<uint32_t>, TMany<float>, TMany<char>, TMany<wchar_t>, TMany<char8_t>, TMany<Langulus::Byte>,
   TMany<AMeta>, TMany<TMeta>, TMany<CMeta>, TMany<DMeta>, TMany<VMeta>,
   TMany<ForcefullyPod>,
   TMany<NonDestructible>,
   TMany<PrivatelyConstructible>
) {
   using T = TestType;
   static_assert(CT::MoveMakable<T>);
   static_assert(CT::MoveMakable<T*>);
   static_assert(CT::SemanticMakable<Moved, T>);
   static_assert(CT::SemanticMakable<Moved, T*>);
   static_assert(CT::SemanticMakableAlt<Moved<T>>);
   static_assert(CT::SemanticMakableAlt<Moved<T*>>);

   auto meta1 = MetaDataOf<T>();
   REQUIRE(meta1);
   REQUIRE(meta1->mMoveConstructor);

   auto meta2 = MetaDataOf<T*>();
   REQUIRE(meta2);
   REQUIRE(meta2->mMoveConstructor);
}

/*TEMPLATE_TEST_CASE("Testing non-move-makable types", "[semantics]",
   //TMany<IncompleteType>,
   TMany<NonDestructible>,
   TMany<PrivatelyConstructible>
) {
   using T = TestType;
   static_assert(not CT::MoveMakable<T>);
   static_assert(    CT::MoveMakable<T*>);
   static_assert(not CT::SemanticMakable<Moved, T>);
   static_assert(    CT::SemanticMakable<Moved, T*>);
   static_assert(not CT::SemanticMakableAlt<Moved<T>>);
   static_assert(    CT::SemanticMakableAlt<Moved<T*>>);

   auto meta = MetaDataOf<Conditional<CT::Complete<T>, T, T*>>();
   REQUIRE(meta);
   REQUIRE_FALSE(meta->mMoveConstructor);
}*/

TEMPLATE_TEST_CASE("Testing move-assignable types", "[semantics]",
   TMany<AggregateType>,
   TMany<ImplicitlyConstructible>,
   TMany<Destructible>,
   TMany<NonSemanticConstructible>,
   TMany<AllSemanticConstructible>,
   TMany<AllSemanticConstructibleAndAssignable>,
   TMany<PartiallySemanticConstructible>,
   TMany<DescriptorConstructible>,
   TMany<bool>, TMany<uint32_t>, TMany<float>, TMany<char>, TMany<wchar_t>, TMany<char8_t>, TMany<Langulus::Byte>,
   TMany<AMeta>, TMany<TMeta>, TMany<CMeta>, TMany<DMeta>, TMany<VMeta>,
   TMany<ForcefullyPod>,
   TMany<NonDestructible>,
   TMany<Complex>,
   TMany<ContainsComplex>,
   TMany<PrivatelyConstructible>
) {
   using T = TestType;
   static_assert(    CT::MoveAssignable<T>);
   static_assert(not CT::MoveAssignable<const T>);
   static_assert(    CT::MoveAssignable<T*>);
   static_assert(    CT::MoveAssignable<const T*>);
   static_assert(    CT::SemanticAssignable<Moved, T>);
   static_assert(not CT::SemanticAssignable<Moved, const T>);
   static_assert(    CT::SemanticAssignable<Moved, T*>);
   static_assert(    CT::SemanticAssignable<Moved, const T*>);
   static_assert(    CT::SemanticAssignableAlt<Moved<T>>);
   static_assert(not CT::SemanticAssignableAlt<Moved<const T>>);
   static_assert(    CT::SemanticAssignableAlt<Moved<T*>>);
   static_assert(    CT::SemanticAssignableAlt<Moved<const T*>>);

   auto meta1 = MetaDataOf<T>();
   REQUIRE(meta1);
   REQUIRE(meta1->mMoveAssigner);

   auto meta2 = MetaDataOf<T*>();
   REQUIRE(meta2);
   REQUIRE(meta2->mMoveAssigner);

   auto meta3 = MetaDataOf<const T>();
   REQUIRE(meta3);
   REQUIRE(meta3->mMoveAssigner);
}

/*TEMPLATE_TEST_CASE("Testing non-move-assignable types", "[semantics]",
   //TMany<IncompleteType>,
   TMany<NonDestructible>,
   TMany<Complex>,
   TMany<ContainsComplex>,
   TMany<PrivatelyConstructible>
) {
   using T = TestType;
   static_assert(not CT::MoveAssignable<T>);
   static_assert(not CT::MoveAssignable<const T>);
   static_assert(    CT::MoveAssignable<T*>);
   static_assert(    CT::MoveAssignable<const T*>);
   static_assert(not CT::SemanticAssignable<Moved, T>);
   static_assert(not CT::SemanticAssignable<Moved, const T>);
   static_assert(    CT::SemanticAssignable<Moved, T*>);
   static_assert(    CT::SemanticAssignable<Moved, const T*>);
   static_assert(not CT::SemanticAssignableAlt<Moved<T>>);
   static_assert(not CT::SemanticAssignableAlt<Moved<const T>>);
   static_assert(    CT::SemanticAssignableAlt<Moved<T*>>);
   static_assert(    CT::SemanticAssignableAlt<Moved<const T*>>);

   auto meta = MetaDataOf<Conditional<CT::Complete<T>, T, T*>>();
   REQUIRE(meta);
   REQUIRE_FALSE(meta->mMoveAssigner);
}*/


///                                                                           
///   Copy semantics                                                          
///                                                                           
TEMPLATE_TEST_CASE("Testing copy-makable types", "[semantics]",
   TMany<ImplicitlyConstructible>,
   TMany<AggregateType>,
   TMany<AllSemanticConstructible>,
   TMany<AllSemanticConstructibleImplicit>,
   TMany<AllSemanticConstructibleAndAssignable>,
   TMany<PartiallySemanticConstructible>,
   TMany<bool>, TMany<uint32_t>, TMany<float>, TMany<char>, TMany<wchar_t>, TMany<char8_t>, TMany<Langulus::Byte>,
   TMany<AMeta>, TMany<TMeta>, TMany<CMeta>, TMany<DMeta>, TMany<VMeta>,
   TMany<ForcefullyPod>,
   TMany<Destructible>,
   TMany<Complex>,
   TMany<ContainsComplex>,
   TMany<NonSemanticConstructible>,
   TMany<DescriptorConstructible>,
   TMany<AggregateTypeComplex>
) {
   using T = TestType;
   static_assert(CT::CopyMakable<T>);
   static_assert(CT::CopyMakable<T*>);
   static_assert(CT::SemanticMakable<Copied, T>);
   static_assert(CT::SemanticMakable<Copied, T*>);
   static_assert(CT::SemanticMakableAlt<Copied<T>>);
   static_assert(CT::SemanticMakableAlt<Copied<T*>>);

   auto meta1 = MetaDataOf<T>();
   REQUIRE(meta1);
   REQUIRE(meta1->mCopyConstructor);

   auto meta2 = MetaDataOf<T*>();
   REQUIRE(meta2);
   REQUIRE(meta2->mCopyConstructor);
}

TEMPLATE_TEST_CASE("Testing non-copy-makable types", "[semantics]",
   //TMany<IncompleteType>,
   TMany<NonDestructible>,
   TMany<PrivatelyConstructible>
) {
   using T = TestType;
   static_assert(not CT::CopyMakable<T>);
   static_assert(    CT::CopyMakable<T*>);
   static_assert(not CT::SemanticMakable<Copied, T>);
   static_assert(    CT::SemanticMakable<Copied, T*>);
   static_assert(not CT::SemanticMakableAlt<Copied<T>>);
   static_assert(    CT::SemanticMakableAlt<Copied<T*>>);

   auto meta = MetaDataOf<Conditional<CT::Complete<T>, T, T*>>();
   REQUIRE(meta);
   REQUIRE_FALSE(meta->mCopyConstructor);
}

TEMPLATE_TEST_CASE("Testing copy-assignable types", "[semantics]",
   TMany<ImplicitlyConstructible>,
   TMany<AggregateType>,
   TMany<AllSemanticConstructibleImplicit>,
   TMany<AllSemanticConstructibleAndAssignable>,
   TMany<bool>, TMany<uint32_t>, TMany<float>, TMany<char>, TMany<wchar_t>, TMany<char8_t>, TMany<Langulus::Byte>,
   TMany<AMeta>, TMany<TMeta>, TMany<CMeta>, TMany<DMeta>, TMany<VMeta>,
   TMany<ForcefullyPod>,
   TMany<Destructible>,
   TMany<Complex>,
   TMany<ContainsComplex>,
   TMany<NonSemanticConstructible>,
   TMany<DescriptorConstructible>,
   TMany<AllSemanticConstructible>,
   TMany<PartiallySemanticConstructible>,
   TMany<AggregateTypeComplex>
) {
   using T = TestType;
   static_assert(    CT::CopyAssignable<T>);
   static_assert(not CT::CopyAssignable<const T>);
   static_assert(    CT::CopyAssignable<T*>);
   static_assert(    CT::CopyAssignable<const T*>);
   static_assert(    CT::SemanticAssignable<Copied, T>);
   static_assert(not CT::SemanticAssignable<Copied, const T>);
   static_assert(    CT::SemanticAssignable<Copied, T*>);
   static_assert(    CT::SemanticAssignable<Copied, const T*>);
   static_assert(    CT::SemanticAssignableAlt<Copied<T>>);
   static_assert(not CT::SemanticAssignableAlt<Copied<const T>>);
   static_assert(    CT::SemanticAssignableAlt<Copied<T*>>);
   static_assert(    CT::SemanticAssignableAlt<Copied<const T*>>);

   auto meta1 = MetaDataOf<T>();
   REQUIRE(meta1);
   REQUIRE(meta1->mCopyAssigner);

   auto meta2 = MetaDataOf<T*>();
   REQUIRE(meta2);
   REQUIRE(meta2->mCopyAssigner);

   auto meta3 = MetaDataOf<const T>();
   REQUIRE(meta3);
   REQUIRE(meta3->mCopyAssigner);
}

TEMPLATE_TEST_CASE("Testing non-copy-assignable types", "[semantics]",
   //TMany<IncompleteType>,
   TMany<NonDestructible>,
   TMany<PrivatelyConstructible>
) {
   using T = TestType;
   static_assert(not CT::CopyAssignable<T>);
   static_assert(not CT::CopyAssignable<const T>);
   static_assert(    CT::CopyAssignable<T*>);
   static_assert(    CT::CopyAssignable<const T*>);
   static_assert(not CT::SemanticAssignable<Copied, T>);
   static_assert(not CT::SemanticAssignable<Copied, const T>);
   static_assert(    CT::SemanticAssignable<Copied, T*>);
   static_assert(    CT::SemanticAssignable<Copied, const T*>);
   static_assert(not CT::SemanticAssignableAlt<Copied<T>>);
   static_assert(not CT::SemanticAssignableAlt<Copied<const T>>);
   static_assert(    CT::SemanticAssignableAlt<Copied<T*>>);
   static_assert(    CT::SemanticAssignableAlt<Copied<const T*>>);

   auto meta = MetaDataOf<Conditional<CT::Complete<T>, T, T*>>();
   REQUIRE(meta);
   REQUIRE_FALSE(meta->mCopyAssigner);
}


///                                                                           
///   Clone semantics                                                         
///                                                                           
TEMPLATE_TEST_CASE("Testing clone-makable types", "[semantics]",
   TMany<ImplicitlyConstructible>,
   TMany<AllSemanticConstructible>,
   TMany<AllSemanticConstructibleAndAssignable>,
   TMany<PartiallySemanticConstructible>,
   TMany<bool>, TMany<uint32_t>, TMany<float>, TMany<char>, TMany<wchar_t>, TMany<char8_t>, TMany<Langulus::Byte>,
   TMany<AMeta>, TMany<TMeta>, TMany<CMeta>, TMany<DMeta>, TMany<VMeta>,
   TMany<ForcefullyPod>,
   TMany<AggregateType>
) {
   using T = TestType;
   static_assert(CT::CloneMakable<T>);
   static_assert(CT::CloneMakable<T*>);
   static_assert(CT::SemanticMakable<Cloned, T>);
   static_assert(CT::SemanticMakable<Cloned, T*>);
   static_assert(CT::SemanticMakableAlt<Cloned<T>>);
   static_assert(CT::SemanticMakableAlt<Cloned<T*>>);

   auto meta1 = MetaDataOf<T>();
   REQUIRE(meta1);
   REQUIRE(meta1->mCloneConstructor);

   auto meta2 = MetaDataOf<T*>();
   REQUIRE(meta2);
   REQUIRE(meta2->mCloneConstructor);
}

TEMPLATE_TEST_CASE("Testing non-clone-makable types", "[semantics]",
   //TMany<IncompleteType>,
   TMany<NonDestructible>,
   TMany<Destructible>,
   TMany<PrivatelyConstructible>,
   TMany<NonSemanticConstructible>,
   TMany<DescriptorConstructible>,
   TMany<Complex>,
   TMany<ContainsComplex>,
   TMany<AggregateTypeComplex>
) {
   using T = TestType;
   static_assert(not CT::DeepMakable<TypeOf<T>, Cloned<TestType>>);
   static_assert(not CT::CloneMakable<T>);
   static_assert(not CT::CloneMakable<T*>);
   static_assert(not CT::SemanticMakable<Cloned, T>);
   static_assert(not CT::SemanticMakable<Cloned, T*>);
   static_assert(not CT::SemanticMakableAlt<Cloned<T>>);
   static_assert(not CT::SemanticMakableAlt<Cloned<T*>>);

   auto meta = MetaDataOf<Conditional<CT::Complete<T>, T, T*>>();
   REQUIRE(meta);
   REQUIRE_FALSE(meta->mCloneConstructor);
}

TEMPLATE_TEST_CASE("Testing clone-assignable types", "[semantics]",
   TMany<ImplicitlyConstructible>,
   TMany<AllSemanticConstructibleImplicit>,
   TMany<AllSemanticConstructibleAndAssignable>,
   TMany<bool>, TMany<uint32_t>, TMany<float>, TMany<char>, TMany<wchar_t>, TMany<char8_t>, TMany<Langulus::Byte>,
   TMany<AMeta>, TMany<TMeta>, TMany<CMeta>, TMany<DMeta>, TMany<VMeta>,
   TMany<ForcefullyPod>,
   TMany<AggregateType>,
   TMany<AllSemanticConstructible>,       // see @attention notice in the test below
   TMany<PartiallySemanticConstructible>  // see @attention notice in the test below
) {
   using T = TestType;
   static_assert(    CT::CloneAssignable<T>);
   static_assert(not CT::CloneAssignable<const T>);
   static_assert(    CT::CloneAssignable<T*>);
   static_assert(not CT::CloneAssignable<const T*>);
   static_assert(    CT::SemanticAssignable<Cloned, T>);
   static_assert(not CT::SemanticAssignable<Cloned, const T>);
   static_assert(    CT::SemanticAssignable<Cloned, T*>);
   static_assert(not CT::SemanticAssignable<Cloned, const T*>);
   static_assert(    CT::SemanticAssignableAlt<Cloned<T>>);
   static_assert(not CT::SemanticAssignableAlt<Cloned<const T>>);
   static_assert(    CT::SemanticAssignableAlt<Cloned<T*>>);
   static_assert(not CT::SemanticAssignableAlt<Cloned<const T*>>);

   auto meta1 = MetaDataOf<T>();
   REQUIRE(meta1);
   REQUIRE(meta1->mCloneAssigner);

   auto meta2 = MetaDataOf<T*>();
   REQUIRE(meta2);
   REQUIRE(meta2->mCloneAssigner);

   auto meta3 = MetaDataOf<const T>();
   REQUIRE(meta3);
   REQUIRE(meta3->mCloneAssigner);
}

TEMPLATE_TEST_CASE("Testing non-clone-assignable types", "[semantics]",
   //TMany<IncompleteType>,
   TMany<NonDestructible>,
   TMany<Destructible>,
   TMany<PrivatelyConstructible>,
   TMany<NonSemanticConstructible>,
   TMany<Complex>,
   TMany<ContainsComplex>,
   //TMany<AllSemanticConstructible>,        // see @attention notice below    
   //TMany<PartiallySemanticConstructible>,  // see @attention notice below    
   TMany<DescriptorConstructible>,
   TMany<AggregateTypeComplex>
) {
   using T = TestType;
   static_assert(not CT::DeepAssignable<TypeOf<T>, Cloned<TestType>>);
   // @attention since TMany semantic constructor isn't explicit, types 
   // that have explicit semantic constructors (and thus no implicit    
   // semantic assigners), will still be clone-assignable when wrapped  
   // in a TMany - new elements will simply be re-constructed in the    
   // container. This might not be intended, but is too hard to work    
   // around currently.                                                 
   static_assert(not CT::CloneAssignable<T>);
   static_assert(not CT::CloneAssignable<const T>);
   static_assert(not CT::CloneAssignable<T*>);
   static_assert(not CT::CloneAssignable<const T*>);
   static_assert(not CT::SemanticAssignable<Cloned, T>);
   static_assert(not CT::SemanticAssignable<Cloned, const T>);
   static_assert(not CT::SemanticAssignable<Cloned, T*>);
   static_assert(not CT::SemanticAssignable<Cloned, const T*>);
   static_assert(not CT::SemanticAssignableAlt<Cloned<T>>);
   static_assert(not CT::SemanticAssignableAlt<Cloned<const T>>);
   static_assert(not CT::SemanticAssignableAlt<Cloned<T*>>);
   static_assert(not CT::SemanticAssignableAlt<Cloned<const T*>>);

   auto meta = MetaDataOf<Conditional<CT::Complete<T>, T, T*>>();
   REQUIRE(meta);
   REQUIRE_FALSE(meta->mCloneAssigner);
}


///                                                                           
///   Disown semantics                                                        
///                                                                           
TEMPLATE_TEST_CASE("Testing disown-makable types", "[semantics]",
   TMany<ImplicitlyConstructible>,
   TMany<AllSemanticConstructible>,
   TMany<AllSemanticConstructibleAndAssignable>,
   TMany<PartiallySemanticConstructible>,
   TMany<bool>, TMany<uint32_t>, TMany<float>, TMany<char>, TMany<wchar_t>, TMany<char8_t>, TMany<Langulus::Byte>,
   TMany<AMeta>, TMany<TMeta>, TMany<CMeta>, TMany<DMeta>, TMany<VMeta>,
   TMany<ForcefullyPod>,
   TMany<AggregateType>,
   TMany<NonDestructible>,
   TMany<Destructible>,
   TMany<PrivatelyConstructible>,
   TMany<NonSemanticConstructible>,
   TMany<DescriptorConstructible>,
   TMany<Complex>,
   TMany<ContainsComplex>,
   TMany<AggregateTypeComplex>
) {
   using T = TestType;
   static_assert(CT::DisownMakable<T>);
   static_assert(CT::DisownMakable<T*>);
   static_assert(CT::SemanticMakable<Disowned, T>);
   static_assert(CT::SemanticMakable<Disowned, T*>);
   static_assert(CT::SemanticMakableAlt<Disowned<T>>);
   static_assert(CT::SemanticMakableAlt<Disowned<T*>>);

   auto meta1 = MetaDataOf<T>();
   REQUIRE(meta1);
   REQUIRE(meta1->mDisownConstructor);

   auto meta2 = MetaDataOf<T*>();
   REQUIRE(meta2);
   REQUIRE(meta2->mDisownConstructor);
}

/*TEMPLATE_TEST_CASE("Testing non-disown-makable types", "[semantics]",
   //TMany<IncompleteType>,
   TMany<PrivatelyConstructible>,
   TMany<NonSemanticConstructible>,
   TMany<DescriptorConstructible>,
   TMany<Complex>,
   TMany<ContainsComplex>,
   TMany<AggregateTypeComplex>
) {
   using T = TestType;
   static_assert(not CT::DisownMakable<T>);
   static_assert(    CT::DisownMakable<T*>);
   static_assert(not CT::SemanticMakable<Disowned, T>);
   static_assert(    CT::SemanticMakable<Disowned, T*>);
   static_assert(not CT::SemanticMakableAlt<Disowned<T>>);
   static_assert(    CT::SemanticMakableAlt<Disowned<T*>>);

   auto meta = MetaDataOf<Conditional<CT::Complete<T>, T, T*>>();
   REQUIRE(meta);
   REQUIRE_FALSE(meta->mDisownConstructor);
}*/

TEMPLATE_TEST_CASE("Testing disown-assignable types", "[semantics]",
   TMany<ImplicitlyConstructible>,
   TMany<AllSemanticConstructibleImplicit>,
   TMany<AllSemanticConstructibleAndAssignable>,
   TMany<bool>, TMany<uint32_t>, TMany<float>, TMany<char>, TMany<wchar_t>, TMany<char8_t>, TMany<Langulus::Byte>,
   TMany<AMeta>, TMany<TMeta>, TMany<CMeta>, TMany<DMeta>, TMany<VMeta>,
   TMany<ForcefullyPod>,
   TMany<AggregateType>,
   TMany<NonDestructible>,
   TMany<Destructible>,
   TMany<PrivatelyConstructible>,
   TMany<NonSemanticConstructible>,
   TMany<DescriptorConstructible>,
   TMany<Complex>,
   TMany<ContainsComplex>,
   TMany<AllSemanticConstructible>,
   TMany<PartiallySemanticConstructible>
) {
   using T = TestType;
   static_assert(    CT::DisownAssignable<T>);
   static_assert(not CT::DisownAssignable<const T>);
   static_assert(    CT::DisownAssignable<T*>);
   static_assert(    CT::DisownAssignable<const T*>);
   static_assert(    CT::SemanticAssignable<Disowned, T>);
   static_assert(not CT::SemanticAssignable<Disowned, const T>);
   static_assert(    CT::SemanticAssignable<Disowned, T*>);
   static_assert(    CT::SemanticAssignable<Disowned, const T*>);
   static_assert(    CT::SemanticAssignableAlt<Disowned<T>>);
   static_assert(not CT::SemanticAssignableAlt<Disowned<const T>>);
   static_assert(    CT::SemanticAssignableAlt<Disowned<T*>>);
   static_assert(    CT::SemanticAssignableAlt<Disowned<const T*>>);

   auto meta1 = MetaDataOf<T>();
   REQUIRE(meta1);
   REQUIRE(meta1->mDisownAssigner);

   auto meta2 = MetaDataOf<T*>();
   REQUIRE(meta2);
   REQUIRE(meta2->mDisownAssigner);

   auto meta3 = MetaDataOf<const T>();
   REQUIRE(meta3);
   REQUIRE(meta3->mDisownAssigner);
}

/*TEMPLATE_TEST_CASE("Testing non-disown-assignable types", "[semantics]",
   //TMany<IncompleteType>,
   TMany<NonDestructible>,
   TMany<Destructible>,
   TMany<PrivatelyConstructible>,
   TMany<NonSemanticConstructible>,
   TMany<DescriptorConstructible>,
   TMany<Complex>,
   TMany<ContainsComplex>,
   TMany<AllSemanticConstructible>,
   TMany<PartiallySemanticConstructible>
) {
   using T = TestType;
   static_assert(not CT::DisownAssignable<T>);
   static_assert(not CT::DisownAssignable<const T>);
   static_assert(    CT::DisownAssignable<T*>);
   static_assert(    CT::DisownAssignable<const T*>);
   static_assert(not CT::SemanticAssignable<Disowned, T>);
   static_assert(not CT::SemanticAssignable<Disowned, const T>);
   static_assert(    CT::SemanticAssignable<Disowned, T*>);
   static_assert(    CT::SemanticAssignable<Disowned, const T*>);
   static_assert(not CT::SemanticAssignableAlt<Disowned<T>>);
   static_assert(not CT::SemanticAssignableAlt<Disowned<const T>>);
   static_assert(    CT::SemanticAssignableAlt<Disowned<T*>>);
   static_assert(    CT::SemanticAssignableAlt<Disowned<const T*>>);

   auto meta = MetaDataOf<Conditional<CT::Complete<T>, T, T*>>();
   REQUIRE(meta);
   REQUIRE_FALSE(meta->mDisownAssigner);
}*/


///                                                                           
///   Abandon semantics                                                       
///                                                                           
TEMPLATE_TEST_CASE("Testing abandon-makable types", "[semantics]",
   TMany<ImplicitlyConstructible>,
   TMany<Destructible>,
   TMany<NonSemanticConstructible>,
   TMany<AllSemanticConstructible>,
   TMany<AllSemanticConstructibleAndAssignable>,
   TMany<PartiallySemanticConstructible>,
   TMany<DescriptorConstructible>,
   TMany<Complex>,
   TMany<ContainsComplex>,
   TMany<bool>, TMany<uint32_t>, TMany<float>, TMany<char>, TMany<wchar_t>, TMany<char8_t>, TMany<Langulus::Byte>,
   TMany<AMeta>, TMany<TMeta>, TMany<CMeta>, TMany<DMeta>, TMany<VMeta>,
   TMany<AggregateType>,
   TMany<ForcefullyPod>,
   TMany<NonDestructible>,
   TMany<PrivatelyConstructible>
) {
   using T = TestType;
   static_assert(CT::AbandonMakable<T>);
   static_assert(CT::AbandonMakable<T*>);
   static_assert(CT::SemanticMakable<Abandoned, T>);
   static_assert(CT::SemanticMakable<Abandoned, T*>);
   static_assert(CT::SemanticMakableAlt<Abandoned<T>>);
   static_assert(CT::SemanticMakableAlt<Abandoned<T*>>);

   auto meta1 = MetaDataOf<T>();
   REQUIRE(meta1);
   REQUIRE(meta1->mAbandonConstructor);

   auto meta2 = MetaDataOf<T*>();
   REQUIRE(meta2);
   REQUIRE(meta2->mAbandonConstructor);
}

/*TEMPLATE_TEST_CASE("Testing non-abandon-makable types", "[semantics]",
   //TMany<IncompleteType>,
   TMany<NonDestructible>,
   TMany<PrivatelyConstructible>
) {
   using T = TestType;
   static_assert(not CT::AbandonMakable<T>);
   static_assert(    CT::AbandonMakable<T*>);
   static_assert(not CT::SemanticMakable<Abandoned, T>);
   static_assert(    CT::SemanticMakable<Abandoned, T*>);
   static_assert(not CT::SemanticMakableAlt<Abandoned<T>>);
   static_assert(    CT::SemanticMakableAlt<Abandoned<T*>>);

   auto meta = MetaDataOf<Conditional<CT::Complete<T>, T, T*>>();
   REQUIRE(meta);
   REQUIRE_FALSE(meta->mAbandonConstructor);
}*/

TEMPLATE_TEST_CASE("Testing abandon-assignable types", "[semantics]",
   TMany<ImplicitlyConstructible>,
   TMany<Destructible>,
   TMany<NonSemanticConstructible>,
   TMany<AllSemanticConstructible>,
   TMany<AllSemanticConstructibleAndAssignable>,
   TMany<PartiallySemanticConstructible>,
   TMany<DescriptorConstructible>,
   TMany<bool>, TMany<uint32_t>, TMany<float>, TMany<char>, TMany<wchar_t>, TMany<char8_t>, TMany<Langulus::Byte>,
   TMany<AMeta>, TMany<TMeta>, TMany<CMeta>, TMany<DMeta>, TMany<VMeta>,
   TMany<AggregateType>,
   TMany<ForcefullyPod>,
   TMany<NonDestructible>,
   TMany<Complex>,
   TMany<ContainsComplex>,
   TMany<PrivatelyConstructible>
) {
   using T = TestType;
   static_assert(    CT::AbandonAssignable<T>);
   static_assert(not CT::AbandonAssignable<const T>);
   static_assert(    CT::AbandonAssignable<T*>);
   static_assert(    CT::AbandonAssignable<const T*>);
   static_assert(    CT::SemanticAssignable<Abandoned, T>);
   static_assert(not CT::SemanticAssignable<Abandoned, const T>);
   static_assert(    CT::SemanticAssignable<Abandoned, T*>);
   static_assert(    CT::SemanticAssignable<Abandoned, const T*>);
   static_assert(    CT::SemanticAssignableAlt<Abandoned<T>>);
   static_assert(not CT::SemanticAssignableAlt<Abandoned<const T>>);
   static_assert(    CT::SemanticAssignableAlt<Abandoned<T*>>);
   static_assert(    CT::SemanticAssignableAlt<Abandoned<const T*>>);

   auto meta1 = MetaDataOf<T>();
   REQUIRE(meta1);
   REQUIRE(meta1->mAbandonAssigner);

   auto meta2 = MetaDataOf<T*>();
   REQUIRE(meta2);
   REQUIRE(meta2->mAbandonAssigner);

   auto meta3 = MetaDataOf<const T>();
   REQUIRE(meta3);
   REQUIRE(meta3->mAbandonAssigner);
}

/*TEMPLATE_TEST_CASE("Testing non-abandon-assignable types", "[semantics]",
   //TMany<IncompleteType>,
   TMany<NonDestructible>,
   TMany<Complex>,
   TMany<ContainsComplex>,
   TMany<PrivatelyConstructible>
) {
   using T = TestType;
   static_assert(not CT::AbandonAssignable<T>);
   static_assert(not CT::AbandonAssignable<const T>);
   static_assert(    CT::AbandonAssignable<T*>);
   static_assert(    CT::AbandonAssignable<const T*>);
   static_assert(not CT::SemanticAssignable<Abandoned, T>);
   static_assert(not CT::SemanticAssignable<Abandoned, const T>);
   static_assert(    CT::SemanticAssignable<Abandoned, T*>);
   static_assert(    CT::SemanticAssignable<Abandoned, const T*>);
   static_assert(not CT::SemanticAssignableAlt<Abandoned<T>>);
   static_assert(not CT::SemanticAssignableAlt<Abandoned<const T>>);
   static_assert(    CT::SemanticAssignableAlt<Abandoned<T*>>);
   static_assert(    CT::SemanticAssignableAlt<Abandoned<const T*>>);

   auto meta = MetaDataOf<Conditional<CT::Complete<T>, T, T*>>();
   REQUIRE(meta);
   REQUIRE_FALSE(meta->mAbandonAssigner);
}
*/

///                                                                           
///   Descriptor semantics                                                    
///                                                                           
TEMPLATE_TEST_CASE("Testing descriptor-makable types", "[semantics]",
   TMany<AllSemanticConstructible>,
   TMany<AllSemanticConstructibleAndAssignable>,
   TMany<DescriptorConstructible>
) {
   using T = TestType;
   static_assert(    CT::DeepMakable<TypeOf<T>, Describe>);
   static_assert(not CT::DeepAssignable<TypeOf<T>, Describe>);
   static_assert(    CT::DescriptorMakable<T>);
   static_assert(not CT::DescriptorMakable<T*>);
   static_assert(not CT::SemanticMakableAlt<Describe>);

   auto meta1 = MetaDataOf<T>();
   REQUIRE(meta1);
   REQUIRE(meta1->mDescriptorConstructor);

   auto meta2 = MetaDataOf<T*>();
   REQUIRE(meta2);
   REQUIRE(meta2->mDescriptorConstructor);
}

TEMPLATE_TEST_CASE("Testing non-descriptor-makable types", "[semantics]",
   //TMany<IncompleteType>,
   TMany<ImplicitlyConstructible>,
   TMany<NonDestructible>,
   TMany<Destructible>,
   TMany<PrivatelyConstructible>,
   TMany<NonSemanticConstructible>,
   TMany<PartiallySemanticConstructible>,
   TMany<Complex>,
   TMany<ContainsComplex>,
   TMany<bool>, TMany<uint32_t>, TMany<float>, TMany<char>, TMany<wchar_t>, TMany<char8_t>, TMany<Langulus::Byte>,
   TMany<AMeta>, TMany<TMeta>, TMany<CMeta>, TMany<DMeta>, TMany<VMeta>,
   TMany<AggregateType>,
   TMany<ForcefullyPod>,
   TMany<AggregateThatCanBeConfusedWithDescriptorMakable>
) {
   using T = TestType;
   static_assert(not CT::UnfoldMakableFrom<TypeOf<T>, Describe>);
   static_assert(not CT::DeepMakable<TypeOf<T>, Describe>);
   static_assert(not CT::DeepAssignable<TypeOf<T>, Describe>);
   static_assert(not CT::DescriptorMakable<T>);
   static_assert(not CT::DescriptorMakable<T*>);
   static_assert(not CT::SemanticMakableAlt<Describe>);

   auto meta = MetaDataOf<Conditional<CT::Complete<T>, T, T*>>();
   REQUIRE(meta);
   REQUIRE_FALSE(meta->mDescriptorConstructor);
}
