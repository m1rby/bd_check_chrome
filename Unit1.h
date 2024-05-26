#ifndef Unit1H
#define Unit1H
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include "VirtualTrees.AncestorVCL.hpp"
#include "VirtualTrees.BaseAncestorVCL.hpp"
#include "VirtualTrees.BaseTree.hpp"
#include "VirtualTrees.hpp"
#include "sqlite3.h"
#include <Vcl.ComCtrls.hpp>
#include <Vcl.Dialogs.hpp>
#include <vector> // Add this include
#include <string> // Add this include
#include <Vcl.Dialogs.hpp>
#include <Vcl.ComCtrls.hpp>
using namespace std; // Добавьте эту директиву
//---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published:    // IDE-managed Components
	TButton *Button1;
	TVirtualStringTree *VirtualStringTree1;
	TButton *Button2;
    TFileOpenDialog *FileOpenDialog1;
	TVirtualStringTree *VirtualStringTree2;
	TButton *Button3;
	TProgressBar *ProgressBar1;
	void __fastcall VirtualStringTree1GetText(TBaseVirtualTree *Sender, PVirtualNode Node,
		  TColumnIndex Column, TVstTextType TextType, UnicodeString &CellText);
	void __fastcall Button1Click(TObject *Sender);
	void __fastcall Button2Click(TObject *Sender);
	void __fastcall VirtualStringTree1AddToSelection(TBaseVirtualTree *Sender, PVirtualNode Node);
	void __fastcall Button3Click(TObject *Sender);
	void __fastcall VirtualStringTree2GetText(TBaseVirtualTree *Sender, PVirtualNode Node,
		  TColumnIndex Column, TVstTextType TextType, UnicodeString &CellText);




private:    // User declarations
public:        // User declarations
	__fastcall TForm1(TComponent* Owner);
    struct TRowData
    {
        UnicodeString Url;
        UnicodeString Title;
        int VisitCount;
    };


};
class Drone {
public:
	std::string name; // Use std::string instead of string
	int batteryLevel;
	int weight;
	int maxBattery;

	Drone(std::string _name, int _batteryLevel, int _weight, int _maxBattery)
		: name(_name), batteryLevel(_batteryLevel), weight(_weight), maxBattery(_maxBattery) {};
};

// Container for vector of drones
class DroneContainer {
private:
    std::vector<Drone> drones; // Use std::vector instead of vector

public:
    void AddDrone(const Drone& drone) {
        drones.push_back(drone);
    }

	// Iterator to traverse the drone container
    class Iterator {
    private:
        const DroneContainer& container;
        int currentIndex;

    public:
        Iterator(const DroneContainer& _container) : container(_container), currentIndex(0) {}

		bool HasNext() const {
            return currentIndex < container.drones.size();
        }

        Drone Next() {
            return container.drones[currentIndex++];
        }
    };

    // Clear the container
    void Clear() {
        drones.clear();
    }
};

//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
