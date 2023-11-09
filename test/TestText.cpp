///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#include <Anyness/Text.hpp>
#include <Anyness/Path.hpp>
#include <Anyness/Trait.hpp>
#include "Common.hpp"


/// A type that is reflected, as convertible to Debug                         
struct Stringifiable {
   LANGULUS_CONVERSIONS(Debug);
   
   explicit operator Debug() {
      return "Stringifiable converted to Debug";
   }
};

/// A type that is reflected, as convertible to Debug                         
struct StringifiableConst {
   LANGULUS_CONVERSIONS(Debug);
   
   explicit operator Debug() const {
      return "Stringifiable converted to Debug";
   }
};


///                                                                           
/// Possible states:                                                          
///   - uninitialized                                                         
///   - default                                                               
void CheckState_Default(const Text&);
///   - invariant                                                             
void CheckState_Invariant(const Text&);
///   - owned-full                                                            
void CheckState_OwnedFull(const Text&);
///   - owned-full-const                                                      
void CheckState_OwnedFullConst(const Text&);
///   - owned-empty                                                           
void CheckState_OwnedEmpty(const Text&);
///   - disowned-full                                                         
void CheckState_DisownedFull(const Text&);
///   - disowned-full-const                                                   
void CheckState_DisownedFullConst(const Text&);
///   - abandoned                                                             
void CheckState_Abandoned(const Text&);

///                                                                           
/// Possible actions for each state:                                          
///   - uninitialized                                                         
///      - constexpr-default-initialized                                      
///      - runtime-default-initialized                                        
///      - semantic-initialized from container                                
///      - semantic-initialized from dense letter                             
///      - semantic-initialized from dense std::string                        
///      - semantic-initialized from dense std::string_view                   
///      - semantic-initialized from dense number (stringification)           
///      - semantic-initialized from sparse meta (stringification)            
///      - semantic-initialized from exception (stringification)              
///      - semantic-initialized from sparse element, zero-terminated          
///      - semantic-initialized from sparse element, bound-terminated         
///      - semantic-initialized from sparse element, count-terminated         

