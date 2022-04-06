//#define PC_UNOWNED_POINTER_WARNING

namespace Langulus::Anyness
{

	#define TEMPLATE1 template<class T>
	#define TEMPLATE template<class T, bool DR>

	/// Initialize with a value																
	///	@param value - value to copy (no referencing shall occur if sparse)	
	TEMPLATE1 constexpr TOwned<T>::TOwned(const T& value) noexcept
		: mValue {value} { }

	/// Move ownership, resetting source value to default								
	///	@param value - value to move														
	TEMPLATE1 constexpr TOwned<T>::TOwned(TOwned&& value) noexcept
		: mValue {value.mValue} {
		value.mValue = {};
	}

	/// Default pointer initialization														
	TEMPLATE TPointer<T, DR>::TPointer() {
		if (!sMeta)
			sMeta = MetaData::Of<SparseT>();
	}

	/// Copy a shared pointer																	
	///	@param other - pointer to reference												
	TEMPLATE TPointer<T, DR>::TPointer(const TPointer& other)
		: BASE {other} {
		if (BASE::mValue) {
			PCMEMORY.Reference(sMeta, BASE::mValue, 1);
			if constexpr (DR && Referencable<T>)
				BASE::mValue->Reference(1);
		}
	}

	/// Move a shared pointer																	
	///	@param other - pointer to move													
	TEMPLATE TPointer<T, DR>::TPointer(TPointer&& other) noexcept
		: BASE {pcForward<BASE>(other)} { }

	/// Reference a pointer																		
	///	@param ptr - pointer to reference												
	TEMPLATE TPointer<T, DR>::TPointer(SparseT ptr)
		: BASE {ptr} {
		if (!sMeta)
			sMeta = MetaData::Of<SparseT>();

		#ifdef PC_UNOWNED_POINTER_WARNING
			if (ptr && !PCMEMORY.CheckJurisdiction(sMeta, ptr))
				pcLogFuncWarning << "Given an unowned pointer!";
		#endif

		if (BASE::mValue) {
			PCMEMORY.Reference(sMeta, BASE::mValue, 1);
			if constexpr (DR && Referencable<T>)
				BASE::mValue->Reference(1);
		}
	}

	/// Shared pointer destruction															
	TEMPLATE TPointer<T, DR>::~TPointer() {
		Reset();
	}

	/// Create a new instance by moving														
	/// Resulting pointer created that way has exactly one reference				
	///	@param initializer - instance to move											
	///	@return the pointer																	
	TEMPLATE TPointer<T, DR> TPointer<T, DR>::Create(pcDecay<T>&& initializer) requires MoveConstructible<pcDecay<T>> {
		TPointer<T> pointer;
		pointer.mValue = new pcDecay<T>{ pcForward<pcDecay<T>>(initializer) };
		return pointer;
	}

	/// Create a new instance by copying													
	/// Resulting pointer created that way has exactly one reference				
	///	@param initializer - instance to copy											
	///	@return the pointer																	
	TEMPLATE TPointer<T, DR> TPointer<T, DR>::Create(const pcDecay<T>& initializer) requires CopyConstructible<pcDecay<T>> {
		TPointer<T> pointer;
		pointer.mValue = new pcDecay<T>{ initializer };
		return pointer;
	}

	/// Create a default new instance														
	/// Resulting pointer created that way has exactly one reference				
	///	@return the pointer																	
	TEMPLATE TPointer<T, DR> TPointer<T, DR>::Create() requires DefaultConstructible<pcDecay<T>> {
		TPointer<T> pointer;
		pointer.mValue = new pcDecay<T>;
		return pointer;
	}

	/// Create a new instance of T by providing constructor arguments				
	///	@tparam ...ARGS - the deduced arguments										
	///	@param arguments - the arguments													
	///	@return the new instance															
	TEMPLATE template<typename... ARGS>
	TPointer<T, DR> TPointer<T, DR>::New(ARGS&&... arguments) {
		TPointer<T> pointer;
		pointer.mValue = new pcDecay<T>{ pcForward<ARGS>(arguments)... };
		return pointer;
	}

	/// Reset the value																			
	TEMPLATE1 void TOwned<T>::Reset() noexcept {
		mValue = {};
	}

	/// Reset the pointer																		
	TEMPLATE void TPointer<T, DR>::Reset() {
		if (BASE::mValue) {
			if constexpr (DR && Referencable<T>)
				BASE::mValue->Reference(-1);

			// This will call destructor on the pointer first					
			// and then the data behind it, if references reach zero			
			// It will zero the mValue for us										
			GetBlock().CallDestructors();
		}
	}

	/// Copy a shared pointer																	
	///	@param other - pointer to reference												
	TEMPLATE TPointer<T, DR>& TPointer<T, DR>::operator = (const TPointer<T, DR>& other) {
		Reset();
		new (this) TPointer<T, DR> { other };
		return *this;
	}

