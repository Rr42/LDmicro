///Includes
#include <wincodec.h>
#include <stdio.h>
//#include <string.h>
#include <commctrl.h>
#include <Windowsx.h>

#include "componentstructs.h"	
#include "componentfunctions.h"
#include "componentimages.h"
#include "components.h"

///Window handles
static HWND DPDTState1;
static HWND DPDTState2;
HWND* SettingsDialogDPDT;

///Global variables
enum DPDT_Pins { in1 = 0, in2, out11, out12, out21, out22 };

///Function definitions
int InitDpdt(void * ComponentAddress)
{
	DpdtStruct* d = (DpdtStruct*)ComponentAddress;
	d->image = DPDT_1;
	d->NS1 = TRUE;
	d->Volt[in1] = V_OPEN;
	d->Volt[in2] = V_OPEN;
	d->Volt[out11] = V_OPEN;
	d->Volt[out12] = V_OPEN;
	d->Volt[out21] = V_OPEN;
	d->Volt[out22] = V_OPEN;

	return DPDT_1;
}

void SetDpdtIds(int* id, void* ComponentAddress)
{
	DpdtStruct* d = (DpdtStruct*)ComponentAddress;
	d->PinId[in1] = *id++;
	d->PinId[in2] = *id++;
	d->PinId[out11] = *id++;
	d->PinId[out12] = *id++;
	d->PinId[out21] = *id++;
	d->PinId[out22] = *id++;
}

void MakeSettingsDialogDPDT()
{
	HWND InitOut = CreateWindowEx(0, WC_BUTTON, ("Initial output state"),
		WS_CHILD | BS_GROUPBOX | WS_VISIBLE | WS_TABSTOP,
		7, 3, 120, 65, *SettingsDialogDPDT, NULL, NULL, NULL);
	FontNice(InitOut);

	DPDTState1 = CreateWindowEx(0, WC_BUTTON, ("State 1"),
		WS_CHILD | BS_AUTORADIOBUTTON | WS_TABSTOP | WS_VISIBLE | WS_GROUP,
		16, 21, 100, 20, *SettingsDialogDPDT, NULL, NULL, NULL);
	FontNice(DPDTState1);

	DPDTState2 = CreateWindowEx(0, WC_BUTTON, ("State 2"),
		WS_CHILD | BS_AUTORADIOBUTTON | WS_TABSTOP | WS_VISIBLE,
		16, 41, 100, 20, *SettingsDialogDPDT, NULL, NULL, NULL);
	FontNice(DPDTState2);
}

void LoadSettings(DpdtStruct* d)
{
	if (d->NS1)
		Button_SetCheck(DPDTState1, BST_CHECKED);
	else
		Button_SetCheck(DPDTState2, BST_CHECKED);
}

BOOL SaveSettings(DpdtStruct* d, void* ImageLocation)
{
	BOOL NState;
	if (Button_GetState(DPDTState1) == BST_CHECKED)
		NState = TRUE;
	else if (Button_GetState(DPDTState2) == BST_CHECKED)
		NState = FALSE;
	else
	{
		MessageBox(*SettingsDialogDPDT,
			("Incomplete"), ("Warning"), MB_OK | MB_ICONWARNING);
		return FALSE;
	}

	d->NS1 = NState;

	if (NState)
		d->image = DPDT_1;
	else
		d->image = DPDT_2;

	SetImage(d->image, ImageLocation);
	RefreshImages();

	return TRUE;
}

void DpdtSettingsDialog(void* ComponentAddress, void* ImageLocation)
{
	DpdtStruct* d = (DpdtStruct*)ComponentAddress;
	BOOL exitStatus;

	//Create dialog window instance
	SettingsDialogDPDT = CreateDialogWindow("SPDT Settings Dialog", 100, 100, 263, 145, STYLE_VERTICAL);

	//Make the settings dialog
	MakeSettingsDialogDPDT();

	//Load settings
	LoadSettings(d);

	//Show dialog window
	ShowDialogWindow();

	exitStatus = ProcessDialogWindow();
	while (exitStatus == FALSE)
	{
		exitStatus = SaveSettings(d, ImageLocation);
		if (exitStatus == TRUE)
			break;
		else
		{
			exitStatus = TRUE;
			exitStatus = ProcessDialogWindow();
		}
	}

	DestroyWindow(*SettingsDialogDPDT);
}

