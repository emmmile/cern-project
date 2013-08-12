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
        string outdir;

        correlator* window;


        sql::Driver *driver;
        sql::Connection *con;
        sql::ResultSet *res;
        sql::Statement *stm;


        ///
        /// \brief Read a single table from the mysql server.
        /// \param table the table name.
        /// \param queryformat an convenience string that contains the query structure.
        ///
        void get ( const string& table, const string& queryformat ) {


                char query[512];
                string startdate = tostr( start, dateformat );
                string enddate = tostr( end, dateformat );

                stm = con->createStatement();


                sprintf(query, queryformat.c_str(), "*", table.c_str(), "PollDate", startdate.c_str(), "PollDate", enddate.c_str() );
                res = stm->executeQuery( query );

                fstream out ( outdir + table + ".dat", std::fstream::out );

                out << "#";
                // prints an header
                // start from 2 because the first column is always the ID
                sql::ResultSetMetaData* meta = res->getMetaData();
                for ( int i = 2; i <= meta->getColumnCount(); ++i )
                        out << meta->getColumnLabel(i) << "\t";
                out << endl;

                // prints the data rows
                while (res->next()) {
                        time_t current_time;
                        float  current_value;

                        for ( int i = 2; i <= meta->getColumnCount(); ++i ) {
                                if ( meta->getColumnLabel(i) == "PollDate" ) {
                                        current_time = totime( res->getString( meta->getColumnLabel(i) ), dateformat );
                                        out << current_time << "\t";
                                } else {
                                        current_value = atof( res->getString( meta->getColumnLabel(i) ).c_str() );
                                        out << res->getString( meta->getColumnLabel(i) ) << "\t";

                                        window->lock();
                                        window->addPoint( meta->getColumnLabel(i), current_time - start, current_value );
                                        window->unlock();
                                }
                        }

                        out << endl;
                }
                out << endl;

                delete stm;
                delete res;
        }

public:
        ~slowreader ( ) {
                //if ( driver ) delete driver;
                //if( con ) delete con;
        }

        ///
        /// \param s starting time.
        /// \param e ending time.
        /// \param w a pointer to the gui window.
        /// \param out path of the output directory.
        ///
        slowreader(  time_t s, time_t e, correlator* w, const string& out = "./output/" )
                : start( s ), end( e ), window( w ), outdir( out ) {
                dateformat = "%Y-%m-%d %H:%M:%S";
        }

        ///
        /// \brief Reads al the SLOW data corresponding to the selected time window.
        /// The read date is also saved to ./output.
        ///
        /// \param url the URL of the mysql server. The default user used to access the databese is ntof, that requires no
        /// password.
        ///
        void read ( const string& url ) {

                string queryformat = "SELECT %s FROM %s WHERE %s >= \"%s\" AND %s <= \"%s\"";

                string user = "ntof";
                string tables[3] = { "Protons", "Cooling", "Radiation" };

                try {
                        mkdir( outdir.c_str(), 0777 );
                        /* Create a connection */
                        driver = get_driver_instance();
                        con = driver->connect(url, user, "");

                        cout << "[slow] Successfully connected to " << url << "." << endl;
                        con->setSchema( "nTOFDAQ" );
                        for ( int i = 0; i < 3; ++i ) get( tables[i], queryformat );

                } catch (sql::SQLException &e) {
                        /*
                        MySQL Connector/C++ throws three different exceptions:

                        - sql::MethodNotImplementedException (derived from sql::SQLException)
                        - sql::InvalidArgumentException (derived from sql::SQLException)
                        - sql::SQLException (derived from std::runtime_error)
                        */
                        cerr << "# ERR: SQLException in " << __FILE__;
                        cerr << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
                        /* what() (derived from std::runtime_error) fetches error message */
                        cerr << "# ERR: " << e.what();
                        cerr << " (MySQL error code: " << e.getErrorCode();
                        cerr << ", SQLState: " << e.getSQLState() << " )" << endl;
                        con = NULL;
                }


                cerr << "[slow] Done." << endl;
        }
};

#endif // SLOWREADER_HPP
