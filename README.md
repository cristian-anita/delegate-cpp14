# A non-owning, const-correct and equality comparable callable wrapper.


## Delegate:

- is copyable, small (two pointers) and fast (no dynamic memory allocation, no virtual functions,
all functions are inline (except *stubs* because their address is needed),
delegate call involves only two extra calls (`operator()()` and *stub* function)).
- supports (non-member) functions, methods (member functions) and functors/lambda.
- is a single header library so it is easy to integrate in any project.


## Attention:

Because `Delegate` is non-owning special care is needed to ensure that the *binded* target outlive it.


## Remarks:

- Two delegates are considered equal if they are not *binded* or *binded* to the same target.
- Equality comparison and `Delegate::IsBindedTo()` allows delegates to be easily used as callbacks that can be
registered/unregistered (`::std::function_ref` is not equality comparable).
- The solutions presented in *references* directory are not const-correct
(they have the same problem as `::std::function`).


## See also:

- [Rich Hickey: Callbacks in C++ Using Template Functors](http://www.tutok.sk/fastgl/callback.html)
- Bert Peers: Callbacks in C++
- Don Clugston, CodeProject: Member Function Pointers and the Fastest Possible C++ Delegates
- Sergey Ryazanov, CodeProject: The Impossibly Fast C++ Delegates
- Sergey Alexandrovich Kryukov, CodeProject: The Impossibly Fast C++ Delegates, Fixed
- Aleksei Trunov, CodeProject: Yet Another Generalized Functors Implementation in C++
- JaeWook Choi, CodeProject: Fast C++ Delegate
- [Matthew Rodusek: Creating a Fast and Efficient Delegate Type (Part 1)](https://rodusek.com/posts/2021/02/24/creating-a-fast-and-efficient-delegate-type-part-1)
- [Matthew Rodusek: Creating a Fast and Efficient Delegate Type (Part 2)](https://rodusek.com/posts/2021/02/26/creating-a-fast-and-efficient-delegate-type-part-2)
- [Matthew Rodusek: Creating a Fast and Efficient Delegate Type (Part 3)](https://rodusek.com/posts/2021/02/26/creating-a-fast-and-efficient-delegate-type-part-3)
- [Matthew Rodusek: Unbelievably Fast Delegate](https://github.com/bitwizeshift/Delegate)
- [user1095108: Impossibly fast delegate in C++11](https://codereview.stackexchange.com/questions/14730/impossibly-fast-delegate-in-c11)
- [user1095108: delegate.hpp](https://github.com/jamboree/CxxFunctionBenchmark/blob/master/delegate.hpp)
- *references* directory in this project that contains the above.


## Tested on:

- openSUSE Tumbleweed: gcc 15.2.1, clang 21.1.8
- Windows 10 22H2: Visual Studio Community 2022 17.14.29, cl 19.44.35225 and clang-cl 19.1.5
- macOS 13.7.8: Xcode 14.3.1 (Apple clang 14.0.3)
- FreeBSD 15.0: clang 19.1.7


## Minimal introduction/examples (please see tests for more details):

```cpp
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
```