TEMPLATE_TEST_CASE("Testing text containers", "[text]",
   Text, Debug, Path
) {
   IF_LANGULUS_MANAGED_MEMORY(Allocator::CollectGarbage());

   GIVEN("Default text container") {
      TestType text;

      WHEN("Nothing is done") {
         CheckState_Default(text);
      }

      WHEN("Capacity is reserved") {
         text.Reserve(500);

         CheckState_OwnedEmpty(text);
         REQUIRE(text.GetReserved() >= 500);
      }

      WHEN("Directly assigned to itself") {
         text = text;

         CheckState_Default(text);
      }

      WHEN("Indirectly assigned to itself") {
         const auto anothertext = text;
         text = anothertext;

         CheckState_Default(text);
      }
   }

   GIVEN("Uninitialized text container") {
      TestType* text;

      WHEN("Constructed with a null-terminated literal") {
         text = new TestType {"test1"};

         CheckState_OwnedFull(*text);
         REQUIRE((*text).GetCount() == 5);
         REQUIRE((*text).GetReserved() >= 5);
         REQUIRE((*text) == "test1");
         REQUIRE((*text)[0] == 't');
         REQUIRE((*text)[1] == 'e');
         REQUIRE((*text)[2] == 's');
         REQUIRE((*text)[3] == 't');
         REQUIRE((*text)[4] == '1');
         REQUIRE_THROWS((*text)[5] == '?');
         delete text;
      }

      WHEN("Constructed with a count-terminated literal") {
         text = new TestType {"test2", 5};

         CheckState_OwnedFull(*text);
         REQUIRE((*text).GetCount() == 5);
         REQUIRE((*text).GetReserved() >= 5);
         REQUIRE((*text) == "test2");
         REQUIRE((*text)[0] == 't');
         REQUIRE((*text)[1] == 'e');
         REQUIRE((*text)[2] == 's');
         REQUIRE((*text)[3] == 't');
         REQUIRE((*text)[4] == '2');
         REQUIRE_THROWS((*text)[5] == '?');
         delete text;
      }

      WHEN("Constructed with a c-array") {
         char test1[] = "test3";
         text = new TestType {test1};

         CheckState_OwnedFull(*text);
         REQUIRE((*text).GetCount() == 5);
         REQUIRE((*text).GetReserved() >= 5);
         REQUIRE((*text) == "test3");
         REQUIRE((*text)[0] == 't');
         REQUIRE((*text)[1] == 'e');
         REQUIRE((*text)[2] == 's');
         REQUIRE((*text)[3] == 't');
         REQUIRE((*text)[4] == '3');
         REQUIRE_THROWS((*text)[5] == '?');
         delete text;
      }

      WHEN("Constructed with a nullptr_t") {
         text = new TestType {nullptr};

         CheckState_Default(*text);
         delete text;
      }

      WHEN("Constructed with a nullptr c-array") {
         text = new TestType {(char*)nullptr};

         CheckState_Default(*text);
         delete text;
      }

      WHEN("Constructed with empty c-array") {
         text = new TestType {""};

         CheckState_Default(*text);
         delete text;
      }

      WHEN("Constructed with a single character") {
         text = new TestType {'?'};

         CheckState_OwnedFull(*text);
         REQUIRE((*text).GetCount() == 1);
         REQUIRE((*text).GetReserved() >= 1);
         REQUIRE((*text)[0] == '?');
         REQUIRE_THROWS((*text)[1] == '?');
         delete text;
      }
   }

   GIVEN("Reserved text container") {
      TestType text;
      text.Reserve(500);
      auto memory = text.GetRaw();

      WHEN("Text is extended") {
         auto region = text.Extend(10);

         THEN("The count change") {
            REQUIRE(text.GetCount() == 10);
            REQUIRE(text.GetReserved() >= 500);
            REQUIRE(text.GetRaw() == memory);
            REQUIRE(text.HasAuthority());
            REQUIRE(region.GetCount() == 10);
            REQUIRE(region.GetRaw() == memory);
         }
      }

      WHEN("Text is concatenated") {
         text += "test";

         THEN("The count change") {
            REQUIRE(text.GetCount() == 4);
            REQUIRE(text.GetReserved() >= 500);
            REQUIRE(text.GetRaw() == memory);
            REQUIRE(text.HasAuthority());
            REQUIRE(text == "test");
         }
      }

      WHEN("Text is cleared") {
         text += "test";
         text.Clear();

         THEN("Nothing should change") {
            REQUIRE(text.GetCount() == 0);
            REQUIRE(text.GetReserved() >= 500);
            REQUIRE(text.GetRaw() == memory);
            REQUIRE(text.HasAuthority());
            REQUIRE(text != "test");
         }
      }

      WHEN("Text is reset") {
         text += "test";
         text.Reset();

         THEN("Memory is released") {
            REQUIRE(text.GetCount() == 0);
            REQUIRE(text.GetReserved() == 0);
            REQUIRE(text.GetRaw() == nullptr);
            REQUIRE(text.GetType() == MetaData::Of<Letter>());
            REQUIRE_FALSE(text.HasAuthority());
            REQUIRE(text != "test");
         }
      }
   }

   GIVEN("Full text container") {
      IF_LANGULUS_MANAGED_MEMORY(Allocator::CollectGarbage());

      TestType text {"test1"};
      auto memory = text.GetRaw();

      WHEN("Add more text") {
         text += "test2";
         THEN("The size and capacity change, type will never change, and memory won't move if MANAGED_MEMORY feature is enabled") {
            REQUIRE(text == "test1test2");
            REQUIRE(text.GetCount() == 10);
            REQUIRE(text.GetReserved() >= 10);
            #if LANGULUS_FEATURE(MANAGED_MEMORY)
               REQUIRE(text.GetRaw() == memory);
            #endif
            REQUIRE(text.HasAuthority());
            REQUIRE(text.template Is<Letter>());
         }
      }

      WHEN("More capacity is reserved") {
         text.Reserve(20);
         THEN("The capacity changes but not the size, memory will move in order to have jurisdiction") {
            REQUIRE(text.GetCount() == 5);
            REQUIRE(text.GetReserved() >= 20);
            #if LANGULUS_FEATURE(MANAGED_MEMORY)
               REQUIRE(text.GetRaw() == memory);
            #endif
            REQUIRE(text.HasAuthority());
         }
      }

      WHEN("More capacity is reserved, via Extend()") {
         auto region = text.Extend(10);
         THEN("The capacity and size change") {
            REQUIRE(text.GetCount() == 15);
            REQUIRE(text.GetReserved() >= 15);
            #if LANGULUS_FEATURE(MANAGED_MEMORY)
               REQUIRE(text.GetRaw() == memory);
            #endif
            REQUIRE(text.HasAuthority());
            REQUIRE(region.GetCount() == 10);
            REQUIRE(region.GetRaw() == text.GetRaw() + 5);
         }
      }

      WHEN("Less capacity is reserved") {
         text.Reserve(2);
         THEN("Capacity is not changed, but count is trimmed; memory will not move, and memory will still be outside jurisdiction") {
            REQUIRE(text.GetCount() == 2);
            REQUIRE(text.GetReserved() >= 5);
            REQUIRE(text.GetRaw() == memory);
            REQUIRE(text.HasAuthority());
         }
      }

      WHEN("Text is cleared") {
         text.Clear();
         THEN("Size goes to zero, capacity and type are unchanged") {
            REQUIRE(text.GetCount() == 0);
            REQUIRE(text.GetReserved() >= 5);
            REQUIRE(text.GetRaw() == memory);
            REQUIRE(text.HasAuthority());
            REQUIRE(text.template Is<Letter>());
         }
      }

      WHEN("Text is reset") {
         text.Reset();
         THEN("Size and capacity goes to zero, type is unchanged, because it's a templated container") {
            REQUIRE(text.GetCount() == 0);
            REQUIRE(text.GetReserved() == 0);
            REQUIRE_FALSE(text.GetRaw());
            REQUIRE(text.template Is<Letter>());
         }
      }

      WHEN("Text is copied shallowly") {
         TestType copy = text;
         THEN("The new container shall contain the same data, in the same memory block; data will only be referenced") {
            REQUIRE(text.GetCount() == copy.GetCount());
            REQUIRE(text.GetReserved() == copy.GetReserved());
            REQUIRE(text.GetRaw() == copy.GetRaw());
            REQUIRE(text.GetType() == copy.GetType());
            REQUIRE(text.HasAuthority());
            REQUIRE(copy.HasAuthority());
            REQUIRE(copy.GetUses() == 2);
            REQUIRE(text.GetUses() == 2);
         }
      }

      WHEN("Text is cloned (deep copy)") {
         TestType copy = Clone(text);
         THEN("The new container shall contain the same data, but in separate memory block") {
            REQUIRE(text.GetCount() == copy.GetCount());
            REQUIRE(text.GetReserved() >= copy.GetReserved());
            REQUIRE(text.GetRaw() != copy.GetRaw());
            REQUIRE(text.GetType() == copy.GetType());
            REQUIRE(text.HasAuthority());
            REQUIRE(copy.HasAuthority());
            REQUIRE(copy.GetUses() == 1);
            REQUIRE(text.GetUses() == 1);
         }
      }

      WHEN("Text is reset, then allocated again") {
         text.Reset();
         text += "kurec";
         THEN("Block manager should reuse the memory") {
            REQUIRE(text.GetCount() == 5);
            REQUIRE(text.GetReserved() >= 5);
            REQUIRE(text.HasAuthority());
            REQUIRE(text.template Is<Letter>());
         }
      }

      WHEN("Texts are compared") {
         THEN("The results should match") {
            REQUIRE(text == "test1");
            REQUIRE(text != "Tests");
         }
      }
   }
}

