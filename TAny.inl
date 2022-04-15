#pragma once
#include "TAny.hpp"

#define TEMPLATE() template<ReflectedData T>

namespace Langulus::Anyness
{

	/// Default construction																	
	TEMPLATE()
	TAny<T>::TAny()
		: Any {Block{ DataState::Typed, MetaData::Of<T>() }} { }

	/// Shallow copy construction																
	///	@param copy - the anyness to reference											
	TEMPLATE()
	TAny<T>::TAny(const TAny<T>& copy)
		: Any {static_cast<const Any&>(copy)} { }

	/// Move construction																		
	///	@param copy - the anyness to move												
	TEMPLATE()
	TAny<T>::TAny(TAny<T>&& copy) noexcept
		: Any {Forward<Any>(copy)} { }

	/// Shallow copy construction from anyness, that checks type					
	///	@param copy - the anyness to reference											
	TEMPLATE()
	TAny<T>::TAny(const Any& copy)
		: TAny { } {
		if (copy.mType && !mType->InterpretsAs(copy.mType)) {
			throw Except::Copy(Logger::Error()
				<< "Bad memory assignment for type-constrained any: from "
				<< GetToken() << " to " << copy.GetToken());
		}

		// Overwrite everything except the type-constraint						
		mRaw = copy.mRaw;
		mCount = copy.mCount;
		mReserved = copy.mReserved;
		mState = copy.mState.mState | DataState::Typed;
		Block::Keep();
	}

	/// Move construction from anyness, that checks type								
	///	@param copy - the anyness to move												
	TEMPLATE()
	TAny<T>::TAny(Any&& copy)
		: TAny { } {
		if (copy.mType && !mType->InterpretsAs(copy.mType)) {
			throw Except::Move(Logger::Error()
				<< "Bad memory assignment for type-constrained any: from "
				<< GetToken() << " to " << copy.GetToken());
		}

		// Overwrite everything except the type-constraint						
		mRaw = copy.mRaw;
		mCount = copy.mCount;
		mReserved = copy.mReserved;
		mState = copy.mState.mState | DataState::Typed;
		copy.ResetInner();
	}

	/// Shallow copy construction from blocks, that checks type						
	///	@param copy - the anyness to reference											
	TEMPLATE()
	TAny<T>::TAny(const Block& copy)
		: TAny {Any { copy }} { }

	/// Move construction - moves block and references content						
	/// Since we are not aware if that block is referenced, we reference it		
	/// We do not reset the other block to avoid memory leaks						
	///	@param other - the block to move													
	TEMPLATE()
	TAny<T>::TAny(Block&& copy)
		: TAny {Any { Forward<Block>(copy) }} { }

	/// Construct by moving a dense value of non-block type							
	///	@param initial - the dense value to forward and emplace					
	TEMPLATE()
	TAny<T>::TAny(T&& initial) requires (TAny<T>::NotCustom)
		: Any {Forward<T>(initial)} { }

	/// Construct by copying/referencing value of non-block type					
	///	@param other - the dense value to shallow-copy								
	TEMPLATE()
	TAny<T>::TAny(const T& initial) requires (TAny<T>::NotCustom)
		: Any {initial} { }

	/// Construct by copying/referencing value of non-block type					
	///	@param other - the dense value to shallow-copy								
	TEMPLATE()
	TAny<T>::TAny(T& other) requires (TAny<T>::NotCustom)
		: TAny {const_cast<const T&>(other)} { }

	/// Shallow copy operator																	
	TEMPLATE()
	TAny<T>& TAny<T>::operator = (const TAny<T>& other) {
		Block::Free();
		Block::operator = (other);
		Block::Keep();
		return *this;
	}

	/// Move operator																				
	TEMPLATE()
	TAny<T>& TAny<T>::operator = (TAny<T>&& other) noexcept {
		Block::Free();
		Block::operator = (Forward<Block>(other));
		return *this;
	}

