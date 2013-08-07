
#include <boost/program_options.hpp>
#include <fast/fastreader.hpp>
#include <slow/slowreader.hpp>
#include <thread>
#include <string>
#include <sstream>
#include "gui.hpp"
#include "fast/peakanalyzer.hpp"
using namespace std;
using namespace boost::program_options;


int main ( int argc, char** argv ) {
//    peakanalyzer p( "boh" );
//    fstream sample( "../project/fast/sample data/SCIN4_r15185_segm0_ev44_s1.dat");
//    if ( sample ) p.analyze( sample );


    string start;
    string end;
    string format = "%d/%m/%Y %H:%M";
    string localdb = "NTOFDB";
    time_t starttime, endtime;
    string dburl = "ntofdaq.cern.ch";


    // Declare the supported program options
    options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "produce this help message")
        ("format,f", value<string>(&format), "set date format")
        ("start,s", value<string>(&start), "set starting date")
        ("end,e", value<string>(&end), "set ending date")
        ("localdb,l", value<string>(&localdb), "set location of the local CASTOR db")
        ("db,d",value<string>(&dburl),"set the url for the SLOW data db")
    ;

    variables_map vm;

    try {
        store(parse_command_line(argc, argv, desc), vm);

        if (vm.count("help")) {
            cout << desc << "\n";
            exit( 1 );
        }


        notify(vm);
    } catch ( error& e ) {
        cerr << desc << endl;
        cerr << e.what() << endl;
        exit( 1 );
    }


    if (!vm.count("start")) {
        cerr << desc << endl;
        cerr << "option --start is required.\n";
        exit( 1 );
    }


    if (!vm.count("end")) {
        cerr << desc << endl;
        cerr << "option --end is required.\n";
        exit( 1 );
    }


    try {
        starttime = totime( start, format );
    } catch ( exception e ) {
        cerr << "starting date \"" << start << "\" is not valid for format " << format << ".\n";
        exit( 1 );
    }

    try {
        endtime = totime( end, format );
    } catch ( exception e ) {
        cerr << "ending date \"" << end << "\" is not valid for format " << format << ".\n";
        exit( 1 );
    }


    fstream dbfile( localdb );
    if ( !dbfile ) {
        cerr << "local database file \"" << localdb << "\" not found.\n";
        exit( 1 );
    }





    TApplication app( "Viewer", &argc, argv );
    gui* window = new gui( gClient->GetRoot(), start, end, 1440, 900 );

    fastreader freader( starttime, endtime, window );
    slowreader sreader( starttime, endtime, window );

    thread fthread( &fastreader::read, freader, std::ref( dbfile ) );
    thread sthread( &slowreader::read, sreader, std::ref( dburl ) );

    app.Run();
    sthread.join();
    fthread.join();

    return 0;
}
