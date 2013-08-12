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
#include <TList.h>
#include <TApplication.h>
#include <TGListBox.h>
#include <TGButtonGroup.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <string>
#include <vector>
#include "correlation.hpp"


using namespace std;


///
/// \brief The gui class implements the main window GUI.
///
class gui : public TGMainFrame {
        TRootEmbeddedCanvas *ec;
        //TGButtonGroup *bg;
        TGListBox *listbox;
        TTimer *t;
        int x; ///< the number of columns.
        int y; ///< the number of rows.

        correlator* data;
public:
        gui( const TGWindow *p, const string& start, const string& end, int w, int h, correlator* holder )
                : TGMainFrame( p, w, h ), data( holder ) {
                x = 3;
                y = 3;
                //frame = new TGMainFrame( p, w, h );
                int lateralFrameWidth = 200;

                t = new TTimer();
                TGHorizontalFrame* hframe = new TGHorizontalFrame(this, w, h);

                ec = new TRootEmbeddedCanvas( "ec", hframe, w - lateralFrameWidth, h );
                ec->GetCanvas()->Divide( x, y );

                // creates and add the buttons
                TGVerticalFrame* vframe = new TGVerticalFrame(hframe, lateralFrameWidth, h);
                TGCheckButton *automatic = new TGCheckButton(vframe, "&Automatic refresh" );
                TGTextButton *draw = new TGTextButton(vframe,"&Refresh" );
                TGTextButton *exit = new TGTextButton(vframe,"&Exit", "gApplication->Terminate(0)");
                vframe->AddFrame( automatic, new TGLayoutHints(kLHintsCenterX | kLHintsExpandX, 6,6,6,6 ) );
                vframe->AddFrame( draw, new TGLayoutHints(kLHintsCenterX | kLHintsExpandX, 6,6,6,6 ) );
                vframe->AddFrame( exit, new TGLayoutHints(kLHintsCenterX | kLHintsExpandX, 6,6,6,6 ) );
                //vframe->AddFrame( bg, new TGLayoutHints( kLHintsCenterX | kLHintsExpandX, 6,6,6,6 ) );

                // creates and add the list box
                listbox = new TGListBox(vframe);
                listbox->SetMultipleSelections();
                //int usedSpace = automatic->GetHeight() + draw->GetHeight() + exit->GetHeight();
                listbox->Resize(vframe->GetWidth(), vframe->GetHeight() );
                vframe->AddFrame(listbox,new TGLayoutHints(kLHintsTop | kLHintsExpandY | kLHintsLeft,6,6,6,6));

                // finally add the two layouts (one for the plots, one for the buttons and stuff) to the main widget
                hframe->AddFrame( ec, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 6,6,6,6 ));
                hframe->AddFrame( vframe, new TGLayoutHints( kLHintsExpandY, 6,6,6,6 ) );
                this->AddFrame( hframe, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY ) );


                // Set a name to the main frame
                string title = "View from " + start + " to " + end;
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
                listbox->Connect("Selected(Int_t)", "gui", this, "handleSelection(Int_t)" );
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
                cout << "[gui] Refreshing..." << endl;
                draw();
        }

        void handleSelection ( Int_t v ) {
                data->lock();
                //cout << "[gui] selected entry " << data->name(v) << endl;
                TList selected;

                listbox->GetSelectedEntries( &selected );
                int selectedentries = selected.GetSize();

                // I don't want more than x * y entries selected
                if ( selectedentries == x * y + 1 )
                        listbox->GetEntry( v )->Activate( false );

                data->unlock();
        }

        void draw() {
                TCanvas *c = ec->GetCanvas();

                // updates the list box
                data->lock();

                for ( int i = 0; i < data->plotsnumber(); ++i ) {
                        string name = data->name( i );
                        // if an entry is already present, just skip it
                        // in this way the display order is the same of the one present in data
                        if ( listbox->FindEntry( name.c_str() ) != NULL ) continue;
                        listbox->AddEntry( name.c_str(), i );
                        listbox->Layout();
                }



                // draw the plots starting from the first canvas
                TList selected;
                listbox->GetSelectedEntries( &selected );

                int j = 1;
                TGLBEntry* entry;
                TIterator* i = selected.MakeIterator();
                while( entry = (TGLBEntry*) i->Next() ) {
                        int index = entry->EntryId();
                        TGraph* current = data->plot( index );

                        //current->SetMarkerStyle(1);
                        current->SetMarkerColor(j);
                        current->SetLineColor(j);
                        c->cd(j);
                        c->SetGridx();
                        c->SetGridy();

                        current->Draw("AL");
                        ++j;
                }

                data->unlock();


                // clear the remaining spaces
                for ( j; j <= x * y; ++j ) c->cd(j)->Clear();


                c->Update();
        }

        void handleAutomaticRefresh( Bool_t on ) {
                cout << "[gui] automatic refresh is " << ( on ? "ON" : "OFF" ) << endl;

                if ( on ) t->Start(2000);
                else      t->Stop();
        }

        ClassDef(gui,0);
};

#endif // GUI_HPP
