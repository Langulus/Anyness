///																									
/// Langulus::Anyness																			
/// Copyright(C) 2012 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#pragma once
#include "TAny.hpp"
#include <cctype>

#define TEMPLATE() template<CT::Data T>

namespace Langulus::Anyness
{

	/// Default construction																	
	/// TAny is type-constrained and always has a type									
	TEMPLATE()
	TAny<T>::TAny()
		: Any {Block {DataState::Typed, MetaData::Of<Decay<T>>()}} {
		if constexpr (CT::Sparse<T>)
			MakeSparse();
		if constexpr (CT::Constant<T>)
			MakeConst();
	}

	/// Shallow-copy construction (const)													
	///	@param other - the TAny to reference											
	TEMPLATE()
	TAny<T>::TAny(const TAny& other)
		: Any {static_cast<const Any&>(other)} { }

	/// Move construction																		
	///	@param other - the TAny to move													
	TEMPLATE()
	TAny<T>::TAny(TAny&& other) noexcept
		: Any {Forward<Any>(other)} { }

	/// Copy-construct from another container by performing runtime type check	
	///	@tparam KEEP - whether or not to reference contents						
	///	@tparam ALT_T - the container type (deducible)								
	///	@param other - the container to incorporate									
	TEMPLATE()
	template<bool KEEP, CT::Deep ALT_T>
	void TAny<T>::ConstructFromContainer(const ALT_T& other) {
		if constexpr (CT::Same<ALT_T, TAny>) {
			CopyProperties<false, false>(other);

			if constexpr (KEEP)
				Keep();
		}
		else {
			if (CastsToMeta(other.GetType())) {
				// Always attempt to copy containers directly first,			
				// instead of doing allocations										
				CopyProperties<false, false>(other);

				if constexpr (KEEP)
					Keep();
				return;
			}

			// Then attempt to push other container, if this container		
			// allows for it																
			if constexpr (CT::Deep<T>) {
				auto& compatible = static_cast<const Decay<T>&>(other);
				Insert<IndexBack, KEEP>(&compatible, &compatible + 1);
			}
			else Throw<Except::Copy>("Bad copy-construction", LANGULUS_LOCATION());
		}
	}

	/// Move-construct from another container by performing runtime type check	
	///	@tparam KEEP - whether or not to reference contents						
	///	@tparam ALT_T - the container type (deducible)								
	///	@param other - the container to incorporate									
	TEMPLATE()
	template<bool KEEP, CT::Deep ALT_T>
	void TAny<T>::ConstructFromContainer(ALT_T&& other) {
		if constexpr (CT::Same<ALT_T, TAny>) {
			CopyProperties<false, true>(other);

			if constexpr (!KEEP)
				other.mEntry = nullptr;
			else {
				other.ResetMemory();
				other.ResetState();
			}
		}
		else {
			if (CastsToMeta(other.GetType())) {
				// Always attempt to copy containers directly first,			
				// instead of doing allocations										
				CopyProperties<false, true>(other);

				if constexpr (!KEEP)
					other.mEntry = nullptr;
				else {
					other.ResetMemory();
					other.ResetState();
				}
				return;
			}

			// Then attempt to push other container, if this container		
			// allows for it																
			if constexpr (CT::Deep<T>)
				Insert<IndexBack, KEEP>(Forward<Decay<T>>(other));
			else
				Throw<Except::Copy>("Bad move-construction", LANGULUS_LOCATION());
		}
	}

	/// Copy-construction from any deep container, with a bit of					
	/// runtime type-checking overhead														
	///	@param other - the anyness to reference										
	TEMPLATE()
	template<CT::Deep ALT_T>
	TAny<T>::TAny(const ALT_T& other) requires DenseButNotTheSame<ALT_T>
		: TAny {} {
		ConstructFromContainer<true>(other);
	}

	TEMPLATE()
	template<CT::Deep ALT_T>
	TAny<T>::TAny(ALT_T& other) requires DenseButNotTheSame<ALT_T>
		: TAny {const_cast<const ALT_T&>(other)} {}

	/// Move-construction from any deep container, with a bit of					
	/// runtime type-checking overhead														
	///	@param other - the anyness to reference										
	TEMPLATE()
	template<CT::Deep ALT_T>
	TAny<T>::TAny(ALT_T&& other) requires DenseButNotTheSame<ALT_T>
		: TAny {} {
		ConstructFromContainer<true>(Forward<Any>(other));
	}

	/// Disown-construction from any deep container, with a bit of					
	/// runtime type-checking overhead														
	///	@param other - the anyness to copy												
	TEMPLATE()
	template<CT::Deep ALT_T>
	constexpr TAny<T>::TAny(Disowned<ALT_T>&& other) requires CT::Dense<ALT_T>
		: TAny {} {
		ConstructFromContainer<false>(other.mValue);
	}

	/// Abandon-construction from any deep container, with a bit of				
	/// runtime type-checking overhead														
	///	@param other - the anyness to copy												
	TEMPLATE()
	template<CT::Deep ALT_T>
	constexpr TAny<T>::TAny(Abandoned<ALT_T>&& other) requires CT::Dense<ALT_T>
		: TAny {} {
		ConstructFromContainer<false>(Move(other.mValue));
	}

	/// Construct by copying/referencing an array of non-block type				
	///	@param start - start of the array												
	///	@param end - end of the array														
	TEMPLATE()
	TAny<T>::TAny(const T* start, const T* end) requires CT::Data<T>
		: Any {start, end} { }

	/// Construct by copying/referencing value of non-block type					
	///	@param other - the value to shallow-copy										
	TEMPLATE()
	TAny<T>::TAny(const T& other) requires CT::CustomData<T>
		: Any {other} { }

	/// Construct by moving a dense value of non-block type							
	///	@param initial - the value to forward and emplace							
	TEMPLATE()
	TAny<T>::TAny(T&& initial) requires CT::CustomData<T>
		: Any {Forward<T>(initial)} { }

	/// Construct by inserting a disowned non-block element							
	///	@param other - the value to insert												
	TEMPLATE()
	TAny<T>::TAny(Disowned<T>&& other) requires CT::CustomData<T>
		: Any {other.Forward()} { }

	/// Construct by inserting an abandoned non-block element						
	///	@param other - the value to insert												
	TEMPLATE()
	TAny<T>::TAny(Abandoned<T>&& other) requires CT::CustomData<T>
		: Any {other.Forward()} { }

	/// Construct manually by interfacing memory directly								
	/// Data will be copied, if not in jurisdiction, which involves a slow		
	/// authority check. If you want to avoid checking and copying, use the		
	/// Disowned override of this function													
	///	@param raw - raw memory to reference, or clone if not owned				
	///	@param count - number of items inside 'raw'									
	TEMPLATE()
	TAny<T>::TAny(const T* raw, const Count& count)
		: Any {Block {DataState::Constrained, MetaData::Of<Decay<T>>(), count, raw}} {
		TakeAuthority();
	}

	/// Construct manually by interfacing memory directly								
	///	@attention unsafe, make sure that lifetime of memory is sane			
	///	@param raw - raw memory to interface without referencing or copying	
	///	@param count - number of items inside 'raw'									
	TEMPLATE()
	TAny<T>::TAny(Disowned<const T*>&& raw, const Count& count) noexcept
		: Any {Block {
			DataState::Constrained, MetaData::Of<Decay<T>>(), count, raw.mValue, nullptr
		}} {}

	/// Destructor																					
	TEMPLATE()
	TAny<T>::~TAny() {
		Free();
	}

	/// Dereference and eventually destroy all elements								
	TEMPLATE()
	void TAny<T>::Free() {
		if (!mEntry)
			return;

		if (mEntry->GetUses() == 1) {
			if constexpr (CT::Sparse<T> || !CT::POD<T>)
				CallKnownDestructors<T>();
			Inner::Allocator::Deallocate(mEntry);
			mEntry = nullptr;
			return;
		}

		mEntry->Free();
		mEntry = nullptr;
	}

	/// Shallow-copy assignment																
	///	@param other - the container to shallow-copy									
	///	@return a reference to this container											
	TEMPLATE()
	TAny<T>& TAny<T>::operator = (const TAny& other) {
		if (this == &other)
			return *this;

		Free();
		other.Keep();
		CopyProperties<true, true>(other);
		return *this;
	}

	/// Move assignment																			
	///	@param other - the container to move											
	///	@return a reference to this container											
	TEMPLATE()
	TAny<T>& TAny<T>::operator = (TAny&& other) noexcept {
		if (this == &other)
			return *this;

		Free();
		CopyProperties<true, true>(other);
		other.ResetMemory();
		other.ResetState();
		return *this;
	}

