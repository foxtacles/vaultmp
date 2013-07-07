#pragma once

#include <dinput.h>

#include "common.h"

#include "DirectInputHook.h"

/*
Class: MyDirectDevice
	This class implements the interface IDirectInputDevice8 and represents the core of the hooking procedure,
	together with <NamedPipeServer>.

	On creation, this class creates a <NamedPipeServer> with a pipe named "DInput-Hook". The pipe accepts
	messages in the form of strings. Each message (string) is a space-delimited list of scan-codes.
	The keys with the corresponding scan-codes will appear pressed to the hooked application the next time
	it retrieves the device state (in immediate mode) via <GetDeviceState>.

	If a malformed message is received, no action is taken - there's no error reporting or anything.
	You might want to implement a more sophisticated communication protocol to suit your needs.

	The class only works in immediate mode. I didn't implement anything for buffered input because you need
	to define the 'semantics' first - i.e. how would you like your injected scancodes interact with genuine
	user-input ones. Totally override? Come before? Come after?

	The bulk of functionality, thus, lies in <GetDeviceState>. Check it out.

Sidenote:
	Typing out all these functions and their parameters literally WRECKED my fingers. My right-hand is in
	pain. This is about the only thing I hate about hooking!
*/
class MyDirectDevice : public IDirectInputDevice8
{
public:
	MyDirectDevice(IDirectInputDevice8* device) : m_device(device)
	{
	}

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(REFIID riid, void** ppvObj)
	{
		SendToLog("MyDirectDevice::QueryInterface");
		return m_device->QueryInterface(riid, ppvObj);
	}

	STDMETHOD_(ULONG,AddRef)()
	{
		SendToLog("MyDirectDevice::AddRef");
		return m_device->AddRef();
	}

	STDMETHOD_(ULONG,Release)()
	{
		SendToLog("MyDirectDevice::Release");
		ULONG count = m_device->Release();
		if(0 == count)
		{
			SendToLog("DirectInput Hook Released");
			delete this;
		}

		return count;
	}

	/*** IDirectInputDevice8 methods ***/
	STDMETHOD(GetCapabilities)(LPDIDEVCAPS devCaps)
	{
		SendToLog("MyDirectDevice::GetCapabilities");
		return m_device->GetCapabilities(devCaps);
	}

	STDMETHOD(EnumObjects)(LPDIENUMDEVICEOBJECTSCALLBACK callback, LPVOID ref, DWORD flags)	
	{
		SendToLog("MyDirectDevice::EnumObjects");
		return m_device->EnumObjects(callback, ref, flags);
	}

	STDMETHOD(GetProperty)(REFGUID rguid, LPDIPROPHEADER ph)
	{
		SendToLog("MyDirectDevice::GetProperty");
		return m_device->GetProperty(rguid, ph);
	}

	STDMETHOD(SetProperty)(REFGUID rguid, LPCDIPROPHEADER ph)
	{
		SendToLog("MyDirectDevice::SetProperty");
		return m_device->SetProperty(rguid, ph);

		/*
		You can detect immediate/buffered modes as such:
		
		HRESULT hr = m_device->SetProperty(rguid, ph);

		if(SUCCEEDED(hr) && rguid == DIPROP_BUFFERSIZE)
		{
			DWORD data = *reinterpret_cast<const DWORD*>(ph + 1);
			m_is_immediate = (data == 0);
		}

		return hr;
		*/
	}

	STDMETHOD(Acquire)()
	{
		HRESULT hr = m_device->Acquire();

		return hr;
	}

	STDMETHOD(Unacquire)()
	{
		HRESULT hr = m_device->Unacquire();
		SendToLog("MyDirectDevice::Unacquire %d");
		return hr;
	}

	STDMETHOD(GetDeviceState)(DWORD size, LPVOID data)
	{
		HRESULT hr = m_device->GetDeviceState(size, data);
		if (SUCCEEDED(hr))
		{
			RealityKeyDown(this, size, data);
			SimulateKeyDown(this, size, data);
		}
		return hr;
	}

	STDMETHOD(GetDeviceData)(DWORD size, LPDIDEVICEOBJECTDATA data, LPDWORD numElements, DWORD flags)
	{
		SendToLog("MyDirectDevice::GetDeviceData");
		return m_device->GetDeviceData(size, data, numElements, flags);
	}

	STDMETHOD(SetDataFormat)(LPCDIDATAFORMAT df)
	{
		SendToLog("MyDirectDevice::SetDataFormat");
		dataFormat=df;
		return m_device->SetDataFormat(df);
	}

	STDMETHOD(SetEventNotification)(HANDLE event)
	{
		SendToLog("MyDirectDevice::SetEventNotification");
		return m_device->SetEventNotification(event);
	}