	/// Shallow copy operator																	
	TEMPLATE()
	TAny<T>& TAny<T>::operator = (const Any& other) {
		Block::Free();
		if (other.mType && !mType->InterpretsAs(other.mType)) {
			throw Except::Copy(Logger::Error()
				<< "Bad memory assignment for type-constrained any: from "
				<< GetToken() << " to " << other.GetToken());
		}

		// Overwrite everything except the type-constraint						
		mRaw = other.mRaw;
		mCount = other.mCount;
		mReserved = other.mReserved;
		mState = other.mState.mState | DataState::Typed;
		Block::Keep();
		return *this;
	}

	/// Move operator																				
	TEMPLATE()
	TAny<T>& TAny<T>::operator = (Any&& other) {
		SAFETY(const Block otherProbe = other);
		Block::Free();
		SAFETY(if (otherProbe != other || (other.CheckJurisdiction() && !other.CheckUsage()))
			throw Except::Move(Logger::Error()
				<< "You've hit a really nasty corner case, where trying to move a container destroys it, "
				<< "due to a circular referencing. Try to move a shallow-copy, instead of a reference to "
				<< "the original. Data may be incorrect at this point, but the moved container was: " << otherProbe.GetToken());
		);

		if (other.mType && !mType->InterpretsAs(other.mType)) {
			throw Except::Move(Logger::Error()
				<< "Bad memory assignment for type-constrained any: from "
				<< GetToken() << " to " << other.GetToken());
		}

		// Overwrite everything except the type-constraint						
		mRaw = other.mRaw;
		mCount = other.mCount;
		mReserved = other.mReserved;
		mState = other.mState.mState | DataState::Typed;
		other.ResetInner();
		return *this;
	}

	/// Shallow copy operator																	
	TEMPLATE()
	TAny<T>& TAny<T>::operator = (const Block& other) {
		return TAny<T>::operator = (Any {other});
	}

	/// Shallow copy operator																	
	TEMPLATE()
	TAny<T>& TAny<T>::operator = (Block&& other) {
		return TAny<T>::operator = (Any {Forward<Block>(other)});
	}

	/// Assign by shallow-copying some value different from Any or Block			
	///	@param value - the value to copy													
	TEMPLATE()
	TAny<T>& TAny<T>::operator = (const T& value) requires (TAny<T>::NotCustom) {
		return TAny<T>::operator = (TAny<T> { value });
	}

	TEMPLATE()
	TAny<T>& TAny<T>::operator = (T& value) requires (TAny<T>::NotCustom) {
		return TAny<T>::operator = (const_cast<const T&>(value));
	}

	/// Assign by moving some value different from Any or Block						
	///	@param value - the value to move													
	TEMPLATE()
	TAny<T>& TAny<T>::operator = (T&& value) requires (TAny<T>::NotCustom) {
		return TAny<T>::operator = (TAny<T> { Forward<T>(value) });
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

		if (GetReferences() == 1) {
			// Only one use - just destroy elements and reset count,			
			// reusing the allocation for later										
			Block::CallDestructors();
			Block::ClearInner();
		}
		else {
			// We're forced to reset the memory, because it's in use			
			// Keep the type and state, though										
			const auto state = GetUnconstrainedState();
			Reset();
			mState.mState |= state.mState;
		}
	}

	/// Reset the container, destroying all elements, and deallocating			
	TEMPLATE()
	void TAny<T>::Reset() {
		Block::Free();
		mRaw = nullptr;
		mCount = mReserved = 0;
		mState.mState = DataState::Typed;
	}

	/// Clone the templated container														
	TEMPLATE()
	TAny<T> TAny<T>::Clone() const {
		return {Any::Clone()};
	}

	/// Return the typed raw data (const)													
	TEMPLATE()
	const T* TAny<T>::GetRaw() const noexcept {
		return Any::GetRawAs<T>();
	}

	/// Return the typed raw data																
	TEMPLATE()
	T* TAny<T>::GetRaw() noexcept {
		return Any::GetRawAs<T>();
	}

