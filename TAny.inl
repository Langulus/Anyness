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
	}

	/// Destructor																					
	TEMPLATE()
	TAny<T>::~TAny() {
		Free();
	}
	
	/// Destructor																					
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

	/// Shallow copy construction from Any, that checks type							
	/// Any can contain anything, so there's a bit of type-checking overhead	
	///	@param other - the anyness to reference										
	TEMPLATE()
	TAny<T>::TAny(const Any& other) : TAny {} {
		if (!CastsToMeta(other.GetType())) {
			Throw<Except::Copy>(Logger::Error()
				<< "Bad shallow-copy-construction for TAny: from "
				<< GetToken() << " to " << other.GetToken());
		}

		CopyProperties<false>(other);
		Keep();
	}

	/// Move-construction from Any, that checks type									
	/// Any can contain anything, so there's a bit of type-checking overhead	
	///	@param other - the container to move											
	TEMPLATE()
	TAny<T>::TAny(Any&& other) : TAny {} {
		if (!CastsToMeta(other.GetType())) {
			Throw<Except::Copy>(Logger::Error()
				<< "Bad move-construction for TAny: from "
				<< GetToken() << " to " << other.GetToken());
		}

		CopyProperties<false>(other);
		other.ResetMemory();
		other.ResetState();
	}

	/// Shallow copy construction from blocks (const)									
	/// Block can contain anything, so there's a bit of type-checking overhead	
	///	@param copy - the block to reference											
	TEMPLATE()
	TAny<T>::TAny(const Block& copy)
		: TAny {Any {copy}} { }

	/// Move construction - moves block and references content						
	///	@attention since we are not aware if that block is referenced, we		
	///				  reference it, and we do not reset the other block to		
	///				  avoid memory leaks														
	///	@param other - the block to move													
	TEMPLATE()
		TAny<T>::TAny(Block&& copy)
		: TAny {Any {Forward<Block>(copy)}} { }

	/// Copy other but do not reference it, because it is disowned					
	///	@param other - the block to copy													
	TEMPLATE()
	TAny<T>::TAny(Disowned<TAny>&& other) noexcept
		: Any {other.template Forward<Any>()} { }
	
	/// Move other, but do not bother cleaning it up, because it is disowned	
	///	@param other - the block to move													
	TEMPLATE()
	TAny<T>::TAny(Abandoned<TAny>&& other) noexcept
		: Any {other.template Forward<Any>()} { }
	
	/// Construct by copying/referencing value of non-block type					
	///	@param other - the dense value to shallow-copy								
	TEMPLATE()
	TAny<T>::TAny(const T& other) requires CT::CustomData<T>
		: Any {other} { }

	/// Construct by moving a dense value of non-block type							
	///	@param initial - the dense value to forward and emplace					
	TEMPLATE()
		TAny<T>::TAny(T&& initial) requires CT::CustomData<T>
		: Any {Forward<T>(initial)} { }

	/// Construct manually by referencing memory if owned								
	/// If you want to avoid copying, use the Disowned alternative					
	///	@param raw - raw memory to reference, or clone if not owned				
	///	@param count - number of items inside 'raw'									
	TEMPLATE()
	TAny<T>::TAny(const T* raw, const Count& count)
		: Any {Block {
				DataState::Constrained, MetaData::Of<T>(), count, 
				reinterpret_cast<const Byte*>(raw)}} {
		TakeAuthority();
	}

	/// Construct manually by wrapping an array											
	///	@param raw - raw memory to interface without referencing and copying	
	///	@param count - number of items inside 'raw'									
	TEMPLATE()
	TAny<T>::TAny(Disowned<const T*>&& raw, const Count& count)
		: Any {Block {
				DataState::Constrained, MetaData::Of<T>(), count, 
				reinterpret_cast<const Byte*>(raw.mValue), nullptr}} {}

	/// Shallow-copy assignment																
	///	@param other - the container to shallow-copy									
	///	@return a reference to this container											
	TEMPLATE()
	TAny<T>& TAny<T>::operator = (const TAny<T>& other) {
		if (this == &other)
			return *this;

		Free();
		other.Keep();
		CopyProperties<true>(other);
		return *this;
	}

	/// Move assignment																			
	///	@param other - the container to move											
	///	@return a reference to this container											
	TEMPLATE()
	TAny<T>& TAny<T>::operator = (TAny<T>&& other) noexcept {
		if (this == &other)
			return *this;

		Free();
		CopyProperties<true>(other);
		other.ResetMemory();
		other.ResetState();
		return *this;
	}

	/// Shallow-copy runtime container														
	/// This is a bit slower, because checks type compatibility at runtime		
	///	@param other - the container to shallow-copy									
	///	@return a reference to this container											
	TEMPLATE()
	TAny<T>& TAny<T>::operator = (const Any& other) {
		if (static_cast<Any*>(this) == &other)
			return *this;

		if (!CastsToMeta(other.mType)) {
			Throw<Except::Copy>(Logger::Error()
				<< "Bad shallow-copy-assignment for TAny: from "
				<< GetToken() << " to " << other.GetToken());
		}

		Free();
		other.Keep();
		ResetState();
		CopyProperties<false>(other);
	}

	/// Move another runtime container in this one										
	/// This is a bit slower, because checks type compatibility at runtime		
	///	@param other - the container to move											
	///	@return a reference to this container											
	TEMPLATE()
	TAny<T>& TAny<T>::operator = (Any&& other) noexcept {
		if (static_cast<Any*>(this) == &other)
			return *this;

		if (!CastsToMeta(other.mType)) {
			Throw<Except::Copy>(Logger::Error()
				<< "Bad move-assignment for TAny: from "
				<< GetToken() << " to " << other.GetToken());
		}

		Free();
		ResetState();
		CopyProperties<false>(other);
		other.ResetMemory();
		other.ResetState();
	}

	/// Shallow-copy Block																		
	/// This is a bit slower, because checks type compatibility at runtime		
	///	@param other - the container to shallow-copy									
	///	@return a reference to this container											
	TEMPLATE()
	TAny<T>& TAny<T>::operator = (const Block& other) {
		if (static_cast<Block*>(this) == &other)
			return *this;

		if (!CastsToMeta(other.mType)) {
			Throw<Except::Copy>(Logger::Error()
				<< "Bad shallow-copy-assignment for TAny: from "
				<< GetToken() << " to " << other.GetToken());
		}

		Free();
		other.Keep();
		ResetState();
		CopyProperties<false>(other);
	}

	/// Seems like a move, but is actually a shallow-copy of a Block				
	/// This is a bit slower, because checks type compatibility at runtime		
	///	@attention other is never reset													
	///	@param other - the container to shallow-copy									
	///	@return a reference to this container											
	TEMPLATE()
	TAny<T>& TAny<T>::operator = (Block&& other) noexcept {
		return operator = (static_cast<const Block&>(other));
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
			InsertInner<T, true>(&other, 1, 0);
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
		if constexpr (CT::Same<T, Disowned<TAny>> || CT::Same<T, Abandoned<TAny>>) {
			// Data is guaranteed to be compatible, replace it					
			Free();
			CopyProperties<true>(other.mValue);
			if constexpr (CT::Same<T, Abandoned<TAny>>)
				other.mValue.mEntry = nullptr;
		}
		else if constexpr (CT::Same<T, Any> || CT::Same<T, Disowned<Any>> || CT::Same<T, Abandoned<Any>>) {
			// Move the other onto this if type is compatible					
			if (!CastsToMeta(other.mType)) {
				Throw<Except::Copy>(Logger::Error()
					<< "Bad move-assignment for TAny: from "
					<< GetToken() << " to " << other.GetToken());
			}
	
			// Overwrite everything except the type-constraint					
			Free();
			ResetState();
			if constexpr (CT::Same<T, Any>) {
				CopyProperties<false>(other);
				other.ResetMemory();
				other.ResetState();
			}
			else {
				CopyProperties<true>(other.mValue);
				if constexpr (CT::Same<T, Abandoned<Any>>)
					other.mValue.mEntry = nullptr;
			}
		}
		else if constexpr (CT::Same<T, Block>) {
			// Always reference a Block, by wrapping it in an Any				
			operator = (TAny {Forward<Block>(other)});
		}
		else {
			if (GetUses() == 1) {
				// Just destroy and reuse memory										
				CallKnownDestructors<T>();
				mCount = 0;
				EmplaceInner<T, true>(Forward<T>(other), 0);
			}
			else {
				// Reset and allocate new memory										
				Reset();
				operator << (Forward<T>(other));
			}
		}

		return *this;
	}

	/// An internal function used to copy members, without copying type and		
	/// without overwriting states, if required											
	///	@param other - the block to copy from											
	TEMPLATE()
	template<bool OVERWRITE>
	void TAny<T>::CopyProperties(const Block& other) noexcept {
		mRaw = other.mRaw;
		mCount = other.mCount;
		mReserved = other.mReserved;
		if constexpr (OVERWRITE)
			mState = other.mState;
		else
			mState += other.mState;
		mEntry = other.mEntry;
	}

	/// Reset container state																	
	TEMPLATE()
	void TAny<T>::ResetState() noexcept {
		Block::ResetState<true, CT::Sparse<T>>();
	}

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
	TAny<T> TAny<T>::Wrap(const T& anything) {
		TAny<T> temp;
		temp << anything;
		return temp;
	}

	/// Pack a c-array inside container, doing a shallow copy						
	///	@param anything - pack it inside a dense container							
	///	@returns the pack containing the data											
	TEMPLATE()
	template<Count COUNT>
	TAny<T> TAny<T>::Wrap(const T(&anything)[COUNT]) {
		TAny<T> temp;
		temp.Reserve(COUNT);
		for (Count i = 0; i < COUNT; ++i)
			temp << anything[i];
		return temp;
	}

	/// Pack an array inside container, doing a shallow copy							
	///	@param anything - pack it inside a dense container							
	///	@param count - number of items													
	///	@returns the pack containing the data											
	TEMPLATE()
	TAny<T> TAny<T>::Wrap(const T* anything, const Count& count) {
		TAny<T> temp;
		temp.Reserve(count);
		for (Count i = 0; i < count; ++i)
			temp << anything[i];
		return temp;
	}

	/// Allocate 'count' elements and fill the container with zeroes				
	TEMPLATE()
	void TAny<T>::Null(const Count& count) {
		Allocate(count, false, true);
		FillMemory(mRaw, {}, GetSize());
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
	TEMPLATE()
	TAny<T> TAny<T>::Clone() const {
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
				
				if constexpr (CT::CloneMakable<T>)
					new (&coalesced[counter]) Type {(*from)->Clone()};
				else if constexpr (CT::POD<T>)
					CopyMemory(**from, &coalesced[counter], sizeof(Type));
				else
					LANGULUS_ASSERT("Can't clone a container made of non-clonable/non-POD type");
				
				*to = &coalesced[counter];
				++from; ++to;
				++counter;
			}
			
			coalesced.Reference(counter);
		}
		else if constexpr (CT::CloneMakable<T>) {
			// Clone dense elements by calling Clone() methods one by one	
			while (from < GetRawEnd()) {
				new (to) Type {from->Clone()};
				++from; ++to;
			}
		}
		else if constexpr (CT::POD<T>) {
			// Batch copy everything at once											
			CopyMemory(from, to, sizeof(Type) * mCount);
		}
		else LANGULUS_ASSERT("Can't clone a container made of non-clonable/non-POD type");
	
		return Abandon(result);
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
		return Any::GetRawAs<T>() + mCount;
	}

	/// Return the typed raw data	end pointer												
	///	@return a mutable pointer to one past the last element in the array	
	TEMPLATE()
	auto TAny<T>::GetRawEnd() noexcept {
		return Any::GetRawAs<T>() + mCount;
	}

	/// Get an element in the way you want (const, unsafe)							
	/// This is a statically optimized variant of Block::Get							
	TEMPLATE()
	template<CT::Data ALT_T>
	decltype(auto) TAny<T>::Get(const Offset& index) const SAFETY_NOEXCEPT() {
		SAFETY(if (index >= mCount)
			Throw<Except::Access>("Index out of range"));

		const T& element = GetRaw()[index];
		if constexpr (CT::Dense<T> && CT::Dense<ALT_T>)
			// Dense -> Dense (returning a reference)								
			return static_cast<const Decay<ALT_T>&>(element);
		else if constexpr (CT::Sparse<T> && CT::Dense<ALT_T>)
			// Sparse -> Dense (returning a reference)							
			return static_cast<const Decay<ALT_T>&>(*element->mPointer);
		else if constexpr (CT::Dense<T> && CT::Sparse<ALT_T>)
			// Dense -> Sparse (returning a pointer)								
			return static_cast<const Decay<ALT_T>*>(&element);
		else
			// Sparse -> Sparse (returning a reference to pointer)			
			return static_cast<const Decay<ALT_T>* const&>(element->mPointer);
	}

	/// Get an element in the way you want (unsafe)										
	/// This is a statically optimized variant of Block::Get							
	TEMPLATE()
	template<CT::Data ALT_T>
	decltype(auto) TAny<T>::Get(const Offset& index) SAFETY_NOEXCEPT() {
		SAFETY(if (index >= mCount)
			Throw<Except::Access>("Index out of range"));

		T& element = GetRaw()[index];
		if constexpr (CT::Dense<T> && CT::Dense<ALT_T>)
			// Dense -> Dense (returning a reference)								
			return static_cast<Decay<ALT_T>&>(element);
		else if constexpr (CT::Sparse<T> && CT::Dense<ALT_T>)
			// Sparse -> Dense (returning a reference)							
			return static_cast<Decay<ALT_T>&>(*element->mPointer);
		else if constexpr (CT::Dense<T> && CT::Sparse<ALT_T>)
			// Dense -> Sparse (returning a pointer)								
			return static_cast<Decay<ALT_T>*>(&element);
		else
			// Sparse -> Sparse (returning a reference to pointer)			
			return static_cast<Decay<ALT_T>*&>(element->mPointer);
	}

	/// Access typed dense elements via a simple index (unsafe)						
	///	@param idx - the index to get														
	///	@return a reference to the element												
	TEMPLATE()
	decltype(auto) TAny<T>::operator [] (const Offset& index) SAFETY_NOEXCEPT() requires CT::Dense<T> {
		return Get<T>(index);
	}

	TEMPLATE()
	decltype(auto) TAny<T>::operator [] (const Offset& index) const SAFETY_NOEXCEPT() requires CT::Dense<T> {
		return Get<T>(index);
	}

	/// Access typed dense elements via a complex index (const, safe)				
	///	@param idx - the index to get														
	///	@return a constant reference to the element									
	TEMPLATE()
	decltype(auto) TAny<T>::operator [] (const Index& index) requires CT::Dense<T> {
		return Get<T>(Block::ConstrainMore<T>(index).GetOffset());
	}

	TEMPLATE()
	decltype(auto) TAny<T>::operator [] (const Index& index) const requires CT::Dense<T> {
		return const_cast<TAny<T>&>(*this).operator [] (index);
	}

	/// Access typed sparse elements via a simple index (unsafe)					
	/// Will wrap pointer in a SparseElement wrapper, in order to reference		
	///	@param idx - the index to get														
	///	@return a reference to the element												
	TEMPLATE()
	typename TAny<T>::KnownPointer& TAny<T>::operator [] (const Offset& index) SAFETY_NOEXCEPT() requires CT::Sparse<T> {
		return Get<T>(index);
	}

	TEMPLATE()
	decltype(auto) TAny<T>::operator [] (const Offset& index) const SAFETY_NOEXCEPT() requires CT::Sparse<T> {
		return Get<T>(index);
	}

	/// Access typed sparse elements via a complex index (const)					
	/// Will wrap pointer in a SparseElement wrapper, in order to reference		
	///	@param idx - the index to get														
	///	@return a constant reference to the element									
	TEMPLATE()
	typename TAny<T>::KnownPointer& TAny<T>::operator [] (const Index& index) requires CT::Sparse<T> {
		return Get<T>(ConstrainMore<T>(index).GetOffset());
	}

	TEMPLATE()
	decltype(auto) TAny<T>::operator [] (const Index& index) const requires CT::Sparse<T> {
		return Get<T>(ConstrainMore<T>(index).GetOffset());
	}

	/// Access last element (unsafe)															
	///	@return a reference to the last element										
	TEMPLATE()
	decltype(auto) TAny<T>::Last() SAFETY_NOEXCEPT() {
		return Get<T>(mCount - 1);
	}

	/// Access last element (const, unsafe)												
	///	@return a constant reference to the last element							
	TEMPLATE()
	decltype(auto) TAny<T>::Last() const SAFETY_NOEXCEPT() {
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
	TEMPLATE()
	constexpr bool TAny<T>::IsAbstract() const noexcept {
		return CT::Abstract<T>;
	}
	
	/// Check if contained type is default-constructible								
	TEMPLATE()
	constexpr bool TAny<T>::IsConstructible() const noexcept {
		return CT::Defaultable<T>;
	}
	
	/// Check if contained type is deep														
	TEMPLATE()
	constexpr bool TAny<T>::IsDeep() const noexcept {
		return CT::Deep<T>;
	}

	/// Check if the contained type is a pointer											
	///	@return true if container contains pointers									
	TEMPLATE()
	constexpr bool TAny<T>::IsSparse() const noexcept {
		return CT::Sparse<T>;
	}

	/// Check if the contained type is not a pointer									
	///	@return true if container contains sequential data							
	TEMPLATE()
	constexpr bool TAny<T>::IsDense() const noexcept {
		return CT::Dense<T>;
	}

	/// Get the size of a single contained element, in bytes							
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
	constexpr Size TAny<T>::GetSize() const noexcept {
		return GetStride() * mCount;
	}

	/// Insert an item by move-construction												
	///	@param item - the item to move													
	///	@param index - the place where to emplace the item							
	///	@return 1 if successful																
	TEMPLATE()
	Count TAny<T>::Emplace(T&& item, const Index& index) {
		const auto starter = ConstrainMore<T>(index).GetOffset();

		// Allocate																			
		Allocate<false>(mCount + 1);

		// Move memory if required														
		if (starter < mCount) {
			if (GetUses() > 1) {
				Throw<Except::Reference>(Logger::Error()
					<< "Moving elements that are used from multiple places");
			}

			CropInner(starter + 1, 0, mCount - starter)
				.template CallKnownMoveConstructors<T>(
					mCount - starter,
					CropInner(starter, mCount - starter, mCount - starter)
				);
		}

		EmplaceInner<T, true>(Forward<T>(item), starter);
		return 1;
	}

	/// Insert item(s) by copy-construction												
	//TODO do a dedicated Append function
	///	@param items - an array of items to insert									
	///	@param count - the number of items to insert									
	///	@param index - the place where to insert the items							
	///	@return number of inserted items													
	TEMPLATE()
	Count TAny<T>::Insert(const T* items, const Count& count, const Index& index) {
		const auto starter = ConstrainMore<T>(index).GetOffset();

		// Allocate																			
		Allocate<false>(mCount + count);

		// Move memory if required														
		if (starter < mCount) {
			if (GetUses() > 1) {
				Throw<Except::Reference>(Logger::Error()
					<< "Moving elements that are used from multiple places");
			}

			CropInner(starter + count, 0, mCount - starter)
				.template CallKnownMoveConstructors<T>(
					mCount - starter,
					CropInner(starter, mCount - starter, mCount - starter)
				);
		}

		InsertInner<T, true>(items, count, starter);
		return count;
	}

	/// Push data at the back by copy-construction										
	///	@param other - the item to insert												
	///	@return a reference to this container for chaining							
	TEMPLATE()
	TAny<T>& TAny<T>::operator << (const T& other) {
		Insert(&other, 1, Index::Back);
		return *this;
	}

	/// Push data at the back by move-construction										
	///	@param other - the item to move													
	///	@return a reference to this container for chaining							
	TEMPLATE()
	TAny<T>& TAny<T>::operator << (T&& other) {
		Emplace(Forward<T>(other), Index::Back);
		return *this;
	}

	/// Push data at the front by copy-construction										
	///	@param other - the item to insert												
	///	@return a reference to this container for chaining							
	TEMPLATE()
	TAny<T>& TAny<T>::operator >> (const T& other) {
		Insert(&other, 1, Index::Front);
		return *this;
	}

	/// Push data at the front by move-construction										
	///	@param other - the item to move													
	///	@return a reference to this container for chaining							
	TEMPLATE()
	TAny<T>& TAny<T>::operator >> (T&& other) {
		Emplace(Forward<T>(other), Index::Front);
		return *this;
	}

	/// Merge anything compatible to container											
	/// By merging we mean anything that is not found is pushed						
	///	@param items - the items to find and push										
	///	@param count - number of items to push											
	///	@param idx - the place to insert them											
	///	@return the number of inserted items											
	TEMPLATE()
	Count TAny<T>::Merge(const T* items, const Count& count, const Index& idx) {
		return Any::Merge<T, false, TAny>(items, count, idx);
	}

	/// Push data at the back																	
	TEMPLATE()
	TAny<T>& TAny<T>::operator <<= (const T& other) {
		Merge(&other, 1, Index::Back);
		return *this;
	}

	/// Push data at the front																	
	TEMPLATE()
	TAny<T>& TAny<T>::operator >>= (const T& other) {
		Merge(&other, 1, Index::Front);
		return *this;
	}

	/// Find element(s) index inside container											
	///	@tparam ALT_T - type of the element to search for							
	///	@tparam REVERSE - whether to search in reverse order						
	///	@param item - the item to search for											
	///	@return the index of the found item, or Index::None if none found		
	TEMPLATE()
	template<CT::Data ALT_T, bool REVERSE>
	Index TAny<T>::Find(const ALT_T& item) const {
		static_assert(CT::Comparable<T, ALT_T>, 
			"Provided type ALT_T is not comparable to the contained type T");

		if constexpr (CT::Sparse<ALT_T>) {
			if constexpr (CT::Sparse<T>) {
				// Searching for pointer inside a sparse container				
				if constexpr (REVERSE) {
					// Searching in reverse												
					auto start = GetRawEnd() - 1;
					if (*start == item)
						return mCount - 1;

					const auto end = start - mCount;
					while (--start != end) {
						if constexpr (CT::DerivedFrom<T, ALT_T>) {
							if (static_cast<MakeConst<T>>(*start) == item)
								return start - GetRaw();
						}
						else if constexpr (CT::DerivedFrom<ALT_T, T>) {
							if (*start == static_cast<MakeConst<ALT_T>>(item))
								return start - GetRaw();
						}
						else LANGULUS_ASSERT("Type is not comparable to contained elements");
					}
				}
				else {
					// Searching forward													
					auto start = GetRaw();
					if (*start == item)
						return 0;

					const auto end = start + mCount;
					while (++start != end) {
						if constexpr (CT::DerivedFrom<T, ALT_T>) {
							if (static_cast<MakeConst<T>>(*start) == item)
								return start - GetRaw();
						}
						else if constexpr (CT::DerivedFrom<ALT_T, T>) {
							if (*start == static_cast<MakeConst<ALT_T>>(item))
								return start - GetRaw();
						}
						else LANGULUS_ASSERT("Type is not comparable to contained elements");
					}
				}
			}
			else {
				// Searching for pointer inside a dense container				
				// Pointer should reside inside container, so a single 		
				// check is completely enough											
				auto start = GetRaw();
				if constexpr (CT::DerivedFrom<T, ALT_T>) {
					const auto difference = item - static_cast<MakeConst<T>>(start);
					if (difference >= 0 && static_cast<Count>(difference) < mCount)
						return Index {difference};
				}
				else if constexpr (CT::DerivedFrom<ALT_T, T>) {
					const auto difference = static_cast<MakeConst<ALT_T>>(item) - start;
					if (difference >= 0 && static_cast<Count>(difference) < mCount)
						return Index {difference};
				}
				else LANGULUS_ASSERT("Type is not comparable to contained elements");
			}
		}
		else if constexpr (CT::Sparse<T>) {
			// Searching for value inside a sparse container					
			if constexpr (REVERSE) {
				// Searching in reverse													
				auto start = GetRawEnd() - 1;
				if (**start == item)
					return mCount - 1;

				const auto end = start - mCount;
				while (--start != end) {
					// Test pointers first, but only if T is too big for a	
					// single instruction comparison									
					if constexpr (sizeof(ALT_T) > sizeof(void*)) {
						if constexpr (CT::DerivedFrom<T, ALT_T>) {
							if (static_cast<MakeConst<T>>(*start) == &item)
								return start - GetRaw();
						}
						else if constexpr (CT::DerivedFrom<ALT_T, T>) {
							if (*start == static_cast<MakeConst<ALT_T>>(&item))
								return start - GetRaw();
						}
					}

					// Test by value														
					if (**start == item)
						return start - GetRaw();
				}
			}
			else {
				// Searching forward														
				auto start = GetRaw();
				if (**start == item)
					return 0;

				const auto end = start + mCount;
				while (++start != end) {
					// Test pointers first, but only if T is too big for a	
					// single instruction comparison									
					if constexpr (sizeof(ALT_T) > sizeof(void*)) {
						if constexpr (CT::DerivedFrom<T, ALT_T>) {
							if (static_cast<MakeConst<T>>(*start) == &item)
								return start - GetRaw();
						}
						else if constexpr (CT::DerivedFrom<ALT_T, T>) {
							if (*start == static_cast<MakeConst<ALT_T>>(&item))
								return start - GetRaw();
						}
					}

					// Test by value														
					if (**start == item)
						return start - GetRaw();
				}
			}
		}
		else {
			// Searching for value inside a dense container						
			// Test by value																
			if constexpr (REVERSE) {
				// Searching in reverse													
				auto start = GetRawEnd() - 1;
				if (*start == item)
					return mCount - 1;

				const auto end = start - mCount;
				while (--start != end) {
					if (*start == item)
						return start - GetRaw();
				}
			}
			else {
				// Searching forward														
				auto start = GetRaw();
				if (*start == item)
					return 0;

				const auto end = start + mCount;
				while (++start != end) {
					if (*start == item)
						return start - GetRaw();
				}
			}
		}

		// If this is reached, then no match was found							
		return Index::None;
	}

	/// Remove matching items																	
	///	@tparam ALT_T - type of the element to remove								
	///	@tparam REVERSE - whether to search in reverse order						
	///	@param item - the item to search for to remove								
	///	@return the number of removed items												
	TEMPLATE()
	template<CT::Data ALT_T, bool REVERSE>
	Count TAny<T>::RemoveValue(const ALT_T& item) {
		const auto found = Find<ALT_T, REVERSE>(item);
		if (found)
			return RemoveIndex(found.GetOffset(), 1);
		return 0;
	}

	/// Remove sequential raw indices in a given range									
	///	@param starter - simple index to start removing from						
	///	@param count - number of elements to remove									
	///	@return the number of removed elements											
	TEMPLATE()
	Count TAny<T>::RemoveIndex(const Count& starter, const Count& count) {
		SAFETY(if (starter >= mCount)
			Throw<Except::Access>(Logger::Error()
				<< "Index " << starter << " out of range " << mCount));
		SAFETY(if (count > mCount || starter + count > mCount)
			Throw<Except::Access>(Logger::Error()
				<< "Index " << starter << " out of range " << mCount));
		SAFETY(if (GetUses() > 1)
			Throw<Except::Reference>(Logger::Error()
				<< "Removing elements from a memory block, that is used from multiple places"));

		const auto ender = starter + count;
		if constexpr (CT::POD<T>) {
			// If data is POD and elements are on the back, we can			
			// get around constantness and staticness, by simply				
			// truncating the count without any reprecussions					
			if (ender == mCount)
				mCount = starter;
			else {
				if (IsConstant()) {
					Throw<Except::Access>(Logger::Error()
						<< "Attempting to RemoveIndex in a constant container");
				}

				if (IsStatic()) {
					Throw<Except::Access>(Logger::Error()
						<< "Attempting to RemoveIndex in a static container");
				}

				MoveMemory(GetRaw() + ender, GetRaw() + starter, sizeof(T) * (mCount - ender));
				mCount -= count;
			}

			return count;
		}
		else {
			if (IsConstant()) {
				Throw<Except::Access>(Logger::Error()
					<< "Attempting to RemoveIndex in a constant container");
			}

			if (IsStatic()) {
				Throw<Except::Access>(Logger::Error()
					<< "Attempting to RemoveIndex in a static container");
			}

			// Call the destructors on the correct region						
			CropInner(starter, count, count).template CallKnownDestructors<T>();

			if (ender < mCount) {
				// Fill gap	if any by invoking move constructions				
				const auto remains = mCount - ender;
				CropInner(starter, 0, remains)
					.template CallKnownMoveConstructors<T>(
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
		else LANGULUS_ASSERT("Can't sort container - T is not sortable");
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



	///																								
	///	Known pointer implementation														
	///																								
	
	/// When overwriting the element, previous pointer must be dereferenced		
	/// and the new one - referenced															
	///	@param pointer - the pointer to set												
	///	@return a reference to this sparse element									
	TEMPLATE()
	typename TAny<T>::KnownPointer& TAny<T>::KnownPointer::operator = (T pointer) {
		using DT = Decay<T>;
		if (mPointer == pointer)
			return *this;

		if (mEntry) {
			// Dereference/destroy the previous element							
			if (mEntry->GetUses() == 1) {
				mPointer->~DT();
				Inner::Allocator::Deallocate(mEntry);
			}
			else mEntry->Free();
		}

		// Set and reference the new element										
		mPointer = pointer;
		#if LANGULUS_FEATURE(MANAGED_MEMORY)
			mEntry = Inner::Allocator::Find(MetaData::Of<DT>(), pointer);
			if (mEntry)
				mEntry->Keep();
		#else
			mEntry = nullptr;
		#endif
		return *this;
	}

	/// When overwriting with nullptr, just dereference/destroy previous			
	///	@param pointer - null pointer														
	///	@return a reference to this sparse element									
	TEMPLATE()
	typename TAny<T>::KnownPointer& TAny<T>::KnownPointer::operator = (::std::nullptr_t) {
		if (mEntry) {
			// Dereference/destroy the previous element							
			if (mEntry->GetUses() == 1) {
				using DT = Decay<T>;
				mPointer->~DT();
				Inner::Allocator::Deallocate(mEntry);
			}
			else mEntry->Free();

			mEntry = nullptr;
			mPointer = nullptr;
		}

		return *this;
	}

	/// Implicit cast to a constant pointer												
	TEMPLATE()
	TAny<T>::KnownPointer::operator T() const noexcept {
		return mPointer;
	}

	/// Implicit cast to a mutable pointer													
	TEMPLATE()
	TAny<T>::KnownPointer::operator T() noexcept {
		return mPointer;
	}

	/// Pointer dereferencing (const)														
	TEMPLATE()
	auto TAny<T>::KnownPointer::operator -> () const {
		if (!mPointer)
			Throw<Except::Access>("Invalid pointer");
		return mPointer;
	}

	/// Pointer dereferencing																	
	TEMPLATE()
	auto TAny<T>::KnownPointer::operator -> () {
		if (!mPointer)
			Throw<Except::Access>("Invalid pointer");
		return mPointer;
	}

	/// Pointer dereferencing (const)														
	TEMPLATE()
	decltype(auto) TAny<T>::KnownPointer::operator * () const {
		if (!mPointer)
			Throw<Except::Access>("Invalid pointer");
		return *mPointer;
	}

	/// Pointer dereferencing																	
	TEMPLATE()
	decltype(auto) TAny<T>::KnownPointer::operator * () {
		if (!mPointer)
			Throw<Except::Access>("Invalid pointer");
		return *mPointer;
	}

	/// Call default constructors in a region and initialize memory				
	///	@param count - the number of elements to initialize						
	TEMPLATE()
	void TAny<T>::CallDefaultConstructors(const Count& count) {
		if constexpr (CT::Nullifiable<T>) {
			// Just zero the memory (optimization)									
			FillMemory(GetRawEnd(), {}, count * GetStride());
			mCount += count;
		}
		else if constexpr (CT::Defaultable<T>) {
			// Construct requested elements in place								
			new (GetRawEnd()) T [count];
			mCount += count;
		}
		else LANGULUS_ASSERT("Trying to default-construct elements that are incapable of default-construction");
	}
			
	/// Call copy constructors in a region and initialize memory					
	///	@param source - the elements to copy											
	TEMPLATE()
	void TAny<T>::CallCopyConstructors(const Count& count, const TAny& source) {
		if constexpr(CT::Sparse<T> || CT::POD<T>) {
			// Just copy the POD/pointer memory (optimization)					
			CopyMemory(source.GetRaw(), GetRawEnd(), GetStride() * count);

			if constexpr(CT::Sparse<T>) {
				// Since we're copying pointers, we have to reference the	
				// dense memory behind each one of them							
				auto p = GetRawSparse() + mCount;
				const auto pEnd = p + count;
				while (p != pEnd) {
					// Reference each pointer											
					if (p->mEntry)
						p->mEntry->Keep();
					++p;
				}
			}
		}
		else {
			// Both RHS and LHS are dense and non POD								
			// Call the reflected copy-constructor for each element			
			static_assert(CT::CopyMakable<T>, 
				"Trying to copy-construct but it's impossible for this type");

			auto from = source.GetRaw();
			auto to = GetRaw() + mCount;
			const auto toEnd = to + count;
			while (to != toEnd) {
				new (to) T {*from};
				++to;
				++from;
			}
		}
	}
	
	/// Get a constant part of this container												
	///	@tparam WRAPPER - the container to use for the part						
	///			            use Block for unreferenced container					
	///	@return a container that represents the cropped part						
	TEMPLATE()
	template<CT::Block WRAPPER>
	WRAPPER TAny<T>::Crop(const Offset& start, const Count& count) const {
		auto result = const_cast<TAny*>(this)->Crop<WRAPPER>(start, count);
		result.MakeConstant();
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
	///	@tparam CREATE - true to call constructors									
	///	@param elements - number of elements to allocate							
	TEMPLATE()
	template<bool CREATE>
	void TAny<T>::Allocate(Count elements) {
		static_assert(!CT::Abstract<T>, "Can't allocate abstract items");
		const auto request = RequestSize(elements);

		// Allocate/reallocate															
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
					if (mCount < elements)
						CallDefaultConstructors(elements - mCount);
				}

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
				if (!mEntry)
					Throw<Except::Allocate>("Out of memory on TAny reallocation");

				if (mEntry != previousBlock.mEntry) {
					// Memory moved, and we should call move-construction		
					mRaw = mEntry->GetBlockStart();
					mCount = 0;
					CallKnownMoveConstructors<T>(previousBlock.mCount, Move(previousBlock));
				}
				
				if constexpr (CREATE) {
					// Default-construct the rest										
					CallDefaultConstructors(elements - mCount);
				}
			}
			else {
				// Memory is used from multiple locations, and we must		
				// copy the memory for this block - we can't move it!			
				mEntry = Inner::Allocator::Allocate(request.mByteSize);
				if (!mEntry)
					Throw<Except::Allocate>("Out of memory on additional TAny allocation");

				mRaw = mEntry->GetBlockStart();
				mCount = 0;
				CallCopyConstructors(previousBlock.mCount, previousBlock);
				
				if constexpr (CREATE) {
					// Default-construct the rest										
					CallDefaultConstructors(elements - mCount);
				}
			}
		}
		else {
			// Allocate a fresh set of elements										
			mEntry = Inner::Allocator::Allocate(request.mByteSize);
			if (!mEntry)
				Throw<Except::Allocate>("Out of memory on fresh TAny allocation");

			mRaw = mEntry->GetBlockStart();
			if constexpr (CREATE) {
				// Default-construct everything										
				CallDefaultConstructors(elements);
			}
		}
		
		mReserved = request.mElementCount;
	}
	
	/// Extend the container and return the new part									
	///	@tparam WRAPPER - the container to use for the extended part			
	///			            use Block for unreferenced container					
	///	@return a container that represents the extended part						
	TEMPLATE()
	template<CT::Block WRAPPER>
	WRAPPER TAny<T>::Extend(const Count& count) {
		if (!count || IsStatic())
			// You can not extend static containers								
			return {};

		const auto newCount = mCount + count;
		const auto oldCount = mCount;
		if (newCount <= mReserved) {
			// There is enough available space										
			if constexpr (CT::POD<T>)
				// No need to call constructors for POD items					
				mCount += count;
			else
				CallDefaultConstructors(count);
		}
		else if (mEntry) {
			// Allocate more space														
			auto previousBlock = static_cast<Block&>(*this);
			mEntry = Inner::Allocator::Reallocate(GetStride() * newCount, mEntry);
			if (!mEntry)
				Throw<Except::Allocate>("Out of memory on TAny extension");

			mRaw = mEntry->GetBlockStart();
			if constexpr (CT::POD<T>) {
				// No need to call constructors for POD items					
				mCount = mReserved = newCount;
			}
			else {
				mReserved = newCount;
				CallDefaultConstructors(count);
			}
		}

		WRAPPER result {*this};
		result.MakeStatic();
		result.mRaw += oldCount;
		result.mCount = result.mReserved = count;
		return Abandon(result);
	}
	
	/// Destructive concatenation																
	///	@param rhs - any type that is convertible to this TAny<T>				
	///	@return a reference to this container											
	TEMPLATE()
	template<class WRAPPER, class RHS>
	TAny<T>& TAny<T>::operator += (const RHS& rhs) {
		if constexpr (CT::Sparse<RHS> && !CT::Array<RHS>) {
			// Dereference pointers that are not bound arrays					
			return operator += <WRAPPER>(*rhs);
		}
		else if constexpr (CT::POD<T> && CT::DerivedFrom<RHS, TAny>) {
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
		else LANGULUS_ASSERT("Can't concatenate - RHS is not convertible to WRAPPER");
	}

	/// Concatenate containers																	
	///	@param rhs - any type that is convertible to this TAny<T>				
	///	@return a new container																
	TEMPLATE()
	template<class WRAPPER, class RHS>
	WRAPPER TAny<T>::operator + (const RHS& rhs) const {
		if constexpr (CT::Sparse<RHS> && !CT::Array<RHS>) {
			// Dereference pointers that are not bound arrays					
			return operator + <WRAPPER>(*rhs);
		}
		else if constexpr (CT::POD<T> && CT::DerivedFrom<RHS, TAny>) {
			// Concatenate bytes															
			WRAPPER result {Disown(*this)};
			result.mCount += rhs.mCount;
			if (result.mCount) {
				const auto request = RequestSize(result.mCount);
				result.mEntry = Inner::Allocator::Allocate(request.mByteSize);
				if (!result.mEntry)
					Throw<Except::Allocate>("Out of memory on concatenating TAny");

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
		else LANGULUS_ASSERT("Can't concatenate - RHS is not convertible to WRAPPER");
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
		while (t1 < GetRawEnd() && *t1 == *t2) {
			++t1;
			++t2;
		}

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
		while (t1 < GetRawEnd() && (*t1 == *t2 || (::std::isalpha(*t1) && ::std::isalpha(*t2) && (*t1 + 32 == *t2 || *t1 == *t2 + 32)))) {
			++t1;
			++t2;
		}

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
		while (t1 < GetRawEnd() && *t1 == *t2) {
			++t1;
			++t2;
		}

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
		while (t1 < GetRawEnd() && (*t1 == *t2 || (::std::isalpha(*t1) && ::std::isalpha(*t2) && (*t1 + 32 == *t2 || *t1 == *t2 + 32)))) {
			++t1;
			++t2;
		}

		return t1 - GetRaw();
	}

} // namespace Langulus::Anyness

#undef TEMPLATE
