#include <iostream>
#include "Vector.h"

class A {
public:
	int a;
};

int main() {
	// Vector<int>
	Vector<int> v1(3); // ctor with size
	v1[0] = 1;
	v1[1] = 2;
	v1[2] = 3;
	Vector<int> v2, v3; // default ctor
	Vector<int> v4(v1); // copy ctor
	v2 = v3 = v1; // operator '='
	std::cout << v2[0] << v3[0] << v4[0] << v2[1] << v3[1] << v4[1] << v2[2] << v3[2] << v4[2] << std::endl;
	std::cout << v1.size() << std::endl;
	std::cout << v1.inflate(2) << std::endl; // inflate
	std::cout << v1.size() << std::endl;
	v1[3] = 4;
	v1[4] = 5;
	std::cout << v1[3] << v1[4] << std::endl;

	// Vector<string>
	Vector<std::string> s1(3); // ctor with size
	s1[0] = "hello";
	s1[1] = "world";
	s1[2] = "Hello";
	Vector<std::string> s2, s3; // default ctor
	Vector<std::string> s4(s1); // copy ctor
	s2 = s3 = s1; // operator '='
	std::cout << s2[0] << s3[0] << s4[0] << s2[1] << s3[1] << s4[1] << s2[2] << s3[2] << s4[2] << std::endl;
	std::cout << s1.size() << std::endl;
	std::cout << s1.inflate(2) << std::endl; // inflate
	std::cout << s1.size() << std::endl;
	s1[3] = "World";
	s1[4] = "!";
	std::cout << s1[3] << s1[4] << std::endl;

	// Vector of other class
	Vector<A> a1(3); // ctor with size
	a1[0].a = 1;
	a1[1].a = 2;
	a1[2].a = 3;
	Vector<A> a2, a3; // default ctor
	Vector<A> a4(a1); // copy ctor
	a2 = a3 = a1; // operator '='
	std::cout << a2[0].a << a3[0].a << a4[0].a << a2[1].a << a3[1].a << a4[1].a << a2[2].a << a3[2].a << a4[2].a << std::endl;
	std::cout << a1.size() << std::endl;
	std::cout << a1.inflate(2) << std::endl; // inflate
	std::cout << a1.size() << std::endl;
	a1[3].a = 4;
	a1[4].a = 5;
	std::cout << a1[3].a << a1[4].a << std::endl;

	// Vector< Vector<int> >
	Vector< Vector<int> > vv1(4);
	vv1[0] = v1;
	vv1[1] = v2;
	vv1[2] = v3;
	vv1[3] = v4;
	std::cout << vv1[0][0] << std::endl;
	std::cout << vv1[0][4] << std::endl;
	std::cout << vv1[1][4] << std::endl; // Out of Bounds

	return 0;
}