TEMPLATE_TEST_CASE("Unsigned number stringification", "[text]",
   /*uint8_t,*/ uint16_t, uint32_t, uint64_t
) {
   WHEN("Constructed Text with a number") {
      Text* text = new Text {TestType{66}};

      THEN("The capacity and size change") {
         REQUIRE((*text).GetCount() == 2);
         REQUIRE((*text).GetReserved() >= 2);
         REQUIRE((*text).template Is<Letter>());
         REQUIRE((*text).GetRaw());
         REQUIRE((*text).HasAuthority());
         REQUIRE((*text) == "66");
      }

      delete text;
   }

   WHEN("Constructed Debug with a number") {
      Debug* text = new Debug {TestType{66}};

      THEN("The capacity and size change") {
         REQUIRE((*text).GetCount() == 2);
         REQUIRE((*text).GetReserved() >= 2);
         REQUIRE((*text).template Is<Letter>());
         REQUIRE((*text).GetRaw());
         REQUIRE((*text).HasAuthority());
         REQUIRE((*text) == "66");
      }

      delete text;
   }

   WHEN("Constructed Path with a number") {
      Path* text = new Path {TestType{66}};

      THEN("The capacity and size change") {
         REQUIRE((*text).GetCount() == 2);
         REQUIRE((*text).GetReserved() >= 2);
         REQUIRE((*text).template Is<Letter>());
         REQUIRE((*text).GetRaw());
         REQUIRE((*text).HasAuthority());
         REQUIRE((*text) == "66");
      }

      delete text;
   }
}