	/// Get an element in the way you want (const, unsafe)							
	/// This is a statically optimized variant of Block::Get							
	TEMPLATE()
	template<ReflectedData K>
	decltype(auto) TAny<T>::Get(const Offset& index) const noexcept {
		const T& element = GetRaw()[index];
		if constexpr (Dense<T> && Dense<K>)
			// Dense -> Dense (returning a reference)								
			return static_cast<const Decay<K>&>(element);
		else if constexpr (Sparse<T> && Dense<K>)
			// Sparse -> Dense (returning a reference)							
			return static_cast<const Decay<K>&>(*element);
		else if constexpr (Dense<T> && Sparse<K>)
			// Dense -> Sparse (returning a pointer)								
			return static_cast<const Decay<K>*>(&element);
		else
			// Sparse -> Sparse (returning a reference to pointer)			
			return static_cast<const Decay<K>* const&>(element);
	}

	/// Get an element in the way you want (unsafe)										
	/// This is a statically optimized variant of Block::Get							
	TEMPLATE()
	template<ReflectedData K>
	decltype(auto) TAny<T>::Get(const Offset& index) noexcept {
		T& element = GetRaw()[index];
		if constexpr (Dense<T> && Dense<K>)
			// Dense -> Dense (returning a reference)								
			return static_cast<Decay<K>&>(element);
		else if constexpr (Sparse<T> && Dense<K>)
			// Sparse -> Dense (returning a reference)							
			return static_cast<Decay<K>&>(*element);
		else if constexpr (Dense<T> && Sparse<K>)
			// Dense -> Sparse (returning a pointer)								
			return static_cast<Decay<K>*>(&element);
		else
			// Sparse -> Sparse (returning a reference to pointer)			
			return static_cast<Decay<K>*&>(element);
	}

	/// Access typed dense elements via a simple index (unsafe)						
	///	@param idx - the index to get														
	///	@return a reference to the element												
	TEMPLATE()
	decltype(auto) TAny<T>::operator [] (const Offset& index) noexcept requires Dense<T> {
		return Get<T>(index);
	}

	TEMPLATE()
	decltype(auto) TAny<T>::operator [] (const Offset& index) const noexcept requires Dense<T> {
		return Get<T>(index);
	}

	/// Access typed dense elements via a complex index (const, safe)				
	///	@param idx - the index to get														
	///	@return a constant reference to the element									
	TEMPLATE()
	decltype(auto) TAny<T>::operator [] (const Index& index) requires Dense<T> {
		return Get<T>(Block::ConstrainMore<T>(index).GetOffset());
	}

	TEMPLATE()
	decltype(auto) TAny<T>::operator [] (const Index& index) const requires Dense<T> {
		return const_cast<TAny<T>&>(*this).operator [] (index);
	}

	/// Access typed sparse elements via a simple index (unsafe)					
	/// Will wrap pointer in a SparseElement wrapper, in order to reference		
	///	@param idx - the index to get														
	///	@return a reference to the element												
	TEMPLATE()
	typename TAny<T>::SparseElement TAny<T>::operator [] (const Offset& index) noexcept requires Sparse<T> {
		return {Get<T>(index)};
	}

	TEMPLATE()
	decltype(auto) TAny<T>::operator [] (const Offset& index) const noexcept requires Sparse<T> {
		return Get<T>(index);
	}

	/// Access typed sparse elements via a complex index (const)					
	/// Will wrap pointer in a SparseElement wrapper, in order to reference		
	///	@param idx - the index to get														
	///	@return a constant reference to the element									
	TEMPLATE()
	typename TAny<T>::SparseElement TAny<T>::operator [] (const Index& index) requires Sparse<T> {
		return {Get<T>(Block::ConstrainMore<T>(index).GetOffset())};
	}

	TEMPLATE()
	decltype(auto) TAny<T>::operator [] (const Index& index) const requires Sparse<T> {
		return Get<T>(Block::ConstrainMore<T>(index).GetOffset());
	}

	/// Access last element (unsafe)															
	///	@return a reference to the last element										
	TEMPLATE()
	decltype(auto) TAny<T>::Last() {
		return Get<T>(mCount - 1);
	}