	STDMETHOD(SetCooperativeLevel)(HWND window, DWORD level)
	{
		SendToLog("MyDirectDevice::SetCooperativeLevel");
		return m_device->SetCooperativeLevel(window, level);
	}

	STDMETHOD(GetObjectInfo)(LPDIDEVICEOBJECTINSTANCE object, DWORD objId, DWORD objHow)
	{
		SendToLog("MyDirectDevice::GetObjectInfo");
		return m_device->GetObjectInfo(object, objId, objHow);
	}

	STDMETHOD(GetDeviceInfo)(LPDIDEVICEINSTANCE di)
	{
		SendToLog("MyDirectDevice::GetDeviceInfo");
		return m_device->GetDeviceInfo(di);
	}

	STDMETHOD(RunControlPanel)(HWND owner, DWORD flags)
	{
		SendToLog("MyDirectDevice::RunControlPanel");
		return m_device->RunControlPanel(owner, flags);
	}

	STDMETHOD(Initialize)(HINSTANCE instance, DWORD version, REFGUID rguid)
	{
		SendToLog("MyDirectDevice::Initialize");
		return m_device->Initialize(instance, version, rguid);
	}

	STDMETHOD(CreateEffect)(REFGUID rguid, LPCDIEFFECT effect_params, LPDIRECTINPUTEFFECT* effect, LPUNKNOWN unknown)
	{
		SendToLog("MyDirectDevice::CreateEffect");
		return m_device->CreateEffect(rguid, effect_params, effect, unknown);
	}

    STDMETHOD(EnumEffects)(LPDIENUMEFFECTSCALLBACK callback, LPVOID ref, DWORD type)
	{
		SendToLog("MyDirectDevice::EnumEffects");
		return m_device->EnumEffects(callback, ref, type);
	}

    STDMETHOD(GetEffectInfo)(LPDIEFFECTINFO effect_info, REFGUID rguid)
	{
		SendToLog("MyDirectDevice::GetEffectInfo");
		return m_device->GetEffectInfo(effect_info, rguid);
	}

    STDMETHOD(GetForceFeedbackState)(LPDWORD state)
	{
		SendToLog("MyDirectDevice::GetForceFeedbackState");
		return m_device->GetForceFeedbackState(state);
	}

    STDMETHOD(SendForceFeedbackCommand)(DWORD flags)
	{
		SendToLog("MyDirectDevice::SendForceFeedbackCommand");
		return m_device->SendForceFeedbackCommand(flags);
	}

    STDMETHOD(EnumCreatedEffectObjects)(LPDIENUMCREATEDEFFECTOBJECTSCALLBACK callback, LPVOID ref, DWORD flags)
	{
		SendToLog("MyDirectDevice::EnumCreatedEffectObjects");
		return m_device->EnumCreatedEffectObjects(callback, ref, flags);
	}

    STDMETHOD(Escape)(LPDIEFFESCAPE escape)
	{
		SendToLog("MyDirectDevice::Escape");
		return m_device->Escape(escape);
	}

    STDMETHOD(Poll)()
	{
		return m_device->Poll();
	}

	STDMETHOD(SendDeviceData)(DWORD size, LPCDIDEVICEOBJECTDATA data, LPDWORD num_elements, DWORD flags)
	{
		SendToLog("MyDirectDevice::SendDeviceData");
		return m_device->SendDeviceData(size, data, num_elements, flags);
	}

	STDMETHOD(EnumEffectsInFile)(LPCTSTR file_name, LPDIENUMEFFECTSINFILECALLBACK callback, LPVOID ref, DWORD flags)
	{
		SendToLog("MyDirectDevice::EnumEffectsInFile");
		return m_device->EnumEffectsInFile(file_name, callback, ref, flags);
	}

    STDMETHOD(WriteEffectToFile)(LPCTSTR file_name, DWORD num_entries, LPDIFILEEFFECT effects, DWORD flags)
	{
		SendToLog("MyDirectDevice::WriteEffectToFile");
		return m_device->WriteEffectToFile(file_name, num_entries, effects, flags);
	}

    STDMETHOD(BuildActionMap)(LPDIACTIONFORMAT format, LPCTSTR username, DWORD flags)
	{
		SendToLog("MyDirectDevice::BuildActionMap");
		return m_device->BuildActionMap(format, username, flags);
	}

    STDMETHOD(SetActionMap)(LPDIACTIONFORMAT format, LPCTSTR username, DWORD flags)
	{
		SendToLog("MyDirectDevice::SetActionMap");
		return m_device->SetActionMap(format, username, flags);
	}

    STDMETHOD(GetImageInfo)(LPDIDEVICEIMAGEINFOHEADER image_header)
	{
		SendToLog("MyDirectDevice::GetImageInfo");
		return m_device->GetImageInfo(image_header);
	}

private:
	IDirectInputDevice8* m_device;

public:
	LPCDIDATAFORMAT dataFormat;
};