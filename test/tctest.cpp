// (c) Copyright 2022 Aaron Kimball

#include <tiny-collections.h>
#include <iostream>
#include <string>

using namespace tc;

using std::cout;
using std::endl;
using std::string;

static int testFailures = 0;
static int testRuns = 0;

static void evalTestcase(const string &testName, bool expectedValue, bool actualValue) {
  testRuns++;

  cout << testName << ": (expected=" << expectedValue << ", actual=" << actualValue << ")";
  if (expectedValue != actualValue) {
    testFailures++;
    cout << ": *** FAILED ***" << endl;
  } else {
    cout << ": OK" << endl;
  }
}

int main(int argc, char **argv) {
  cout << std::boolalpha; // pretty-print 'true' and 'false'.

  // reinitialize counters.
  testFailures = 0;
  testRuns = 0;

  cout << "Test suite start" << endl;
  cout << "================================" << endl;
  evalTestcase("True type value", true, tc_true_type::value());
  evalTestcase("False type value", false, tc_false_type::value());

  evalTestcase("Conditional true value", true,
      tc_conditional<true, tc_true_type, tc_false_type>::type::value());
  evalTestcase("Conditional false value", false,
      tc_conditional<false, tc_true_type, tc_false_type>::type::value());

  evalTestcase("AND()", true, tc_conjunction<>::value());
  evalTestcase("AND(true)", true, tc_conjunction<tc_true_type>::value());
  evalTestcase("AND(false)", false, tc_conjunction<tc_false_type>::value());
  evalTestcase("AND(true, true)", true,
      tc_conjunction<tc_true_type, tc_true_type>::value());
  evalTestcase("AND(true, false)", false,
      tc_conjunction<tc_true_type, tc_false_type>::value());
  evalTestcase("AND(false, true)", false,
      tc_conjunction<tc_false_type, tc_true_type>::value());
  evalTestcase("AND(false, false)", false,
      tc_conjunction<tc_false_type, tc_false_type>::value());
  evalTestcase("AND(true, true, true)", true,
      tc_conjunction<tc_true_type, tc_true_type, tc_true_type>::value());
  evalTestcase("AND(true, true, false)", false,
      tc_conjunction<tc_true_type, tc_true_type, tc_false_type>::value());
  evalTestcase("AND(true, false, true)", false,
      tc_conjunction<tc_true_type, tc_false_type, tc_true_type>::value());
  evalTestcase("AND(false, true, true)", false,
      tc_conjunction<tc_false_type, tc_true_type, tc_true_type>::value());

  evalTestcase("OR()", false, tc_disjunction<>::value());
  evalTestcase("OR(true)", true, tc_disjunction<tc_true_type>::value());
  evalTestcase("OR(false)", false, tc_disjunction<tc_false_type>::value());
  evalTestcase("OR(true, true)", true,
      tc_disjunction<tc_true_type, tc_true_type>::value());
  evalTestcase("OR(true, false)", true,
      tc_disjunction<tc_true_type, tc_false_type>::value());
  evalTestcase("OR(false, true)", true,
      tc_disjunction<tc_false_type, tc_true_type>::value());
  evalTestcase("OR(false, false)", false,
      tc_disjunction<tc_false_type, tc_false_type>::value());
  evalTestcase("OR(true, true, true)", true,
      tc_disjunction<tc_true_type, tc_true_type, tc_true_type>::value());
  evalTestcase("OR(true, true, false)", true,
      tc_disjunction<tc_true_type, tc_true_type, tc_false_type>::value());
  evalTestcase("OR(true, false, true)", true,
      tc_disjunction<tc_true_type, tc_false_type, tc_true_type>::value());
  evalTestcase("OR(false, true, true)", true,
      tc_disjunction<tc_false_type, tc_true_type, tc_true_type>::value());
  evalTestcase("OR(false, false, false)", false,
      tc_disjunction<tc_false_type, tc_false_type, tc_false_type>::value());


  evalTestcase("IsArithmetic(int)", true, tc_is_arithmetic_type<int>::value());
  evalTestcase("IsArithmetic(char)", true, tc_is_arithmetic_type<char>::value());
  evalTestcase("IsArithmetic(unsigned int)", true, tc_is_arithmetic_type<unsigned int>::value());
  evalTestcase("IsArithmetic(unsigned char)", true, tc_is_arithmetic_type<unsigned char>::value());
  evalTestcase("IsArithmetic(long)", true, tc_is_arithmetic_type<long>::value());
  evalTestcase("IsArithmetic(unsigned long)", true, tc_is_arithmetic_type<unsigned long>::value());
  evalTestcase("IsArithmetic(float)", true, tc_is_arithmetic_type<float>::value());
  evalTestcase("IsArithmetic(const int)", true, tc_is_arithmetic_type<const int>::value());
  evalTestcase("IsArithmetic(volatile int)", true, tc_is_arithmetic_type<volatile int>::value());
  evalTestcase("IsArithmetic(const volatile int)", true, tc_is_arithmetic_type<const volatile int>::value());
  evalTestcase("IsArithmetic(volatile const int)", true, tc_is_arithmetic_type<volatile const int>::value());

  evalTestcase("IsArithmetic(int*)", false, tc_is_arithmetic_type<int*>::value());
  evalTestcase("IsArithmetic(int&)", false, tc_is_arithmetic_type<int&>::value());
  evalTestcase("IsArithmetic(string*)", false, tc_is_arithmetic_type<string*>::value());
  evalTestcase("IsArithmetic(const string &)", false, tc_is_arithmetic_type<const string &>::value());
  evalTestcase("IsArithmetic(void*)", false, tc_is_arithmetic_type<void *>::value());

  enum fooEnum { A, B, C };
  evalTestcase("IsArithmetic(enum fooEnum)", false, tc_is_arithmetic_type<fooEnum>::value());
  evalTestcase("IsEnum(enum fooEnum)", true, tc_is_enum<fooEnum>::value());

  evalTestcase("IsPointer(int*)", true, tc_is_pointer<int*>::value());
  evalTestcase("IsPointer(int**)", true, tc_is_pointer<int**>::value());
  evalTestcase("IsPointer(int&)", false, tc_is_pointer<int&>::value());
  evalTestcase("IsPointer(string*)", true, tc_is_pointer<string*>::value());
  evalTestcase("IsPointer(const string*)", true, tc_is_pointer<const string*>::value());
  evalTestcase("IsPointer(const string *const)", true, tc_is_pointer<const string *const>::value());
  evalTestcase("IsPointer(string *const)", true, tc_is_pointer<string *const>::value());
  evalTestcase("IsPointer(const volatile string*)", true, tc_is_pointer<const volatile string*>::value());
  evalTestcase("IsPointer(volatile const string*)", true, tc_is_pointer<volatile const string*>::value());
  evalTestcase("IsPointer(volatile string*)", true, tc_is_pointer<volatile string*>::value());
  evalTestcase("IsPointer(const string &)", false, tc_is_pointer<const string &>::value());
  evalTestcase("IsPointer(void*)", true, tc_is_pointer<void *>::value());

  evalTestcase("IsNullPointer(nullptr_t)", true, tc_is_null_pointer<nullptr_t>::value());
  evalTestcase("IsNullPointer(const nullptr_t)", true, tc_is_null_pointer<const nullptr_t>::value());
  evalTestcase("IsNullPointer(volatile nullptr_t)", true, tc_is_null_pointer<volatile nullptr_t>::value());
  evalTestcase("IsNullPointer(const volatile nullptr_t)", true,
      tc_is_null_pointer<const volatile nullptr_t>::value());
  evalTestcase("IsNullPointer(volatile const nullptr_t)", true,
      tc_is_null_pointer<volatile const nullptr_t>::value());


  evalTestcase("IsScalar(int)", true, tc_is_scalar<int>::value());;
  evalTestcase("IsScalar(int*)", true, tc_is_scalar<int*>::value());;
  evalTestcase("IsScalar(string*)", true, tc_is_scalar<string*>::value());;
  evalTestcase("IsScalar(const string*)", true, tc_is_scalar<const string*>::value());;
  evalTestcase("IsScalar(string&)", false, tc_is_scalar<string&>::value());;
  evalTestcase("IsScalar(class string)", false, tc_is_scalar<string>::value());;
  evalTestcase("IsScalar(enum fooEnum)", true, tc_is_scalar<fooEnum>::value());;
  evalTestcase("IsScalar(nullptr_t)", true, tc_is_scalar<nullptr_t>::value());;
  evalTestcase("IsScalar(const nullptr_t)", true, tc_is_scalar<const nullptr_t>::value());;

  cout << endl;
  cout << "Ran " << testRuns << " test cases." << endl;
  if (testFailures == 0) {
    cout << "Tests PASSED" << endl;
    return 0;
  } else {
    cout << "*** " << testFailures << " test cases FAILED." << endl;
    return 1;
  }
}