	/// Access last element (const, unsafe)												
	///	@return a constant reference to the last element							
	TEMPLATE()
	decltype(auto) TAny<T>::Last() const {
		return Get<T>(mCount - 1);
	}

	/// Check if the contained type is a pointer											
	///	@return true if container contains pointers									
	TEMPLATE()
	constexpr bool TAny<T>::IsSparse() const noexcept {
		return Sparse<T>; 
	}

	/// Check if the contained type is not a pointer									
	///	@return true if container contains sequential data							
	TEMPLATE()
	constexpr bool TAny<T>::IsDense() const noexcept {
		return Dense<T>;
	}

	/// Get the size of a single contained element, in bytes							
	///	@return the number of bytes a single element contains						
	TEMPLATE()
	constexpr Stride TAny<T>::GetStride() const noexcept {
		return sizeof(T); 
	}

	/// Insert an item by move-construction												
	///	@param item - the item to move													
	///	@param idx - the place where to emplace the item							
	///	@return 1 if successful																
	TEMPLATE()
	Count TAny<T>::Emplace(T&& item, const Index& idx) {
		return Any::Emplace<T, false>(Forward<T>(item), idx);
	}

	/// Insert by copy-construction															
	///	@param items - an array of items to insert									
	///	@param count - the number of items to insert									
	///	@param idx - the place where to insert the items							
	///	@return number of inserted items													
	TEMPLATE()
	Count TAny<T>::Insert(const T* items, const Count count, const Index& idx) {
		return Any::Insert<T, false>(items, count, idx);
	}

	/// Push data at the back by copy-construction										
	///	@param other - the item to insert												
	///	@return a reference to this container for chaining							
	TEMPLATE()
	TAny<T>& TAny<T>::operator << (const T& other) {
		Any::Insert<T, false>(&other, 1, Index::Back);
		return *this;
	}

	/// Push data at the back by move-construction										
	///	@param other - the item to move													
	///	@return a reference to this container for chaining							
	TEMPLATE()
	TAny<T>& TAny<T>::operator << (T&& other) {
		Any::Emplace<T, false>(Forward<T>(other), Index::Back);
		return *this;
	}

	/// Push data at the front by copy-construction										
	///	@param other - the item to insert												
	///	@return a reference to this container for chaining							
	TEMPLATE()
	TAny<T>& TAny<T>::operator >> (const T& other) {
		Any::Insert<T, false>(&other, 1, Index::Front);
		return *this;
	}

	/// Push data at the front by move-construction										
	///	@param other - the item to move													
	///	@return a reference to this container for chaining							
	TEMPLATE()
	TAny<T>& TAny<T>::operator >> (T&& other) {
		Any::Emplace<T, false>(Forward<T>(other), Index::Front);
		return *this;
	}

	/// Merge anything compatible to container											
	/// By merging we mean anything that is not found is pushed						
	///	@param items - the items to find and push										
	///	@param count - number of items to push											
	///	@param idx - the place to insert them											
	///	@return the number of inserted items											
	TEMPLATE()
	Count TAny<T>::Merge(const T* items, const Count count, const Index& idx) {
		return Any::Merge<T, false>(items, count, idx);
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

	/// Find element(s) position inside container										
	TEMPLATE()
	Index TAny<T>::Find(MakeConst<T> item, const Index& idx) const {
		return Any::Find(pcVal(item), idx);
	}

	/// Remove matching items																	
	TEMPLATE()
	Count TAny<T>::Remove(MakeConst<T> item, const Index& idx) {
		return Any::Remove(pcPtr(item), 1, idx);
	}

	/// Sort the pack																				
	TEMPLATE()
	void TAny<T>::Sort(const Index& first) {
		if constexpr (Sortable<T>)
			Any::Sort<T>(first);
		else LANGULUS_ASSERT("Can't sort container");
	}

	/// Pick a region																				
	TEMPLATE()
	TAny<T> TAny<T>::Crop(const Offset& start, const Count& count) const {
		return Any::Crop(start, count);
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

} // namespace Langulus::Anyness

#undef TEMPLATE
