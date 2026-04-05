// Copyright 2005, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// The Google C++ Testing and Mocking Framework (Google Test)
#include <cstddef>
#include <cstdint>
#include <algorithm>
#include <limits>
#include <type_traits>


namespace FromGoogleTest {


// This template class serves as a compile-time function from size to
// type.  It maps a size in bytes to a primitive type with that
// size. e.g.
//
//   TypeWithSize<4>::UInt
//
// is typedef-ed to be unsigned int (unsigned integer made up of 4
// bytes).
//
// Such functionality should belong to STL, but I cannot find it
// there.
//
// Google Test uses this class in the implementation of floating-point
// comparison.
//
// For now it only handles UInt (unsigned int) as that's all Google Test
// needs.  Other types can be easily added in the future if need
// arises.
template <size_t size>
class [[nodiscard]] TypeWithSize {
public:
  // This prevents the user from using TypeWithSize<N> with incorrect
  // values of N.
  using UInt = void;
};

// The specialization for size 4.
template <>
class [[nodiscard]] TypeWithSize<4> {
public:
  using Int = std::int32_t;
  using UInt = std::uint32_t;
};

// The specialization for size 8.
template <>
class [[nodiscard]] TypeWithSize<8> {
public:
  using Int = std::int64_t;
  using UInt = std::uint64_t;
};



template <typename RawType>
class [[nodiscard]] FloatingPoint {
public:
  // Defines the unsigned integer type that has the same size as the
  // floating point number.
  typedef typename TypeWithSize<sizeof(RawType)>::UInt Bits;

  // Constants.

  // # of bits in a number.
  static const size_t kBitCount = 8 * sizeof(RawType);

  // # of fraction bits in a number.
  static const size_t kFractionBitCount =
      std::numeric_limits<RawType>::digits - 1;

  // # of exponent bits in a number.
  static const size_t kExponentBitCount = kBitCount - 1 - kFractionBitCount;

  // The mask for the sign bit.
  static const Bits kSignBitMask = static_cast<Bits>(1) << (kBitCount - 1);

  // The mask for the fraction bits.
  static const Bits kFractionBitMask = ~static_cast<Bits>(0) >>
                                       (kExponentBitCount + 1);

  // The mask for the exponent bits.
  static const Bits kExponentBitMask = ~(kSignBitMask | kFractionBitMask);

  // How many ULP's (Units in the Last Place) we want to tolerate when
  // comparing two numbers.  The larger the value, the more error we
  // allow.  A 0 value means that two numbers must be exactly the same
  // to be considered equal.
  //
  // The maximum error of a single floating-point operation is 0.5
  // units in the last place.  On Intel CPU's, all floating-point
  // calculations are done with 80-bit precision, while double has 64
  // bits.  Therefore, 4 should be enough for ordinary use.
  //
  // See the following article for more details on ULP:
  // https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
  static const uint32_t kMaxUlps = 4;

  // Constructs a FloatingPoint from a raw floating-point number.
  //
  // On an Intel CPU, passing a non-normalized NAN (Not a Number)
  // around may change its bits, although the new value is guaranteed
  // to be also a NAN.  Therefore, don't expect this constructor to
  // preserve the bits in x when x is a NAN.
  explicit FloatingPoint(RawType x) { memcpy(&bits_, &x, sizeof(x)); }

  // Static methods

  // Reinterprets a bit pattern as a floating-point number.
  //
  // This function is needed to test the AlmostEquals() method.
  static RawType ReinterpretBits(Bits bits) {
    RawType fp;
    memcpy(&fp, &bits, sizeof(fp));
    return fp;
  }

  // Returns the floating-point number that represent positive infinity.
  static RawType Infinity() { return ReinterpretBits(kExponentBitMask); }

  // Non-static methods

  // Returns the bits that represents this number.
  const Bits& bits() const { return bits_; }

  // Returns the exponent bits of this number.
  Bits exponent_bits() const { return kExponentBitMask & bits_; }

  // Returns the fraction bits of this number.
  Bits fraction_bits() const { return kFractionBitMask & bits_; }

  // Returns the sign bit of this number.
  Bits sign_bit() const { return kSignBitMask & bits_; }

