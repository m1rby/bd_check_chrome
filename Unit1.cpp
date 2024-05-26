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
   //FileOpenDialog1 отладка(оставлю на всякий случай)
   if (!FileOpenDialog1->Execute())
	{
		ShowMessage("Файл не выбран"); // Сообщаем пользователю, если файл не выбран
		return;
	}

	String dbPath = FileOpenDialog1->FileName;
	sqlite3 *db;
	sqlite3_stmt *stmt;
	int rc;

	rc = sqlite3_open(AnsiString(dbPath).c_str(), &db);  // Открываем базу данных
	if (rc) //отладка
	{
		ShowMessage("Ошибка при открытии базы данных: " + UnicodeString(sqlite3_errmsg(db)));
		return;
	}

	rc = sqlite3_prepare_v2(db, "SELECT url, title, visit_count FROM urls", -1, &stmt, NULL);
	if (rc != SQLITE_OK) //отладка
    {
        ShowMessage("Ошибка при выполнении запроса: " + UnicodeString(sqlite3_errmsg(db)));
		sqlite3_close(db);
		return;
    }

	VirtualStringTree1->Clear();   //очищаем дерево
	VirtualStringTree1->NodeDataSize = sizeof(TRowData);  // Устанавливаем размер данных узла

	VirtualStringTree1->Header->Columns->Clear(); // Очищаем заголовки столбцов
	VirtualStringTree1->Header->Options = VirtualStringTree1->Header->Options << hoVisible; // Устанавливаем видимость заголовка

	VirtualStringTree1->Header->Columns->Add();
	VirtualStringTree1->Header->Columns->Items[0]->Text = "URL";  //заголовок столбца

	VirtualStringTree1->Header->Columns->Add();
	VirtualStringTree1->Header->Columns->Items[1]->Text = "Title";  //заголовок столбца, 3 столбец не укзываем, так как его выводим черезе AddToSel...

	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)   // выполняем запрос и обрабатываем каждую строку результата
	{
		PVirtualNode Node = VirtualStringTree1->AddChild(nullptr);
		TRowData *Data = static_cast<TRowData*>(VirtualStringTree1->GetNodeData(Node));
		if (Data)
		{
			Data->Url = UTF8ToWideString(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));  //поле 1 (0)
			Data->Title = UTF8ToWideString(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))); //поле 2
			Data->VisitCount = sqlite3_column_int(stmt, 2); // поле 3
		}
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);  //закрываем соединение
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
				ShowMessage("Ошибка при открытии базы данных: " + UnicodeString(sqlite3_errmsg(db))); //отладка
				return;
            }

            sqlite3_stmt *stmt;
			rc = sqlite3_prepare_v2(db, "DELETE FROM urls WHERE url = ?", -1, &stmt, NULL);
			if (rc != SQLITE_OK)
            {
				ShowMessage("Ошибка при выполнении запроса: " + UnicodeString(sqlite3_errmsg(db))); //отладка
				sqlite3_close(db);
				return;
            }

			sqlite3_bind_text(stmt, 1, AnsiString(Data->Url).c_str(), -1, SQLITE_STATIC);
			//1 значение строки Data->Url к первому параметру (?) в подготовленном SQL-запросе stmt.
			//2 Data->Url в AnsiString, чтобы получить строку в кодировке ANSI.
			//3 метод .c_str(), чтобы передать строку как указатель на char.
			//4, что строка завершена нулевым символом (\0), поэтому ее длина определяется автоматически.
			//5, что строка управляется пользователем и не будет освобождена SQLite (SQLITE_STATIC).

			rc = sqlite3_step(stmt);
			if (rc != SQLITE_DONE)
			{
				ShowMessage("Ошибка при удалении записи: " + UnicodeString(sqlite3_errmsg(db))); //отладка
			}
			else
			{
				VirtualStringTree1->DeleteNode(SelectedNode);  //удаляем
			}

			sqlite3_finalize(stmt);
			sqlite3_close(db);
		}
	}
	else
	{
		ShowMessage("Нет выделенной записи для удаления");  //отладка
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
  // Создание контейнера дронов
	DroneContainer droneContainer;
	srand(time(0));
	// Заполнение контейнера случайными дронами
	for (int i = 0; i < 1000; ++i) {
		string name = "Drone " + to_string(i + 1);
		int batteryLevel = rand() % 101; // Случайный уровень заряда батареи от 0 до 100
		int maxBattery = 100;
		int weight = rand() % 101;
		Drone drone(name, weight, batteryLevel, maxBattery);
		droneContainer.AddDrone(drone);
	}

	// Итератор для обхода контейнера дронов
	DroneContainer::Iterator it(droneContainer);

	// Очистка ProgressBar перед началом обхода
	ProgressBar1->Position = 0;
	ProgressBar1->Max = 1000; // Для 10 дронов

	// Обход контейнера и вывод дронов на VirtualStringTree
	while (it.HasNext()) {
		  Drone drone = it.Next();
		// Добавление дрона в VirtualStringTree
		PVirtualNode newNode = VirtualStringTree2->AddChild(NULL);
		Drone *dronePtr = new Drone(drone); // Копируем дрона, чтобы сохранить его в пользовательском указателе узла
		Drone **nodeData = static_cast<Drone**>(VirtualStringTree2->GetNodeData(newNode));
		*nodeData = dronePtr; // Сохраняем дрона в пользовательском указателе узла

		// Обновление ProgressBar
		ProgressBar1->StepBy(1);
		Application->ProcessMessages(); // Обновление интерфейса
	}
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
//---------------------------------------------------------------------------

