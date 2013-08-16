

#include "slowreader.hpp"



void slowreader::get ( const string& table, const string& queryformat ) {

        char query[512];
        string startdate = tostr( start, dateformat );
        string enddate = tostr( end, dateformat );

        stm = con->createStatement();


        sprintf(query, queryformat.c_str(), "*", table.c_str(), "PollDate", startdate.c_str(), "PollDate", enddate.c_str() );
        res = stm->executeQuery( query ); // get the data

        sql::ResultSetMetaData* meta = res->getMetaData();

        while (res->next()) {
                time_t current_time;
                double current_value;

                // I start from 2 because the first column is always the ID of the record (not very interesting)
                for ( int i = 2; i <= meta->getColumnCount(); ++i ) {
                        // I do not make any check but the date has to be parsed BEFORE any other valye
                        // (check the structure of the tables in the mysql database)
                        if ( meta->getColumnLabel(i) == "PollDate" ) {
                                current_time = totime( res->getString( meta->getColumnLabel(i) ), dateformat );
                        } else {
                                current_value = atof( res->getString( meta->getColumnLabel(i) ).c_str() );

                                data->lock();
                                // send the value to the correlator
                                data->addPoint( meta->getColumnLabel(i), current_time - start, current_value );
                                data->unlock();
                        }
                }
        }

        delete stm;
        delete res;
}







slowreader::~slowreader ( ) {
        //if ( driver ) delete driver;
        //if( con ) delete con;
}


slowreader::slowreader(  time_t s, time_t e, correlator* d )
        : start( s ), end( e ), data( d ) {
        dateformat = "%Y-%m-%d %H:%M:%S";
}



void slowreader::read ( const string& url ) {
        // since the query structure is the same for all the tables, here it is
        string queryformat = "SELECT %s FROM %s WHERE %s >= \"%s\" AND %s <= \"%s\"";

        string user = "ntof";   // the user of the mysql database
        string tables[3] = { "Protons", "Cooling", "Radiation" }; // the tables to take

        try {
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


        cout << "[slow] Done." << endl;
}

