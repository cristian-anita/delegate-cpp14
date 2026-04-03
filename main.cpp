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


int FunctionAddInt(int x, int y);

class FunctorAddInt {
public:
	int operator()(int x, int y);
	
	int GetNumCalls() const;
private:
	int numCalls_ = 0;
};

class ClassAddInt {
public:
	int Add(int x, int y);
	
	int GetNumCalls() const;
private:
	int numCalls_ = 0;
};


class ConstFunctorAddInt {
public:
	int operator()(int x, int y) const;
};

class ConstClassAddInt {
public:
	int Add(int x, int y) const;
};


void TestHeaderDoc() {
	// (non-member) function
	Delegate<int (int)> functionDelegate;
	functionDelegate.Bind<&::islower>();
	if (!functionDelegate.IsBindedTo<&::islower>()) throw runtime_error("function test failed");
	if (!functionDelegate('a')) throw runtime_error("function test failed");
	
	// lambda/functor
	unsigned int numCalls = 0U;
	auto lambda = [&numCalls](int x, int y) mutable -> bool {
		++numCalls;
		return x > y;
	};
	Delegate<bool (int, int)> lambdaDelegate;
	lambdaDelegate.Bind(lambda);
	if (!lambdaDelegate.IsBindedTo(lambda)) throw runtime_error("lambda test failed");
	if (!lambdaDelegate(2, 1)) throw runtime_error("lambda test failed");
	if (numCalls != 1U) throw runtime_error("lambda test failed");
	
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
		Delegate<int (int, int)> delegate;
		delegate.Bind<&FunctionAddInt>();
		BRICXX_CHECK(delegate.IsBinded());
		BRICXX_CHECK(delegate.IsBindedTo<&FunctionAddInt>());
		BRICXX_CHECK(delegate(1, 2) == 3);
		delegate.Unbind();
		BRICXX_CHECK(!delegate.IsBinded());
		
		Delegate<int (int, int)> delegate1 = Delegate<int (int, int)>::CreateAndBind<&FunctionAddInt>();
		BRICXX_CHECK(delegate1.IsBinded());
		BRICXX_CHECK(delegate1.IsBindedTo<&FunctionAddInt>());
		BRICXX_CHECK(delegate1(1, 2) == 3);
	}
	
	// 2.2 functor
	{
		FunctorAddInt functorAddInt;
		
		Delegate<int (int, int)> delegate;
		delegate.Bind(functorAddInt);
		BRICXX_CHECK(delegate.IsBinded());
		BRICXX_CHECK(delegate.IsBindedTo(functorAddInt));
		BRICXX_CHECK(delegate(1, 2) == 3);
		BRICXX_CHECK(functorAddInt.GetNumCalls() == 1);
		delegate.Unbind();
		BRICXX_CHECK(!delegate.IsBinded());
		
		Delegate<int (int, int)> delegate1 = Delegate<int (int, int)>::CreateAndBind(functorAddInt);
		BRICXX_CHECK(delegate1.IsBinded());
		BRICXX_CHECK(delegate1.IsBindedTo(functorAddInt));
		BRICXX_CHECK(delegate1(1, 2) == 3);
		BRICXX_CHECK(functorAddInt.GetNumCalls() == 2);
	}
	
	// 2.3 lambda
	{
		int numCalls = 0;
		auto lambdaAddInt = [&numCalls](int x, int y) mutable -> int {
			++numCalls;
			return x + y;
		};
		
		Delegate<int (int, int)> delegate;
		delegate.Bind(lambdaAddInt);
		BRICXX_CHECK(delegate.IsBinded());
		BRICXX_CHECK(delegate.IsBindedTo(lambdaAddInt));
		BRICXX_CHECK(delegate(1, 2) == 3);
		BRICXX_CHECK(numCalls == 1);
		delegate.Unbind();
		BRICXX_CHECK(!delegate.IsBinded());
		
		Delegate<int (int, int)> delegate1 = Delegate<int (int, int)>::CreateAndBind(lambdaAddInt);
		BRICXX_CHECK(delegate1.IsBinded());
		BRICXX_CHECK(delegate1.IsBindedTo(lambdaAddInt));
		BRICXX_CHECK(delegate1(1, 2) == 3);
		BRICXX_CHECK(numCalls == 2);
	}
	
	// 2.4 member
	{
		ClassAddInt classAddInt;
		
		Delegate<int (int, int)> delegate;
		delegate.Bind<ClassAddInt, &ClassAddInt::Add>(classAddInt);
		BRICXX_CHECK(delegate.IsBinded());
		bool classAddIntIsBindedTo = delegate.IsBindedTo<ClassAddInt, &ClassAddInt::Add>(classAddInt);
		BRICXX_CHECK(classAddIntIsBindedTo);
		BRICXX_CHECK(delegate(1, 2) == 3);
		BRICXX_CHECK(classAddInt.GetNumCalls() == 1);
		delegate.Unbind();
		BRICXX_CHECK(!delegate.IsBinded());
		
		Delegate<int (int, int)> delegate1 = Delegate<int (int, int)>::CreateAndBind<ClassAddInt,
			&ClassAddInt::Add>(classAddInt);
		BRICXX_CHECK(delegate1.IsBinded());
		classAddIntIsBindedTo = delegate1.IsBindedTo<ClassAddInt, &ClassAddInt::Add>(classAddInt);
		BRICXX_CHECK(classAddIntIsBindedTo);
		BRICXX_CHECK(delegate1(1, 2) == 3);
		BRICXX_CHECK(classAddInt.GetNumCalls() == 2);
	}
	
	
	//************************************************************
	// 3 copy constructor/assignment, equality
	
	// 3.1 function
	{
		Delegate<int (int, int)> delegate;
		delegate.Bind<&FunctionAddInt>();
		
		Delegate<int (int, int)> delegate2 = delegate;
		BRICXX_CHECK(delegate2.IsBinded());
		BRICXX_CHECK(delegate2.IsBindedTo<&FunctionAddInt>());
		BRICXX_CHECK(delegate2(1, 2) == 3);
		
		Delegate<int (int, int)> delegate3;
		delegate3 = delegate;
		BRICXX_CHECK(delegate3.IsBinded());
		BRICXX_CHECK(delegate3.IsBindedTo<&FunctionAddInt>());
		BRICXX_CHECK(delegate3(2, 3) == 5);
		
		BRICXX_CHECK(delegate2 == delegate3);
		delegate3.Unbind();
		BRICXX_CHECK(delegate2 != delegate3);
	}
	
	// 3.2 functor
	{
		FunctorAddInt functorAddInt;
		
		Delegate<int (int, int)> delegate;
		delegate.Bind(functorAddInt);
		
		Delegate<int (int, int)> delegate2 = delegate;
		BRICXX_CHECK(delegate2.IsBinded());
		BRICXX_CHECK(delegate2.IsBindedTo(functorAddInt));
		BRICXX_CHECK(delegate2(1, 2) == 3);
		BRICXX_CHECK(functorAddInt.GetNumCalls() == 1);
		
		Delegate<int (int, int)> delegate3;
		delegate3 = delegate;
		BRICXX_CHECK(delegate3.IsBinded());
		BRICXX_CHECK(delegate3.IsBindedTo(functorAddInt));
		BRICXX_CHECK(delegate3(2, 3) == 5);
		BRICXX_CHECK(functorAddInt.GetNumCalls() == 2);
		
		BRICXX_CHECK(delegate2 == delegate3);
		delegate3.Unbind();
		BRICXX_CHECK(delegate2 != delegate3);
	}
	
	// 3.3 lambda
	{
		int numCalls = 0;
		auto lambdaAddInt = [&numCalls](int x, int y) mutable -> int {
			++numCalls;
			return x + y;
		};
		
		Delegate<int (int, int)> delegate;
		delegate.Bind(lambdaAddInt);
		
		Delegate<int (int, int)> delegate2 = delegate;
		BRICXX_CHECK(delegate2.IsBinded());
		BRICXX_CHECK(delegate2.IsBindedTo(lambdaAddInt));
		BRICXX_CHECK(delegate2(1, 2) == 3);
		BRICXX_CHECK(numCalls == 1);
		
		Delegate<int (int, int)> delegate3;
		delegate3 = delegate;
		BRICXX_CHECK(delegate3.IsBinded());
		BRICXX_CHECK(delegate3.IsBindedTo(lambdaAddInt));
		BRICXX_CHECK(delegate3(2, 3) == 5);
		BRICXX_CHECK(numCalls == 2);
		
		BRICXX_CHECK(delegate2 == delegate3);
		delegate3.Unbind();
		BRICXX_CHECK(delegate2 != delegate3);
	}
	
	// 3.4 member
	{
		ClassAddInt classAddInt;
		
		Delegate<int (int, int)> delegate;
		delegate.Bind<ClassAddInt, &ClassAddInt::Add>(classAddInt);
		
		Delegate<int (int, int)> delegate2 = delegate;
		BRICXX_CHECK(delegate2.IsBinded());
		bool classAddIntIsBindedTo = delegate2.IsBindedTo<ClassAddInt, &ClassAddInt::Add>(classAddInt);
		BRICXX_CHECK(classAddIntIsBindedTo);
		BRICXX_CHECK(delegate2(1, 2) == 3);
		BRICXX_CHECK(classAddInt.GetNumCalls() == 1);
		
		Delegate<int (int, int)> delegate3;
		delegate3 = delegate;
		BRICXX_CHECK(delegate3.IsBinded());
		classAddIntIsBindedTo = delegate3.IsBindedTo<ClassAddInt, &ClassAddInt::Add>(classAddInt);
		BRICXX_CHECK(classAddIntIsBindedTo);
		BRICXX_CHECK(delegate3(2, 3) == 5);
		BRICXX_CHECK(classAddInt.GetNumCalls() == 2);
		
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
	
	// 2.1 function
	{
		Delegate<int (int, int) const> delegate;
		delegate.Bind<&FunctionAddInt>();
		BRICXX_CHECK(delegate.IsBinded());
		BRICXX_CHECK(delegate.IsBindedTo<&FunctionAddInt>());
		BRICXX_CHECK(delegate(1, 2) == 3);
		delegate.Unbind();
		BRICXX_CHECK(!delegate.IsBinded());
		
		Delegate<int (int, int) const> delegate1 = Delegate<int (int, int) const>::CreateAndBind<&FunctionAddInt>();
		BRICXX_CHECK(delegate1.IsBinded());
		BRICXX_CHECK(delegate1.IsBindedTo<&FunctionAddInt>());
		BRICXX_CHECK(delegate1(1, 2) == 3);
	}
	
	// 2.2 functor
	{
		ConstFunctorAddInt constFunctorAddInt;
		
		Delegate<int (int, int) const> delegate;
		delegate.Bind(constFunctorAddInt);
		BRICXX_CHECK(delegate.IsBinded());
		BRICXX_CHECK(delegate.IsBindedTo(constFunctorAddInt));
		BRICXX_CHECK(delegate(1, 2) == 3);
		delegate.Unbind();
		BRICXX_CHECK(!delegate.IsBinded());
		
		Delegate<int (int, int) const> delegate1 = Delegate<int (int, int) const>::CreateAndBind(constFunctorAddInt);
		BRICXX_CHECK(delegate1.IsBinded());
		BRICXX_CHECK(delegate1.IsBindedTo(constFunctorAddInt));
		BRICXX_CHECK(delegate1(1, 2) == 3);
	}
	
	// 2.3 lambda
	{
		auto constLambdaAddInt = [](int x, int y) -> int {
			return x + y;
		};
		
		Delegate<int (int, int) const> delegate;
		delegate.Bind(constLambdaAddInt);
		BRICXX_CHECK(delegate.IsBinded());
		BRICXX_CHECK(delegate.IsBindedTo(constLambdaAddInt));
		BRICXX_CHECK(delegate(1, 2) == 3);
		delegate.Unbind();
		BRICXX_CHECK(!delegate.IsBinded());
		
		Delegate<int (int, int) const> delegate1 = Delegate<int (int, int) const>::CreateAndBind(constLambdaAddInt);
		BRICXX_CHECK(delegate1.IsBinded());
		BRICXX_CHECK(delegate1.IsBindedTo(constLambdaAddInt));
		BRICXX_CHECK(delegate1(1, 2) == 3);
	}
	
	// 2.4 member
	{
		ConstClassAddInt constClassAddInt;
		
		Delegate<int (int, int) const> delegate;
		delegate.Bind<ConstClassAddInt, &ConstClassAddInt::Add>(constClassAddInt);
		BRICXX_CHECK(delegate.IsBinded());
		bool constClassAddIntIsBindedTo = delegate.IsBindedTo<ConstClassAddInt, &ConstClassAddInt::Add>(constClassAddInt);
		BRICXX_CHECK(constClassAddIntIsBindedTo);
		BRICXX_CHECK(delegate(1, 2) == 3);
		delegate.Unbind();
		BRICXX_CHECK(!delegate.IsBinded());
		
		Delegate<int (int, int) const> delegate1 = Delegate<int (int, int) const>::CreateAndBind<ConstClassAddInt,
			&ConstClassAddInt::Add>(constClassAddInt);
		BRICXX_CHECK(delegate1.IsBinded());
		constClassAddIntIsBindedTo = delegate1.IsBindedTo<ConstClassAddInt, &ConstClassAddInt::Add>(constClassAddInt);
		BRICXX_CHECK(constClassAddIntIsBindedTo);
		BRICXX_CHECK(delegate1(1, 2) == 3);
	}
	
	
	//************************************************************
	// 3 copy constructor/assignment, equality
	
	// 3.1 function
	{
		Delegate<int (int, int) const> delegate;
		delegate.Bind<&FunctionAddInt>();
		
		Delegate<int (int, int) const> delegate2 = delegate;
		BRICXX_CHECK(delegate2.IsBinded());
		BRICXX_CHECK(delegate2.IsBindedTo<&FunctionAddInt>());
		BRICXX_CHECK(delegate2(1, 2) == 3);
		
		Delegate<int (int, int) const> delegate3;
		delegate3 = delegate;
		BRICXX_CHECK(delegate3.IsBinded());
		BRICXX_CHECK(delegate3.IsBindedTo<&FunctionAddInt>());
		BRICXX_CHECK(delegate3(2, 3) == 5);
		
		BRICXX_CHECK(delegate2 == delegate3);
		delegate3.Unbind();
		BRICXX_CHECK(delegate2 != delegate3);
	}
	
	// 3.2 functor
	{
		ConstFunctorAddInt constFunctorAddInt;
		
		Delegate<int (int, int) const> delegate;
		delegate.Bind(constFunctorAddInt);
		
		Delegate<int (int, int) const> delegate2 = delegate;
		BRICXX_CHECK(delegate2.IsBinded());
		BRICXX_CHECK(delegate2.IsBindedTo(constFunctorAddInt));
		BRICXX_CHECK(delegate2(1, 2) == 3);
		
		Delegate<int (int, int) const> delegate3;
		delegate3 = delegate;
		BRICXX_CHECK(delegate3.IsBinded());
		BRICXX_CHECK(delegate3.IsBindedTo(constFunctorAddInt));
		BRICXX_CHECK(delegate3(2, 3) == 5);
		
		BRICXX_CHECK(delegate2 == delegate3);
		delegate3.Unbind();
		BRICXX_CHECK(delegate2 != delegate3);
	}
	
	// 3.3 lambda
	{
		auto constLambdaAddInt = [](int x, int y) -> int {
			return x + y;
		};
		
		Delegate<int (int, int) const> delegate;
		delegate.Bind(constLambdaAddInt);
		
		Delegate<int (int, int) const> delegate2 = delegate;
		BRICXX_CHECK(delegate2.IsBinded());
		BRICXX_CHECK(delegate2.IsBindedTo(constLambdaAddInt));
		BRICXX_CHECK(delegate2(1, 2) == 3);
		
		Delegate<int (int, int) const> delegate3;
		delegate3 = delegate;
		BRICXX_CHECK(delegate3.IsBinded());
		BRICXX_CHECK(delegate3.IsBindedTo(constLambdaAddInt));
		BRICXX_CHECK(delegate3(2, 3) == 5);
		
		BRICXX_CHECK(delegate2 == delegate3);
		delegate3.Unbind();
		BRICXX_CHECK(delegate2 != delegate3);
	}
	
	// 3.4 member
	{
		ConstClassAddInt constClassAddInt;
		
		Delegate<int (int, int) const> delegate;
		delegate.Bind<ConstClassAddInt, &ConstClassAddInt::Add>(constClassAddInt);
		
		Delegate<int (int, int) const> delegate2 = delegate;
		BRICXX_CHECK(delegate2.IsBinded());
		bool constClassAddIntIsBindedTo = delegate2.IsBindedTo<ConstClassAddInt, &ConstClassAddInt::Add>(constClassAddInt);
		BRICXX_CHECK(constClassAddIntIsBindedTo);
		BRICXX_CHECK(delegate2(1, 2) == 3);
		
		Delegate<int (int, int) const> delegate3;
		delegate3 = delegate;
		BRICXX_CHECK(delegate3.IsBinded());
		constClassAddIntIsBindedTo = delegate3.IsBindedTo<ConstClassAddInt, &ConstClassAddInt::Add>(constClassAddInt);
		BRICXX_CHECK(constClassAddIntIsBindedTo);
		BRICXX_CHECK(delegate3(2, 3) == 5);
		
		BRICXX_CHECK(delegate2 == delegate3);
		delegate3.Unbind();
		BRICXX_CHECK(delegate2 != delegate3);
	}
	
	
	//************************************************************
	// 4 const-correctness
	
	// 4.1 functor
	{
		FunctorAddInt functorAddInt;
		
		Delegate<int (int, int) const> delegate;
		//delegate.Bind(functorAddInt);	// OK, does not compile
		(void)functorAddInt;
		(void)delegate;
	}
	
	// 4.2 lambda
	{
		auto lambdaAddInt = [](int x, int y) mutable -> int {
			return x + y;
		};
		
		Delegate<int (int, int) const> delegate;
		//delegate.Bind(lambdaAddInt);	// OK, does not compile
		(void)lambdaAddInt;
		(void)delegate;
	}
	
	// 4.3 member
	{
		ClassAddInt classAddInt;
		
		Delegate<int (int, int) const> delegate;
		//delegate.Bind<ClassAddInt, &ClassAddInt::Add>(classAddInt);	// OK, does not compile
		(void)classAddInt;
		(void)delegate;
	}
}


inline int FunctionAddInt(int x, int y) {
	return x + y;
}

inline int FunctorAddInt::operator()(int x, int y) {
	++numCalls_;
	return x + y;
}

inline int FunctorAddInt::GetNumCalls() const {
	return numCalls_;
}

inline int ClassAddInt::Add(int x, int y) {
	++numCalls_;
	return x + y;
}

inline int ClassAddInt::GetNumCalls() const {
	return numCalls_;
}


inline int ConstFunctorAddInt::operator()(int x, int y) const {
	return x + y;
}

inline int ConstClassAddInt::Add(int x, int y) const {
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