TEMPLATE_TEST_CASE("Signed number stringification", "[text]", int8_t, int16_t, int32_t, int64_t) {
   WHEN("Constructed Text with a number") {
      Text* text = new Text {TestType{-66}};

      THEN("The capacity and size change") {
         REQUIRE((*text).GetCount() == 3);
         REQUIRE((*text).GetReserved() >= 3);
         REQUIRE((*text).template Is<Letter>());
         REQUIRE((*text).GetRaw());
         REQUIRE((*text).HasAuthority());
         REQUIRE((*text) == "-66");
      }

      delete text;
   }

   WHEN("Constructed Debug with a number") {
      Debug* text = new Debug {TestType{-66}};

      THEN("The capacity and size change") {
         REQUIRE((*text).GetCount() == 3);
         REQUIRE((*text).GetReserved() >= 3);
         REQUIRE((*text).template Is<Letter>());
         REQUIRE((*text).GetRaw());
         REQUIRE((*text).HasAuthority());
         REQUIRE((*text) == "-66");
      }

      delete text;
   }

   WHEN("Constructed Path with a number") {
      Path* text = new Path {TestType{-66}};

      THEN("The capacity and size change") {
         REQUIRE((*text).GetCount() == 3);
         REQUIRE((*text).GetReserved() >= 3);
         REQUIRE((*text).template Is<Letter>());
         REQUIRE((*text).GetRaw());
         REQUIRE((*text).HasAuthority());
         REQUIRE((*text) == "-66");
      }

      delete text;
   }
}

TEMPLATE_TEST_CASE("Logging text containers", "[text]", Text, Debug, Path) {
   TestType text {"some text"};

   Logger::Info() << "You should see " << text;
   Logger::Info("You should also see ", text);
}

TEMPLATE_TEST_CASE("Reflected coverters to text", "[text]", Stringifiable, StringifiableConst) {
   GIVEN("A stringifiable type") {
      const auto debugMeta = MetaData::Of<Debug>();
      const auto meta = MetaData::Of<TestType>();
      TestType instance;

      WHEN("Converted") {
         // Calling static_cast<Debug> here doesn't work, because of MSVC bug
         const Debug staticallyConverted = instance.operator Debug();
         
         Debug rttiConverted;
         meta->mConverters.at(debugMeta).mFunction(&instance, &rttiConverted);

         THEN("Results should be correct") {
            REQUIRE(staticallyConverted == rttiConverted);
            REQUIRE(staticallyConverted == "Stringifiable converted to Debug");
         }
      }
   }
}

