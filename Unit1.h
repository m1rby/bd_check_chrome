//---------------------------------------------------------------------------

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
#include <Vcl.Dialogs.hpp>
//---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published:	// IDE-managed Components
	TButton *Button1;
	TVirtualStringTree *VirtualStringTree1;
	TButton *Button2;
	TFileOpenDialog *FileOpenDialog1;
	void __fastcall VirtualStringTree1GetText(TBaseVirtualTree *Sender, PVirtualNode Node,
		  TColumnIndex Column, TVstTextType TextType, UnicodeString &CellText);
	void __fastcall Button1Click(TObject *Sender);
	void __fastcall Button2Click(TObject *Sender);
	void __fastcall VirtualStringTree1AddToSelection(TBaseVirtualTree *Sender, PVirtualNode Node);

private:	// User declarations
public:		// User declarations
	__fastcall TForm1(TComponent* Owner);
	struct TRowData
	{
		UnicodeString Url;
		UnicodeString Title;
		int VisitCount;
	};


};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
