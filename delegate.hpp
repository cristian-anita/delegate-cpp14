// MIT License: https://spdx.org/licenses/MIT.html
// 
// Copyright (c) 2026 Cristian ANITA <cristian_anita@yahoo.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
// OR OTHER DEALINGS IN THE SOFTWARE.


#ifndef BRICXX_DELEGATE_HPP_INCLUDED_
#define BRICXX_DELEGATE_HPP_INCLUDED_


// /Zc:__cplusplus; please see:
// - MSVC now correctly reports __cplusplus: https://devblogs.microsoft.com/cppblog/msvc-now-correctly-reports-__cplusplus
// - /Zc:__cplusplus (Enable updated __cplusplus macro): https://learn.microsoft.com/en-us/cpp/build/reference/zc-cplusplus
#ifdef _MSC_VER
#	if (_MSC_VER < 1914)
#		error "msvc >= 2017 v15.7 required!!! (for /Zc:__cplusplus)"
#	endif
#endif

#if (__cplusplus < 201402L)
#	error "C++ >= 14 required!!!"
#endif


#ifdef _MSC_VER
#pragma warning(push)
// C4646: function declared with __declspec(noreturn) has non-void return type.
// DelegateBase::NullStub() has (possibly non-void) return type (it must conform to stub signature)
// but it actually always throws BadDelegateCall.
#pragma warning(disable: 4646)
#endif


#include <exception>
#include <type_traits>
#include <utility>
#include <memory>


// TODO: If Delegate is used in a shared library define BRICXX_DELEGATE_API to that library export/import macro.
#define BRICXX_DELEGATE_API