	/// Shallow-copy a disowned container, without referencing the data			
	///	@param other - the container to shallow-copy									
	///	@return a reference to this container											
	TEMPLATE()
	TAny<T>& TAny<T>::operator = (Disowned<TAny>&& other) noexcept {
		if (this == &other.mValue)
			return *this;

		Free();
		CopyProperties<true, false>(other.mValue);
		return *this;
	}

	/// Move assignment of an abandoned container, without fully resetting it	
	///	@param other - the container to move											
	///	@return a reference to this container											
	TEMPLATE()
	TAny<T>& TAny<T>::operator = (Abandoned<TAny>&& other) noexcept {
		if (this == &other.mValue)
			return *this;

		Free();
		CopyProperties<true, true>(other.mValue);
		other.mValue.mEntry = nullptr;
		return *this;
	}

	/// Copy-construct from another container by performing runtime type check	
	///	@tparam KEEP - whether or not to reference contents						
	///	@tparam ALT_T - the container type (deducible)								
	///	@param other - the container to incorporate									
	TEMPLATE()
	template<bool KEEP, CT::Deep ALT_T>
	void TAny<T>::AssignFromContainer(const ALT_T& other) {
		if (CastsToMeta(other.GetType())) {
			// Always attempt to copy containers directly first, instead	
			// of doing allocations														
			Free();
			if constexpr (KEEP)
				other.Keep();
			ResetState();
			CopyProperties<false, true>(other);
			return;
		}

		// Then attempt to push other container, if this container allows	
		// for it																			
		if constexpr (CT::Deep<T>) {
			CallKnownDestructors<T>();
			mCount = 0;
			ResetState();
			auto& compatible = static_cast<const Decay<T>&>(other);
			Insert<IndexBack, KEEP>(&compatible, &compatible + 1);
		}
		else Throw<Except::Copy>("Bad copy-assignment", LANGULUS_LOCATION());
	}

	/// Move-construct from another container by performing runtime type check	
	///	@tparam KEEP - whether or not to reference contents						
	///	@tparam ALT_T - the container type (deducible)								
	///	@param other - the container to incorporate									
	TEMPLATE()
	template<bool KEEP, CT::Deep ALT_T>
	void TAny<T>::AssignFromContainer(ALT_T&& other) {
		if (CastsToMeta(other.GetType())) {
			// Always attempt to copy containers directly first, instead	
			// of doing allocations														
			Free();
			ResetState();
			CopyProperties<false, true>(other);

			if constexpr (KEEP)
				other.mEntry = nullptr;
			else {
				other.ResetMemory();
				other.ResetState();
			}
			return;
		}

		// Then attempt to push other container, if this container allows	
		// for it																			
		if constexpr (CT::Deep<T>) {
			CallKnownDestructors<T>();
			mCount = 0;
			ResetState();
			Insert<IndexBack, KEEP>(Forward<Decay<T>>(other));
		}
		else Throw<Except::Copy>("Bad move-assignment", LANGULUS_LOCATION());
	}

	/// Copy-assign an unknown container													
	/// This is a bit slower, because it checks type compatibility at runtime	
	///	@param other - the container to shallow-copy									
	///	@return a reference to this container											
	TEMPLATE()
	TAny<T>& TAny<T>::operator = (const Any& other) {
		if (static_cast<Any*>(this) == &other)
			return *this;

		AssignFromContainer<true>(other);
		return *this;
	}

	/// Move-assign an unknown container													
	/// This is a bit slower, because it checks type compatibility at runtime	
	///	@param other - the container to move											
	///	@return a reference to this container											
	TEMPLATE()
	TAny<T>& TAny<T>::operator = (Any&& other) {
		if (static_cast<Any*>(this) == &other)
			return *this;

		AssignFromContainer<true>(Forward<Any>(other));
		return *this;
	}

	/// Shallow-copy disowned runtime container without referencing contents	
	/// This is a bit slower, because checks type compatibility at runtime		
	///	@param other - the container to shallow-copy									
	///	@return a reference to this container											
	TEMPLATE()
	TAny<T>& TAny<T>::operator = (Disowned<Any>&& other) {
		if (static_cast<Any*>(this) == &other.mValue)
			return *this;

		AssignFromContainer<false>(other.mValue);
		return *this;
	}

	/// Move abandoned runtime container without fully resetting it				
	/// This is a bit slower, because checks type compatibility at runtime		
	///	@param other - the container to move											
	///	@return a reference to this container											
	TEMPLATE()
	TAny<T>& TAny<T>::operator = (Abandoned<Any>&& other) {
		if (static_cast<Any*>(this) == &other.mValue)
			return *this;

		AssignFromContainer<false>(Forward<Any>(other.mValue));
		return *this;
	}

	/// Shallow-copy Block																		
	/// This is a bit slower, because checks type compatibility at runtime		
	///	@param other - the container to shallow-copy									
	///	@return a reference to this container											
	TEMPLATE()
	TAny<T>& TAny<T>::operator = (const Block& other) {
		if (static_cast<Block*>(this) == &other)
			return *this;

		AssignFromContainer<true>(other);
		return *this;
	}

	/// Seems like a move, but is actually a shallow-copy of a Block				
	/// This is a bit slower, because checks type compatibility at runtime		
	///	@attention other is never reset													
	///	@param other - the container to shallow-copy									
	///	@return a reference to this container											
	TEMPLATE()
	TAny<T>& TAny<T>::operator = (Block&& other) {
		if (static_cast<Block*>(this) == &other)
			return *this;

		AssignFromContainer<true>(Forward<Block>(other));
		return *this;
	}

	/// Assign by shallow-copying an element												
	///	@param other - the value to copy													
	///	@return a reference to this container											
	TEMPLATE()
	TAny<T>& TAny<T>::operator = (const T& other) requires CT::CustomData<T> {
		if (GetUses() == 1) {
			// Just destroy and reuse memory											
			CallKnownDestructors<T>();
			mCount = 0;
			InsertInner<true>(&other, &other + 1, 0);
		}
		else {
			// Reset and allocate new memory											
			Reset();
			operator << (other);
		}

		return *this;
	}

	/// Assign by moving an element															
	///	@param other - the value to move													
	///	@return a reference to this container											
	TEMPLATE()
	TAny<T>& TAny<T>::operator = (T&& other) requires CT::CustomData<T> {
		if (GetUses() == 1) {
			// Just destroy and reuse memory											
			CallKnownDestructors<T>();
			mCount = 0;
			InsertInner<true>(Forward<T>(other), 0);
		}
		else {
			// Reset and allocate new memory											
			Reset();
			operator << (Forward<T>(other));
		}

		return *this;
	}

	/// Assign by interfacing a disowned element											
	///	@param other - the element to interface										
	///	@return a reference to this container											
	TEMPLATE()
	TAny<T>& TAny<T>::operator = (Disowned<T>&& other) noexcept requires CT::CustomData<T> {
		if (GetUses() != 1) {
			// Reset and allocate new memory											
			// Disowned-construction will be used if possible					
			Reset();
			operator << (other.Forward());
		}
		else {
			// Just destroy and reuse memory											
			if constexpr (CT::Sparse<T>) {
				CallKnownDestructors<T>();
				mCount = 1;
				GetRawSparse()->mPointer = other.mValue;
				GetRawSparse()->mEntry = nullptr;
			}
			else {
				CallKnownDestructors<T>();
				mCount = 1;
				if constexpr (CT::DisownMakable<T>)
					new (mRaw) T {other.Forward()};
				else
					new (mRaw) T {other.mValue};
			}
		}

		return *this;
	}

	/// Assign by interfacing an abandoned element										
	///	@param other - the element to interface										
	///	@return a reference to this container											
	TEMPLATE()
	TAny<T>& TAny<T>::operator = (Abandoned<T>&& other) noexcept requires CT::CustomData<T> {
		if (GetUses() != 1) {
			// Reset and allocate new memory											
			// Abandoned-construction will be used if possible					
			Reset();
			operator << (other.Forward());
		}
		else {
			// Just destroy and reuse memory											
			if constexpr (CT::Sparse<T>) {
				CallKnownDestructors<T>();
				mCount = 1;
				GetRawSparse()->mPointer = other.mValue;
				GetRawSparse()->mEntry = nullptr;
			}
			else {
				CallKnownDestructors<T>();
				mCount = 1;
				if constexpr (CT::AbandonMakable<T>)
					new (mRaw) T {other.Forward()};
				else
					new (mRaw) T {Forward<T>(other.mValue)};
			}
		}

		return *this;
	}

