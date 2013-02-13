#pragma once

#include "myDirectDevice.h"

class MyDirectInput8 : public IDirectInput8
{
public:
	MyDirectInput8(IDirectInput8* di) : m_di(di)
	{
	}

	/*** IUnknown methods ***/
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj)
	{
		SendToLog("MyDirectInput8::QueryInterface");
		return m_di->QueryInterface(riid, ppvObj);
	}

    ULONG STDMETHODCALLTYPE AddRef()
	{
		SendToLog("MyDirectInput8::AddRef");
		return m_di->AddRef();
	}

    ULONG STDMETHODCALLTYPE Release()
	{
		SendToLog("MyDirectInput8::Release");
		ULONG count = m_di->Release();
		if(0 == count)
			delete this;

		return count;
	}

    /*** IDirectInput8 methods ***/
    STDMETHOD(CreateDevice)(REFGUID rguid, IDirectInputDevice8** device, LPUNKNOWN unknown)
	{
		SendToLog("MyDirectInput8::CreateDevice %x %x %x");
		HRESULT hr = m_di->CreateDevice(rguid, device, unknown);
		if(SUCCEEDED(hr)/* && rguid == GUID_SysKeyboard*/)
		{
			// Return our own keyboard device that checks for injected keypresses
			*device = new MyDirectDevice(*device);

			::CreateDevice(*device, rguid == GUID_SysKeyboard);
		}

		return hr;
	}

    STDMETHOD(EnumDevices)(DWORD devType,LPDIENUMDEVICESCALLBACK callback, LPVOID ref, DWORD flags)
	{
		SendToLog("MyDirectInput8::EnumDevices %x");
		return m_di->EnumDevices(devType, callback, ref, flags);
	}

    STDMETHOD(GetDeviceStatus)(REFGUID rguid)
	{
		SendToLog("MyDirectInput8::GetDeviceStatus");
		return m_di->GetDeviceStatus(rguid);
	}

    STDMETHOD(RunControlPanel)(HWND owner, DWORD flags)
	{
		SendToLog("MyDirectInput8::RunControlPanel");
		return m_di->RunControlPanel(owner, flags);
	}

    STDMETHOD(Initialize)(HINSTANCE instance, DWORD version)
	{
		SendToLog("MyDirectInput8::Initialize");
		return m_di->Initialize(instance, version);
	}

    STDMETHOD(FindDevice)(REFGUID rguid, LPCTSTR name, LPGUID guidInstance)
	{
		SendToLog("MyDirectInput8::FindDevice");
		return m_di->FindDevice(rguid, name, guidInstance);
	}

    STDMETHOD(EnumDevicesBySemantics)(LPCTSTR username, LPDIACTIONFORMAT action,
		LPDIENUMDEVICESBYSEMANTICSCB callback, LPVOID ref, DWORD flags)
	{
		SendToLog("MyDirectInput8::EnumDevicesBySemantics");
		return m_di->EnumDevicesBySemantics(username, action, callback, ref, flags);
	}

    STDMETHOD(ConfigureDevices)(LPDICONFIGUREDEVICESCALLBACK callback, LPDICONFIGUREDEVICESPARAMS params,
		DWORD flags, LPVOID ref)
	{
		SendToLog("MyDirectInput8::ConfigureDevices");
		return m_di->ConfigureDevices(callback, params, flags, ref);
	}

private:
	IDirectInput8* m_di;
};