namespace bricxx {


//***************************************************************************************************************************
/// \brief A non-owning, const-correct and equality comparable callable wrapper. Primary template, not defined.
/// \details \c Delegate:
/// - is copyable, small (two pointers) and fast (no dynamic memory allocation, no virtual functions,
/// all functions are inline (except stubs because their address is needed),
/// delegate call involves only two extra calls (operator()() and stub function)).
/// - supports (non-member) functions, methods (member functions) and functors/lambda.
/// - is a single header library so it is easy to integrate in any project.
///
/// \attention Because \c Delegate is non-owning special care is needed to ensure that the binded target outlive it.
/// \remarks
/// - Two delegates are considered equal if they are not binded or binded to the same target.
/// - Equality comparison and \c Delegate::IsBindedTo() allows delegates to be easily used as callbacks that can be
/// registered/unregistered (\c ::std::function_ref is not equality comparable).
/// - The solutions presented in \b references directory are not const-correct
/// (they have the same problem as \c ::std::function).
/// \sa
/// - <a href="http://www.tutok.sk/fastgl/callback.html">Rich Hickey: Callbacks in C++ Using Template Functors</a>
/// - Bert Peers: Callbacks in C++
/// - Don Clugston, CodeProject: Member Function Pointers and the Fastest Possible C++ Delegates
/// - Sergey Ryazanov, CodeProject: The Impossibly Fast C++ Delegates
/// - Sergey Alexandrovich Kryukov, CodeProject: The Impossibly Fast C++ Delegates, Fixed
/// - Aleksei Trunov, CodeProject: Yet Another Generalized Functors Implementation in C++
/// - JaeWook Choi, CodeProject: Fast C++ Delegate
/// - <a href="https://rodusek.com/posts/2021/02/24/creating-a-fast-and-efficient-delegate-type-part-1/">Matthew Rodusek: Creating a Fast and Efficient Delegate Type (Part 1)</a>
/// - <a href="https://rodusek.com/posts/2021/02/26/creating-a-fast-and-efficient-delegate-type-part-2/">Matthew Rodusek: Creating a Fast and Efficient Delegate Type (Part 2)</a>
/// - <a href="https://rodusek.com/posts/2021/02/26/creating-a-fast-and-efficient-delegate-type-part-3/">Matthew Rodusek: Creating a Fast and Efficient Delegate Type (Part 3)</a>
/// - <a href="https://github.com/bitwizeshift/Delegate">Matthew Rodusek: Unbelievably Fast Delegate</a>
/// - <a href="https://codereview.stackexchange.com/questions/14730/impossibly-fast-delegate-in-c11">user1095108: Impossibly fast delegate in C++11</a>
/// - <a href="https://github.com/jamboree/CxxFunctionBenchmark/blob/master/delegate.hpp">user1095108: delegate.hpp</a>
/// - \b references directory in this project that contains the above.
///
/// <b>Minimal introduction/examples</b> (please see tests for more details):
/// \code
/// // (non-member) function
/// Delegate<int (int)> functionDelegate;
/// functionDelegate.Bind<&::islower>();
/// if (!functionDelegate.IsBindedTo<&::islower>()) throw runtime_error("function test failed");
/// if (!functionDelegate('a')) throw runtime_error("function test failed");
/// 
/// // lambda/functor
/// unsigned int numCalls = 0U;
/// auto lambda = [&numCalls](int x, int y) mutable -> bool {
/// 	++numCalls;
/// 	return x > y;
/// };
/// Delegate<bool (int, int)> lambdaDelegate;
/// lambdaDelegate.Bind(lambda);
/// if (!lambdaDelegate.IsBindedTo(lambda)) throw runtime_error("lambda test failed");
/// if (!lambdaDelegate(2, 1)) throw runtime_error("lambda test failed");
/// if (numCalls != 1U) throw runtime_error("lambda test failed");
/// 
/// // method (member function)
/// string hello = "Hello, World!";
/// Delegate<void ()> methodDelegate;
/// methodDelegate.Bind<string, &string::clear>(hello);
/// methodDelegate();
/// if (!hello.empty()) throw runtime_error("method test failed");
/// 
/// // comparison
/// Delegate<int (int)> functionDelegate1;
/// functionDelegate1.Bind<&::isupper>();
/// if (functionDelegate == functionDelegate1) throw runtime_error("comparison test failed");
/// functionDelegate1.Bind<&::islower>();
/// if (functionDelegate != functionDelegate1) throw runtime_error("comparison test failed");
/// 
/// // const-correctness
/// Delegate<void () const> constDelegate;
/// //constDelegate.Bind<string, &string::clear>(hello);	// OK, does not compile
/// (void)constDelegate;
/// 
/// // unbind
/// functionDelegate.Unbind();
/// if (functionDelegate.IsBinded()) throw runtime_error("unbind test failed");
/// 
/// // BadDelegateCall
/// bool caught = false;
/// try {
/// 	functionDelegate('a');
/// }
/// catch (const BadDelegateCall&) {
/// 	caught = true;
/// }
/// if (!caught) throw runtime_error("BadDelegateCall test failed");
/// \endcode
template <typename TSignature>
class Delegate;




//***************************************************************************************************************************
/// Exception thrown by \c Delegate::operator()() if the delegate is not binded.
class BRICXX_DELEGATE_API BadDelegateCall final: public ::std::exception {
public:
	const char* what() const noexcept override;
};




//***************************************************************************************************************************
/// Base class for \c Delegate partial specializations.
template <typename TRetVal, typename... TParams>
class DelegateBase {
public:
	constexpr bool IsBinded() const noexcept;
	explicit constexpr operator bool() const noexcept;	///< Same as \c IsBinded()
	
	constexpr void Unbind() noexcept;
	constexpr void Reset() noexcept;	///< Same as \c Unbind()
protected:
	using ErasedObjectType = void;
	using StubFnPtr = TRetVal (*)(ErasedObjectType* pTypeErasedObject, TParams... params);
	
	
	/// \name Constructors.
	/// @{
	constexpr DelegateBase() noexcept;	///< Creates a delegate that is not binded.
	constexpr DelegateBase(const DelegateBase& other) noexcept;
	/// @}
	
	~DelegateBase() noexcept;	///< \remark Not virtual.
	