	/// An internal function used to copy members, without copying type and		
	/// without overwriting states, if required											
	///	@param other - the block to copy from											
	TEMPLATE()
	template<bool OVERWRITE_STATE, bool OVERWRITE_ENTRY>
	void TAny<T>::CopyProperties(const Block& other) noexcept {
		mRaw = other.mRaw;
		mCount = other.mCount;
		mReserved = other.mReserved;
		if constexpr (OVERWRITE_STATE)
			mState = other.mState;
		else
			mState += other.mState;

		if constexpr (OVERWRITE_ENTRY)
			mEntry = other.mEntry;
	}

	/// Reset container state (inner function)											
	TEMPLATE()
	constexpr void TAny<T>::ResetState() noexcept {
		mState = mState.mState & (DataState::Typed | DataState::Sparse);
	}

	/// Reset container type (does nothing for typed container)						
	TEMPLATE()
	constexpr void TAny<T>::ResetType() noexcept {}

	/// Check if contained data can be interpreted as a given type					
	/// Beware, direction matters (this is the inverse of CanFit)					
	///	@param type - the type check if current type interprets to				
	///	@return true if able to interpret current type to 'type'					
	TEMPLATE()
	bool TAny<T>::CastsToMeta(DMeta type) const {
		return mType->CastsTo<CT::Sparse<T>>(type);
	}

	/// Check if contained data can be interpreted as a given count of type		
	/// For example: a vec4 can interpret as float[4]									
	/// Beware, direction matters (this is the inverse of CanFit)					
	///	@param type - the type check if current type interprets to				
	///	@param count - the number of elements to interpret as						
	///	@return true if able to interpret current type to 'type'					
	TEMPLATE()
	bool TAny<T>::CastsToMeta(DMeta type, Count count) const {
		return mType->CastsTo(type, count);
	}

	/// Wrap stuff in a container																
	///	@param anything - pack it inside a dense container							
	///	@returns the pack containing the data											
	TEMPLATE()
	template<CT::Data... LIST_T>
	TAny<T> TAny<T>::Wrap(LIST_T&&... list) {
		TAny<T> temp;
		temp.Allocate(sizeof...(list));
		(temp << ... << Forward<LIST_T>(list));
		return Abandon(temp);
	}

	/// Allocate 'count' elements and fill the container with zeroes				
	TEMPLATE()
	void TAny<T>::Null(const Count& count) {
		Allocate(count, false, true);
		FillMemory(mRaw, {}, GetByteSize());
	}

	/// Clear the container, destroying all elements,									
	/// but retaining allocation if possible												
	TEMPLATE()
	void TAny<T>::Clear() {
		if (!mCount)
			return;

		if (GetUses() == 1) {
			// Only one use - just destroy elements and reset count,			
			// reusing the allocation for later										
			CallKnownDestructors<T>();
			ClearInner();
		}
		else {
			// We're forced to reset the memory, because it's in use			
			// Keep the type and state, though										
			const auto state = GetUnconstrainedState();
			Reset();
			mState += state;
		}
	}

	/// Reset the container, destroying all elements, and deallocating			
	TEMPLATE()
	void TAny<T>::Reset() {
		Free();
		ResetMemory();
		ResetState();
	}

	/// Clone the templated container														
	///	@returns either a deep clone of the container data, or a shallow		
	///				copy, if contained type is not clonable							
	TEMPLATE()
	TAny<T> TAny<T>::Clone() const {
		if constexpr (CT::Clonable<T> || CT::POD<T>) {
			// Always clone the state, but make it unconstrained				
			TAny<T> result {Disown(*this)};
			result.mState -= DataState::Static | DataState::Constant;

			if (!IsAllocated())
				return Abandon(result);

			result.ResetMemory();
			result.Allocate<false>(mCount);
			result.mCount = mCount;
			auto from = GetRaw();
			auto to = result.GetRaw();
			using Type = Decay<T>;

			if constexpr (CT::Sparse<T>) {
				// Clone all data in the same block									
				TAny<Decay<T>> coalesced;
				coalesced.Allocate(mCount);

				// Clone data behind each valid pointer							
				Count counter {};
				while (from < GetRawEnd()) {
					if (!*from) {
						*to = nullptr;
						++from; ++to;
						continue;
					}

					if constexpr (CT::Clonable<T>)
						new (&coalesced[counter]) Type {(*from)->Clone()};
					else if constexpr (CT::POD<T>)
						CopyMemory(**from, &coalesced[counter], sizeof(Type));
					else
						LANGULUS_ERROR("Can't clone a container made of non-clonable/non-POD type");

					*to = &coalesced[counter];
					++from; ++to;
					++counter;
				}

				coalesced.Reference(counter);
			}
			else if constexpr (CT::Clonable<T>) {
				// Clone dense elements by calling their Clone()				
				while (from < GetRawEnd()) {
					new (to) Type {from->Clone()};
					++from; ++to;
				}
			}
			else if constexpr (CT::POD<T>) {
				// Batch copy everything at once										
				CopyMemory(from, to, sizeof(Type) * mCount);
			}
			else LANGULUS_ERROR("Can't clone a container made of non-clonable/non-POD type");

			return Abandon(result);
		}
		else {
			// Can't clone the data, just return a shallow-copy				
			return *this;
		}
	}

	/// Return the typed raw data (const)													
	///	@return a constant pointer to the first element in the array			
	TEMPLATE()
	auto TAny<T>::GetRaw() const noexcept {
		if constexpr (CT::Dense<T>)
			return Any::GetRawAs<T>();
		else
			return Any::GetRawAs<KnownPointer>();
	}

	/// Return the typed raw data																
	///	@return a mutable pointer to the first element in the array				
	TEMPLATE()
	auto TAny<T>::GetRaw() noexcept {
		if constexpr (CT::Dense<T>)
			return Any::GetRawAs<T>();
		else
			return Any::GetRawAs<KnownPointer>();
	}

	/// Return the typed raw data end pointer (const)									
	///	@return a constant pointer to one past the last element in the array	
	TEMPLATE()
	auto TAny<T>::GetRawEnd() const noexcept {
		return GetRaw() + mCount;
	}

	/// Return the typed raw data	end pointer												
	///	@return a mutable pointer to one past the last element in the array	
	TEMPLATE()
	auto TAny<T>::GetRawEnd() noexcept {
		return GetRaw() + mCount;
	}

	/// Return the typed raw sparse data (const)											
	///	@return a constant pointer to the first element in the array			
	TEMPLATE()
	auto TAny<T>::GetRawSparse() const noexcept {
		return reinterpret_cast<const KnownPointer*>(mRawSparse);
	}

	/// Return the typed raw sparse data													
	///	@return a mutable pointer to the first element in the array				
	TEMPLATE()
	auto TAny<T>::GetRawSparse() noexcept {
		return reinterpret_cast<KnownPointer*>(mRawSparse);
	}

	/// Get an element in the way you want (const, unsafe)							
	/// This is a statically optimized variant of Block::Get							
	TEMPLATE()
	template<CT::Data ALT_T>
	decltype(auto) TAny<T>::Get(const Offset& index) const noexcept {
		const auto& element = GetRaw()[index];
		if constexpr (CT::Dense<T> && CT::Dense<ALT_T>)
			// Dense -> Dense (returning a reference)								
			return static_cast<const Decay<ALT_T>&>(element);
		else if constexpr (CT::Dense<T> && CT::Sparse<ALT_T>)
			// Dense -> Sparse (returning a pointer)								
			return static_cast<const Decay<ALT_T>*>(&element);
		else if constexpr (CT::Sparse<T> && CT::Dense<ALT_T>)
			// Sparse -> Dense (returning a reference)							
			return static_cast<const Decay<ALT_T>&>(*element.mPointer);
		else
			// Sparse -> Sparse (returning a reference to pointer)			
			return static_cast<const Decay<ALT_T>* const&>(element.mPointer);
	}