  // Returns true if and only if this is NAN (not a number).
  bool is_nan() const {
    // It's a NAN if the exponent bits are all ones and the fraction
    // bits are not entirely zeros.
    return (exponent_bits() == kExponentBitMask) && (fraction_bits() != 0);
  }

  // Returns true if and only if this number is at most kMaxUlps ULP's away
  // from rhs.  In particular, this function:
  //
  //   - returns false if either number is (or both are) NAN.
  //   - treats really large numbers as almost equal to infinity.
  //   - thinks +0.0 and -0.0 are 0 ULP's apart.
  bool AlmostEquals(const FloatingPoint& rhs) const {
    // The IEEE standard says that any comparison operation involving
    // a NAN must return false.
    if (is_nan() || rhs.is_nan()) return false;

    return DistanceBetweenSignAndMagnitudeNumbers(bits_, rhs.bits_) <= kMaxUlps;
  }

private:
  // Converts an integer from the sign-and-magnitude representation to
  // the biased representation.  More precisely, let N be 2 to the
  // power of (kBitCount - 1), an integer x is represented by the
  // unsigned number x + N.
  //
  // For instance,
  //
  //   -N + 1 (the most negative number representable using
  //          sign-and-magnitude) is represented by 1;
  //   0      is represented by N; and
  //   N - 1  (the biggest number representable using
  //          sign-and-magnitude) is represented by 2N - 1.
  //
  // Read https://en.wikipedia.org/wiki/Signed_number_representations
  // for more details on signed number representations.
  static Bits SignAndMagnitudeToBiased(Bits sam) {
    if (kSignBitMask & sam) {
      // sam represents a negative number.
      return ~sam + 1;
    } else {
      // sam represents a positive number.
      return kSignBitMask | sam;
    }
  }

  // Given two numbers in the sign-and-magnitude representation,
  // returns the distance between them as an unsigned number.
  static Bits DistanceBetweenSignAndMagnitudeNumbers(Bits sam1, Bits sam2) {
    const Bits biased1 = SignAndMagnitudeToBiased(sam1);
    const Bits biased2 = SignAndMagnitudeToBiased(sam2);
    return (biased1 >= biased2) ? (biased1 - biased2) : (biased2 - biased1);
  }

  Bits bits_;  // The bits that represent the number.
};


// Typedefs the instances of the FloatingPoint template class that we
// care to use.
typedef FloatingPoint<float> Float;
typedef FloatingPoint<double> Double;




template<typename T>
int8_t compareValues(T const& a, T const& b) noexcept
{
    if constexpr (!std::is_floating_point_v<T>) {
        return a > b ? 
            1 
            : 
            a < b ? -1 : 0;
    } else {
        /*
            Using googletest' compare function for equality (while not optimal)
            is the best method I have currently. 
            If you need something better than this,
            write your own specific comparison functions,
            and overload them for operator<, operator> and operator==
            See:
            https://stackoverflow.com/a/3423299
        */
        Double big_a{a}, big_b{b};
        auto greaterThan = (a - b) > ( std::max(std::abs(a), std::abs(b)) * 4 * std::numeric_limits<T>::epsilon());
        auto lessThan    = (b - a) > ( std::max(std::abs(a), std::abs(b)) * 4 * std::numeric_limits<T>::epsilon());
        auto equal       = big_a.AlmostEquals(big_b);
        if(greaterThan || a > b) {
            return 1;
        }
        if(lessThan || a < b) {
            return -1;
        } 
        if(equal) {
            return 0;
        }
        /* Floating Point comparison doesn't always work like you'd like it to */
        return INT8_MAX;
    }
}

template<typename T>
bool checkEqualValues(T const& a, T const& b) noexcept
{
    if constexpr (!std::is_floating_point_v<T>) {
        return a == b;
    } else {
      /* 
          AlmostEquals may not always hold. regardless, its good enough for most scenarios. 
          see:
          https://stackoverflow.com/a/13941383
      */
      Double big_a{a}, big_b{b};
      return big_a.AlmostEquals(big_b);
    }
}


} /* namespace FromGoogleTest */