	constexpr DelegateBase& operator=(const DelegateBase& other) noexcept;
	
	
	/// \name Equality operators.
	/// @{
	constexpr bool operator==(const DelegateBase& other) const noexcept;
	constexpr bool operator!=(const DelegateBase& other) const noexcept;
	/// @}
	
	
	/// \throws BadDelegateCall This stub is used when the delegate is not binded to a "real" target.
	[[noreturn]] static TRetVal NullStub(ErasedObjectType* pTypeErasedObject, TParams... params);
	
	
	StubFnPtr pStub_ = &NullStub;
	ErasedObjectType* pTypeErasedObject_ = nullptr;
};




//***************************************************************************************************************************
/// \c Delegate partial specialization for non-const.
template <typename TRetVal, typename... TParams>
class Delegate<TRetVal (TParams...)> final: private DelegateBase<TRetVal, TParams...> {
public:
	using DelegateBase<TRetVal, TParams...>::IsBinded;
	using DelegateBase<TRetVal, TParams...>::operator bool;
	
	using DelegateBase<TRetVal, TParams...>::Unbind;
	using DelegateBase<TRetVal, TParams...>::Reset;
	
	
	/// \name Constructors.
	/// @{
	constexpr Delegate() noexcept;	///< Creates a delegate that is not binded.
	constexpr Delegate(const Delegate& other) noexcept;
	/// @}
	
	~Delegate() noexcept;	///< \remark Not virtual.
	
	constexpr Delegate& operator=(const Delegate& other) noexcept;
	
	
	/// \name Equality operators.
	/// @{
	constexpr bool operator==(const Delegate& other) const noexcept;
	constexpr bool operator!=(const Delegate& other) const noexcept;
	/// @}
	
	
	/// \name Binders.
	/// Binds the delegate to a target.
	/// @{
	
	template <TRetVal (*TFunction)(TParams...)>
	constexpr void Bind() noexcept;	///< Binds the delegate to a (non-member) function.
	
	template <class TFunctor,
		typename = typename ::std::enable_if<
			::std::is_class<TFunctor>::value
		>::type
	>
	void Bind(TFunctor& functor) noexcept;	///< Binds the delegate to a \p functor / lambda (\c TFunctor is auto-deduced).
	
	template <class TClass, TRetVal (TClass::*TMethod)(TParams...)>
	void Bind(TClass& object) noexcept;	///< Binds the delegate to an \p object + method (member function).
	
	/// @}
	
	
	/// \name Binder target testers (convenience methods).
	/// Test if the delegate is binded to the given target.
	/// @{
	
	template <TRetVal (*TFunction)(TParams...)>
	constexpr bool IsBindedTo() const noexcept;	///< Test if the delegate is binded to the given (non-member) function.
	
	/// Test if the delegate is binded to the given \p functor / lambda (\c TFunctor is auto-deduced).
	template <class TFunctor,
		typename = typename ::std::enable_if<
			::std::is_class<TFunctor>::value
		>::type
	>
	bool IsBindedTo(TFunctor& functor) const noexcept;
	
	/// Test if the delegate is binded to the given \p object + method (member function).
	template <class TClass, TRetVal (TClass::*TMethod)(TParams...)>
	bool IsBindedTo(TClass& object) const noexcept;
	
	/// @}
	
	
	/// Invokes the stub function with the given parameters.
	/// \remark Perfect forwarding and converting parameter pack to stub function
	/// (\c UParams is a deduced type, \c TParams is not).
	/// \throws BadDelegateCall if the delegate is not binded.
	template <typename... UParams>
	TRetVal operator()(UParams&&... params);
	
	
	/// \name Create and bind (convenience methods).
	/// Creates a delegate and binds it to the given target.
	/// @{
	
	template <TRetVal (*TFunction)(TParams...)>
	static constexpr Delegate CreateAndBind() noexcept;	///< Creates a delegate and binds it to the given (non-member) function.
	