	/// Get an element in the way you want (unsafe)										
	/// This is a statically optimized variant of Block::Get							
	TEMPLATE()
	template<CT::Data ALT_T>
	decltype(auto) TAny<T>::Get(const Offset& index) noexcept {
		auto& element = GetRaw()[index];
		if constexpr (CT::Dense<T> && CT::Dense<ALT_T>)
			// Dense -> Dense (returning a reference)								
			return static_cast<Decay<ALT_T>&>(element);
		else if constexpr (CT::Dense<T> && CT::Sparse<ALT_T>)
			// Dense -> Sparse (returning a pointer)								
			return static_cast<Decay<ALT_T>*>(&element);
		else if constexpr (CT::Sparse<T> && CT::Dense<ALT_T>)
			// Sparse -> Dense (returning a reference)							
			return static_cast<Decay<ALT_T>&>(*element.mPointer);
		else
			// Sparse -> Sparse (returning a reference to pointer)			
			return static_cast<Decay<ALT_T>*&>(element.mPointer);
	}

	/// Access typed dense elements by index												
	///	@param idx - the index to get														
	///	@return a reference to the element												
	TEMPLATE()
	template<CT::Index IDX>
	decltype(auto) TAny<T>::operator [] (const IDX& index) const {
		const auto offset = SimplifyIndex<T>(index);
		return TAny<T>::GetRaw()[offset];
	}

	TEMPLATE()
	template<CT::Index IDX>
	decltype(auto) TAny<T>::operator [] (const IDX& index) {
		const auto offset = SimplifyIndex<T>(index);
		return TAny<T>::GetRaw()[offset];
	}

	/// Access last element																		
	///	@attention assumes container has at least one item							
	///	@return a mutable reference to the last element								
	TEMPLATE()
	decltype(auto) TAny<T>::Last() {
		LANGULUS_ASSUME(UserAssumes, mCount, "Can't get last index");
		return Get<T>(mCount - 1);
	}

	/// Access last element																		
	///	@attention assumes container has at least one item							
	///	@return a constant reference to the last element							
	TEMPLATE()
	decltype(auto) TAny<T>::Last() const {
		LANGULUS_ASSUME(UserAssumes, mCount, "Can't get last index");
		return Get<T>(mCount - 1);
	}
	
	/// Templated Any containers are always typed										
	TEMPLATE()
	constexpr bool TAny<T>::IsUntyped() const noexcept {
		return false;
	}
	
	/// Templated Any containers are always type-constrained							
	TEMPLATE()
	constexpr bool TAny<T>::IsTypeConstrained() const noexcept {
		return true;
	}
	
	/// Check if contained type is abstract												
	/// This is a statically optimized alternative to Block::IsAbstract			
	TEMPLATE()
	constexpr bool TAny<T>::IsAbstract() const noexcept {
		return CT::Abstract<T>;
	}
	
	/// Check if contained type is default-constructible								
	/// This is a statically optimized alternative to Block::IsDefaultable		
	TEMPLATE()
	constexpr bool TAny<T>::IsDefaultable() const noexcept {
		return CT::Defaultable<T>;
	}
	
	/// Check if contained type is deep														
	/// This is a statically optimized alternative to Block::IsDeep				
	///	@return true if this container contains deep items							
	TEMPLATE()
	constexpr bool TAny<T>::IsDeep() const noexcept {
		// Sparse types are never considered deep, but when contained,		
		// it's safe to erase that aspect											
		return CT::Deep<Decay<T>>;
	}

	/// Check if the contained type is a pointer											
	/// This is a statically optimized alternative to Block::IsSparse				
	///	@return true if container contains pointers									
	TEMPLATE()
	constexpr bool TAny<T>::IsSparse() const noexcept {
		return CT::Sparse<T>;
	}

	/// Check if the contained type is not a pointer									
	/// This is a statically optimized alternative to Block::IsDense				
	///	@return true if container contains sequential data							
	TEMPLATE()
	constexpr bool TAny<T>::IsDense() const noexcept {
		return CT::Dense<T>;
	}

	/// Check if block contains POD items - if so, it's safe to directly copy	
	/// raw memory from container. Note, that this doesn't only consider the	
	/// standard c++ type traits, like trivially_constructible. You also need	
	/// to explicitly reflect your type with LANGULUS(POD) true;					
	/// This gives a lot more control over your code									
	/// This is a statically optimized alternative to Block::IsPOD					
	///	@return true if contained data is plain old data							
	TEMPLATE()
	constexpr bool TAny<T>::IsPOD() const noexcept {
		return CT::POD<T>;
	}

	/// Check if block contains resolvable items, that is, items that have a	
	/// GetBlock() function, that can be used to represent themselves as their	
	/// most concretely typed block															
	/// This is a statically optimized alternative to Block::IsResolvable		
	///	@return true if contained data can be resolved on element basis		
	TEMPLATE()
	constexpr bool TAny<T>::IsResolvable() const noexcept {
		return CT::Resolvable<T>;
	}

	/// Check if block data can be safely set to zero bytes							
	/// This is tied to LANGULUS(NULLIFIABLE) reflection parameter					
	/// This is a statically optimized alternative to Block::IsNullifiable		
	///	@return true if contained data can be memset(0) safely					
	TEMPLATE()
	constexpr bool TAny<T>::IsNullifiable() const noexcept {
		return CT::Nullifiable<T>;
	}

	/// Get the size of a single contained element, in bytes							
	/// This is a statically optimized alternative to Block::GetStride			
	///	@return the number of bytes a single element contains						
	TEMPLATE()
	constexpr Size TAny<T>::GetStride() const noexcept {
		if constexpr (CT::Dense<T>)
			return sizeof(T);
		else
			return sizeof(KnownPointer);
	}
	
	/// Get the size of all elements, in bytes											
	///	@return the total amount of initialized bytes								
	TEMPLATE()
	constexpr Size TAny<T>::GetByteSize() const noexcept {
		return GetStride() * mCount;
	}

	/// Copy-insert item(s) at an index														
	///	@attention assumes index is in the container's limits, if simple		
	///	@tparam KEEP - whether or not to reference inserted data					
	///	@tparam IDX - type of indexing to use (deducible)							
	///	@param start - pointer to the first element to insert						
	///	@param end - pointer to the end of elements to insert						
	///	@param index - the index to insert at											
	///	@return number of inserted items													
	TEMPLATE()
	template<bool KEEP, CT::Index IDX>
	Count TAny<T>::InsertAt(const T* start, const T* end, const IDX& index) {
		const auto offset = SimplifyIndex<T>(index);
		const auto count = end - start;
		Allocate<false>(mCount + count);

		if (offset < mCount) {
			// Move memory if required													
			LANGULUS_ASSERT(GetUses() == 1, Except::Move,
				"Inserting elements to memory block, used from multiple places, "
				"requires memory to move");

			CropInner(offset + count, 0, mCount - offset)
				.template CallKnownMoveConstructors<false, T>(
					mCount - offset,
					CropInner(offset, mCount - offset, mCount - offset)
				);
		}

		InsertInner<KEEP, T>(start, end, offset);
		return count;
	}

	/// Move-insert an item at an index														
	///	@attention assumes index is in the container's limits, if simple		
	///	@tparam KEEP - whether or not to reference inserted data					
	///	@tparam IDX - type of indexing to use (deducible)							
	///	@param item - the item to move in												
	///	@param index - the index to insert at											
	///	@return number of inserted items													
	TEMPLATE()
	template<bool KEEP, CT::Index IDX>
	Count TAny<T>::InsertAt(T&& item, const IDX& index) {
		const auto offset = SimplifyIndex<T>(index);
		Allocate<false>(mCount + 1);

		if (offset < mCount) {
			// Move memory if required													
			LANGULUS_ASSERT(GetUses() == 1, Except::Move,
				"Inserting elements to memory block, used from multiple places, "
				"requires memory to move");

			CropInner(offset + 1, 0, mCount - offset)
				.template CallKnownMoveConstructors<false, T>(
					mCount - offset,
					CropInner(offset, mCount - offset, mCount - offset)
				);
		}

		InsertInner<KEEP, T>(Move(item), offset);
		return 1;
	}

	/// Copy-insert elements either at the start or the end							
	///	@tparam INDEX - use IndexBack or IndexFront to append accordingly		
	///	@tparam KEEP - whether to reference data on copy							
	///	@param start - pointer to the first item										
	///	@param end - pointer to the end of items										
	///	@return number of inserted elements												
	TEMPLATE()
	template<Index INDEX, bool KEEP>
	Count TAny<T>::Insert(const T* start, const T* end) {
		static_assert(CT::Sparse<T> || CT::Mutable<T>,
			"Can't copy-insert into container of constant elements");
		static_assert(INDEX == IndexFront || INDEX == IndexBack,
			"Invalid index provided; use either IndexBack "
			"or IndexFront, or Block::InsertAt to insert at an offset");

		// Allocate																			
		const auto count = end - start;
		Allocate<false>(mCount + count);

		// Move memory if required														
		if constexpr (INDEX == IndexFront) {
			LANGULUS_ASSERT(GetUses() == 1, Except::Move,
				"Inserting elements to memory block, used from multiple places, "
				"requires memory to move");

			CropInner(count, 0, mCount)
				.template CallKnownMoveConstructors<false, T>(
					mCount, CropInner(0, mCount, mCount)
				);

			InsertInner<KEEP>(start, end, 0);
		}
		else InsertInner<KEEP>(start, end, mCount);

		return count;
	}

