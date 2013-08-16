#ifndef DATEUTILS_HPP
#define DATEUTILS_HPP
#include <string>
#include <stdexcept>
using namespace std;

// from time to string
string tostr ( const time_t& time, const string& format );

// from string to time
time_t totime ( const string& time, const string& format );


#endif // DATEUTILS_HPP