	/// Creates a delegate and binds it to the given \p functor / lambda (\c TFunctor is auto-deduced).
	template <class TFunctor,
		typename = typename ::std::enable_if<
			::std::is_class<TFunctor>::value
		>::type
	>
	static Delegate CreateAndBind(TFunctor& functor) noexcept;
	
	/// Creates a delegate and binds it to the given \p object + method (member function).
	template <class TClass, TRetVal (TClass::*TMethod)(TParams...)>
	static Delegate CreateAndBind(TClass& object) noexcept;
	
	/// @}
private:
	using typename DelegateBase<TRetVal, TParams...>::ErasedObjectType;
	using typename DelegateBase<TRetVal, TParams...>::StubFnPtr;
	
	
	/// \name Stubs.
	/// @{
	
	template <TRetVal (*TFunction)(TParams...)>
	static TRetVal FunctionStub(ErasedObjectType* pTypeErasedObject, TParams... /* params */);
	
	template <class TFunctor>
	static TRetVal FunctorStub(ErasedObjectType* pTypeErasedObject, TParams... params);
	
	template <class TClass, TRetVal (TClass::*TMethod)(TParams...)>
	static TRetVal MethodStub(ErasedObjectType* pTypeErasedObject, TParams... params);
	
	/// @}
};




//***************************************************************************************************************************
/// \c Delegate partial specialization for const.
template <typename TRetVal, typename... TParams>
class Delegate<TRetVal (TParams...) const> final: private DelegateBase<TRetVal, TParams...> {
public:
	using DelegateBase<TRetVal, TParams...>::IsBinded;
	using DelegateBase<TRetVal, TParams...>::operator bool;
	
	using DelegateBase<TRetVal, TParams...>::Unbind;
	using DelegateBase<TRetVal, TParams...>::Reset;
	
	
	/// \name Constructors.
	/// @{
	constexpr Delegate() noexcept;	///< Creates a delegate that is not binded.
	constexpr Delegate(const Delegate& other) noexcept;
	/// @}
	
	~Delegate() noexcept;	///< \remark Not virtual.
	
	constexpr Delegate& operator=(const Delegate& other) noexcept;
	
	
	/// \name Equality operators.
	/// @{
	constexpr bool operator==(const Delegate& other) const noexcept;
	constexpr bool operator!=(const Delegate& other) const noexcept;
	/// @}
	
	
	/// \name Binders.
	/// Binds the delegate to a target.
	/// @{
	
	template <TRetVal (*TFunction)(TParams...)>
	constexpr void Bind() noexcept;	///< Binds the delegate to a (non-member) function.
	
	/// Binds the delegate to a \p functor / lambda (\c TFunctor is auto-deduced).
	template <class TFunctor,
		typename = typename ::std::enable_if<
			::std::is_class<TFunctor>::value
		>::type
	>
	void Bind(const TFunctor& functor) noexcept;
	
	template <class TClass, TRetVal (TClass::*TMethod)(TParams...) const>
	void Bind(const TClass& object) noexcept;	///< Binds the delegate to an \p object + method (member function).
	
	/// @}
	
	
	/// \name Binder target testers (convenience methods).
	/// Test if the delegate is binded to the given target.
	/// @{
	
	template <TRetVal (*TFunction)(TParams...)>
	constexpr bool IsBindedTo() const noexcept;	///< Test if the delegate is binded to the given (non-member) function.
	
	/// Test if the delegate is binded to the given \p functor / lambda (\c TFunctor is auto-deduced).
	template <class TFunctor,
		typename = typename ::std::enable_if<
			::std::is_class<TFunctor>::value
		>::type
	>
	bool IsBindedTo(const TFunctor& functor) const noexcept;
	
	/// Test if the delegate is binded to the given \p object + method (member function).
	template <class TClass, TRetVal (TClass::*TMethod)(TParams...) const>
	bool IsBindedTo(const TClass& object) const noexcept;
	
