#pragma once

namespace Langulus::Anyness
{

	///																								
	///	Data state flags																		
	///																								
	struct DataState {
		enum Enum {
			// Default data state														
			// Default state is inclusive, mutable, nonpolar, nonvacuum		
			// nonstatic, nonencrypted, noncompressed, and dense				
			Default = 0,

			// Enables phase for the data				 								
			// Phases is very useful to mark data dependencies without		
			// actually changing the data itself									
			Phased = 1,

			// Enables vacuum for the data											
			// A vacuum data is considered only a hint, that is used to		
			// direct expansion for the data											
			// You can enable that by using the '?' symbol in GASM			
			Missing = 2,

			// The data is compressed, and it is your respondibility to		
			// decompress it before using it											
			Compressed = 4,

			// The data is encrypted, and it is your respondibility to		
			// decrypt it with the correct key before using it					
			Encrypted = 8,

			// Enables inhibition (or so called exclusive (OR) container)	
			// An OR container means that data can be used in multiple		
			// parallel ways. Verbs in such containers, for example, are	
			// considered branched. Beware, using OR containers might		
			// cause	huge overhead, because in some contexts a full			
			// stack cloning might occur												
			Or = 16,

			// Future phase, when phased												
			// If state is only Phased, then it is considered past			
			Future = 32,

			// Data won't move, reallocate, deallocate, etc.					
			// Used to constrain the memory manipulations. That way you		
			// can reference static or unmovable memory as any other.		
			// Data can still change in-place, unless constant.				
			// Reflected members are interfaced in that way						
			Static = 64,

			// Data won't move, reallocate, deallocate, or even change		
			// Used to constrain the memory manipulations for safety			
			// That way you can interface constant memory as any other		
			Constant = 128,

			// Data won't ever change type - useful for templated packs		
			// Used to constrain the memory manipulations for safety			
			Typed = 256,

			// Data is sparse, essentially made of pointers						
			Sparse = 512,

			// Data is constrained														
			Constrained = Static | Constant | Typed,
			Member = Static | Typed,
			ConstantMember = Constrained
		};

		using Type = std::underlying_type_t<Enum>;

		Type mState {Default};

	public:
		constexpr DataState() noexcept = default;
		constexpr DataState(const Type& state) noexcept
			: mState {state} {}

		explicit operator bool() const noexcept { return mState != 0; }
		constexpr bool operator == (const DataState&) const noexcept = default;
		constexpr bool operator != (const DataState&) const noexcept = default;
	};

} // namespace Langulus::Anyness