TEMPLATE_TEST_CASE("Text container interoperability", "[text]",
   (TypePair<Text, Debug>),
   (TypePair<Debug, Text>),
   (TypePair<Path, Text>),
   (TypePair<Text, Path>),
   (TypePair<Debug, Path>),
   (TypePair<Path, Debug>)
) {
   using LHS = typename TestType::LHS;
   using RHS = typename TestType::RHS;

   GIVEN("Two types of text containers") {
      WHEN("Constructed") {
         LHS text {RHS{"one"}};

         THEN("Should be identical") {
            REQUIRE(text == "one");
         }
      }

      WHEN("Assigned") {
         LHS text {"one"};
         text = RHS {"two"};

         THEN("Should be identical") {
            REQUIRE(text == "two");
         }
      }

      WHEN("Concatenated (destructively)") {
         LHS text {"one"};
         text += RHS {"two"};

         THEN("Should be concatenated") {
            REQUIRE(text == "onetwo");
         }
      }

      WHEN("Concatenated") {
         LHS text {"one"};
         LHS text2 = text + RHS {"two"};

         THEN("Should be concatenated") {
            REQUIRE(text == "one");
            REQUIRE(text2 == "onetwo");
         }
      }
   }
}

TEMPLATE_TEST_CASE("Containing literals", "[text]",
   Any, Trait
) {
   GIVEN("Two types of text containers") {
      WHEN("Constructed") {
         TestType text {"one"};

         THEN("Should be identical") {
            REQUIRE(text.GetCount() == 1);
            REQUIRE(text.template IsExact<Text>());
            REQUIRE(text.template As<Text>() == "one");
         }
      }

      WHEN("Assigned") {
         TestType text {"one"};
         text = "two";

         THEN("Should be identical") {
            REQUIRE(text.GetCount() == 1);
            REQUIRE(text.template IsExact<Text>());
            REQUIRE(text.template As<Text>() == "two");
         }
      }

      WHEN("Concatenated (destructively)") {
         TestType text {"one"};
         text += TestType {"two"};

         THEN("Should be concatenated") {
            REQUIRE(text.GetCount() == 2);
            REQUIRE(text.template IsExact<Text>());
            REQUIRE(text.template As<Text>(0) == "one");
            REQUIRE(text.template As<Text>(1) == "two");
         }
      }

      WHEN("Concatenated") {
         TestType text {"one"};
         TestType text2 = text + TestType {"two"};

         THEN("Should be concatenated") {
            REQUIRE(text.GetCount() == 1);
            REQUIRE(text2.GetCount() == 2);
            REQUIRE(text.template IsExact<Text>());
            REQUIRE(text2.template IsExact<Text>());
            REQUIRE(text.template As<Text>() == "one");
            REQUIRE(text2.template As<Text>(0) == "one");
            REQUIRE(text2.template As<Text>(1) == "two");
         }
      }
   }
}

void CheckState_Default(const Text& text) {
   REQUIRE_FALSE(text.IsAbstract());
   REQUIRE_FALSE(text.IsCompressed());
   REQUIRE_FALSE(text.IsConstant());
   REQUIRE_FALSE(text.IsDeep());
   REQUIRE_FALSE(text.IsSparse());
   REQUIRE_FALSE(text.IsEncrypted());
   REQUIRE_FALSE(text.IsMissing());
   REQUIRE_FALSE(text.IsOr());
   REQUIRE_FALSE(text.IsStatic());
   REQUIRE      (text.IsTyped());
   REQUIRE_FALSE(text.IsUntyped());
   REQUIRE_FALSE(text.IsValid());
   REQUIRE      (text.IsInvalid());
   REQUIRE_FALSE(text.IsAllocated());
   REQUIRE      (text.IsEmpty());
   REQUIRE_FALSE(text.HasAuthority());
   REQUIRE      (text.IsTypeConstrained());
   REQUIRE      (text.GetType() == MetaData::Of<Letter>());
   REQUIRE      (text.template Is<Letter>());
   REQUIRE      (text.IsNow());
   REQUIRE_FALSE(text.IsFuture());
   REQUIRE_FALSE(text.IsPast());
   REQUIRE      (text.IsDense());
   REQUIRE      (text.GetCount() == 0);
   REQUIRE      (text.GetReserved() == 0);
   REQUIRE      (text.GetUses() == 0);
   REQUIRE      (text.GetRaw() == nullptr);
   REQUIRE      (text == nullptr);
   REQUIRE_FALSE(text != nullptr);
   REQUIRE      (text == (char*)nullptr);
   REQUIRE_FALSE(text != (char*)nullptr);
   REQUIRE      (not text);
   REQUIRE_FALSE(text);
   REQUIRE      (text == "");
   REQUIRE_FALSE(text != "");
   REQUIRE_FALSE(text == "no match");
}