	/// @}
	
	
	/// Invokes the stub function with the given parameters.
	/// \remark Perfect forwarding and converting parameter pack to stub function
	/// (\c UParams is a deduced type, \c TParams is not).
	/// \throws BadDelegateCall if the delegate is not binded.
	template <typename... UParams>
	TRetVal operator()(UParams&&... params) const;
	
	
	/// \name Create and bind (convenience methods).
	/// Creates a delegate and binds it to the given target.
	/// @{
	
	template <TRetVal (*TFunction)(TParams...)>
	static constexpr Delegate CreateAndBind() noexcept;	///< Creates a delegate and binds it to the given (non-member) function.
	
	/// Creates a delegate and binds it to the given \p functor / lambda (\c TFunctor is auto-deduced).
	template <class TFunctor,
		typename = typename ::std::enable_if<
			::std::is_class<TFunctor>::value
		>::type
	>
	static Delegate CreateAndBind(const TFunctor& functor) noexcept;
	
	/// Creates a delegate and binds it to the given \p object + method (member function).
	template <class TClass, TRetVal (TClass::*TMethod)(TParams...) const>
	static Delegate CreateAndBind(const TClass& object) noexcept;
	
	/// @}
private:
	using typename DelegateBase<TRetVal, TParams...>::ErasedObjectType;
	using typename DelegateBase<TRetVal, TParams...>::StubFnPtr;
	
	
	/// \name Stubs.
	/// @{
	
	template <TRetVal (*TFunction)(TParams...)>
	static TRetVal FunctionStub(ErasedObjectType* pTypeErasedObject, TParams... /* params */);
	
	template <class TFunctor>
	static TRetVal FunctorStub(ErasedObjectType* pTypeErasedObject, TParams... params);
	
	template <class TClass, TRetVal (TClass::*TMethod)(TParams...) const>
	static TRetVal MethodStub(ErasedObjectType* pTypeErasedObject, TParams... params);
	
