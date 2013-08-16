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
/// The most tricky part is the signal/slot connection mechanism.
/// For details take a loot at http://root.cern.ch/root/CintGenerator.html
///
class gui : public TGMainFrame {
        TRootEmbeddedCanvas*    ec;
        TGListBox*              listbox;
        TTimer*                 t;
        int                     x;                  ///< the number of columns.
        int                     y;                  ///< the number of rows.

        correlator*             data;
public:
        gui( const TGWindow *p, const string& start, const string& end, int w, int h, int x, int y, correlator* holder );

        virtual ~gui();
        void exit( );
        void draw();
        void refresh( );
        void handleSelection ( Int_t v );
        void handleAutomaticRefresh( Bool_t on );

        ClassDef(gui,0);
};

#endif // GUI_HPP
