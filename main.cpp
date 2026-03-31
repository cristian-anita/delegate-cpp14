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


#include "delegate.hpp"

#include <string>
#include <cctype>
#include <cstddef>
#include <stdexcept>
#include <iostream>


#define BRICXX_CHECK(expr)	\
	do { \
		if (!(expr)) { \
			throw runtime_error(#expr); \
		} \
	} while (false)


using namespace bricxx;
using namespace std;


void TestHeaderDoc();
void TestDelegate();
void TestConstDelegate();


long FunctionAddLong(long x, long y);

class FunctorAddLong {
public:
	long operator()(long x, long y);
	
	int GetNumCalls() const;
private:
	int numCalls_ = 0;
};

class ClassAddLong {
public:
	long Add(long x, long y);
	
	int GetNumCalls() const;
private:
	int numCalls_ = 0;
};


class ConstFunctorAddLong {
public:
	long operator()(long x, long y) const;
};

class ConstClassAddLong {
public:
	long Add(long x, long y) const;
};


void TestHeaderDoc() {
	// (non-member) function
	Delegate<int (int)> functionDelegate;
	functionDelegate.Bind<&::islower>();
	if (!functionDelegate.IsBindedTo<&::islower>()) throw runtime_error("function test failed");
	if (!functionDelegate('a')) throw runtime_error("function test failed");
	
	// lambda/functor
	int numCalls = 0;
	auto lambda = [&numCalls](int x, int y) mutable -> bool {
		++numCalls;
		return x > y;
	};
	Delegate<bool (int, int)> lambdaDelegate;
	lambdaDelegate.Bind(lambda);
	if (!lambdaDelegate.IsBindedTo(lambda)) throw runtime_error("lambda test failed");
	if (!lambdaDelegate(2, 1)) throw runtime_error("lambda test failed");
	if (numCalls != 1) throw runtime_error("lambda test failed");
	
	// method (member function)
	string hello = "Hello, World!";
	Delegate<void ()> methodDelegate;
	methodDelegate.Bind<string, &string::clear>(hello);
	methodDelegate();
	if (!hello.empty()) throw runtime_error("method test failed");
	
	// comparison
	Delegate<int (int)> functionDelegate1;
	functionDelegate1.Bind<&::isupper>();
	if (functionDelegate == functionDelegate1) throw runtime_error("comparison test failed");
	functionDelegate1.Bind<&::islower>();
	if (functionDelegate != functionDelegate1) throw runtime_error("comparison test failed");
	
	// const-correctness
	Delegate<void () const> constDelegate;
	//constDelegate.Bind<string, &string::clear>(hello);	// OK, does not compile
	(void)constDelegate;
	
	// unbind
	functionDelegate.Unbind();
	if (functionDelegate.IsBinded()) throw runtime_error("unbind test failed");
	
	// BadDelegateCall
	bool caught = false;
	try {
		functionDelegate('a');
	}
	catch (const BadDelegateCall&) {
		caught = true;
	}
	if (!caught) throw runtime_error("BadDelegateCall test failed");
}

void TestDelegate() {
	// 1 constructor, BadDelegateCall
	{
		Delegate<void ()> delegate;
		BRICXX_CHECK(!delegate.IsBinded());
		
		bool caught = false;
		try {
			delegate();
		}
		catch (const BadDelegateCall&) {
			caught = true;
		}
		BRICXX_CHECK(caught);
	}
	
	
	//************************************************************
	// 2 bind/unbind
	
	// 2.1 function
	{
		Delegate<long (long, long)> delegate;
		delegate.Bind<&FunctionAddLong>();
		BRICXX_CHECK(delegate.IsBinded());
		BRICXX_CHECK(delegate.IsBindedTo<&FunctionAddLong>());
		BRICXX_CHECK(delegate(1L, 2L) == 3L);
		delegate.Unbind();
		BRICXX_CHECK(!delegate.IsBinded());
	}
	
	// 2.2 functor
	{
		FunctorAddLong functorAddLong;
		
		Delegate<long (long, long)> delegate;
		delegate.Bind(functorAddLong);
		BRICXX_CHECK(delegate.IsBinded());
		BRICXX_CHECK(delegate.IsBindedTo(functorAddLong));
		BRICXX_CHECK(delegate(1L, 2L) == 3L);
		BRICXX_CHECK(functorAddLong.GetNumCalls() == 1);
		delegate.Unbind();
		BRICXX_CHECK(!delegate.IsBinded());
	}
	
	// 2.3 lambda
	{
		int numCalls = 0;
		auto lambdaAddLong = [&numCalls](long x, long y) mutable -> long {
			++numCalls;
			return x + y;
		};
		
		Delegate<long (long, long)> delegate;
		delegate.Bind(lambdaAddLong);
		BRICXX_CHECK(delegate.IsBinded());
		BRICXX_CHECK(delegate.IsBindedTo(lambdaAddLong));
		BRICXX_CHECK(delegate(1L, 2L) == 3L);
		BRICXX_CHECK(numCalls == 1);
		delegate.Unbind();
		BRICXX_CHECK(!delegate.IsBinded());
	}
	
	// 2.4 member
	{
		ClassAddLong classAddLong;
		
		Delegate<long (long, long)> delegate;
		delegate.Bind<ClassAddLong, &ClassAddLong::Add>(classAddLong);
		BRICXX_CHECK(delegate.IsBinded());
		bool classAddLongIsBindedTo = delegate.IsBindedTo<ClassAddLong, &ClassAddLong::Add>(classAddLong);
		BRICXX_CHECK(classAddLongIsBindedTo);
		BRICXX_CHECK(delegate(1L, 2L) == 3L);
		BRICXX_CHECK(classAddLong.GetNumCalls() == 1);
		delegate.Unbind();
		BRICXX_CHECK(!delegate.IsBinded());
	}
	
	
	//************************************************************
	// 3 copy constructor/assignment, equality
	
	// 3.1 function
	{
		Delegate<long (long, long)> delegate;
		delegate.Bind<&FunctionAddLong>();
		
		Delegate<long (long, long)> delegate2 = delegate;
		BRICXX_CHECK(delegate2.IsBinded());
		BRICXX_CHECK(delegate2.IsBindedTo<&FunctionAddLong>());
		BRICXX_CHECK(delegate2(1L, 2L) == 3L);
		
		Delegate<long (long, long)> delegate3;
		delegate3 = delegate;
		BRICXX_CHECK(delegate3.IsBinded());
		BRICXX_CHECK(delegate3.IsBindedTo<&FunctionAddLong>());
		BRICXX_CHECK(delegate3(2L, 3L) == 5L);
		
		BRICXX_CHECK(delegate2 == delegate3);
		delegate3.Unbind();
		BRICXX_CHECK(delegate2 != delegate3);
	}
	
	// 3.2 functor
	{
		FunctorAddLong functorAddLong;
		
		Delegate<long (long, long)> delegate;
		delegate.Bind(functorAddLong);
		
		Delegate<long (long, long)> delegate2 = delegate;
		BRICXX_CHECK(delegate2.IsBinded());
		BRICXX_CHECK(delegate2.IsBindedTo(functorAddLong));
		BRICXX_CHECK(delegate2(1L, 2L) == 3L);
		BRICXX_CHECK(functorAddLong.GetNumCalls() == 1);
		
		Delegate<long (long, long)> delegate3;
		delegate3 = delegate;
		BRICXX_CHECK(delegate3.IsBinded());
		BRICXX_CHECK(delegate3.IsBindedTo(functorAddLong));
		BRICXX_CHECK(delegate3(2L, 3L) == 5L);
		BRICXX_CHECK(functorAddLong.GetNumCalls() == 2);
		
		BRICXX_CHECK(delegate2 == delegate3);
		delegate3.Unbind();
		BRICXX_CHECK(delegate2 != delegate3);
	}
	
	// 3.3 lambda
	{
		int numCalls = 0;
		auto lambdaAddLong = [&numCalls](long x, long y) mutable -> long {
			++numCalls;
			return x + y;
		};
		
		Delegate<long (long, long)> delegate;
		delegate.Bind(lambdaAddLong);
		
		Delegate<long (long, long)> delegate2 = delegate;
		BRICXX_CHECK(delegate2.IsBinded());
		BRICXX_CHECK(delegate2.IsBindedTo(lambdaAddLong));
		BRICXX_CHECK(delegate2(1L, 2L) == 3L);
		BRICXX_CHECK(numCalls == 1);
		
		Delegate<long (long, long)> delegate3;
		delegate3 = delegate;
		BRICXX_CHECK(delegate3.IsBinded());
		BRICXX_CHECK(delegate3.IsBindedTo(lambdaAddLong));
		BRICXX_CHECK(delegate3(2L, 3L) == 5L);
		BRICXX_CHECK(numCalls == 2);
		
		BRICXX_CHECK(delegate2 == delegate3);
		delegate3.Unbind();
		BRICXX_CHECK(delegate2 != delegate3);
	}
	
	// 3.4 member
	{
		ClassAddLong classAddLong;
		
		Delegate<long (long, long)> delegate;
		delegate.Bind<ClassAddLong, &ClassAddLong::Add>(classAddLong);
		
		Delegate<long (long, long)> delegate2 = delegate;
		BRICXX_CHECK(delegate2.IsBinded());
		bool classAddLongIsBindedTo = delegate2.IsBindedTo<ClassAddLong, &ClassAddLong::Add>(classAddLong);
		BRICXX_CHECK(classAddLongIsBindedTo);
		BRICXX_CHECK(delegate2(1L, 2L) == 3L);
		BRICXX_CHECK(classAddLong.GetNumCalls() == 1);
		
		Delegate<long (long, long)> delegate3;
		delegate3 = delegate;
		BRICXX_CHECK(delegate3.IsBinded());
		classAddLongIsBindedTo = delegate3.IsBindedTo<ClassAddLong, &ClassAddLong::Add>(classAddLong);
		BRICXX_CHECK(classAddLongIsBindedTo);
		BRICXX_CHECK(delegate3(2L, 3L) == 5L);
		BRICXX_CHECK(classAddLong.GetNumCalls() == 2);
		
		BRICXX_CHECK(delegate2 == delegate3);
		delegate3.Unbind();
		BRICXX_CHECK(delegate2 != delegate3);
	}
}

void TestConstDelegate() {
	// 1 constructor, BadDelegateCall
	{
		Delegate<void () const> delegate;
		BRICXX_CHECK(!delegate.IsBinded());
		
		bool caught = false;
		try {
			delegate();
		}
		catch (const BadDelegateCall&) {
			caught = true;
		}
		BRICXX_CHECK(caught);
	}
	
	
	//************************************************************
	// 2 bind/unbind
	
	// 2.1 functor
	{
		ConstFunctorAddLong constFunctorAddLong;
		
		Delegate<long (long, long) const> delegate;
		delegate.Bind(constFunctorAddLong);
		BRICXX_CHECK(delegate.IsBinded());
		BRICXX_CHECK(delegate.IsBindedTo(constFunctorAddLong));
		BRICXX_CHECK(delegate(1L, 2L) == 3L);
		delegate.Unbind();
		BRICXX_CHECK(!delegate.IsBinded());
	}
	
	// 2.2 lambda
	{
		auto constLambdaAddLong = [](long x, long y) -> long {
			return x + y;
		};
		
		Delegate<long (long, long) const> delegate;
		delegate.Bind(constLambdaAddLong);
		BRICXX_CHECK(delegate.IsBinded());
		BRICXX_CHECK(delegate.IsBindedTo(constLambdaAddLong));
		BRICXX_CHECK(delegate(1L, 2L) == 3L);
		delegate.Unbind();
		BRICXX_CHECK(!delegate.IsBinded());
	}
	
	// 2.3 member
	{
		ConstClassAddLong constClassAddLong;
		
		Delegate<long (long, long) const> delegate;
		delegate.Bind<ConstClassAddLong, &ConstClassAddLong::Add>(constClassAddLong);
		BRICXX_CHECK(delegate.IsBinded());
		bool constClassAddLongIsBindedTo = delegate.IsBindedTo<ConstClassAddLong, &ConstClassAddLong::Add>(constClassAddLong);
		BRICXX_CHECK(constClassAddLongIsBindedTo);
		BRICXX_CHECK(delegate(1L, 2L) == 3L);
		delegate.Unbind();
		BRICXX_CHECK(!delegate.IsBinded());
	}
	
	
	//************************************************************
	// 3 copy constructor/assignment, equality
	
	// 3.1 functor
	{
		ConstFunctorAddLong constFunctorAddLong;
		
		Delegate<long (long, long) const> delegate;
		delegate.Bind(constFunctorAddLong);
		
		Delegate<long (long, long) const> delegate2 = delegate;
		BRICXX_CHECK(delegate2.IsBinded());
		BRICXX_CHECK(delegate2.IsBindedTo(constFunctorAddLong));
		BRICXX_CHECK(delegate2(1L, 2L) == 3L);
		
		Delegate<long (long, long) const> delegate3;
		delegate3 = delegate;
		BRICXX_CHECK(delegate3.IsBinded());
		BRICXX_CHECK(delegate3.IsBindedTo(constFunctorAddLong));
		BRICXX_CHECK(delegate3(2L, 3L) == 5L);
		
		BRICXX_CHECK(delegate2 == delegate3);
		delegate3.Unbind();
		BRICXX_CHECK(delegate2 != delegate3);
	}
	
	// 3.2 lambda
	{
		auto constLambdaAddLong = [](long x, long y) -> long {
			return x + y;
		};
		
		Delegate<long (long, long) const> delegate;
		delegate.Bind(constLambdaAddLong);
		
		Delegate<long (long, long) const> delegate2 = delegate;
		BRICXX_CHECK(delegate2.IsBinded());
		BRICXX_CHECK(delegate2.IsBindedTo(constLambdaAddLong));
		BRICXX_CHECK(delegate2(1L, 2L) == 3L);
		
		Delegate<long (long, long) const> delegate3;
		delegate3 = delegate;
		BRICXX_CHECK(delegate3.IsBinded());
		BRICXX_CHECK(delegate3.IsBindedTo(constLambdaAddLong));
		BRICXX_CHECK(delegate3(2L, 3L) == 5L);
		
		BRICXX_CHECK(delegate2 == delegate3);
		delegate3.Unbind();
		BRICXX_CHECK(delegate2 != delegate3);
	}
	
	// 3.3 member
	{
		ConstClassAddLong constClassAddLong;
		
		Delegate<long (long, long) const> delegate;
		delegate.Bind<ConstClassAddLong, &ConstClassAddLong::Add>(constClassAddLong);
		
		Delegate<long (long, long) const> delegate2 = delegate;
		BRICXX_CHECK(delegate2.IsBinded());
		bool constClassAddLongIsBindedTo = delegate2.IsBindedTo<ConstClassAddLong, &ConstClassAddLong::Add>(constClassAddLong);
		BRICXX_CHECK(constClassAddLongIsBindedTo);
		BRICXX_CHECK(delegate2(1L, 2L) == 3L);
		
		Delegate<long (long, long) const> delegate3;
		delegate3 = delegate;
		BRICXX_CHECK(delegate3.IsBinded());
		constClassAddLongIsBindedTo = delegate3.IsBindedTo<ConstClassAddLong, &ConstClassAddLong::Add>(constClassAddLong);
		BRICXX_CHECK(constClassAddLongIsBindedTo);
		BRICXX_CHECK(delegate3(2L, 3L) == 5L);
		
		BRICXX_CHECK(delegate2 == delegate3);
		delegate3.Unbind();
		BRICXX_CHECK(delegate2 != delegate3);
	}
	
	
	//************************************************************
	// 4 const-correctness
	
	// 4.1 functor
	{
		FunctorAddLong functorAddLong;
		
		Delegate<long (long, long) const> delegate;
		//delegate.Bind(functorAddLong);	// OK, does not compile
		(void)functorAddLong;
		(void)delegate;
	}
	
	// 4.2 lambda
	{
		auto lambdaAddLong = [](long x, long y) mutable -> long {
			return x + y;
		};
		
		Delegate<long (long, long) const> delegate;
		//delegate.Bind(lambdaAddLong);	// OK, does not compile
		(void)lambdaAddLong;
		(void)delegate;
	}
	
	// 4.3 member
	{
		ClassAddLong classAddLong;
		
		Delegate<long (long, long) const> delegate;
		//delegate.Bind<ClassAddLong, &ClassAddLong::Add>(classAddLong);	// OK, does not compile
		(void)classAddLong;
		(void)delegate;
	}
}


inline long FunctionAddLong(long x, long y) {
	return x + y;
}

inline long FunctorAddLong::operator()(long x, long y) {
	++numCalls_;
	return x + y;
}

inline int FunctorAddLong::GetNumCalls() const {
	return numCalls_;
}

inline long ClassAddLong::Add(long x, long y) {
	++numCalls_;
	return x + y;
}

inline int ClassAddLong::GetNumCalls() const {
	return numCalls_;
}


inline long ConstFunctorAddLong::operator()(long x, long y) const {
	return x + y;
}

inline long ConstClassAddLong::Add(long x, long y) const {
	return x + y;
}




int main() try {
	TestHeaderDoc();
	TestDelegate();
	TestConstDelegate();
	
	cout << "test completed" << endl;
	return EXIT_SUCCESS;
}
catch (const exception& exc) {
	cout << "test failed: " << exc.what() << endl;
	return EXIT_FAILURE;
}