	/// Move-insert an element at the start or the end									
	///	@tparam INDEX - use IndexBack or IndexFront to append accordingly		
	///	@tparam KEEP - whether to completely reset source after move			
	///	@param item - item to move int													
	///	@return 1 if element was pushed													
	TEMPLATE()
	template<Index INDEX, bool KEEP>
	Count TAny<T>::Insert(T&& item) {
		static_assert(CT::Sparse<T> || CT::Mutable<T>,
			"Can't copy-insert into container of constant elements");
		static_assert(INDEX == IndexFront || INDEX == IndexBack,
			"Invalid index provided; use either IndexBack "
			"or IndexFront, or Block::InsertAt to insert at an offset");

		// Allocate																			
		Allocate<false>(mCount + 1);

		// Move memory if required														
		if constexpr (INDEX == IndexFront) {
			LANGULUS_ASSERT(GetUses() == 1, Except::Move,
				"Inserting elements to memory block, used from multiple places, "
				"requires memory to move");

			CropInner(1, 0, mCount)
				.template CallKnownMoveConstructors<false, T>(
					mCount, CropInner(0, mCount, mCount)
				);

			InsertInner<KEEP>(Move(item), 0);
		}
		else InsertInner<KEEP>(Move(item), mCount);

		return 1;
	}

	/// Push data at the back by copy-construction										
	///	@param other - the item to insert												
	///	@return a reference to this container for chaining							
	TEMPLATE()
	TAny<T>& TAny<T>::operator << (const T& other) {
		Insert<IndexBack>(&other, &other + 1);
		return *this;
	}

	/// Push data at the back by move-construction										
	///	@param other - the item to move													
	///	@return a reference to this container for chaining							
	TEMPLATE()
	TAny<T>& TAny<T>::operator << (T&& other) {
		Insert<IndexBack>(Forward<T>(other));
		return *this;
	}

	/// Push data at the back by copy-construction, but don't reference the		
	/// new element, because it's disowned													
	///	@param other - the item to insert												
	///	@return a reference to this container for chaining							
	TEMPLATE()
	TAny<T>& TAny<T>::operator << (Disowned<T>&& other) {
		Insert<IndexBack, false>(&other.mValue, &other.mValue + 1);
		return *this;
	}

	/// Push data at the back by move-construction, but don't fully reset the	
	/// source, because it's abandoned														
	///	@param other - the item to move													
	///	@return a reference to this container for chaining							
	TEMPLATE()
	TAny<T>& TAny<T>::operator << (Abandoned<T>&& other) {
		Insert<IndexBack, false>(Move(other.mValue));
		return *this;
	}

	/// Push data at the front by copy-construction										
	///	@param other - the item to insert												
	///	@return a reference to this container for chaining							
	TEMPLATE()
	TAny<T>& TAny<T>::operator >> (const T& other) {
		Insert<IndexFront>(&other, &other + 1);
		return *this;
	}

	/// Push data at the front by move-construction										
	///	@param other - the item to move													
	///	@return a reference to this container for chaining							
	TEMPLATE()
	TAny<T>& TAny<T>::operator >> (T&& other) {
		Insert<IndexFront>(Forward<T>(other));
		return *this;
	}

	/// Push data at the front by copy-construction, but don't reference the	
	/// new element, because it's disowned													
	///	@param other - the item to insert												
	///	@return a reference to this container for chaining							
	TEMPLATE()
	TAny<T>& TAny<T>::operator >> (Disowned<T>&& other) {
		Insert<IndexFront, false>(&other.mValue, &other.mValue + 1);
		return *this;
	}

	/// Push data at the front by move-construction, but don't fully reset the	
	/// source, because it's abandoned														
	///	@param other - the item to move													
	///	@return a reference to this container for chaining							
	TEMPLATE()
	TAny<T>& TAny<T>::operator >> (Abandoned<T>&& other) {
		Insert<IndexFront, false>(Move(other.mValue));
		return *this;
	}

	/// Copy-insert elements that are not found, at an index							
	///	@attention assumes index is in container's limits, if simple			
	///	@tparam KEEP - whether or not to reference inserted data					
	///	@tparam IDX - type for indexing (deducible)									
	///	@param start - pointer to the first element to insert						
	///	@param end - pointer to the end of elements to insert						
	///	@param index - the index to insert at											
	///	@return the number of inserted items											
	TEMPLATE()
	template<bool KEEP, CT::Index IDX>
	Count TAny<T>::MergeAt(const T* start, const T* end, const IDX& index) {
		return Block::MergeAt<TAny, KEEP, true, T>(start, end, index);
	}

	/// Move-insert element, if not found, at an index									
	///	@attention assumes index is in container's limits, if simple			
	///	@tparam KEEP - whether or not to reference inserted data					
	///	@tparam IDX - type for indexing (deducible)									
	///	@param item - the item to find and push										
	///	@param index - the index to insert at											
	///	@return the number of inserted items											
	TEMPLATE()
	template<bool KEEP, CT::Index IDX>
	Count TAny<T>::MergeAt(T&& item, const IDX& index) {
		return Block::MergeAt<TAny, KEEP, true, T>(Forward<T>(item), index);
	}

	/// Copy-construct element at the back, if element is not found				
	///	@param other - the element to shallow-copy									
	///	@return a reference to this container											
	TEMPLATE()
	TAny<T>& TAny<T>::operator <<= (const T& other) {
		Merge<IndexBack>(&other, &other + 1);
		return *this;
	}

	/// Move-construct element at the back, if element is not found				
	///	@param other - the element to move												
	///	@return a reference to this container											
	TEMPLATE()
	TAny<T>& TAny<T>::operator <<= (T&& other) {
		Merge<IndexBack>(Forward<T>(other));
		return *this;
	}

	/// Copy-construct element at the back, if element is not found				
	/// The element's contents won't be referenced, because it is disowned		
	///	@param other - the element to shallow-copy									
	///	@return a reference to this container											
	TEMPLATE()
	TAny<T>& TAny<T>::operator <<= (Disowned<T>&& other) {
		Merge<IndexBack, false>(&other.mValue, &other.mValue + 1);
		return *this;
	}

	/// Move-construct element at the back, if element is not found				
	/// The element won't be fully reset, because it's abandoned					
	///	@param other - the element to move												
	///	@return a reference to this container											
	TEMPLATE()
	TAny<T>& TAny<T>::operator <<= (Abandoned<T>&& other) {
		Merge<IndexBack, false>(Move(other.mValue));
		return *this;
	}

	/// Copy-construct element at the front, if element is not found				
	///	@param other - the element to shallow-copy									
	///	@return a reference to this container											
	TEMPLATE()
	TAny<T>& TAny<T>::operator >>= (const T& other) {
		Merge<IndexFront>(&other, &other + 1);
		return *this;
	}

	/// Move-construct element at the front, if element is not found				
	///	@param other - the element to move												
	///	@return a reference to this container											
	TEMPLATE()
	TAny<T>& TAny<T>::operator >>= (T&& other) {
		Merge<IndexFront>(Forward<T>(other));
		return *this;
	}

	/// Copy-construct element at the front, if element is not found				
	/// The element's contents won't be referenced, because it is disowned		
	///	@param other - the element to shallow-copy									
	///	@return a reference to this container											
	TEMPLATE()
	TAny<T>& TAny<T>::operator >>= (Disowned<T>&& other) {
		Merge<IndexFront, false>(&other.mValue, &other.mValue + 1);
		return *this;
	}

	/// Move-construct element at the front, if element is not found				
	/// The element won't be fully reset, because it's abandoned					
	///	@param other - the element to move												
	///	@return a reference to this container											
	TEMPLATE()
	TAny<T>& TAny<T>::operator >>= (Abandoned<T>&& other) {
		Merge<IndexFront, false>(Move(other.mValue));
		return *this;
	}

