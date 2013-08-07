#ifndef GUI_HPP
#define GUI_HPP

#include <TGraph.h>
#include <TGClient.h>
#include <TRootEmbeddedCanvas.h>
#include <TGFrame.h>
#include <TMath.h>
#include <TGButton.h>
#include <TQObject.h>
#include <RQ_OBJECT.h>
#include <TCanvas.h>
#include <TTimer.h>
#include <TFile.h>
#include <TApplication.h>
#include <TGButtonGroup.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <string>
#include <vector>

// every C++11 object must be enclosed for safety reasons into this guard
// http://root.cern.ch/phpBB3/viewtopic.php?f=5&t=15858
// otherwise rootcint fails, at least for what concerns ROOT 5
#ifndef __CINT__
#include <mutex>
#include <map>
#include <set>
#endif
using namespace std;


///
/// \brief The gui class implements the main window GUI.
///
class gui : public TGMainFrame {
    TRootEmbeddedCanvas *ec;
    TGButtonGroup *bg;
    TTimer *t;
    int x; ///< the number of columns.
    int y; ///< the number of rows.


#ifndef __CINT__
    typedef map<string, TGraph*> map_type;
    typedef set<string> set_type;


    // the following datastructures has to be protected by a mutex
    // since they could be changed from other threads
    mutex plot_mutex;
    vector<TGCheckButton*> checkboxes;

    set_type choosen;
    map_type plot;
#endif
public:
    gui( const TGWindow *p, const string& start, const string& end, int w, int h ) : TGMainFrame( p, w, h ) {
        x = 3;
        y = 3;
        //frame = new TGMainFrame( p, w, h );

        t = new TTimer();
        TGHorizontalFrame* hframe = new TGHorizontalFrame(this, w, h);

        ec = new TRootEmbeddedCanvas( "ec", hframe, w-300, h );
        ec->GetCanvas()->Divide( x, y );

        TGVerticalFrame* vframe = new TGVerticalFrame(hframe, 300, h);
        TGCheckButton *automatic = new TGCheckButton(vframe, "&Automatic refresh" );
        TGTextButton *draw = new TGTextButton(vframe,"&Refresh" );
        TGTextButton *exit = new TGTextButton(vframe,"&Exit", "gApplication->Terminate(0)");
        bg = new TGButtonGroup(vframe,"Choose plots");

        vframe->AddFrame( automatic, new TGLayoutHints(kLHintsCenterX | kLHintsExpandX, 6,6,6,6 ) );
        vframe->AddFrame( draw, new TGLayoutHints(kLHintsCenterX | kLHintsExpandX, 6,6,6,6 ) );
        vframe->AddFrame( exit, new TGLayoutHints(kLHintsCenterX | kLHintsExpandX, 6,6,6,6 ) );
        vframe->AddFrame( bg, new TGLayoutHints( kLHintsCenterX | kLHintsExpandX, 6,6,6,6 ) );

        hframe->AddFrame( ec, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 6,6,6,6 ));
        hframe->AddFrame( vframe, new TGLayoutHints( kLHintsExpandY, 6,6,6,6 ) );
        this->AddFrame( hframe, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY ) );


        // Set a name to the main frame
        string title = "Plot view from " + start + " to " + end;
        this->SetWindowName( title.c_str() );

        // Map all subwindows of main frame
        this->MapSubwindows();

        // Initialize the layout algorithm
        this->Resize(this->GetDefaultSize());

        // Map main frame
        this->MapWindow();




        // Connections
        t->Connect("Timeout()", "gui", this, "refresh()");
        this->Connect("CloseWindow()", "gui", this, "exit()" );
        draw->Connect("Clicked()","gui",this,"refresh()");
        bg->Connect("Clicked(Int_t)", "gui", this, "handleCheck(Int_t)" );
        automatic->Connect("Toggled(Bool_t", "gui", this, "handleAutomaticRefresh(Bool_t)" );

    }

    virtual ~gui() {
        t->Stop();
        delete t;

        // Clean up used widgets: frames, buttons, layout hints
        this->Cleanup();
        delete ec;
    }

    void exit( ) {
        gApplication->Terminate(0);
    }

    void refresh( ) {
        //cout << "[gui] Refreshing plots..." << endl;
        draw();
    }

    void draw() {
        TCanvas *c = ec->GetCanvas();


#ifndef __CINT__
        plot_mutex.lock();

        int j = 0;
        for ( set_type::iterator i = choosen.begin(); i != choosen.end(); ++i, ++j ) {
            //cout << "[gui] " << *i << " is about to be drawn " << plot.count(*i) << "\n";


            TGraph* current = plot.at(*i);
            current->SetMarkerStyle(1);
            current->SetMarkerColor(j+1);
            current->SetLineColor(j+1);
            c->cd(j + 1);
            c->SetGridx();
            c->SetGridy();

            current->Draw("ALP");
        }

        for ( j; j < x * y; ++j ) {
            c->cd(j + 1)->Clear();
        }


        c->Update();


        plot_mutex.unlock();
#endif
    }

    void handleAutomaticRefresh( Bool_t on ) {
        cout << "[gui] automatic refresh is " << ( on ? "ON" : "OFF" ) << endl;

        if ( on ) t->Start(2000);
        else      t->Stop();
    }

    void handleCheck ( Int_t v ) {
        cout << "[gui] something clicked: " << v << endl;


        // since the checkboxes are dynamically created I can't create Signal/Slot connections
        // that has to be known at compile time. What I do is to handle the event at a higher level
        // that is at the button group level.

        // I basically see which are the checked boxes and if their number is less than the available
        // plots everything OK, they will be drawn.
        // Otherwise the checked box that exceed that number (that will be the last checked box) is
        // forced to be OFF.
#ifndef __CINT__
        set_type checked;

        plot_mutex.lock();
        for ( int i = 0; i < checkboxes.size(); ++i ) {
            if ( checkboxes[i]->IsDown() ) checked.insert( checkboxes[i]->GetString().Data() );
            //cout << checkboxes[i]->GetString() << " checked." << endl;
        }


        if ( checked.size() <= x * y ) {
            choosen = checked;
            cout << "[gui] " << choosen.size() << " plots will be drawn.\n";
        } else {
            for ( int i = 0; i < checkboxes.size(); ++i ) {
                // if the box is checked but is not in the choosen set (== it wasn't checked before)
                // I disable it
                if ( checkboxes[i]->IsDown() && choosen.count(checkboxes[i]->GetString().Data()) == 0 )
                    checkboxes[i]->SetState(kButtonUp);
            }
        }

        plot_mutex.unlock();
#endif
    }


    void addPoint( const string& name, float t, float y ) {
#ifndef __CINT__
        plot_mutex.lock();
        if ( plot.count( name ) == 0 ) {
            TGraph* newone = new TGraph();
            newone->SetTitle( name.c_str() );

            plot.insert( map_type::value_type( name, newone ) );

            // since a new plot has been added, update the checkbox area
            checkboxes.push_back( new TGCheckButton(bg, name.c_str(), 1 ) );
            bg->Show();
        }

        // add a point to the plot
        int n = plot[name]->GetN();
        //cout << "Adding " << name << " " << n << " " << t << " " << y << endl;
        plot[name]->SetPoint( n, t, y );


        plot_mutex.unlock();
#endif
    }

    ClassDef(gui,0);
};

#endif // GUI_HPP