	/// Move a shared pointer																	
	///	@param other - pointer to move													
	TEMPLATE TPointer<T, DR>& TPointer<T, DR>::operator = (TPointer<T, DR>&& other) {
		Reset();
		new (this) TPointer<T, DR> { pcForward<TPointer<T, DR>>(other) };
		return *this;
	}

	/// Reference a raw pointer																
	///	@param ptr - pointer to reference												
	TEMPLATE TPointer<T, DR>& TPointer<T, DR>::operator = (SparseT ptr) {
		#ifdef PC_UNOWNED_POINTER_WARNING
			if (ptr && !PCMEMORY.CheckJurisdiction(sMeta, ptr))
				pcLogFuncWarning << "Given an unowned pointer!";
		#endif
		Reset();
		new (this) TPointer<T, DR> { ptr };
		return *this;
	}

	/// Move-assign a value																		
	///	@param value - the new value														
	TEMPLATE1 constexpr TOwned<T>& TOwned<T>::operator = (TOwned&& value) noexcept {
		mValue = value.mValue;
		value.mValue = {};
		return *this;
	}

	/// Overwrite the value																		
	///	@param value - the new value														
	TEMPLATE1 constexpr TOwned<T>& TOwned<T>::operator = (const T& value) noexcept {
		mValue = value;
		return *this;
	}

	/// Attempt to cast any pointer to the contained pointer							
	///	@param ptr - pointer to reference												
	TEMPLATE template<Sparse ANY_POINTER>
	TPointer<T, DR>& TPointer<T, DR>::operator = (ANY_POINTER ptr) {
		static_assert(Constant<T> || !Constant<ANY_POINTER>,
			"Can't assign a constant pointer to a non-constant pointer wrapper");

		#ifdef PC_UNOWNED_POINTER_WARNING
			if (ptr && !PCMEMORY.CheckJurisdiction(sMeta, ptr))
				pcLogFuncWarning << "Given an unowned pointer!";
		#endif
		Reset();
		new (this) TPointer<T, DR> {
			dynamic_cast<Conditional<Constant<ANY_POINTER>, const T*, T*>>(ptr)
		};
		return *this;
	}

	/// Attempt to cast any pointer to the contained pointer							
	///	@param ptr - pointer to reference												
	TEMPLATE template<class ANY_POINTER>
	TPointer<T, DR>& TPointer<T, DR>::operator = (const TPointer<ANY_POINTER, DR>& ptr) {
		static_assert(Constant<T> || !Constant<ANY_POINTER>,
			"Can't assign a constant pointer to a non-constant pointer wrapper");

		Reset();
		new (this) TPointer<T, DR> {
			dynamic_cast<Conditional<Constant<ANY_POINTER>, const T*, T*>>(ptr.Get())
		};
		return *this;
	}

	/// Get the pointer																			
	///	@return the contained pointer														
	TEMPLATE1 decltype(auto) TOwned<T>::Get() const noexcept {
		return mValue;
	}
	TEMPLATE1 decltype(auto) TOwned<T>::Get() noexcept {
		return mValue;
	}

	/// Get the hash of the contained type													
	///	@return the hash of the container type											
	TEMPLATE1 Hash TOwned<T>::GetHash() const requires Hashable<T> {
		if (!mValue)
			return {};
		return mValue->GetHash();
	}

	/// Perform a dynamic cast on the pointer												
	///	@tparam D - the desired type to cast to										
	///	@return the result of a dynamic_cast to the specified type				
	TEMPLATE1 template<class D>
	auto TOwned<T>::As() const noexcept requires Sparse<T> {
		using RESOLVED = Conditional<Constant<T>, const pcDecay<D>*, pcDecay<D>*>;
		return dynamic_cast<RESOLVED>(mValue);
	}

	/// Access the pointer																		
	///	@attention does not check if contained pointer is valid					
	///	@return the contained constant raw pointer									
	TEMPLATE1 auto TOwned<T>::operator -> () const SAFE_NOEXCEPT() requires Sparse<T> {
		SAFETY(if (!mValue)
			throw Except::BadAccess("Invalid pointer"));
		return mValue;
	}

	TEMPLATE1 auto TOwned<T>::operator -> () SAFE_NOEXCEPT() requires Sparse<T> {
		SAFETY(if (!mValue)
			throw Except::BadAccess("Invalid pointer"));
		return mValue;
	}

	/// Access the dereferenced pointer (const)											
	///	@attention does not check if contained pointer is valid					
	///	@return the contained constant dereferenced pointer						
	TEMPLATE1 decltype(auto) TOwned<T>::operator * () const SAFE_NOEXCEPT() requires Sparse<T> {
		SAFETY(if (!mValue)
			throw Except::BadAccess("Invalid pointer"));
		return *mValue;
	}

	TEMPLATE1 decltype(auto) TOwned<T>::operator * () SAFE_NOEXCEPT() requires Sparse<T> {
		SAFETY(if (!mValue)
			throw Except::BadAccess("Invalid pointer"));
		return *mValue;
	}

	/// Explicit boolean cast																	
	///	@return true if value differs from default value							
	TEMPLATE1
	TOwned<T>::operator bool() const noexcept {
		return mValue != T {};
	}