	/// Find element(s) index inside container											
	///	@tparam REVERSE - whether to search in reverse order						
	///	@param item - the item to search for											
	///	@return the index of the found item, or IndexNone if none found		
	TEMPLATE()
	template<bool REVERSE, bool BY_ADDRESS_ONLY>
	Index TAny<T>::Find(const T& item) const {
		// Searching for value inside a sparse container						
		if constexpr (REVERSE) {
			// Searching in reverse														
			auto start = GetRawEnd() - 1;
			const auto end = start - mCount;
			do {
				if constexpr (BY_ADDRESS_ONLY) {
					if constexpr (CT::Sparse<T>) {
						if (*start == item)
							return start - GetRaw();
					}
					else if (start == &item)
						return start - GetRaw();
				}
				else {
					if constexpr (CT::Sparse<T>) {
						if (*start == item)
							return start - GetRaw();
					}
					else if (start == &item || *start == item)
						return start - GetRaw();
				}
			} while (--start != end);
		}
		else {
			// Searching forward															
			auto start = GetRaw();
			const auto end = start + mCount;
			do {
				if constexpr (BY_ADDRESS_ONLY) {
					if constexpr (CT::Sparse<T>) {
						if (*start == item)
							return start - GetRaw();
					}
					else if (start == &item)
						return start - GetRaw();
				}
				else {
					if constexpr (CT::Sparse<T>) {
						if (*start == item)
							return start - GetRaw();
					}
					else if (start == &item || *start == item)
						return start - GetRaw();
				}
			} while (++start != end);
		}

		// If this is reached, then no match was found							
		return IndexNone;
	}

	/// Remove matching items by value														
	///	@tparam REVERSE - whether to search in reverse order						
	///	@param item - the item to search for to remove								
	///	@return the number of removed items												
	TEMPLATE()
	template<bool REVERSE>
	Count TAny<T>::RemoveValue(const T& item) {
		const auto found = Find<REVERSE>(item);
		if (found)
			return RemoveIndex(found.GetOffset(), 1);
		return 0;
	}

	/// Remove matching items by address													
	///	@tparam REVERSE - whether to search in reverse order						
	///	@param item - the item to search for to remove								
	///	@return the number of removed items												
	TEMPLATE()
	Count TAny<T>::RemovePointer(const T* item) {
		if (!Owns(item))
			return 0;

		return RemoveIndex(item - GetRaw(), 1);
	}

	/// Remove sequential raw indices in a given range									
	///	@attention assumes starter + count <= mCount									
	///	@param starter - simple index to start removing from						
	///	@param count - number of elements to remove									
	///	@return the number of removed elements											
	TEMPLATE()
	Count TAny<T>::RemoveIndex(const Offset& starter, const Count& count) {
		LANGULUS_ASSUME(UserAssumes, starter + count <= mCount,
			"Index out of range");

		const auto ender = starter + count;
		if constexpr (CT::POD<T>) {
			if (ender == mCount) {
				// If data is POD and elements are on the back, we can		
				// get around constantness and staticness, by simply			
				// truncating the count without any reprecussions				
				// We can completely skip destroying POD things					
				mCount = starter;
				return count;
			}

			LANGULUS_ASSERT(GetUses() == 1, Except::Move,
				"Removing elements from memory block, used from multiple places, "
				"requires memory to move");
			LANGULUS_ASSERT(IsMutable(), Except::Access,
				"Attempting to remove from constant container");
			LANGULUS_ASSERT(!IsStatic(), Except::Access,
				"Attempting to remove from static container");

			MoveMemory(GetRaw() + ender, GetRaw() + starter, sizeof(T) * (mCount - ender));
			mCount -= count;
			return count;
		}
		else {
			if (IsStatic() && ender == mCount) {
				// If data is static and elements are on the back, we can	
				// get around constantness and staticness, by simply			
				// truncating the count without any reprecussions				
				// We can't destroy static element anyways						
				mCount = starter;
				return count;
			}

			LANGULUS_ASSERT(GetUses() == 1, Except::Move,
				"Removing elements from memory block, used from multiple places, "
				"requires memory to move");
			LANGULUS_ASSERT(IsMutable(), Except::Access,
				"Attempting to remove from constant container");
			LANGULUS_ASSERT(!IsStatic(), Except::Access,
				"Attempting to remove from static container");

			// Call the destructors on the correct region						
			CropInner(starter, count, count)
				.template CallKnownDestructors<T>();

			if (ender < mCount) {
				// Fill gap	if any by invoking move constructions				
				const auto remains = mCount - ender;
				CropInner(starter, 0, remains)
					.template CallKnownMoveConstructors<false, T>(
						remains, CropInner(ender, remains, remains)
					);
			}

			mCount -= count;
			return count;
		}
	}

	/// Sort the pack																				
	TEMPLATE()
	template<bool ASCEND>
	void TAny<T>::Sort() {
		if constexpr (CT::Sortable<T>)
			Any::Sort<T, ASCEND>();
		else LANGULUS_ERROR("Can't sort container - T is not sortable");
	}

	/// Remove elements on the back															
	TEMPLATE()
	TAny<T>& TAny<T>::Trim(const Count& count) {
		Any::Trim(count);
		return *this;
	}

	/// Swap two elements																		
	///	@param from - the first element													
	///	@param to - the second element													
	TEMPLATE()
	void TAny<T>::Swap(const Offset& from, const Offset& to) {
		Any::Swap<T>(from, to);
	}

	/// Swap two elements using special indices											
	///	@param from - the first element													
	///	@param to - the second element													
	TEMPLATE()
	void TAny<T>::Swap(const Index& from, const Index& to) {
		Any::Swap<T>(from, to);
	}

	/// Clone container array into a new owned memory block							
	/// If we have jurisdiction, the memory won't move at all						
	TEMPLATE()
	void TAny<T>::TakeAuthority() {
		if (mEntry)
			return;

		operator = (Clone());
	}

	/// Get a constant part of this container												
	///	@tparam WRAPPER - the container to use for the part						
	///			            use Block for unreferenced container					
	///	@return a container that represents the cropped part						
	TEMPLATE()
	template<CT::Block WRAPPER>
	WRAPPER TAny<T>::Crop(const Offset& start, const Count& count) const {
		auto result = const_cast<TAny*>(this)->Crop<WRAPPER>(start, count);
		result.MakeConst();
		return Abandon(result);
	}
	
	/// Get a part of this container															
	///	@tparam WRAPPER - the container to use for the part						
	///			            use Block for unreferenced container					
	///	@return a container that represents the cropped part						
	TEMPLATE()
	template<CT::Block WRAPPER>
	WRAPPER TAny<T>::Crop(const Offset& start, const Count& count) {
		CheckRange(start, count);
		if (count == 0) {
			WRAPPER result {Disown(*this)};
			result.ResetMemory();
			return Abandon(result);
		}

		WRAPPER result {*this};
		result.MakeStatic();
		result.mCount = result.mReserved = count;
		result.mRaw += start * GetStride();
		return Abandon(result);
	}
	
	/// Get a size based on reflected allocation page and count (unsafe)			
	///	@param count - the number of elements to request							
	///	@returns both the provided byte size and reserved count					
	TEMPLATE()
	auto TAny<T>::RequestSize(const Count& count) const noexcept {
		if constexpr (CT::Sparse<T>) {
			AllocationRequest result;
			const auto requested = sizeof(KnownPointer) * count;
			result.mByteSize = requested > Alignment ? Roof2(requested) : Alignment;
			result.mElementCount = result.mByteSize / sizeof(KnownPointer);
			return result;
		}
		else return mType->RequestSize(sizeof(T) * count);
	}

