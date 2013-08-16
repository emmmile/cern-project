#include "dateutils.hpp"

string tostr ( const time_t& time, const string& format ) {
    struct tm tm = *localtime(&time);

    int bufsize = 100;
    char buf[bufsize];
    //if (strftime(buf,bufsize,"%d/%m/%y %R",&tm) != 0)
    if (strftime(buf,bufsize,format.c_str(),&tm) != 0)
        return string( buf );
    else
        throw runtime_error( "time conversion error (strftime)" );
}




time_t totime ( const string& time, const string& format ) {

    struct tm tmtime = { 0 };
    char* end = strptime(time.c_str(), format.c_str(), &tmtime);
    if ( end == NULL || *end != '\0' )
        throw runtime_error( "time conversion error (strptime)" );

    time_t out = mktime( &tmtime );

    if ( out == -1 )
        throw runtime_error( "time conversion error (mktime)" );

    return out;
}

