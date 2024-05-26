//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
#include "sqlite3.h"
#include "Unit1.h"
#include <vector> // ���������� vector
#include <string>
#include <cstdlib> // ��� rand � srand
#include <ctime>   // ��� time
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "VirtualTrees"
#pragma link "VirtualTrees.AncestorVCL"
#pragma link "VirtualTrees.BaseAncestorVCL"
#pragma link "VirtualTrees.BaseTree"
#pragma resource "*.dfm"
TForm1 *Form1;
//---------------------------------------------------------------------------


__fastcall TForm1::TForm1(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TForm1::VirtualStringTree1GetText(TBaseVirtualTree *Sender, PVirtualNode Node,
		  TColumnIndex Column, TVstTextType TextType, UnicodeString &CellText)
{   // ���������� ������� ��� ��������� ������ ��� ����������� � ���� ������
	TRowData *Data = static_cast<TRowData*>(Sender->GetNodeData(Node));
	if (Data)
	{
		switch (Column)
		{
			case 0: CellText = Data->Url; break;      //������� ������
			case 1: CellText = Data->Title; break;    //������� ��������
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Button1Click(TObject *Sender)
{  // ���������� ������� ��� ������, ������� ��������� ������ ��� ������ ����� ���� ������
   //FileOpenDialog1 �������(������� �� ������ ������)
   if (!FileOpenDialog1->Execute())
	{
		ShowMessage("���� �� ������"); // �������� ������������, ���� ���� �� ������
		return;
	}

	String dbPath = FileOpenDialog1->FileName;
	sqlite3 *db;
	sqlite3_stmt *stmt;
	int rc;

	rc = sqlite3_open(AnsiString(dbPath).c_str(), &db);  // ��������� ���� ������
	if (rc) //�������
	{
		ShowMessage("������ ��� �������� ���� ������: " + UnicodeString(sqlite3_errmsg(db)));
		return;
	}

	rc = sqlite3_prepare_v2(db, "SELECT url, title, visit_count FROM urls", -1, &stmt, NULL);
	if (rc != SQLITE_OK) //�������
    {
        ShowMessage("������ ��� ���������� �������: " + UnicodeString(sqlite3_errmsg(db)));
		sqlite3_close(db);
		return;
    }

	VirtualStringTree1->Clear();   //������� ������
	VirtualStringTree1->NodeDataSize = sizeof(TRowData);  // ������������� ������ ������ ����

	VirtualStringTree1->Header->Columns->Clear(); // ������� ��������� ��������
	VirtualStringTree1->Header->Options = VirtualStringTree1->Header->Options << hoVisible; // ������������� ��������� ���������

	VirtualStringTree1->Header->Columns->Add();
	VirtualStringTree1->Header->Columns->Items[0]->Text = "URL";  //��������� �������

	VirtualStringTree1->Header->Columns->Add();
	VirtualStringTree1->Header->Columns->Items[1]->Text = "Title";  //��������� �������, 3 ������� �� ��������, ��� ��� ��� ������� ������ AddToSel...

	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)   // ��������� ������ � ������������ ������ ������ ����������
	{
		PVirtualNode Node = VirtualStringTree1->AddChild(nullptr);
		TRowData *Data = static_cast<TRowData*>(VirtualStringTree1->GetNodeData(Node));
		if (Data)
		{
			Data->Url = UTF8ToWideString(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));  //���� 1 (0)
			Data->Title = UTF8ToWideString(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))); //���� 2
			Data->VisitCount = sqlite3_column_int(stmt, 2); // ���� 3
		}
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);  //��������� ����������
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Button2Click(TObject *Sender)
{
    PVirtualNode SelectedNode = VirtualStringTree1->GetFirstSelected();
	if (SelectedNode)
	{
		TRowData *Data = static_cast<TRowData*>(VirtualStringTree1->GetNodeData(SelectedNode));
        if (Data)
		{
            sqlite3 *db;
			int rc;

            String dbPath = FileOpenDialog1->FileName;
			rc = sqlite3_open(AnsiString(dbPath).c_str(), &db);
			if (rc)
            {
				ShowMessage("������ ��� �������� ���� ������: " + UnicodeString(sqlite3_errmsg(db))); //�������
				return;
            }

            sqlite3_stmt *stmt;
			rc = sqlite3_prepare_v2(db, "DELETE FROM urls WHERE url = ?", -1, &stmt, NULL);
			if (rc != SQLITE_OK)
            {
				ShowMessage("������ ��� ���������� �������: " + UnicodeString(sqlite3_errmsg(db))); //�������
				sqlite3_close(db);
				return;
            }

			sqlite3_bind_text(stmt, 1, AnsiString(Data->Url).c_str(), -1, SQLITE_STATIC);
			//1 �������� ������ Data->Url � ������� ��������� (?) � �������������� SQL-������� stmt.
			//2 Data->Url � AnsiString, ����� �������� ������ � ��������� ANSI.
			//3 ����� .c_str(), ����� �������� ������ ��� ��������� �� char.
			//4, ��� ������ ��������� ������� �������� (\0), ������� �� ����� ������������ �������������.
			//5, ��� ������ ����������� ������������� � �� ����� ����������� SQLite (SQLITE_STATIC).

			rc = sqlite3_step(stmt);
			if (rc != SQLITE_DONE)
			{
				ShowMessage("������ ��� �������� ������: " + UnicodeString(sqlite3_errmsg(db))); //�������
			}
			else
			{
				VirtualStringTree1->DeleteNode(SelectedNode);  //�������
			}

			sqlite3_finalize(stmt);
			sqlite3_close(db);
		}
	}
	else
	{
		ShowMessage("��� ���������� ������ ��� ��������");  //�������
	}
}
//--------------------------------------------------------------------------
//---------------------------------------------------------------------------
void __fastcall TForm1::VirtualStringTree1AddToSelection(TBaseVirtualTree *Sender,
		  PVirtualNode Node)
{    // ���������� �������, ������� ����������� ��� ���������� ���� � ���������
	TRowData *Data = static_cast<TRowData*>(Sender->GetNodeData(Node));
	if (Data) //���������, ��� Data �� ����� nullptr
	{
		ShowMessage("���������� ���������: " + IntToStr(Data->VisitCount)); //��������� � ������
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button3Click(TObject *Sender)
{
  // ������� VirtualStringTree ����� ����������� ����� ������
	VirtualStringTree2->Clear();
	VirtualStringTree2->NodeDataSize = sizeof(Drone*);
	VirtualStringTree2->Header->Options = VirtualStringTree2->Header->Options << hoVisible;

	VirtualStringTree2->Header->Columns->Add();
	VirtualStringTree2->Header->Columns->Items[0]->Text = "Model";  //��������� �������

	VirtualStringTree2->Header->Columns->Add();
	VirtualStringTree2->Header->Columns->Items[1]->Text = "Weight";

    VirtualStringTree2->Header->Columns->Add();
	VirtualStringTree2->Header->Columns->Items[2]->Text = "Battery Level";

	VirtualStringTree2->Header->Columns->Add();
	VirtualStringTree2->Header->Columns->Items[3]->Text = "Max Battery";
  // �������� ���������� ������
	DroneContainer droneContainer;
	srand(time(0));
	// ���������� ���������� ���������� �������
	for (int i = 0; i < 1000; ++i) {
		string name = "Drone " + to_string(i + 1);
		int batteryLevel = rand() % 101; // ��������� ������� ������ ������� �� 0 �� 100
		int maxBattery = 100;
		int weight = rand() % 101;
		Drone drone(name, weight, batteryLevel, maxBattery);
		droneContainer.AddDrone(drone);
	}

	// �������� ��� ������ ���������� ������
	DroneContainer::Iterator it(droneContainer);

	// ������� ProgressBar ����� ������� ������
	ProgressBar1->Position = 0;
	ProgressBar1->Max = 1000; // ��� 10 ������

	// ����� ���������� � ����� ������ �� VirtualStringTree
	while (it.HasNext()) {
		  Drone drone = it.Next();
		// ���������� ����� � VirtualStringTree
		PVirtualNode newNode = VirtualStringTree2->AddChild(NULL);
		Drone *dronePtr = new Drone(drone); // �������� �����, ����� ��������� ��� � ���������������� ��������� ����
		Drone **nodeData = static_cast<Drone**>(VirtualStringTree2->GetNodeData(newNode));
		*nodeData = dronePtr; // ��������� ����� � ���������������� ��������� ����

		// ���������� ProgressBar
		ProgressBar1->StepBy(1);
		Application->ProcessMessages(); // ���������� ����������
	}
}
//---------------------------------------------------------------------------


void __fastcall TForm1::VirtualStringTree2GetText(TBaseVirtualTree *Sender, PVirtualNode Node,
		  TColumnIndex Column, TVstTextType TextType, UnicodeString &CellText)

{
	// ���������, ���������� �� ���� � ��������� �� ������ �������
	if (Node) {
		// �������� ������ � �����, ������� �� ��������� � ���������������� ��������� ����
		Drone **nodeData = static_cast<Drone**>(VirtualStringTree2->GetNodeData(Node));
		Drone *drone = *nodeData;

		// ���������� ������ � ����� � ������ ������
		if (drone) {
			switch (Column) {
				case 0:
                    CellText = drone->name.c_str(); // ��� �����
					break;
				case 1:
					CellText =  UnicodeString(drone->weight);
					break;
				case 2:
					CellText = UnicodeString(drone->batteryLevel); // ������� ������ �������
					break;
				case 3:
					CellText = UnicodeString(drone->maxBattery);
					break;
			}
		}
	}
}
//---------------------------------------------------------------------------

