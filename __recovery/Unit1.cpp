//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
#include "sqlite3.h"
#include "Unit1.h"
#include <vector> // Подключаем vector
#include <string>
#include <cstdlib> // Для rand и srand
#include <ctime>   // Для time
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
{   // Обработчик события для получения текста для отображения в узле дерева
	TRowData *Data = static_cast<TRowData*>(Sender->GetNodeData(Node));
	if (Data)
	{
		switch (Column)
		{
			case 0: CellText = Data->Url; break;      //столбец ссылок
			case 1: CellText = Data->Title; break;    //столбец названий
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Button1Click(TObject *Sender)
{  // Обработчик события для кнопки, которая открывает диалог для выбора файла базы данных
   if (!FileOpenDialog1->Execute())
    {
        ShowMessage("Файл не выбран");
        return;
    }

    String dbPath = FileOpenDialog1->FileName;
    VirtualStringTree1->Clear();
    VirtualStringTree1->NodeDataSize = sizeof(TRowData);
    VirtualStringTree1->Header->Columns->Clear();
    VirtualStringTree1->Header->Options = VirtualStringTree1->Header->Options << hoVisible;
    VirtualStringTree1->Header->Columns->Add()->Text = "URL";
	VirtualStringTree1->Header->Columns->Add()->Text = "Title";

    TDBThread *dbThread = new TDBThread(this, dbPath);
	dbThread->Start();
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
                ShowMessage("Ошибка при открытии базы данных: " + UnicodeString(sqlite3_errmsg(db)));
				return;
            }

			sqlite3_stmt *stmt;
            rc = sqlite3_prepare_v2(db, "DELETE FROM urls WHERE url = ?", -1, &stmt, NULL);
			if (rc != SQLITE_OK)
            {
				ShowMessage("Ошибка при выполнении запроса: " + UnicodeString(sqlite3_errmsg(db)));
				sqlite3_close(db);
				return;
            }

            sqlite3_bind_text(stmt, 1, AnsiString(Data->Url).c_str(), -1, SQLITE_STATIC);

            rc = sqlite3_step(stmt);
            if (rc != SQLITE_DONE)
            {
                ShowMessage("Ошибка при удалении записи: " + UnicodeString(sqlite3_errmsg(db)));
			}
            else
            {
                VirtualStringTree1->DeleteNode(SelectedNode);
            }

            sqlite3_finalize(stmt);
            sqlite3_close(db);
		}
    }
    else
    {
        ShowMessage("Нет выделенной записи для удаления");
	}
}
//--------------------------------------------------------------------------
//---------------------------------------------------------------------------
void __fastcall TForm1::VirtualStringTree1AddToSelection(TBaseVirtualTree *Sender,
		  PVirtualNode Node)
{    // Обработчик события, который срабатывает при добавлении узла в выделение
	TRowData *Data = static_cast<TRowData*>(Sender->GetNodeData(Node));
	if (Data) //Проверяем, что Data не равно nullptr
	{
		ShowMessage("Количество посещений: " + IntToStr(Data->VisitCount)); //переводим в строку
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button3Click(TObject *Sender)
{
  // Очистка VirtualStringTree перед добавлением новых данных
	VirtualStringTree2->Clear();
	VirtualStringTree2->NodeDataSize = sizeof(Drone*);
	VirtualStringTree2->Header->Options = VirtualStringTree2->Header->Options << hoVisible;

	VirtualStringTree2->Header->Columns->Add();
	VirtualStringTree2->Header->Columns->Items[0]->Text = "Model";  //заголовок столбца

	VirtualStringTree2->Header->Columns->Add();
	VirtualStringTree2->Header->Columns->Items[1]->Text = "Weight";

    VirtualStringTree2->Header->Columns->Add();
	VirtualStringTree2->Header->Columns->Items[2]->Text = "Battery Level";

	VirtualStringTree2->Header->Columns->Add();
	VirtualStringTree2->Header->Columns->Items[3]->Text = "Max Battery";
	TDroneThread *droneThread = new TDroneThread(this);
	droneThread->Start();
}
//---------------------------------------------------------------------------


void __fastcall TForm1::VirtualStringTree2GetText(TBaseVirtualTree *Sender, PVirtualNode Node,
		  TColumnIndex Column, TVstTextType TextType, UnicodeString &CellText)

{
	// Проверяем, существует ли узел и правильно ли указан столбец
	if (Node) {
		// Получаем данные о дроне, которые мы сохранили в пользовательском указателе узла
		Drone **nodeData = static_cast<Drone**>(VirtualStringTree2->GetNodeData(Node));
		Drone *drone = *nodeData;

		// Записываем данные о дроне в ячейку текста
		if (drone) {
			switch (Column) {
				case 0:
                    CellText = drone->name.c_str(); // Имя дрона
					break;
				case 1:
					CellText =  UnicodeString(drone->weight);
					break;
				case 2:
					CellText = UnicodeString(drone->batteryLevel); // Уровень заряда батареи
					break;
				case 3:
					CellText = UnicodeString(drone->maxBattery);
					break;
			}
		}
	}
}

__fastcall TDBThread::TDBThread(TForm1 *Form, String dbPath)
    : TThread(true), Form(Form), dbPath(dbPath)
{
    FreeOnTerminate = true;
}

void __fastcall TDBThread::Execute()
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;

    rc = sqlite3_open(AnsiString(dbPath).c_str(), &db);
    if (rc)
    {
        Synchronize([=] { ShowMessage("Ошибка при открытии базы данных: " + UnicodeString(sqlite3_errmsg(db))); });
		return;
    }

    rc = sqlite3_prepare_v2(db, "SELECT url, title, visit_count FROM urls", -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        Synchronize([=] { ShowMessage("Ошибка при выполнении запроса: " + UnicodeString(sqlite3_errmsg(db))); });
        sqlite3_close(db);
        return;
	}

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        Synchronize([=] {
            PVirtualNode Node = Form->VirtualStringTree1->AddChild(nullptr);
			TRowData *Data = static_cast<TRowData*>(Form->VirtualStringTree1->GetNodeData(Node));
            if (Data)
            {
				Data->Url = UTF8ToWideString(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
                Data->Title = UTF8ToWideString(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
                Data->VisitCount = sqlite3_column_int(stmt, 2);
            }
        });
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    Synchronize(&UpdateUI);
}
void __fastcall TDBThread::UpdateUI()
{
    Form->VirtualStringTree1->Invalidate();
}

__fastcall TDroneThread::TDroneThread(TForm1 *Form)
	: TThread(true), Form(Form)
{
    FreeOnTerminate = true;
}

void __fastcall TDroneThread::Execute()
{
    DroneContainer droneContainer;
    srand(time(0));
	for (int i = 0; i < 1000; ++i) {
        string name = "Drone " + to_string(i + 1);
        int batteryLevel = rand() % 101;
        int maxBattery = 100;
        int weight = rand() % 101;
        Drone drone(name, weight, batteryLevel, maxBattery);
        droneContainer.AddDrone(drone);
    }

	DroneContainer::Iterator it(droneContainer);
    int count = 0;

	while (it.HasNext()) {
        Drone drone = it.Next();

        Synchronize([=] {
            PVirtualNode newNode = Form->VirtualStringTree2->AddChild(nullptr);
            Drone *dronePtr = new Drone(drone);
			Drone **nodeData = static_cast<Drone**>(Form->VirtualStringTree2->GetNodeData(newNode));
            *nodeData = dronePtr;

            Form->ProgressBar1->StepBy(1);
        });

		count++;
		if (count % 10 == 0) { // Обновляем прогресс каждые 10 итераций
            Synchronize([=] {
				Form->ProgressBar1->Position = count;
			});
		}
	}

    Synchronize(&UpdateUI);
}

void __fastcall TDroneThread::UpdateUI()
{
    Form->VirtualStringTree2->Invalidate();
    Form->ProgressBar1->Position = Form->ProgressBar1->Max; // Устанавливаем прогресс-бар на максимум
}
//---------------------------------------------------------------------------

