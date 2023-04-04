#include "AppPrecomp.h"
#include "main.h"
#include "DataEditor.h"
#include "DataManager.h"

DataEditor::DataEditor()
{
	m_exitMode = CLICKED_CANCEL;
}

DataEditor::~DataEditor()
{
}

void DataEditor::SetExitMode(eExitMode exitMode)
{
	m_exitMode = exitMode;
}

bool DataEditor::Init( const string sTitle, const string sDescription, DataManager *pData )
{
	//setup our GUI window

	CL_Point ptSize(550,530);
	CL_Window window(CL_Rect(0, 0, ptSize.x, ptSize.y), sTitle, 0, GetApp()->GetGUI());
	window.set_event_passing(false);
	window.set_position(300, GetScreenY- (ptSize.y+100));
	int offsetY = 130;
	int buttonOffsetX = 10;


	CL_Rect rPos(buttonOffsetX,10,ptSize.x- (buttonOffsetX*2) ,offsetY);
/*
	CL_RichEdit richEdit(rPos, window.get_client_area());
	richEdit.add_text(sDescription, GetApp()->GetFont(C_FONT_GRAY), CL_Color::black);
*/


	CL_Label labelData (rPos, sDescription+"\r\n\r\n(double click to edit a value)", window.get_client_area());
	offsetY+= 20;

	CL_Point ptOffset = CL_Point(buttonOffsetX, offsetY);
	rPos = CL_Rect(0,0,ptSize.x - (buttonOffsetX*2),300);
	rPos.apply_alignment(origin_top_left, -ptOffset.x, -ptOffset.y);

	CL_ListBox listData(rPos, window.get_client_area());
	//listData.set_multi_selection(true);
	offsetY+= 4 + listData.get_height();
	CL_Button buttonAddData (CL_Point(buttonOffsetX, offsetY), "Add", window.get_client_area());
	CL_Button buttonRemoveData (CL_Point(buttonOffsetX+50, offsetY), "Remove Selected", window.get_client_area());
	
	
	offsetY += 20;

	//populate the listbox

	dataList *dList = pData->GetList();
	dataList::iterator ditor = dList->begin();

	string stTemp;

	while (ditor != dList->end())
	{
		listData.insert_item(PropertyMakeItemString(ditor->second));
		ditor++;
	}

	listData.sort();

	//Handle the buttons
	CL_SlotContainer slots;
	CL_Button buttonCancel (CL_Point(buttonOffsetX+100,ptSize.y-50), "Cancel", window.get_client_area());
	CL_Button buttonOk (CL_Point(buttonOffsetX+200,ptSize.y-50), "Ok", window.get_client_area());

	slots.connect(buttonCancel.sig_clicked(), (CL_Component*)&window, &CL_Component::quit);
	slots.connect(buttonOk.sig_clicked(), this, &DataEditor::SetExitMode, CLICKED_OK);
	slots.connect(buttonOk.sig_clicked(), (CL_Component*)&window, &CL_Component::quit);
	slots.connect(listData.sig_mouse_dblclk(), &OnPropertiesEditData, &listData);

	slots.connect(buttonAddData.sig_clicked(), &OnPropertiesAddData, &listData);
	slots.connect(buttonRemoveData.sig_clicked(), &OnPropertiesRemoveData, &listData);

	window.run();	//loop in the menu until quit() is called by hitting a button

	if (m_exitMode == CLICKED_OK )
	{
		PropertiesSetDataManagerFromListBox(pData, listData);
	}

	return true;
}


//some global helpers that we share with other classes too


void OnPropertiesEditData(const CL_InputEvent &input, CL_ListBox *pList)
{
	int index = pList->get_item(input.mouse_pos);
	if (index == -1) return;
	CL_ListItem *pItem = pList->get_item(index);
	vector<string> words = CL_String::tokenize(pItem->str, "|", false);
	string name = words[1].c_str();
	string value = words[3].c_str();

	DataObject o;
	o.Set(name, value);
	CreateEditDataDialog(o);
	//update the listbox
	pList->change_item(PropertyMakeItemString(o), index);
}

void PropertiesSetDataManagerFromListBox(DataManager *pData, CL_ListBox &listBox)
{
	pData->Clear();

	string name;
	string value; 

	//cycle through and add each key/value pair
	for (int i=0; i < listBox.get_count(); i++)
	{
		CL_ListItem *pItem = listBox.get_item(i);

		vector<string> words = CL_String::tokenize(pItem->str, "|", false);
		name = words[1].c_str();
		value = words[3].c_str();

		//OPTIMIZE:  Later, we should examine and if we find a # value add it as a # instead of a string,
		pData->Set(name, value); //it will create it if it doesn't exist
	}
}


string PropertyMakeItemString(DataObject &o)
{
	return ("Name: |" + o.m_key+"|  Value: |"+o.Get()+"|");
}


void CreateEditDataDialog(DataObject &o)
{
	CL_InputDialog dlg("Edit Data Dialog", "Ok", "Cancel", "",GetApp()->GetGUI());
	dlg.set_event_passing(false);

	CL_InputBox *pName = dlg.add_input_box("Name", o.m_key, 600);
	pName->set_tab_id(0);

	CL_InputBox *pValue = dlg.add_input_box("Value", o.Get(), 600);
	pValue->set_tab_id(1);
	dlg.get_button(0)->set_tab_id(2);
	dlg.get_button(1)->set_tab_id(3);

	dlg.set_focus();
	pName->set_focus();
	dlg.run();

	if (dlg.get_result_button() == 0)
	{
		if (pName->get_text().size() > 0)
		{
			o.Set(pName->get_text(), pValue->get_text());
		} else
		{
			LogMsg("Can't have a key with no name, ignoring input.");
		}
	}
}


void OnPropertiesRemoveData(CL_ListBox *pListBox)
{
	//kill all selected items

	for (int i=0; i < pListBox->get_count(); i++)
	{
		CL_ListItem *pItem = pListBox->get_item(i);

		if (pItem->selected)
		{
			pListBox->remove_item(i);
			i = -1;
		}
	}
}


void OnPropertiesAddData(CL_ListBox *pListBox)
{
	DataObject o;
	CreateEditDataDialog(o);
	if (o.m_key.size() > 0)
		pListBox->insert_item(PropertyMakeItemString(o));
}
