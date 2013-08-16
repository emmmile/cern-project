#ifndef SLOWREADER_HPP
#define SLOWREADER_HPP
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <unistd.h>
#include <sys/stat.h>
#include "dateutils.hpp"
#include "gui.hpp"

#include "mysql_connection.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

#include "correlation.hpp"
using namespace std;



///
/// \brief The slowreader class is basically a mysql client that downloads data from ntofdaq.cern.ch.
///
class slowreader {
        time_t start;
        time_t end;
        string dateformat;

        correlator* data;

        sql::Driver *driver;
        sql::Connection *con;
        sql::ResultSet *res;
        sql::Statement *stm;


        ///
        /// \brief Read a single table from the mysql server.
        /// \param table the table name.
        /// \param queryformat an convenience string that contains the query structure.
        ///
        void get ( const string& table, const string& queryformat );

public:
        ~slowreader ( );

        ///
        /// \param s starting time.
        /// \param e ending time.
        /// \param d a pointer to the correlator.
        ///
        slowreader(  time_t s, time_t e, correlator* d );

        ///
        /// \brief Reads al the SLOW data corresponding to the selected time window.
        /// The read date is also saved to ./output.
        ///
        /// \param url the URL of the mysql server. The default user used to access the databese is ntof, that requires no
        /// password.
        ///
        void read ( const string& url );
};

#endif // SLOWREADER_HPP