//Dynamically check and equalise the voltage on all pins that are connected to DPST at runtime
double EqualiseRuntimeVoltageDPDT(void* ComponentAdderss, int index = 0)
{
	DpdtStruct* d = (DpdtStruct*)ComponentAdderss;

	///Check if DPDT switch is in state 1; i.e Input 1 is connected to output11, Input 2 is connected to output21
	if (d->NS1)
	{
		///If DPDT switch is in state 1 then output12 and output22 will be floating/open
		d->Volt[out12] = VoltChange(d->PinId[out12], out12, ComponentAdderss, V_OPEN);
		d->Volt[out22] = VoltChange(d->PinId[out22], out22, ComponentAdderss, V_OPEN);

		///Get voltages at the connected pins
		double Vin = VoltRequest(d->PinId[in1], ComponentAdderss);
		double Vout = VoltRequest(d->PinId[out11], ComponentAdderss);

		///Set 1: input 1, output11, output12
		///If either pin is grounded then all pins are set to GND
		if (Vin == GND || Vout == GND)
		{
			d->Volt[out11] = VoltChange(d->PinId[out11], out11, ComponentAdderss, GND);
			d->Volt[in1] = VoltChange(d->PinId[in1], in1, ComponentAdderss, GND);
		}
		///If no pin is grounded then all pins are set to the max voltage of the pins
		else
		{
			d->Volt[out11] = VoltChange(d->PinId[out11], out11, ComponentAdderss, max(Vin, Vout));
			d->Volt[in1] = VoltChange(d->PinId[in1], in1, ComponentAdderss, max(Vin, Vout));
		}

		Vin = VoltRequest(d->PinId[in2], ComponentAdderss);
		Vout = VoltRequest(d->PinId[out21], ComponentAdderss);
		///Set 2: input 2, output21, output22
		///If either pin is grounded then all pins are set to GND
		if (Vin == GND || Vout == GND)
		{
			d->Volt[out21] = VoltChange(d->PinId[out21], out21, ComponentAdderss, GND);
			d->Volt[in2] = VoltChange(d->PinId[in2], in2, ComponentAdderss, GND);
		}
		///If no pin is grounded then all pins are set to the max voltage of the pins
		else
		{
			d->Volt[out21] = VoltChange(d->PinId[out21], out21, ComponentAdderss, max(Vin, Vout));
			d->Volt[in2] = VoltChange(d->PinId[in2], in2, ComponentAdderss, max(Vin, Vout));
		}
	}
	///Check if DPDT switch is in state 2; i.e Input 1 is connected to output12, Input 2 is connected to output22
	else
	{
		///If DPDT switch is in state 2 then output11 and output21 will be floating/open
		d->Volt[out11] = VoltChange(d->PinId[out11], out11, ComponentAdderss, V_OPEN);
		d->Volt[out21] = VoltChange(d->PinId[out21], out21, ComponentAdderss, V_OPEN);

		///Get voltages at the connected pins
		double Vin = VoltRequest(d->PinId[in1], ComponentAdderss);
		double Vout = VoltRequest(d->PinId[out12], ComponentAdderss);

		///Set 1: input 1, output11, output12
		///If either pin is grounded then all pins are set to GND
		if (Vin == GND || Vout == GND)
		{
			d->Volt[out12] = VoltChange(d->PinId[out12], out12, ComponentAdderss, GND);
			d->Volt[in1] = VoltChange(d->PinId[in1], in1, ComponentAdderss, GND);
		}
		///If no pin is grounded then all pins are set to the max voltage of the pins
		else
		{
			d->Volt[out12] = VoltChange(d->PinId[out12], out12, ComponentAdderss, max(Vin, Vout));
			d->Volt[in1] = VoltChange(d->PinId[in1], in1, ComponentAdderss, max(Vin, Vout));
		}

		Vin = VoltRequest(d->PinId[in2], ComponentAdderss);
		Vout = VoltRequest(d->PinId[out22], ComponentAdderss);
		///Set 2: input 2, output21, output22
		///If either pin is grounded then all pins are set to GND
		if (Vin == GND || Vout == GND)
		{
			d->Volt[out22] = VoltChange(d->PinId[out22], out22, ComponentAdderss, GND);
			d->Volt[in2] = VoltChange(d->PinId[in2], in2, ComponentAdderss, GND);
		}
		///If no pin is grounded then all pins are set to the max voltage of the pins
		else
		{
			d->Volt[out22] = VoltChange(d->PinId[out22], out22, ComponentAdderss, max(Vin, Vout));
			d->Volt[in2] = VoltChange(d->PinId[in2], in2, ComponentAdderss, max(Vin, Vout));
		}
	}

	return d->Volt[index];
}

void ToggleState(DpdtStruct* d, void* ImageLocation)
{
	d->image = (d->image == DPDT_1) ? DPDT_2 : DPDT_1;
	d->NS1 = (d->NS1 == TRUE) ? FALSE : TRUE;
	SetImage(d->image, ImageLocation);
	RefreshImages();
}

void HandleDpdtEvent(void * ComponentAddress, int Event, BOOL SimulationStarted, void * ImageLocation, UINT ImageId, HWND * h)
{

	DpdtStruct* d = (DpdtStruct*)ComponentAddress;
	
	if (SimulationStarted)
	{
		switch (Event)
		{
		case EVENT_MOUSE_CLICK:
			ToggleState(d, ImageLocation);
			EqualiseRuntimeVoltageDPDT(ComponentAddress);
			break;
		default:
			break;
		}
	}
	else
	{
		switch (Event)
		{
		case EVENT_MOUSE_DBLCLICK:
			DpdtSettingsDialog(ComponentAddress, ImageLocation);
			break;
		default:
			break;
		}
	}
}

double DpdtVoltChanged(void * ComponentAddress, BOOL SimulationStarted, int index, double Volt, int Source, void * ImageLocation)
{
	
	if (SimulationStarted)
		return EqualiseRuntimeVoltageDPDT(ComponentAddress, index);
		
	return Volt;
}