void CheckState_OwnedEmpty(const Text& text) {
   REQUIRE_FALSE(text.IsAbstract());
   REQUIRE_FALSE(text.IsCompressed());
   REQUIRE_FALSE(text.IsConstant());
   REQUIRE_FALSE(text.IsDeep());
   REQUIRE_FALSE(text.IsSparse());
   REQUIRE_FALSE(text.IsEncrypted());
   REQUIRE_FALSE(text.IsMissing());
   REQUIRE_FALSE(text.IsOr());
   REQUIRE_FALSE(text.IsStatic());
   REQUIRE      (text.IsTyped());
   REQUIRE_FALSE(text.IsUntyped());
   REQUIRE_FALSE(text.IsValid());
   REQUIRE      (text.IsInvalid());
   REQUIRE      (text.IsAllocated());
   REQUIRE      (text.IsEmpty());
   REQUIRE      (text.HasAuthority());
   REQUIRE      (text.IsTypeConstrained());
   REQUIRE      (text.GetType() == MetaData::Of<Letter>());
   REQUIRE      (text.template Is<Letter>());
   REQUIRE      (text.IsNow());
   REQUIRE_FALSE(text.IsFuture());
   REQUIRE_FALSE(text.IsPast());
   REQUIRE      (text.IsDense());
   REQUIRE      (text.GetCount() == 0);
   REQUIRE      (text.GetReserved() > 0);
   REQUIRE      (text.GetUses() == 1);
   REQUIRE      (text.GetRaw());
   REQUIRE      (text == nullptr);
   REQUIRE_FALSE(text != nullptr);
   REQUIRE      (text == (char*)nullptr);
   REQUIRE_FALSE(text != (char*)nullptr);
   REQUIRE      (not text);
   REQUIRE_FALSE(text);
   REQUIRE      (text == "");
   REQUIRE_FALSE(text != "");
   REQUIRE_FALSE(text == "no match");
}

void CheckState_OwnedFull(const Text& text) {
   REQUIRE_FALSE(text.IsAbstract());
   REQUIRE_FALSE(text.IsCompressed());
   REQUIRE_FALSE(text.IsConstant());
   REQUIRE_FALSE(text.IsDeep());
   REQUIRE_FALSE(text.IsSparse());
   REQUIRE_FALSE(text.IsEncrypted());
   REQUIRE_FALSE(text.IsMissing());
   REQUIRE_FALSE(text.IsOr());
   REQUIRE      (text.IsTyped());
   REQUIRE_FALSE(text.IsUntyped());
   REQUIRE      (text.IsValid());
   REQUIRE_FALSE(text.IsInvalid());
   REQUIRE_FALSE(text.IsStatic());
   REQUIRE      (text.IsAllocated());
   REQUIRE_FALSE(text.IsEmpty());
   REQUIRE      (text.HasAuthority());
   REQUIRE      (text.IsTypeConstrained());
   REQUIRE      (text.GetType() == MetaData::Of<Letter>());
   REQUIRE      (text.template Is<Letter>());
   REQUIRE      (text.IsNow());
   REQUIRE_FALSE(text.IsFuture());
   REQUIRE_FALSE(text.IsPast());
   REQUIRE      (text.IsDense());
   REQUIRE      (text.GetCount() > 0);
   REQUIRE      (text.GetReserved() > 0);
   REQUIRE      (text.GetUses() > 0);
   REQUIRE      (text.GetRaw());
   REQUIRE      (text != nullptr);
   REQUIRE_FALSE(text == nullptr);
   REQUIRE      (text != (char*)nullptr);
   REQUIRE_FALSE(text == (char*)nullptr);
   REQUIRE      (text);
   REQUIRE_FALSE(not text);
   REQUIRE      (text != "");
   REQUIRE_FALSE(text == "");
   REQUIRE_FALSE(text == "no match");
}
