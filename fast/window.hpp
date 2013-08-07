#ifndef WINDOW_HPP
#define WINDOW_HPP
#include <vector>
#include <numeric>
#include <iostream>
using namespace std;

typedef unsigned int uint;


///
/// \class window
/// \brief This class represents a time window of values coming from a detector. It used, while analyzing the date,
/// to remember a part of the data without having to read it again.
///
template<class F>
class window {
  vector<F> __times;
  vector<F> __values;
  vector<F> __derivatives;

public:
///
/// Adds a triple to the window
///
  void push ( const F& time, const F& value, const F& dv ) {
    __times.push_back ( time );
    __values.push_back ( value );
    __derivatives.push_back ( dv );
  }

///
/// Clears the window.
///
  void clear ( ) {
    __times.clear();
    __values.clear();
    __derivatives.clear();
  }

  uint size ( ) const {
    return __times.size();
  }


///
/// \return the integral, referred to the argument baseline of the values present in the window.
///
  float integral ( float baseline ) const {
    float out = 0.0;
    for ( uint i = 1; i < __times.size(); ++i ) {
      out += fabs( baseline - __values[i] ) * ( __times[i] - __times[i - 1] );
    }

    return out;
  }

  ///
  /// \return a lower bound for the integral in the current window, to indentify a peak.
  /// It is calculated as the area of a rectangle having width equal to the window and height of 1/4 the
  /// maximum peak (computed with \ref minimum).
  ///
  float minimum_integral( ) const {
    return fabs ( ( __values.back() - __values.front() ) * minimum() * 0.25 );
  }

  ///
  /// \return the minumum value in the window.
  ///
  float minimum ( ) const {
    return *min_element ( __values.begin(), __values.end() );
  }

  ///
  /// \return the time instant correspondent to the minimum value.
  ///
  float minimum_time ( ) const {
    return __times[min_element ( __values.begin(), __values.end() ) - __values.begin()];
  }

  friend ostream& operator << ( ostream& out, const window& w ) {
    for ( uint i = 0; i < w.__times.size(); ++i )
      out << w.__times[i] << " " << w.__values[i] << " " << w.__derivatives[i] << endl;

    //out << "Total integral is " << w.integral() << endl;
    out << "Minimum is " << w.minimum() << endl;
    return out;
  }
};

#endif // WINDOW_HPP