	/// Allocate a number of elements, relying on the type of the container		
	///	@tparam CREATE - true to call constructors and initialize count		
	///	@tparam SETSIZE - true to set size without calling any constructors	
	///	@param elements - number of elements to allocate							
	TEMPLATE()
	template<bool CREATE, bool SETSIZE>
	void TAny<T>::Allocate(Count elements) {
		static_assert(!CREATE || CT::Sparse<T> || !CT::Abstract<T>,
			"Can't allocate and default-construct abstract items in dense TAny");

		// Allocate/reallocate															
		const auto request = RequestSize(elements);
		if (mEntry) {
			if (mReserved >= elements) {
				if (mCount > elements) {
					// Destroy back entries on smaller allocation				
					RemoveIndex(elements, mCount - elements);
					return;
				}

				// Required memory is already available							
				if constexpr (CREATE) {
					// But is not yet initialized, so initialize it				
					if (mCount < elements) {
						const auto count = elements - mCount;
						CropInner(mCount, count, count)
							.template CallKnownDefaultConstructors<T>(count);
					}
				}

				if constexpr (CREATE || SETSIZE)
					mCount = elements;
				return;
			}

			// Reallocate																	
			Block previousBlock = *this;
			if (mEntry->GetUses() == 1) {
				// Memory is used only once and it is safe to move it			
				// Make note, that Allocator::Reallocate doesn't copy			
				// anything, it doesn't use realloc for various reasons, so	
				// we still have to call move construction for all elements	
				// if entry moved (enabling MANAGED_MEMORY feature				
				// significantly reduces the possiblity for a move)			
				// Also, make sure to free the previous mEntry if moved		
				mEntry = Inner::Allocator::Reallocate(request.mByteSize, mEntry);
				LANGULUS_ASSERT(mEntry, Except::Allocate, "Out of memory");

				if (mEntry != previousBlock.mEntry) {
					// Memory moved, and we should call move-construction		
					mRaw = mEntry->GetBlockStart();
					CallKnownMoveConstructors<false, T>(previousBlock.mCount, previousBlock);
				}
			}
			else {
				// Memory is used from multiple locations, and we must		
				// copy the memory for this block - we can't move it!			
				mEntry = Inner::Allocator::Allocate(request.mByteSize);
				LANGULUS_ASSERT(mEntry, Except::Allocate, "Out of memory");
				mRaw = mEntry->GetBlockStart();
				CallKnownCopyConstructors<true, T>(previousBlock.mCount, previousBlock);
			}

			if constexpr (CREATE) {
				// Default-construct the rest											
				const auto count = elements - mCount;
				CropInner(mCount, count, count)
					.template CallKnownDefaultConstructors<T>(count);
			}
		}
		else {
			// Allocate a fresh set of elements										
			mEntry = Inner::Allocator::Allocate(request.mByteSize);
			LANGULUS_ASSERT(mEntry, Except::Allocate, "Out of memory");
			mRaw = mEntry->GetBlockStart();

			if constexpr (CREATE) {
				// Default-construct everything										
				CropInner(mCount, elements, elements)
					.template CallKnownDefaultConstructors<T>(elements);
			}
		}
		
		if constexpr (CREATE || SETSIZE)
			mCount = elements;
		mReserved = request.mElementCount;
	}
	
	/// Extend the container and return the new part									
	///	@tparam WRAPPER - the container to use for the extended part			
	///	@return a container that represents the extended part						
	TEMPLATE()
	template<CT::Block WRAPPER>
	WRAPPER TAny<T>::Extend(const Count& count) {
		if (IsStatic())
			return {};

		const auto newCount = mCount + count;
		if (mEntry && newCount > mReserved) {
			// Allocate more space														
			mEntry = Inner::Allocator::Reallocate(GetStride() * newCount, mEntry);
			LANGULUS_ASSERT(mEntry, Except::Allocate, "Out of memory");
			mRaw = mEntry->GetBlockStart();
			mReserved = newCount;
		}

		// Initialize new elements														
		auto extension = CropInner(mCount, count, count);
		extension.template CallKnownDefaultConstructors<T>(count);
		extension.MakeStatic();

		// Return the extension															
		mCount += count;
		WRAPPER result;
		static_cast<Block&>(result) = extension;
		if constexpr (!CT::Same<WRAPPER, Block>)
			mEntry->Keep();
		return Abandon(result);
	}
	
	/// Destructive concatenation																
	///	@param rhs - any type that is convertible to this TAny<T>				
	///	@return a reference to this container											
	TEMPLATE()
	template<class WRAPPER, class RHS>
	TAny<T>& TAny<T>::operator += (const RHS& rhs) {
		if constexpr (CT::POD<T> && CT::DerivedFrom<RHS, TAny>) {
			// Concatenate POD data directly (optimization)						
			const auto count = rhs.GetCount();
			Allocate<false>(mCount + count);
			CopyMemory(rhs.mRaw, GetRawEnd(), count);
			mCount += count;
			return *this;
		}
		else if constexpr (CT::Convertible<RHS, WRAPPER>) {
			// Finally, attempt converting											
			return operator += <WRAPPER>(static_cast<WRAPPER>(rhs));
		}
		else LANGULUS_ERROR("Can't concatenate - RHS is not convertible to WRAPPER");
	}

	/// Concatenate containers																	
	///	@param rhs - any type that is convertible to this TAny<T>				
	///	@return a new container																
	TEMPLATE()
	template<class WRAPPER, class RHS>
	WRAPPER TAny<T>::operator + (const RHS& rhs) const {
		if constexpr (CT::POD<T> && CT::DerivedFrom<RHS, TAny>) {
			// Concatenate bytes															
			WRAPPER result {Disown(*this)};
			result.mCount += rhs.mCount;
			if (result.mCount) {
				const auto request = RequestSize(result.mCount);
				result.mEntry = Inner::Allocator::Allocate(request.mByteSize);
				LANGULUS_ASSERT(result.mEntry, Except::Allocate, "Out of memory");

				result.mRaw = result.mEntry->GetBlockStart();
				result.mReserved = request.mElementCount;
				CopyMemory(mRaw, result.mRaw, mCount);
				CopyMemory(rhs.mRaw, result.mRaw + mCount, rhs.mCount);
			}
			else {
				result.mEntry = nullptr;
				result.mRaw = nullptr;
				result.mReserved = 0;
			}

			return Abandon(result);
		}
		else if constexpr (CT::Convertible<RHS, WRAPPER>) {
			// Attempt converting														
			return operator + <WRAPPER>(static_cast<WRAPPER>(rhs));
		}
		else LANGULUS_ERROR("Can't concatenate - RHS is not convertible to WRAPPER");
	}
	
	/// Compare with another TAny, order matters											
	///	@param other - container to compare with										
	///	@return true if both containers match completely							
	TEMPLATE()
	bool TAny<T>::Compare(const TAny& other) const noexcept {
		if (mRaw == other.mRaw)
			return mCount == other.mCount;
		else if (mCount != other.mCount)
			return false;

		auto t1 = GetRaw();
		auto t2 = other.GetRaw();
		while (t1 < GetRawEnd() && *(t1++) == *(t2++));
		return (t1 - GetRaw()) == mCount;
	}

	/// Compare with another container of the same type								
	///	@param other - the container to compare with									
	///	@return true if both containers are identical								
	TEMPLATE()
	template<CT::Data ALT_T>
	bool TAny<T>::operator == (const TAny<ALT_T>& other) const noexcept {
		if constexpr (CT::Same<T, ALT_T>)
			return Compare(other);
		else
			return false;
	}

	/// Compare with block of unknown type													
	///	@param other - the block to compare with										
	///	@return true if both containers are identical								
	TEMPLATE()
	bool TAny<T>::operator == (const Any& other) const noexcept {
		static_assert(sizeof(Block) == sizeof(TAny), "Binary incompatiblity");
		if (!Is(other.GetType()))
			return false;
		return Compare(reinterpret_cast<const TAny&>(other));
	}

	/// Compare loosely with another TAny, ignoring case								
	/// This function applies only if T is character									
	///	@param other - text to compare with												
	///	@return true if both containers match loosely								
	TEMPLATE()
	bool TAny<T>::CompareLoose(const TAny& other) const noexcept requires CT::Character<T> {
		if (mRaw == other.mRaw)
			return mCount == other.mCount;
		else if (mCount != other.mCount)
			return false;

		auto t1 = GetRaw();
		auto t2 = other.GetRaw();
		while (t1 < GetRawEnd() && ::std::tolower(*t1++) == ::std::tolower(*t2++));
		return (t1 - GetRaw()) == mCount;
	}

	/// Count how many consecutive elements match in two containers				
	///	@param other - container to compare with										
	///	@return the number of matching items											
	TEMPLATE()
	Count TAny<T>::Matches(const TAny& other) const noexcept {
		if (mRaw == other.mRaw)
			return ::std::min(mCount, other.mCount);

		auto t1 = GetRaw();
		auto t2 = other.GetRaw();
		const auto t1end = GetRawEnd();
		const auto t2end = other.GetRawEnd();
		while (t1 != t1end && t2 != t2end && *t1++ == *t2++);

		/*
		__m128i first = _mm_loadu_si128( reinterpret_cast<__m128i*>( &arr1 ) );
		__m128i second = _mm_loadu_si128( reinterpret_cast<__m128i*>( &arr2 ) );
		return std::popcount(_mm_movemask_epi8( _mm_cmpeq_epi8( first, second ) ));
		*/

		return t1 - GetRaw();
	}

