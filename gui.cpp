#include "gui.hpp"


gui::gui( const TGWindow *p, const string& start, const string& end, int w, int h, int x, int y, correlator* holder )
        : TGMainFrame( p, w, h ), x( x ), y( y ), data( holder ) {
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

        // start with automatic refresh ON
        automatic->SetState(kButtonDown);
        automatic->Toggled(true);
}

gui::~gui() {
        t->Stop();
        delete t;

        // Clean up used widgets: frames, buttons, layout hints
        this->Cleanup();
        delete ec;
}

void gui::exit( ) {
        gApplication->Terminate(0);
}

void gui::refresh( ) {
        boost::timer t;
        draw( );
        cout << "[gui] Refreshed in " << t.elapsed() << endl;
}

void gui::handleSelection ( Int_t v ) {
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

void gui::draw() {
        TCanvas *c = ec->GetCanvas();

        // updates the list box
        data->lock();

        for ( int i = 0; i < data->plotsnumber(); ++i ) {
                string name = data->name( i );
                // if an entry is already present, just skip it
                // in this way the display order is the same of the one present in the correlator
                if ( listbox->FindEntry( name.c_str() ) != NULL ) continue;
                listbox->AddEntry( name.c_str(), i );
                listbox->Layout(); // refresh the listbox
        }



        // draw the plots starting from the first canvas
        TList selected;
        listbox->GetSelectedEntries( &selected );

        int j = 1;
        TGLBEntry* entry;
        // loop over the selected entries
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

void gui::handleAutomaticRefresh( Bool_t on ) {
        cout << "[gui] automatic refresh is " << ( on ? "ON" : "OFF" ) << endl;

        if ( on ) t->Start(2000);
        else      t->Stop();
}