	/// @}
};




/// \cond IMPLEMENTATION


//***************************************************************************************************************************
// Inline functions / template implementation

inline const char* BadDelegateCall::what() const noexcept {
	return "Bad delegate call";
}




template <typename TRetVal, typename... TParams>
inline constexpr bool DelegateBase<TRetVal, TParams...>::IsBinded() const noexcept {
	return pStub_ != &NullStub;
}

template <typename TRetVal, typename... TParams>
inline constexpr DelegateBase<TRetVal, TParams...>::operator bool() const noexcept {
	return IsBinded();
}


template <typename TRetVal, typename... TParams>
inline constexpr void DelegateBase<TRetVal, TParams...>::Unbind() noexcept {
	pStub_ = &NullStub;
	pTypeErasedObject_ = nullptr;
}

template <typename TRetVal, typename... TParams>
inline constexpr void DelegateBase<TRetVal, TParams...>::Reset() noexcept {
	Unbind();
}


template <typename TRetVal, typename... TParams>
inline constexpr DelegateBase<TRetVal, TParams...>::DelegateBase() noexcept = default;

template <typename TRetVal, typename... TParams>
inline constexpr DelegateBase<TRetVal, TParams...>::DelegateBase(const DelegateBase& other) noexcept = default;


template <typename TRetVal, typename... TParams>
inline DelegateBase<TRetVal, TParams...>::~DelegateBase() noexcept = default;


template <typename TRetVal, typename... TParams>
inline constexpr DelegateBase<TRetVal, TParams...>& DelegateBase<TRetVal, TParams...>::operator=(
		const DelegateBase& other) noexcept = default;


template <typename TRetVal, typename... TParams>
inline constexpr bool DelegateBase<TRetVal, TParams...>::operator==(const DelegateBase& other) const noexcept {
	return (pStub_ == other.pStub_) && (pTypeErasedObject_ == other.pTypeErasedObject_);
}

template <typename TRetVal, typename... TParams>
inline constexpr bool DelegateBase<TRetVal, TParams...>::operator!=(const DelegateBase& other) const noexcept {
	return !(*this == other);
}


template <typename TRetVal, typename... TParams>
TRetVal DelegateBase<TRetVal, TParams...>::NullStub(ErasedObjectType* pTypeErasedObject, TParams... /* params */) {
	(void)pTypeErasedObject;
	
	throw BadDelegateCall();
}




template <typename TRetVal, typename... TParams>
inline constexpr Delegate<TRetVal (TParams...)>::Delegate() noexcept = default;

template <typename TRetVal, typename... TParams>
inline constexpr Delegate<TRetVal (TParams...)>::Delegate(const Delegate& other) noexcept = default;


template <typename TRetVal, typename... TParams>
inline Delegate<TRetVal (TParams...)>::~Delegate() noexcept = default;


template <typename TRetVal, typename... TParams>
inline constexpr Delegate<TRetVal (TParams...)>& Delegate<TRetVal (TParams...)>::operator=(const Delegate& other)
		noexcept = default;


template <typename TRetVal, typename... TParams>
inline constexpr bool Delegate<TRetVal (TParams...)>::operator==(const Delegate& other) const noexcept {
	return DelegateBase<TRetVal, TParams...>::operator==(other);
}

template <typename TRetVal, typename... TParams>
inline constexpr bool Delegate<TRetVal (TParams...)>::operator!=(const Delegate& other) const noexcept {
	return !(*this == other);
}


template <typename TRetVal, typename... TParams>
template <TRetVal (*TFunction)(TParams...)>
inline constexpr void Delegate<TRetVal (TParams...)>::Bind() noexcept {
	this->pStub_ = &FunctionStub<TFunction>;
	this->pTypeErasedObject_ = nullptr;
}

template <typename TRetVal, typename... TParams>
template <class TFunctor, typename>
inline void Delegate<TRetVal (TParams...)>::Bind(TFunctor& functor) noexcept {
	this->pStub_ = &FunctorStub<TFunctor>;
	this->pTypeErasedObject_ = ::std::addressof(functor);
}

template <typename TRetVal, typename... TParams>
template <class TClass, TRetVal (TClass::*TMethod)(TParams...)>
inline void Delegate<TRetVal (TParams...)>::Bind(TClass& object) noexcept {
	static_assert(sizeof(TClass*) == sizeof(ErasedObjectType*), "pointers size mismatch");
	
	this->pStub_ = &MethodStub<TClass, TMethod>;
	this->pTypeErasedObject_ = ::std::addressof(object);
}


template <typename TRetVal, typename... TParams>
template <TRetVal (*TFunction)(TParams...)>
inline constexpr bool Delegate<TRetVal (TParams...)>::IsBindedTo() const noexcept {
	Delegate delegate;
	delegate.Bind<TFunction>();
	return *this == delegate;
}

template <typename TRetVal, typename... TParams>
template <class TFunctor, typename>
inline bool Delegate<TRetVal (TParams...)>::IsBindedTo(TFunctor& functor) const noexcept {
	Delegate delegate;
	delegate.Bind(functor);
	return *this == delegate;
}

template <typename TRetVal, typename... TParams>
template <class TClass, TRetVal (TClass::*TMethod)(TParams...)>
inline bool Delegate<TRetVal (TParams...)>::IsBindedTo(TClass& object) const noexcept {
	Delegate delegate;
	delegate.Bind<TClass, TMethod>(object);
	return *this == delegate;
}


template <typename TRetVal, typename... TParams>
template <typename... UParams>
inline TRetVal Delegate<TRetVal (TParams...)>::operator()(UParams&&... params) {
	return (*this->pStub_)(this->pTypeErasedObject_, ::std::forward<UParams>(params)...);
}


template <typename TRetVal, typename... TParams>
template <TRetVal (*TFunction)(TParams...)>
inline constexpr Delegate<TRetVal (TParams...)> Delegate<TRetVal (TParams...)>::CreateAndBind() noexcept {
	Delegate delegate;
	delegate.Bind<TFunction>();
	return delegate;
}

template <typename TRetVal, typename... TParams>
template <class TFunctor, typename>
inline Delegate<TRetVal (TParams...)> Delegate<TRetVal (TParams...)>::CreateAndBind(TFunctor& functor) noexcept {
	Delegate delegate;
	delegate.Bind(functor);
	return delegate;
}

template <typename TRetVal, typename... TParams>
template <class TClass, TRetVal (TClass::*TMethod)(TParams...)>
inline Delegate<TRetVal (TParams...)> Delegate<TRetVal (TParams...)>::CreateAndBind(TClass& object) noexcept {
	Delegate delegate;
	delegate.Bind<TClass, TMethod>(object);
	return delegate;
}


template <typename TRetVal, typename... TParams>
template <TRetVal (*TFunction)(TParams...)>
TRetVal Delegate<TRetVal (TParams...)>::FunctionStub(ErasedObjectType* pTypeErasedObject, TParams... params) {
	(void)pTypeErasedObject;
	
	return (*TFunction)(::std::forward<TParams>(params)...);
}

template <typename TRetVal, typename... TParams>
template <class TFunctor>
TRetVal Delegate<TRetVal (TParams...)>::FunctorStub(ErasedObjectType* pTypeErasedObject, TParams... params) {
	return (*static_cast<TFunctor*>(pTypeErasedObject))(::std::forward<TParams>(params)...);
}

template <typename TRetVal, typename... TParams>
template <class TClass, TRetVal (TClass::*TMethod)(TParams...)>
TRetVal Delegate<TRetVal (TParams...)>::MethodStub(ErasedObjectType* pTypeErasedObject, TParams... params) {
	return (static_cast<TClass*>(pTypeErasedObject)->*TMethod)(::std::forward<TParams>(params)...);
}




template <typename TRetVal, typename... TParams>
inline constexpr Delegate<TRetVal (TParams...) const>::Delegate() noexcept = default;

template <typename TRetVal, typename... TParams>
inline constexpr Delegate<TRetVal (TParams...) const>::Delegate(const Delegate& other) noexcept = default;


template <typename TRetVal, typename... TParams>
inline Delegate<TRetVal (TParams...) const>::~Delegate() noexcept = default;


template <typename TRetVal, typename... TParams>
inline constexpr Delegate<TRetVal (TParams...) const>& Delegate<TRetVal (TParams...) const>::operator=(const Delegate& other)
		noexcept = default;


template <typename TRetVal, typename... TParams>
inline constexpr bool Delegate<TRetVal (TParams...) const>::operator==(const Delegate& other) const noexcept {
	return DelegateBase<TRetVal, TParams...>::operator==(other);
}

template <typename TRetVal, typename... TParams>
inline constexpr bool Delegate<TRetVal (TParams...) const>::operator!=(const Delegate& other) const noexcept {
	return !(*this == other);
}


template <typename TRetVal, typename... TParams>
template <TRetVal (*TFunction)(TParams...)>
inline constexpr void Delegate<TRetVal (TParams...) const>::Bind() noexcept {
	this->pStub_ = &FunctionStub<TFunction>;
	this->pTypeErasedObject_ = nullptr;
}

template <typename TRetVal, typename... TParams>
template <class TFunctor, typename>
inline void Delegate<TRetVal (TParams...) const>::Bind(const TFunctor& functor) noexcept {
	this->pStub_ = &FunctorStub<TFunctor>;
	
	// removed const will be restored in FunctorStub()
	this->pTypeErasedObject_ = const_cast<ErasedObjectType*>(static_cast<const ErasedObjectType*>(::std::addressof(functor)));
}

template <typename TRetVal, typename... TParams>
template <class TClass, TRetVal (TClass::*TMethod)(TParams...) const>
inline void Delegate<TRetVal (TParams...) const>::Bind(const TClass& object) noexcept {
	static_assert(sizeof(TClass*) == sizeof(ErasedObjectType*), "pointers size mismatch");
	
	this->pStub_ = &MethodStub<TClass, TMethod>;
	
	// removed const will be restored in MethodStub()
	this->pTypeErasedObject_ = const_cast<ErasedObjectType*>(static_cast<const ErasedObjectType*>(::std::addressof(object)));
}


template <typename TRetVal, typename... TParams>
template <TRetVal (*TFunction)(TParams...)>
inline constexpr bool Delegate<TRetVal (TParams...) const>::IsBindedTo() const noexcept {
	Delegate delegate;
	delegate.Bind<TFunction>();
	return *this == delegate;
}

template <typename TRetVal, typename... TParams>
template <class TFunctor, typename>
inline bool Delegate<TRetVal (TParams...) const>::IsBindedTo(const TFunctor& functor) const noexcept {
	Delegate delegate;
	delegate.Bind(functor);
	return *this == delegate;
}

template <typename TRetVal, typename... TParams>
template <class TClass, TRetVal (TClass::*TMethod)(TParams...) const>
inline bool Delegate<TRetVal (TParams...) const>::IsBindedTo(const TClass& object) const noexcept {
	Delegate delegate;
	delegate.Bind<TClass, TMethod>(object);
	return *this == delegate;
}


template <typename TRetVal, typename... TParams>
template <typename... UParams>
inline TRetVal Delegate<TRetVal (TParams...) const>::operator()(UParams&&... params) const {
	return (*this->pStub_)(this->pTypeErasedObject_, ::std::forward<UParams>(params)...);
}


template <typename TRetVal, typename... TParams>
template <TRetVal (*TFunction)(TParams...)>
inline constexpr Delegate<TRetVal (TParams...) const> Delegate<TRetVal (TParams...) const>::CreateAndBind() noexcept {
	Delegate delegate;
	delegate.Bind<TFunction>();
	return delegate;
}

template <typename TRetVal, typename... TParams>
template <class TFunctor, typename>
inline Delegate<TRetVal (TParams...) const> Delegate<TRetVal (TParams...) const>::CreateAndBind(const TFunctor& functor)
		noexcept {
	Delegate delegate;
	delegate.Bind(functor);
	return delegate;
}

template <typename TRetVal, typename... TParams>
template <class TClass, TRetVal (TClass::*TMethod)(TParams...) const>
inline Delegate<TRetVal (TParams...) const> Delegate<TRetVal (TParams...) const>::CreateAndBind(const TClass& object)
		noexcept {
	Delegate delegate;
	delegate.Bind<TClass, TMethod>(object);
	return delegate;
}


template <typename TRetVal, typename... TParams>
template <TRetVal (*TFunction)(TParams...)>
TRetVal Delegate<TRetVal (TParams...) const>::FunctionStub(ErasedObjectType* pTypeErasedObject, TParams... params) {
	(void)pTypeErasedObject;
	
	return (*TFunction)(::std::forward<TParams>(params)...);
}

template <typename TRetVal, typename... TParams>
template <class TFunctor>
TRetVal Delegate<TRetVal (TParams...) const>::FunctorStub(ErasedObjectType* pTypeErasedObject, TParams... params) {
	// restoring removed const
	return (*static_cast<const TFunctor*>(pTypeErasedObject))(::std::forward<TParams>(params)...);
}

template <typename TRetVal, typename... TParams>
template <class TClass, TRetVal (TClass::*TMethod)(TParams...) const>
TRetVal Delegate<TRetVal (TParams...) const>::MethodStub(ErasedObjectType* pTypeErasedObject, TParams... params) {
	// restoring removed const
	return (static_cast<const TClass*>(pTypeErasedObject)->*TMethod)(::std::forward<TParams>(params)...);
}


/// \endcond


}	// namespace bricxx


#ifdef _MSC_VER
#pragma warning(pop)
#endif


#endif	// BRICXX_DELEGATE_HPP_INCLUDED_
