///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 - 2022 Dimo Markov <langulusteam@gmail.com>					
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#include "Main.hpp"

#define CATCH_CONFIG_RUNNER

#include <catch2/catch.hpp>

int main(int argc, char* argv[]) {
	Catch::Session session;
	return session.run(argc, argv);
}
