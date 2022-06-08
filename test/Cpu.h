#ifndef MONEYTEST_H
#define MONEYTEST_H

#include <cppunit/extensions/HelperMacros.h>

class Cpu : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( Cpu );
  CPPUNIT_TEST( bitwise );
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void bitwise();
};

#endif  // MONEYTEST_H
