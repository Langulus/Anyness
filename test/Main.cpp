///                                                                           
/// Langulus::Anyness                                                         
/// Copyright (c) 2012 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#include "Main.hpp"

#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

LANGULUS_RTTI_BOUNDARY(RTTI::MainBoundary)

TMany<Many> BANK {};


int main(int argc, char* argv[]) {
	Catch::Session session;
	const auto result = session.run(argc, argv);

   // Destroy BANK before static data - otherwise problems happen if    
   // not using managed reflection                                      
   BANK.Reset();

   return result;
}