	/// Compare loosely with another, ignoring upper-case								
	/// Count how many consecutive letters match in two strings						
	///	@param other - text to compare with												
	///	@return the number of matching symbols											
	TEMPLATE()
	Count TAny<T>::MatchesLoose(const TAny& other) const noexcept requires CT::Character<T> {
		if (mRaw == other.mRaw)
			return ::std::min(mCount, other.mCount);

		auto t1 = GetRaw();
		auto t2 = other.GetRaw();
		const auto t1end = GetRawEnd();
		const auto t2end = other.GetRawEnd();
		while (t1 != t1end && t2 != t2end && ::std::tolower(*t1++) == ::std::tolower(*t2++));
		return t1 - GetRaw();
	}













	///																								
	///	Known pointer implementation														
	///																								
	#define KNOWNPOINTER() TAny<T>::KnownPointer
	
	/// Copy-construct a pointer - references the block								
	///	@param other - the pointer to reference										
	TEMPLATE()
	KNOWNPOINTER()::KnownPointer(const KnownPointer& other) noexcept
		: mPointer {other.mPointer}
		, mEntry {other.mEntry} {
		if (mEntry)
			mEntry->Keep();
	}

	/// Move-construct a pointer 																
	///	@param other - the pointer to move												
	TEMPLATE()
	KNOWNPOINTER()::KnownPointer(KnownPointer&& other) noexcept
		: mPointer {other.mPointer}
		, mEntry {other.mEntry} {
		other.mPointer = nullptr;
		other.mEntry = nullptr;
	}

	/// Copy-construct a pointer, without referencing it								
	///	@param other - the pointer to copy												
	TEMPLATE()
	KNOWNPOINTER()::KnownPointer(Disowned<KnownPointer>&& other) noexcept
		: mPointer {other.mValue.mPointer}
		, mEntry {nullptr} {}

	/// Move-construct a pointer, minimally resetting the source					
	///	@param other - the pointer to move												
	TEMPLATE()
	KNOWNPOINTER()::KnownPointer(Abandoned<KnownPointer>&& other) noexcept
		: mPointer {other.mValue.mPointer}
		, mEntry {other.mValue.mEntry} {
		other.mValue.mEntry = nullptr;
	}

	/// Find and reference a pointer															
	///	@param pointer - the pointer to reference										
	TEMPLATE()
	KNOWNPOINTER()::KnownPointer(const T& pointer)
		: mPointer {pointer} {
		#if LANGULUS_FEATURE(MANAGED_MEMORY)
			// If we're using managed memory, we can search if the pointer	
			// is owned by us, and get its block									
			mEntry = Inner::Allocator::Find(MetaData::Of<Decay<T>>(), pointer);
			if (mEntry)
				mEntry->Keep();
		#endif
	}

	/// Copy a disowned pointer, no search for block will be performed			
	///	@param pointer - the pointer to copy											
	TEMPLATE()
	KNOWNPOINTER()::KnownPointer(Disowned<T>&& pointer) noexcept
		: mPointer {pointer.mValue} {}

	/// Dereference (and eventually destroy)												
	TEMPLATE()
	KNOWNPOINTER()::~KnownPointer() {
		Free();
	}

	/// Free the contents of the known pointer (inner function)						
	///	@attention doesn't reset pointers												
	TEMPLATE()
	void KNOWNPOINTER()::Free() {
		if (!mEntry)
			return;

		if (mEntry->GetUses() == 1) {
			if constexpr (!CT::POD<T> && CT::Destroyable<T>) {
				using DT = Decay<T>;
				mPointer->~DT();
			}
			Inner::Allocator::Deallocate(mEntry);
		}
		else mEntry->Free();
	}

	/// Copy-assign a known pointer, dereferencing old and referencing new		
	///	@param rhs - the new pointer														
	TEMPLATE()
	typename KNOWNPOINTER()& KNOWNPOINTER()::operator = (const KnownPointer& rhs) noexcept {
		Free();
		new (this) KnownPointer {rhs};
		return *this;
	}

	/// Move-assign a known pointer, dereferencing old and moving new				
	///	@param rhs - the new pointer														
	TEMPLATE()
	typename KNOWNPOINTER()& KNOWNPOINTER()::operator = (KnownPointer&& rhs) noexcept {
		Free();
		new (this) KnownPointer {Forward<KnownPointer>(rhs)};
		return *this;
	}

	/// Copy-assign a known pointer, dereferencing old but not referencing new	
	///	@param rhs - the new pointer														
	TEMPLATE()
	typename KNOWNPOINTER()& KNOWNPOINTER()::operator = (Disowned<KnownPointer>&& rhs) noexcept {
		Free();
		new (this) KnownPointer {rhs.Forward()};
		return *this;
	}

	/// Move-assign a known pointer, dereferencing old and moving new				
	///	@param rhs - the new pointer														
	TEMPLATE()
	typename KNOWNPOINTER()& KNOWNPOINTER()::operator = (Abandoned<KnownPointer>&& rhs) noexcept {
		Free();
		new (this) KnownPointer {rhs.Forward()};
		return *this;
	}

	/// Copy-assign a dangling pointer, finding its block and referencing		
	///	@param rhs - pointer to copy and reference									
	TEMPLATE()
	typename KNOWNPOINTER()& KNOWNPOINTER()::operator = (const T& rhs) {
		Free();
		new (this) KnownPointer {rhs};
		return *this;
	}

	/// Copy-assign a dangling pointer, but don't reference it						
	///	@param rhs - pointer to copy														
	TEMPLATE()
	typename KNOWNPOINTER()& KNOWNPOINTER()::operator = (Disowned<T>&& rhs) {
		Free();
		new (this) KnownPointer {rhs.Forward()};
		return *this;
	}

	/// Reset the known pointer																
	TEMPLATE()
	typename KNOWNPOINTER()& KNOWNPOINTER()::operator = (::std::nullptr_t) {
		Free();
		mPointer = nullptr;
		mEntry = nullptr;
		return *this;
	}

	/// Compare two known pointers, by pointer and value								
	///	@param rhs - the pointer to compare against									
	///	@return true if pointers/values match											
	TEMPLATE()
	bool KNOWNPOINTER()::operator == (const KNOWNPOINTER()& rhs) const noexcept {
		return operator == (rhs.mPointer);
	}

	/// Compare two known pointers, by pointer and value								
	///	@param rhs - the pointer to compare against									
	///	@return true if pointers/values match											
	TEMPLATE()
	bool KNOWNPOINTER()::operator == (const Decay<T>* rhs) const noexcept {
		if constexpr (CT::Comparable<T, T>)
			return mPointer == rhs || (mPointer && *mPointer == *rhs);
		else
			return mPointer == rhs;
	}

	/// Compare two known pointers, by pointer and value								
	///	@param rhs - the pointer to compare against									
	///	@return true if pointers/values match											
	TEMPLATE()
	bool KNOWNPOINTER()::operator == (const Decay<T>& rhs) const noexcept {
		return operator == (&rhs);
	}

	/// Get hash of the pointer inside														
	///	@return the hash																		
	TEMPLATE()
	Hash KNOWNPOINTER()::GetHash() const {
		if (!mPointer)
			return {};
		return HashData(*mPointer);
	}

	/// Pointer dereferencing (const)														
	///	@attention assumes contained pointer is valid								
	TEMPLATE()
	auto KNOWNPOINTER()::operator -> () const {
		LANGULUS_ASSUME(UserAssumes, mPointer, "Invalid pointer");
		return mPointer;
	}

	/// Pointer dereferencing																	
	///	@attention assumes contained pointer is valid								
	TEMPLATE()
	auto KNOWNPOINTER()::operator -> () {
		LANGULUS_ASSUME(UserAssumes, mPointer, "Invalid pointer");
		return mPointer;
	}

	/// Pointer dereferencing (const)														
	///	@attention assumes contained pointer is valid								
	TEMPLATE()
	decltype(auto) KNOWNPOINTER()::operator * () const {
		LANGULUS_ASSUME(UserAssumes, mPointer, "Invalid pointer");
		return *mPointer;
	}

	/// Pointer dereferencing																	
	///	@attention assumes contained pointer is valid								
	TEMPLATE()
	decltype(auto) KNOWNPOINTER()::operator * () {
		LANGULUS_ASSUME(UserAssumes, mPointer, "Invalid pointer");
		return *mPointer;
	}

} // namespace Langulus::Anyness

#undef TEMPLATE
#undef KNOWNPOINTER