	/// Cast to a constant pointer, if mutable											
	///	@return the constant equivalent to this pointer								
	TEMPLATE1
	TOwned<T>::operator const T&() const noexcept {
		return mValue;
	}

	TEMPLATE1
	TOwned<T>::operator T&() noexcept {
		return mValue;
	}

	/// Cast to a constant pointer, if mutable											
	///	@return the constant equivalent to this pointer								
	TEMPLATE
	TPointer<T, DR>::operator TPointer<const T, DR>() const noexcept requires Mutable<T> {
		return TPointer<const T, DR> { BASE::mValue };
	}

	/// Compare pointers for equality														
	///	@param rhs - the right pointer													
	///	@return true if pointers match													
	TEMPLATE1 bool TOwned<T>::operator == (const TOwned<T>& rhs) const noexcept {
		return mValue == rhs.mValue;
	}

	/// Compare pointers for inequality														
	///	@param rhs - the right pointer													
	///	@return true if pointers do not match											
	TEMPLATE1 bool TOwned<T>::operator != (const TOwned<T>& rhs) const noexcept {
		return mValue != rhs.mValue;
	}

	/// Compare pointers for equality														
	///	@param rhs - the right pointer													
	///	@return true if pointers match													
	TEMPLATE1 bool TOwned<T>::operator == (const T& rhs) const noexcept {
		return mValue == rhs;
	}

	/// Compare pointers for inequality														
	///	@param rhs - the right pointer													
	///	@return true if pointers do not match											
	TEMPLATE1 bool TOwned<T>::operator != (const T& rhs) const noexcept {
		return mValue != rhs;
	}

	/// Compare pointers for equality														
	///	@param lhs - the left pointer														
	///	@param rhs - the right pointer													
	///	@return true if pointers match													
	TEMPLATE bool operator == (const pcDecay<T>* lhs, const TOwned<T>& rhs) noexcept {
		return lhs == rhs.Get();
	}

	/// Compare pointers for inequality														
	///	@param lhs - the left pointer														
	///	@param rhs - the right pointer													
	///	@return true if pointers match													
	TEMPLATE bool operator != (const pcDecay<T>* lhs, const TOwned<T>& rhs) noexcept {
		return lhs != rhs.Get();
	}

	/// Compare pointer for nullptr															
	///	@param rhs - the right pointer													
	///	@return true if pointers match													
	TEMPLATE1 bool TOwned<T>::operator == (std::nullptr_t) const noexcept {
		return mValue == nullptr;
	}

	/// Compare pointers for not nullptr													
	///	@param rhs - the right pointer													
	///	@return true if pointers do not match											
	TEMPLATE1 bool TOwned<T>::operator != (std::nullptr_t) const noexcept {
		return mValue != nullptr;
	}

	/// Compare pointer for nullptr															
	///	@param lhs - a nullptr																
	///	@param rhs - the right pointer													
	///	@return true if pointers match													
	TEMPLATE bool operator == (std::nullptr_t, const TOwned<T>& rhs) noexcept {
		return nullptr == rhs.Get();
	}

	/// Compare pointer for not nullptr														
	///	@param lhs - a nullptr																
	///	@param rhs - the right pointer													
	///	@return true if pointers match													
	TEMPLATE bool operator != (std::nullptr_t, const TOwned<T>& rhs) noexcept {
		return nullptr != rhs.Get();
	}

	/// Get the contained type																	
	///	@return the meta data of the data												
	TEMPLATE1 DMeta TOwned<T>::GetMeta() const {
		return MetaData::Of<T>();
	}

	TEMPLATE DMeta TPointer<T, DR>::GetMeta() const {
		return sMeta;
	}

	/// Get the block of the contained pointer											
	/// Can be invoked by the reflected resolver											
	///	@return the pointer, interfaced via a memory block							
	TEMPLATE1 Block TOwned<T>::GetBlock() const {
		return { DState::Static + DState::Typed, GetMeta(), 1, &mValue};
	}

	TEMPLATE Block TPointer<T, DR>::GetBlock() const {
		return { DState::Static + DState::Typed, sMeta, 1, &(BASE::mValue) };
	}

	/// Check if contained pointer is in jurisdiction									
	///	@return true if data is inside the managed memory							
	TEMPLATE1 bool TOwned<T>::CheckJurisdiction() const {
		return PCMEMORY.CheckJurisdiction(GetMeta(), mValue);
	}

	TEMPLATE bool TPointer<T, DR>::CheckJurisdiction() const {
		return PCMEMORY.CheckJurisdiction(sMeta, BASE::mValue);
	}

	/// Check memory references																
	///	@return the number of references													
	TEMPLATE1 RefCount TOwned<T>::GetBlockReferences() const {
		return PCMEMORY.GetReferences(GetMeta(), mValue);
	}

	TEMPLATE RefCount TPointer<T, DR>::GetBlockReferences() const {
		return PCMEMORY.GetReferences(sMeta, BASE::mValue);
	}

	#undef TEMPLATE

} // namespace Langulus::Anyness