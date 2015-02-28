#include "vaultscript.h"

#ifdef __cplusplus

extern "C" {
	VAULTVAR VAULTSPACE RawChar vaultprefix = VAULTAPI_PREFIX;

	VAULTSCRIPT VAULTSPACE Void (*VAULTAPI(timestamp))() VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE Timer (*VAULTAPI(CreateTimer))(VAULTSPACE RawFunction(), VAULTSPACE Interval) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE Timer (*VAULTAPI(CreateTimerEx))(VAULTSPACE RawFunction(), VAULTSPACE Interval, VAULTSPACE cRawString, ...) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE Void (*VAULTAPI(KillTimer))(VAULTSPACE Timer) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE Void (*VAULTAPI(MakePublic))(VAULTSPACE RawFunction(), VAULTSPACE cRawString, VAULTSPACE cRawString) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE Result (*VAULTAPI(CallPublic))(VAULTSPACE cRawString, ...) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(IsPAWN))(VAULTSPACE cRawString) VAULTCPP(noexcept);

	VAULTSCRIPT VAULTSPACE Void (*VAULTAPI(SetServerName))(VAULTSPACE cRawString) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE Void (*VAULTAPI(SetServerMap))(VAULTSPACE cRawString) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE Void (*VAULTAPI(SetServerRule))(VAULTSPACE cRawString, VAULTSPACE cRawString) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE UCount (*VAULTAPI(GetMaximumPlayers))() VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE UCount (*VAULTAPI(GetCurrentPlayers))() VAULTCPP(noexcept);

	VAULTSCRIPT VAULTSPACE cRawString (*VAULTAPI(ValueToString))(VAULTSPACE Index) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE cRawString (*VAULTAPI(AxisToString))(VAULTSPACE Index) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE cRawString (*VAULTAPI(AnimToString))(VAULTSPACE Index) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE cRawString (*VAULTAPI(BaseToString))(VAULTSPACE Base) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE cRawString (*VAULTAPI(BaseToType))(VAULTSPACE Base) VAULTCPP(noexcept);

	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(Kick))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(UIMessage))(VAULTSPACE ID, VAULTSPACE cRawString, VAULTSPACE Emoticon) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(ChatMessage))(VAULTSPACE ID, VAULTSPACE cRawString) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE Void (*VAULTAPI(SetRespawnTime))(VAULTSPACE Interval) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE Void (*VAULTAPI(SetSpawnCell))(VAULTSPACE CELL) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE Void (*VAULTAPI(SetConsoleEnabled))(VAULTSPACE State) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE Void (*VAULTAPI(SetGameWeather))(VAULTSPACE WTHR) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE Void (*VAULTAPI(SetGameTime))(VAULTSPACE Time) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE Void (*VAULTAPI(SetGameYear))(VAULTSPACE UCount) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE Void (*VAULTAPI(SetGameMonth))(VAULTSPACE UCount) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE Void (*VAULTAPI(SetGameDay))(VAULTSPACE UCount) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE Void (*VAULTAPI(SetGameHour))(VAULTSPACE UCount) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE Void (*VAULTAPI(SetTimeScale))(VAULTSPACE Value) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(IsValid))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(IsReference))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(IsObject))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(IsItem))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(IsContainer))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(IsActor))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(IsPlayer))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(IsCell))(VAULTSPACE CELL) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(IsInterior))(VAULTSPACE CELL) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(IsItemList))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(IsWindow))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(IsButton))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(IsText))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(IsEdit))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(IsCheckbox))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(IsRadioButton))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(IsListItem))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(IsList))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(IsChatbox))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE Type (*VAULTAPI(GetType))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE UCount (*VAULTAPI(GetConnection))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE UCount (*VAULTAPI(GetCount))(VAULTSPACE Type) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE UCount (*VAULTAPI(GetList))(VAULTSPACE Type, VAULTSPACE RawArray(VAULTSPACE ID)*) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE Interval (*VAULTAPI(GetRespawnTime))() VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE CELL (*VAULTAPI(GetSpawnCell))() VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(GetConsoleEnabled))() VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE WTHR (*VAULTAPI(GetGameWeather))() VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE Time (*VAULTAPI(GetGameTime))() VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE UCount (*VAULTAPI(GetGameYear))() VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE UCount (*VAULTAPI(GetGameMonth))() VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE UCount (*VAULTAPI(GetGameDay))() VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE UCount (*VAULTAPI(GetGameHour))() VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE Value (*VAULTAPI(GetTimeScale))() VAULTCPP(noexcept);

	VAULTSCRIPT VAULTSPACE ID (*VAULTAPI(GetID))(VAULTSPACE Ref) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE Ref (*VAULTAPI(GetReference))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE Base (*VAULTAPI(GetBase))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE Void (*VAULTAPI(GetPos))(VAULTSPACE ID, VAULTSPACE Value*, VAULTSPACE Value*, VAULTSPACE Value*) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE Void (*VAULTAPI(GetAngle))(VAULTSPACE ID, VAULTSPACE Value*, VAULTSPACE Value*, VAULTSPACE Value*) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE CELL (*VAULTAPI(GetCell))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE Lock (*VAULTAPI(GetLock))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE NPC_ (*VAULTAPI(GetOwner))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE cRawString (*VAULTAPI(GetBaseName))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(IsNearPoint))(VAULTSPACE ID, VAULTSPACE Value, VAULTSPACE Value, VAULTSPACE Value, VAULTSPACE Value) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE ID (*VAULTAPI(GetItemContainer))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE UCount (*VAULTAPI(GetItemCount))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE Value (*VAULTAPI(GetItemCondition))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(GetItemEquipped))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(GetItemSilent))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(GetItemStick))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE UCount (*VAULTAPI(GetContainerItemCount))(VAULTSPACE ID, VAULTSPACE Base) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE UCount (*VAULTAPI(GetContainerItemList))(VAULTSPACE ID, VAULTSPACE RawArray(VAULTSPACE ID)*) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE Value (*VAULTAPI(GetActorValue))(VAULTSPACE ID, VAULTSPACE ActorValue) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE Value (*VAULTAPI(GetActorBaseValue))(VAULTSPACE ID, VAULTSPACE ActorValue) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE IDLE (*VAULTAPI(GetActorIdleAnimation))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE Index (*VAULTAPI(GetActorMovingAnimation))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE Index (*VAULTAPI(GetActorWeaponAnimation))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(GetActorAlerted))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(GetActorSneaking))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(GetActorDead))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE RACE (*VAULTAPI(GetActorBaseRace))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE Sex (*VAULTAPI(GetActorBaseSex))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(IsActorJumping))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE Interval (*VAULTAPI(GetPlayerRespawnTime))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE CELL (*VAULTAPI(GetPlayerSpawnCell))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(GetPlayerConsoleEnabled))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE UCount (*VAULTAPI(GetPlayerWindowCount))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE UCount (*VAULTAPI(GetPlayerWindowList))(VAULTSPACE ID, VAULTSPACE RawArray(VAULTSPACE ID)*) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE ID (*VAULTAPI(GetPlayerChatboxWindow))(VAULTSPACE ID) VAULTCPP(noexcept);

	VAULTSCRIPT VAULTSPACE ID (*VAULTAPI(CreateObject))(VAULTSPACE Base, VAULTSPACE CELL, VAULTSPACE Value, VAULTSPACE Value, VAULTSPACE Value) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(CreateVolatile))(VAULTSPACE ID, VAULTSPACE Base, VAULTSPACE Value, VAULTSPACE Value, VAULTSPACE Value) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(DestroyObject))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(Activate))(VAULTSPACE ID, VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(SetPos))(VAULTSPACE ID, VAULTSPACE Value, VAULTSPACE Value, VAULTSPACE Value) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(SetAngle))(VAULTSPACE ID, VAULTSPACE Value, VAULTSPACE Value, VAULTSPACE Value) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(SetCell))(VAULTSPACE ID, VAULTSPACE CELL, VAULTSPACE Value, VAULTSPACE Value, VAULTSPACE Value) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(SetLock))(VAULTSPACE ID, VAULTSPACE ID, VAULTSPACE Lock) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(SetOwner))(VAULTSPACE ID, VAULTSPACE NPC_) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(SetBaseName))(VAULTSPACE ID, VAULTSPACE cRawString) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(PlaySound))(VAULTSPACE ID, VAULTSPACE SOUN) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE ID (*VAULTAPI(CreateItem))(VAULTSPACE Base, VAULTSPACE CELL, VAULTSPACE Value, VAULTSPACE Value, VAULTSPACE Value) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE ID (*VAULTAPI(SetItemContainer))(VAULTSPACE ID, VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(SetItemCount))(VAULTSPACE ID, VAULTSPACE UCount) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(SetItemCondition))(VAULTSPACE ID, VAULTSPACE Value) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(SetItemEquipped))(VAULTSPACE ID, VAULTSPACE State, VAULTSPACE State, VAULTSPACE State) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE ID (*VAULTAPI(CreateContainer))(VAULTSPACE CONT, VAULTSPACE CELL, VAULTSPACE Value, VAULTSPACE Value, VAULTSPACE Value) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE ID (*VAULTAPI(CreateItemList))(VAULTSPACE ID, VAULTSPACE Base) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(DestroyItemList))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE ID (*VAULTAPI(AddItem))(VAULTSPACE ID, VAULTSPACE Base, VAULTSPACE UCount, VAULTSPACE Value, VAULTSPACE State) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE Void (*VAULTAPI(AddItemList))(VAULTSPACE ID, VAULTSPACE ID, VAULTSPACE Base) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE UCount (*VAULTAPI(RemoveItem))(VAULTSPACE ID, VAULTSPACE Base, VAULTSPACE UCount, VAULTSPACE State) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE Void (*VAULTAPI(RemoveAllItems))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE ID (*VAULTAPI(CreateActor))(VAULTSPACE Base, VAULTSPACE CELL, VAULTSPACE Value, VAULTSPACE Value, VAULTSPACE Value) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE Void (*VAULTAPI(SetActorValue))(VAULTSPACE ID, VAULTSPACE ActorValue, VAULTSPACE Value) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE Void (*VAULTAPI(SetActorBaseValue))(VAULTSPACE ID, VAULTSPACE ActorValue, VAULTSPACE Value) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(EquipItem))(VAULTSPACE ID, VAULTSPACE Base, VAULTSPACE State, VAULTSPACE State) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(UnequipItem))(VAULTSPACE ID, VAULTSPACE Base, VAULTSPACE State, VAULTSPACE State) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(PlayIdle))(VAULTSPACE ID, VAULTSPACE IDLE) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(SetActorMovingAnimation))(VAULTSPACE ID, VAULTSPACE Index) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(SetActorWeaponAnimation))(VAULTSPACE ID, VAULTSPACE Index) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(SetActorAlerted))(VAULTSPACE ID, VAULTSPACE State) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(SetActorSneaking))(VAULTSPACE ID, VAULTSPACE State) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(FireWeapon))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE Void (*VAULTAPI(KillActor))(VAULTSPACE ID, VAULTSPACE ID, VAULTSPACE Limb, VAULTSPACE Death) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(SetActorBaseRace))(VAULTSPACE ID, VAULTSPACE RACE) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(AgeActorBaseRace))(VAULTSPACE ID, VAULTSPACE Count) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(SetActorBaseSex))(VAULTSPACE ID, VAULTSPACE Sex) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE Void (*VAULTAPI(SetPlayerRespawnTime))(VAULTSPACE ID, VAULTSPACE Interval) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE Void (*VAULTAPI(SetPlayerSpawnCell))(VAULTSPACE ID, VAULTSPACE CELL) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE Void (*VAULTAPI(SetPlayerConsoleEnabled))(VAULTSPACE ID, VAULTSPACE State) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(AttachWindow))(VAULTSPACE ID, VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(DetachWindow))(VAULTSPACE ID, VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE Void (*VAULTAPI(ForceWindowMode))(VAULTSPACE ID, VAULTSPACE State) VAULTCPP(noexcept);

	VAULTSCRIPT VAULTSPACE ID (*VAULTAPI(GetWindowParent))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE ID (*VAULTAPI(GetWindowRoot))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE UCount (*VAULTAPI(GetWindowChildCount))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE UCount (*VAULTAPI(GetWindowChildList))(VAULTSPACE ID, VAULTSPACE RawArray(VAULTSPACE ID)*) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE Void (*VAULTAPI(GetWindowPos))(VAULTSPACE ID, VAULTSPACE Value*, VAULTSPACE Value*, VAULTSPACE Value*, VAULTSPACE Value*) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE Void (*VAULTAPI(GetWindowSize))(VAULTSPACE ID, VAULTSPACE Value*, VAULTSPACE Value*, VAULTSPACE Value*, VAULTSPACE Value*) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(GetWindowVisible))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(GetWindowLocked))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE cRawString (*VAULTAPI(GetWindowText))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE UCount (*VAULTAPI(GetEditMaxLength))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE cRawString (*VAULTAPI(GetEditValidation))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(GetCheckboxSelected))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(GetRadioButtonSelected))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE UCount (*VAULTAPI(GetRadioButtonGroup))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(GetListMultiSelect))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE UCount (*VAULTAPI(GetListItemCount))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE UCount (*VAULTAPI(GetListItemList))(VAULTSPACE ID, VAULTSPACE RawArray(VAULTSPACE ID)*) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE UCount (*VAULTAPI(GetListSelectedItemCount))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE UCount (*VAULTAPI(GetListSelectedItemList))(VAULTSPACE ID, VAULTSPACE RawArray(VAULTSPACE ID)*) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE ID (*VAULTAPI(GetListItemContainer))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(GetListItemSelected))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE cRawString (*VAULTAPI(GetListItemText))(VAULTSPACE ID) VAULTCPP(noexcept);

	VAULTSCRIPT VAULTSPACE ID (*VAULTAPI(CreateWindow))(VAULTSPACE Value, VAULTSPACE Value, VAULTSPACE Value, VAULTSPACE Value, VAULTSPACE State, VAULTSPACE State, VAULTSPACE cRawString) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(DestroyWindow))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(AddChildWindow))(VAULTSPACE ID, VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(RemoveChildWindow))(VAULTSPACE ID, VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(SetWindowPos))(VAULTSPACE ID, VAULTSPACE Value, VAULTSPACE Value, VAULTSPACE Value, VAULTSPACE Value) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(SetWindowSize))(VAULTSPACE ID, VAULTSPACE Value, VAULTSPACE Value, VAULTSPACE Value, VAULTSPACE Value) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(SetWindowVisible))(VAULTSPACE ID, VAULTSPACE State) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(SetWindowLocked))(VAULTSPACE ID, VAULTSPACE State) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(SetWindowText))(VAULTSPACE ID, VAULTSPACE cRawString) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE ID (*VAULTAPI(CreateButton))(VAULTSPACE Value, VAULTSPACE Value, VAULTSPACE Value, VAULTSPACE Value, VAULTSPACE State, VAULTSPACE State, VAULTSPACE cRawString) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE ID (*VAULTAPI(CreateText))(VAULTSPACE Value, VAULTSPACE Value, VAULTSPACE Value, VAULTSPACE Value, VAULTSPACE State, VAULTSPACE State, VAULTSPACE cRawString) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE ID (*VAULTAPI(CreateEdit))(VAULTSPACE Value, VAULTSPACE Value, VAULTSPACE Value, VAULTSPACE Value, VAULTSPACE State, VAULTSPACE State, VAULTSPACE cRawString) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(SetEditMaxLength))(VAULTSPACE ID, VAULTSPACE UCount) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(SetEditValidation))(VAULTSPACE ID, VAULTSPACE cRawString) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE ID (*VAULTAPI(CreateCheckbox))(VAULTSPACE Value, VAULTSPACE Value, VAULTSPACE Value, VAULTSPACE Value, VAULTSPACE State, VAULTSPACE State, VAULTSPACE cRawString) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(SetCheckboxSelected))(VAULTSPACE ID, VAULTSPACE State) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE ID (*VAULTAPI(CreateRadioButton))(VAULTSPACE Value, VAULTSPACE Value, VAULTSPACE Value, VAULTSPACE Value, VAULTSPACE State, VAULTSPACE State, VAULTSPACE cRawString) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(SetRadioButtonSelected))(VAULTSPACE ID, VAULTSPACE State) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(SetRadioButtonGroup))(VAULTSPACE ID, VAULTSPACE UCount) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE ID (*VAULTAPI(CreateList))(VAULTSPACE Value, VAULTSPACE Value, VAULTSPACE Value, VAULTSPACE Value, VAULTSPACE State, VAULTSPACE State, VAULTSPACE cRawString) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(SetListMultiSelect))(VAULTSPACE ID, VAULTSPACE State) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE ID (*VAULTAPI(AddListItem))(VAULTSPACE ID, VAULTSPACE cRawString) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(RemoveListItem))(VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE ID (*VAULTAPI(SetListItemContainer))(VAULTSPACE ID, VAULTSPACE ID) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(SetListItemSelected))(VAULTSPACE ID, VAULTSPACE State) VAULTCPP(noexcept);
	VAULTSCRIPT VAULTSPACE State (*VAULTAPI(SetListItemText))(VAULTSPACE ID, VAULTSPACE cRawString) VAULTCPP(noexcept);
}

namespace vaultmp
{
	State operator!(State state) { return state ? False : True; }

	GlobalChat Chat;

	Void timestamp() noexcept { return VAULTAPI(timestamp)(); }
	Timer CreateTimer(Function<> function, Interval interval) noexcept { return VAULTAPI(CreateTimer)(function, interval); }

	Void KillTimer(Timer timer) noexcept { return VAULTAPI(KillTimer)(timer); }

	State IsPAWN(const String& name) noexcept { return VAULTAPI(IsPAWN)(name.c_str()); }
	State IsPAWN(cRawString name) noexcept { return VAULTAPI(IsPAWN)(name); }

	Void SetServerName(const String& name) noexcept { return VAULTAPI(SetServerName)(name.c_str()); }
	Void SetServerName(cRawString name) noexcept { return VAULTAPI(SetServerName)(name); }
	Void SetServerMap(const String& map) noexcept { return VAULTAPI(SetServerMap)(map.c_str()); }
	Void SetServerMap(cRawString map) noexcept { return VAULTAPI(SetServerMap)(map); }
	Void SetServerRule(const String& key, const String& value) noexcept { return VAULTAPI(SetServerRule)(key.c_str(), value.c_str()); }
	Void SetServerRule(const String& key, cRawString value) noexcept { return VAULTAPI(SetServerRule)(key.c_str(), value); }
	Void SetServerRule(cRawString key, const String& value) noexcept { return VAULTAPI(SetServerRule)(key, value.c_str()); }
	Void SetServerRule(cRawString key, cRawString value) noexcept { return VAULTAPI(SetServerRule)(key, value); }
	UCount GetMaximumPlayers() noexcept { return VAULTAPI(GetMaximumPlayers)(); }
	UCount GetCurrentPlayers() noexcept { return VAULTAPI(GetCurrentPlayers)(); }

	String ValueToString(Index index) noexcept { return String(VAULTAPI(ValueToString)(index)); }
	String AxisToString(Index index) noexcept { return String(VAULTAPI(AxisToString)(index)); }
	String AnimToString(Index index) noexcept { return String(VAULTAPI(AnimToString)(index)); }
	String BaseToString(Base base) noexcept { return String(VAULTAPI(BaseToString)(base)); }
	String BaseToType(Base base) noexcept { return String(VAULTAPI(BaseToType)(base)); }

	State Kick(ID id) noexcept { return VAULTAPI(Kick)(id); }
	State UIMessage(ID id, const String& message, Emoticon emoticon) noexcept { return VAULTAPI(UIMessage)(id, message.c_str(), emoticon); }
	State UIMessage(ID id, cRawString message, Emoticon emoticon) noexcept { return VAULTAPI(UIMessage)(id, message, emoticon); }
	State UIMessage(const String& message, Emoticon emoticon) noexcept { return VAULTAPI(UIMessage)(static_cast<ID>(0), message.c_str(), emoticon); }
	State UIMessage(cRawString message, Emoticon emoticon) noexcept { return VAULTAPI(UIMessage)(static_cast<ID>(0), message, emoticon); }
	State ChatMessage(ID id, const String& message) noexcept { return VAULTAPI(ChatMessage)(id, message.c_str()); }
	State ChatMessage(ID id, cRawString message) noexcept { return VAULTAPI(ChatMessage)(id, message); }
	State ChatMessage(const String& message) noexcept { return VAULTAPI(ChatMessage)(static_cast<ID>(0), message.c_str()); }
	State ChatMessage(cRawString message) noexcept { return VAULTAPI(ChatMessage)(static_cast<ID>(0), message); }
	Void SetRespawnTime(Interval interval) noexcept { return VAULTAPI(SetRespawnTime)(interval); }
	Void SetSpawnCell(CELL cell) noexcept { return VAULTAPI(SetSpawnCell)(cell); }
	Void SetConsoleEnabled(State enabled) noexcept { return VAULTAPI(SetConsoleEnabled)(enabled); }
	Void SetGameWeather(WTHR weather) noexcept { return VAULTAPI(SetGameWeather)(weather); }
	Void SetGameTime(Time time) noexcept { return VAULTAPI(SetGameTime)(time); }
	Void SetGameYear(UCount year) noexcept { return VAULTAPI(SetGameYear)(year); }
	Void SetGameMonth(UCount month) noexcept { return VAULTAPI(SetGameMonth)(month); }
	Void SetGameDay(UCount day) noexcept { return VAULTAPI(SetGameDay)(day); }
	Void SetGameHour(UCount hour) noexcept { return VAULTAPI(SetGameHour)(hour); }
	Void SetTimeScale(Value scale) noexcept { return VAULTAPI(SetTimeScale)(scale); }
	State IsValid(ID id) noexcept { return VAULTAPI(IsValid)(id); }
	State IsReference(ID id) noexcept { return VAULTAPI(IsReference)(id); }
	State IsObject(ID id) noexcept { return VAULTAPI(IsObject)(id); }
	State IsItem(ID id) noexcept { return VAULTAPI(IsItem)(id); }
	State IsContainer(ID id) noexcept { return VAULTAPI(IsContainer)(id); }
	State IsActor(ID id) noexcept { return VAULTAPI(IsActor)(id); }
	State IsPlayer(ID id) noexcept { return VAULTAPI(IsPlayer)(id); }
	State IsCell(CELL cell) noexcept { return VAULTAPI(IsCell)(cell); }
	State IsInterior(CELL cell) noexcept { return VAULTAPI(IsInterior)(cell); }
	State IsItemList(ID id) noexcept { return VAULTAPI(IsItemList)(id); }
	State IsWindow(ID id) noexcept { return VAULTAPI(IsWindow)(id); }
	State IsButton(ID id) noexcept { return VAULTAPI(IsButton)(id); }
	State IsText(ID id) noexcept { return VAULTAPI(IsText)(id); }
	State IsEdit(ID id) noexcept { return VAULTAPI(IsEdit)(id); }
	State IsCheckbox(ID id) noexcept { return VAULTAPI(IsCheckbox)(id); }
	State IsRadioButton(ID id) noexcept { return VAULTAPI(IsRadioButton)(id); }
	State IsListItem(ID id) noexcept { return VAULTAPI(IsListItem)(id); }
	State IsList(ID id) noexcept { return VAULTAPI(IsList)(id); }
	State IsChatbox(ID id) noexcept { return VAULTAPI(IsChatbox)(id); }
	Type GetType(ID id) noexcept { return VAULTAPI(GetType)(id); }
	UCount GetConnection(ID id) noexcept { return VAULTAPI(GetConnection)(id); }
	UCount GetCount(Type type) noexcept { return VAULTAPI(GetCount)(type); }
	IDVector GetList(Type type) noexcept {
		RawArray<ID> data;
		UCount size = VAULTAPI(GetList)(type, &data);
		return size ? IDVector(data, data + size) : IDVector();
	}
	Interval GetRespawnTime() noexcept { return VAULTAPI(GetRespawnTime)(); }
	CELL GetSpawnCell() noexcept { return VAULTAPI(GetSpawnCell)(); }
	State GetConsoleEnabled() noexcept { return VAULTAPI(GetConsoleEnabled)(); }
	WTHR GetGameWeather() noexcept { return VAULTAPI(GetGameWeather)(); }
	Time GetGameTime() noexcept { return VAULTAPI(GetGameTime)(); }
	UCount GetGameYear() noexcept { return VAULTAPI(GetGameYear)(); }
	UCount GetGameMonth() noexcept { return VAULTAPI(GetGameMonth)(); }
	UCount GetGameDay() noexcept { return VAULTAPI(GetGameDay)(); }
	UCount GetGameHour() noexcept { return VAULTAPI(GetGameHour)(); }
	Value GetTimeScale() noexcept { return VAULTAPI(GetTimeScale)(); }

	ID GetID(Ref ref) noexcept { return VAULTAPI(GetID)(ref); }
	Ref GetReference(ID id) noexcept { return VAULTAPI(GetReference)(id); }
	Base GetBase(ID id) noexcept { return VAULTAPI(GetBase)(id); }
	Void GetPos(ID id, Value& X, Value& Y, Value& Z) noexcept { return VAULTAPI(GetPos)(id, &X, &Y, &Z); }
	Void GetAngle(ID id, Value& X, Value& Y, Value& Z) noexcept { return VAULTAPI(GetAngle)(id, &X, &Y, &Z); }
	CELL GetCell(ID id) noexcept { return VAULTAPI(GetCell)(id); }
	Lock GetLock(ID id) noexcept { return VAULTAPI(GetLock)(id); }
	NPC_ GetOwner(ID id) noexcept { return VAULTAPI(GetOwner)(id); }
	String GetBaseName(ID id) noexcept { return String(VAULTAPI(GetBaseName)(id)); }
	State IsNearPoint(ID id, Value X, Value Y, Value Z, Value R) noexcept { return VAULTAPI(IsNearPoint)(id, X, Y, Z, R); }
	ID GetItemContainer(ID id) noexcept { return VAULTAPI(GetItemContainer)(id); }
	UCount GetItemCount(ID id) noexcept { return VAULTAPI(GetItemCount)(id); }
	Value GetItemCondition(ID id) noexcept { return VAULTAPI(GetItemCondition)(id); }
	State GetItemEquipped(ID id) noexcept { return VAULTAPI(GetItemEquipped)(id); }
	State GetItemSilent(ID id) noexcept { return VAULTAPI(GetItemSilent)(id); }
	State GetItemStick(ID id) noexcept { return VAULTAPI(GetItemStick)(id); }

	#define GetContainerItemCount_Template(type) \
		UCount GetContainerItemCount(ID id, type item) noexcept { return VAULTAPI(GetContainerItemCount)(id, static_cast<Base>(item)); }
	GetContainerItemCount_Template(Base);
	GetContainerItemCount_Template(ALCH);
	GetContainerItemCount_Template(AMMO);
	GetContainerItemCount_Template(ARMA);
	GetContainerItemCount_Template(ARMO);
	GetContainerItemCount_Template(ENCH);
	GetContainerItemCount_Template(KEYM);
	GetContainerItemCount_Template(MISC);
	GetContainerItemCount_Template(NOTE);
	GetContainerItemCount_Template(WEAP);
	#undef GetContainerItemCount_Template

	UCount GetContainerItemCount(ID id) noexcept { return VAULTAPI(GetContainerItemCount)(id, static_cast<Base>(0)); }
	IDVector GetContainerItemList(ID id) noexcept {
		RawArray<ID> data;
		UCount size = VAULTAPI(GetContainerItemList)(id, &data);
		return size ? IDVector(data, data + size) : IDVector();
	}
	Value GetActorValue(ID id, ActorValue index) noexcept { return VAULTAPI(GetActorValue)(id, index); }
	Value GetActorBaseValue(ID id, ActorValue index) noexcept { return VAULTAPI(GetActorBaseValue)(id, index); }
	IDLE GetActorIdleAnimation(ID id) noexcept { return VAULTAPI(GetActorIdleAnimation)(id); }
	Index GetActorMovingAnimation(ID id) noexcept { return VAULTAPI(GetActorMovingAnimation)(id); }
	Index GetActorWeaponAnimation(ID id) noexcept { return VAULTAPI(GetActorWeaponAnimation)(id); }
	State GetActorAlerted(ID id) noexcept { return VAULTAPI(GetActorAlerted)(id); }
	State GetActorSneaking(ID id) noexcept { return VAULTAPI(GetActorSneaking)(id); }
	State GetActorDead(ID id) noexcept { return VAULTAPI(GetActorDead)(id); }
	RACE GetActorBaseRace(ID id) noexcept { return VAULTAPI(GetActorBaseRace)(id); }
	Sex GetActorBaseSex(ID id) noexcept { return VAULTAPI(GetActorBaseSex)(id); }
	State IsActorJumping(ID id) noexcept { return VAULTAPI(IsActorJumping)(id); }
	Interval GetPlayerRespawnTime(ID id) noexcept { return VAULTAPI(GetPlayerRespawnTime)(id); }
	CELL GetPlayerSpawnCell(ID id) noexcept { return VAULTAPI(GetPlayerSpawnCell)(id); }
	State GetPlayerConsoleEnabled(ID id) noexcept { return VAULTAPI(GetPlayerConsoleEnabled)(id); }
	UCount GetPlayerWindowCount(ID id) noexcept { return VAULTAPI(GetPlayerWindowCount)(id); }
	IDVector GetPlayerWindowList(ID id) noexcept {
		RawArray<ID> data;
		UCount size = VAULTAPI(GetPlayerWindowList)(id, &data);
		return size ? IDVector(data, data + size) : IDVector();
	}
	ID GetPlayerChatboxWindow(ID id) noexcept { return VAULTAPI(GetPlayerChatboxWindow)(id); }

	#define CreateObject_Template(type) \
		ID CreateObject(type object, CELL cell, Value X, Value Y, Value Z) noexcept { return VAULTAPI(CreateObject)(static_cast<Base>(object), cell, X, Y, Z); }
	CreateObject_Template(Base);
	CreateObject_Template(DOOR);
	CreateObject_Template(TERM);
	CreateObject_Template(STAT);
	#undef CreateObject_Template

	#define CreateVolatile_Template(type) \
		State CreateVolatile(ID id, type object, Value aX, Value aY, Value aZ) noexcept { return VAULTAPI(CreateVolatile)(id, static_cast<Base>(object), aX, aY, aZ); }
	CreateVolatile_Template(Base);
	CreateVolatile_Template(PROJ);
	CreateVolatile_Template(EXPL);
	#undef CreateVolatile_Template

	State DestroyObject(ID id) noexcept { return VAULTAPI(DestroyObject)(id); }
	State Activate(ID id, ID actor) noexcept { return VAULTAPI(Activate)(id, actor); }
	State SetPos(ID id, Value X, Value Y, Value Z) noexcept { return VAULTAPI(SetPos)(id, X, Y, Z); }
	State SetAngle(ID id, Value X, Value Y, Value Z) noexcept { return VAULTAPI(SetAngle)(id, X, Y, Z); }
	State SetCell(ID id, CELL cell, Value X, Value Y, Value Z) noexcept { return VAULTAPI(SetCell)(id, cell, X, Y, Z); }
	State SetLock(ID id, ID actor, Lock lock) noexcept { return VAULTAPI(SetLock)(id, actor, lock); }
	State SetOwner(ID id, NPC_ owner) noexcept { return VAULTAPI(SetOwner)(id, owner); }
	State SetBaseName(ID id, const String& name) noexcept { return VAULTAPI(SetBaseName)(id, name.c_str()); }
	State SetBaseName(ID id, cRawString name) noexcept { return VAULTAPI(SetBaseName)(id, name); }
	State PlaySound(ID id, SOUN sound) noexcept { return VAULTAPI(PlaySound)(id, sound); }

	#define CreateItem_Template(type) \
		ID CreateItem(type item, CELL cell, Value X, Value Y, Value Z) noexcept { return VAULTAPI(CreateItem)(static_cast<Base>(item), cell, X, Y, Z); }
	CreateItem_Template(Base);
	CreateItem_Template(ALCH);
	CreateItem_Template(AMMO);
	CreateItem_Template(ARMA);
	CreateItem_Template(ARMO);
	CreateItem_Template(ENCH);
	CreateItem_Template(KEYM);
	CreateItem_Template(MISC);
	CreateItem_Template(NOTE);
	CreateItem_Template(WEAP);
	#undef CreateItem_Template

	ID SetItemContainer(ID id, ID container) noexcept { return VAULTAPI(SetItemContainer)(id, container); }
	State SetItemCount(ID id, UCount count) noexcept { return VAULTAPI(SetItemCount)(id, count); }
	State SetItemCondition(ID id, Value condition) noexcept { return VAULTAPI(SetItemCondition)(id, condition); }
	State SetItemEquipped(ID id, State equipped, State silent, State stick) noexcept { return VAULTAPI(SetItemEquipped)(id, equipped, silent, stick); }
	ID CreateContainer(CONT container, CELL cell, Value X, Value Y, Value Z) noexcept { return VAULTAPI(CreateContainer)(container, cell, X, Y, Z); }

	ID CreateItemList(ID source) noexcept { return VAULTAPI(CreateItemList)(source, static_cast<Base>(0)); }
	ID CreateItemList(Base source) noexcept { return VAULTAPI(CreateItemList)(static_cast<ID>(0), source); }

	ID CreateItemList(std::initializer_list<ID> source) noexcept {
		ID result = VAULTAPI(CreateItemList)(static_cast<ID>(0), static_cast<Base>(0));

		for (auto id : source)
			VAULTAPI(AddItemList)(result, id, static_cast<Base>(0));

		return result;
	}

	ID CreateItemList(std::initializer_list<Base> source) noexcept {
		ID result = VAULTAPI(CreateItemList)(static_cast<ID>(0), static_cast<Base>(0));

		for (auto base : source)
			VAULTAPI(AddItemList)(result, static_cast<ID>(0), base);

		return result;
	}

	ID CreateItemList(std::initializer_list<AddItem_Initializer> source) noexcept {
		ID result = VAULTAPI(CreateItemList)(static_cast<ID>(0), static_cast<Base>(0));

		for (const auto& item : source)
		{
			VAULTAPI(AddItem)(result, item.item, item.count, item.condition, item.silent);

			if (item.equipped)
				VAULTAPI(EquipItem)(result, item.item, item.silent, item.stick);
		}

		return result;
	}

	State DestroyItemList(ID id) noexcept { return VAULTAPI(DestroyItemList)(id); }

	#define AddItem_Template(type) \
		ID AddItem(ID id, type item, UCount count, Value condition, State silent) noexcept { return VAULTAPI(AddItem)(id, static_cast<Base>(item), count, condition, silent); }
	AddItem_Template(Base);
	AddItem_Template(ALCH);
	AddItem_Template(AMMO);
	AddItem_Template(ARMA);
	AddItem_Template(ARMO);
	AddItem_Template(ENCH);
	AddItem_Template(KEYM);
	AddItem_Template(MISC);
	AddItem_Template(NOTE);
	AddItem_Template(WEAP);
	#undef AddItem_Template

	Void AddItem(ID id, std::initializer_list<AddItem_Initializer> source) noexcept {
		for (const auto& item : source)
		{
			VAULTAPI(AddItem)(id, item.item, item.count, item.condition, item.silent);

			if (item.equipped)
				VAULTAPI(EquipItem)(id, item.item, item.silent, item.stick);
		}
	}

	Void AddItemList(ID id, ID source) noexcept { return VAULTAPI(AddItemList)(id, source, static_cast<Base>(0)); }

	#define AddItemList_Template(type) \
		Void AddItemList(ID id, type source) noexcept { return VAULTAPI(AddItemList)(id, static_cast<ID>(0), static_cast<Base>(source)); }
	AddItemList_Template(Base);
	AddItemList_Template(NPC_);
	AddItemList_Template(CREA);
	#undef AddItemList_Template

	Void RemoveAllItems(ID id) noexcept { return VAULTAPI(RemoveAllItems)(id); }

	#define CreateActor_Template(type) \
		ID CreateActor(type actor, CELL cell, Value X, Value Y, Value Z) noexcept { return VAULTAPI(CreateActor)(static_cast<Base>(actor), cell, X, Y, Z); }
	CreateActor_Template(Base);
	CreateActor_Template(NPC_);
	CreateActor_Template(CREA);
	#undef CreateActor_Template

	Void SetActorValue(ID id, ActorValue index, Value value) noexcept { return VAULTAPI(SetActorValue)(id, index, value); }
	Void SetActorBaseValue(ID id, ActorValue index, Value value) noexcept { return VAULTAPI(SetActorBaseValue)(id, index, value); }

	#define EquipItem_Template(type) \
		VAULTFUNCTION State EquipItem(ID id, type item, State silent, State stick) noexcept { return VAULTAPI(EquipItem)(id, static_cast<Base>(item), silent, stick); }
	EquipItem_Template(Base);
	EquipItem_Template(ARMA);
	EquipItem_Template(ARMO);
	EquipItem_Template(WEAP);
	#undef EquipItem_Template

	State PlayIdle(ID id, IDLE idle) noexcept { return VAULTAPI(PlayIdle)(id, idle); }
	State SetActorMovingAnimation(ID id, Index anim) noexcept { return VAULTAPI(SetActorMovingAnimation)(id, anim); }
	State SetActorWeaponAnimation(ID id, Index anim) noexcept { return VAULTAPI(SetActorWeaponAnimation)(id, anim); }
	State SetActorAlerted(ID id, State alerted) noexcept { return VAULTAPI(SetActorAlerted)(id, alerted); }
	State SetActorSneaking(ID id, State sneaking) noexcept { return VAULTAPI(SetActorSneaking)(id, sneaking); }
	State FireWeapon(ID id) noexcept { return VAULTAPI(FireWeapon)(id); }
	Void KillActor(ID id, ID killer, Limb limbs, Death cause) noexcept { return VAULTAPI(KillActor)(id, killer, limbs, cause); }
	State SetActorBaseRace(ID id, RACE race) noexcept { return VAULTAPI(SetActorBaseRace)(id, race); }
	State AgeActorBaseRace(ID id, Count age) noexcept { return VAULTAPI(AgeActorBaseRace)(id, age); }
	State SetActorBaseSex(ID id, Sex sex) noexcept { return VAULTAPI(SetActorBaseSex)(id, sex); }
	Void SetPlayerRespawnTime(ID id, Interval interval) noexcept { return VAULTAPI(SetPlayerRespawnTime)(id, interval); }
	Void SetPlayerSpawnCell(ID id, CELL cell) noexcept { return VAULTAPI(SetPlayerSpawnCell)(id, cell); }
	Void SetPlayerConsoleEnabled(ID id, State enabled) noexcept { return VAULTAPI(SetPlayerConsoleEnabled)(id, enabled); }
	State AttachWindow(ID id, ID window) noexcept { return VAULTAPI(AttachWindow)(id, window); }
	State DetachWindow(ID id, ID window) noexcept { return VAULTAPI(DetachWindow)(id, window); }
	Void ForceWindowMode(ID id, State enabled) noexcept { return VAULTAPI(ForceWindowMode)(id, enabled); }

	ID GetWindowParent(ID id) noexcept { return VAULTAPI(GetWindowParent)(id); }
	ID GetWindowRoot(ID id) noexcept { return VAULTAPI(GetWindowRoot)(id); }
	UCount GetWindowChildCount(ID id) noexcept { return VAULTAPI(GetWindowChildCount)(id); }
	IDVector GetWindowChildList(ID id) noexcept {
		RawArray<ID> data;
		UCount size = VAULTAPI(GetWindowChildList)(id, &data);
		return size ? IDVector(data, data + size) : IDVector();
	}
	Void GetWindowPos(ID id, Value& X, Value& Y, Value& offset_X, Value& offset_Y) noexcept { return VAULTAPI(GetWindowPos)(id, &X, &Y, &offset_X, &offset_Y); }
	Void GetWindowSize(ID id, Value& X, Value& Y, Value& offset_X, Value& offset_Y) noexcept { return VAULTAPI(GetWindowSize)(id, &X, &Y, &offset_X, &offset_Y); }
	State GetWindowVisible(ID id) noexcept { return VAULTAPI(GetWindowVisible)(id); }
	State GetWindowLocked(ID id) noexcept { return VAULTAPI(GetWindowLocked)(id); }
	String GetWindowText(ID id) noexcept { return String(VAULTAPI(GetWindowText)(id)); }
	UCount GetEditMaxLength(ID id) noexcept { return VAULTAPI(GetEditMaxLength)(id); }
	String GetEditValidation(ID id) noexcept { return String(VAULTAPI(GetEditValidation)(id)); }
	State GetCheckboxSelected(ID id) noexcept { return VAULTAPI(GetCheckboxSelected)(id); }
	State GetRadioButtonSelected(ID id) noexcept { return VAULTAPI(GetRadioButtonSelected)(id); }
	UCount GetRadioButtonGroup(ID id) noexcept { return VAULTAPI(GetRadioButtonGroup)(id); }
	State GetListMultiSelect(ID id) noexcept { return VAULTAPI(GetListMultiSelect)(id); }
	UCount GetListItemCount(ID id) noexcept { return VAULTAPI(GetListItemCount)(id); }
	IDVector GetListItemList(ID id) noexcept {
		RawArray<ID> data;
		UCount size = VAULTAPI(GetListItemList)(id, &data);
		return size ? IDVector(data, data + size) : IDVector();
	}
	UCount GetListSelectedItemCount(ID id) noexcept { return VAULTAPI(GetListSelectedItemCount)(id); }
	IDVector GetListSelectedItemList(ID id) noexcept {
		RawArray<ID> data;
		UCount size = VAULTAPI(GetListSelectedItemList)(id, &data);
		return size ? IDVector(data, data + size) : IDVector();
	}
	ID GetListItemContainer(ID id) noexcept { return VAULTAPI(GetListItemContainer)(id); }
	State GetListItemSelected(ID id) noexcept { return VAULTAPI(GetListItemSelected)(id); }
	String GetListItemText(ID id) noexcept { return VAULTAPI(GetListItemText)(id); }

	ID CreateWindow(Value posX, Value posY, Value sizeX, Value sizeY, State visible, State locked, const String& text) noexcept { return VAULTAPI(CreateWindow)(posX, posY, sizeX, sizeY, visible, locked, text.c_str()); }
	ID CreateWindow(Value posX, Value posY, Value sizeX, Value sizeY, State visible, State locked, cRawString text) noexcept { return VAULTAPI(CreateWindow)(posX, posY, sizeX, sizeY, visible, locked, text); }
	State DestroyWindow(ID id) noexcept { return VAULTAPI(DestroyWindow)(id); }
	State AddChildWindow(ID id, ID child) noexcept { return VAULTAPI(AddChildWindow)(id, child); }
	State RemoveChildWindow(ID id, ID child) noexcept { return VAULTAPI(RemoveChildWindow)(id, child); }
	State SetWindowPos(ID id, Value X, Value Y, Value offset_X, Value offset_Y) noexcept { return VAULTAPI(SetWindowPos)(id, X, Y, offset_X, offset_Y); }
	State SetWindowSize(ID id, Value X, Value Y, Value offset_X, Value offset_Y) noexcept { return VAULTAPI(SetWindowSize)(id, X, Y, offset_X, offset_Y); }
	State SetWindowVisible(ID id, State visible) noexcept { return VAULTAPI(SetWindowVisible)(id, visible); }
	State SetWindowLocked(ID id, State locked) noexcept { return VAULTAPI(SetWindowLocked)(id, locked); }
	State SetWindowText(ID id, const String& text) noexcept { return VAULTAPI(SetWindowText)(id, text.c_str()); }
	State SetWindowText(ID id, cRawString text) noexcept { return VAULTAPI(SetWindowText)(id, text); }
	ID CreateButton(Value posX, Value posY, Value sizeX, Value sizeY, State visible, State locked, const String& text) noexcept { return VAULTAPI(CreateButton)(posX, posY, sizeX, sizeY, visible, locked, text.c_str()); }
	ID CreateButton(Value posX, Value posY, Value sizeX, Value sizeY, State visible, State locked, cRawString text) noexcept { return VAULTAPI(CreateButton)(posX, posY, sizeX, sizeY, visible, locked, text); }
	ID CreateText(Value posX, Value posY, Value sizeX, Value sizeY, State visible, State locked, const String& text) noexcept { return VAULTAPI(CreateText)(posX, posY, sizeX, sizeY, visible, locked, text.c_str()); }
	ID CreateText(Value posX, Value posY, Value sizeX, Value sizeY, State visible, State locked, cRawString text) noexcept { return VAULTAPI(CreateText)(posX, posY, sizeX, sizeY, visible, locked, text); }
	ID CreateEdit(Value posX, Value posY, Value sizeX, Value sizeY, State visible, State locked, const String& text) noexcept { return VAULTAPI(CreateEdit)(posX, posY, sizeX, sizeY, visible, locked, text.c_str()); }
	ID CreateEdit(Value posX, Value posY, Value sizeX, Value sizeY, State visible, State locked, cRawString text) noexcept { return VAULTAPI(CreateEdit)(posX, posY, sizeX, sizeY, visible, locked, text); }
	State SetEditMaxLength(ID id, UCount length) noexcept { return VAULTAPI(SetEditMaxLength)(id, length); }
	State SetEditValidation(ID id, const String& validation) noexcept { return VAULTAPI(SetEditValidation)(id, validation.c_str()); }
	State SetEditValidation(ID id, cRawString validation) noexcept { return VAULTAPI(SetEditValidation)(id, validation); }
	ID CreateCheckbox(Value posX, Value posY, Value sizeX, Value sizeY, State visible, State locked, const String& text) noexcept { return VAULTAPI(CreateCheckbox)(posX, posY, sizeX, sizeY, visible, locked, text.c_str()); }
	ID CreateCheckbox(Value posX, Value posY, Value sizeX, Value sizeY, State visible, State locked, cRawString text) noexcept { return VAULTAPI(CreateCheckbox)(posX, posY, sizeX, sizeY, visible, locked, text); }
	State SetCheckboxSelected(ID id, State selected) noexcept { return VAULTAPI(SetCheckboxSelected)(id, selected); }
	ID CreateRadioButton(Value posX, Value posY, Value sizeX, Value sizeY, State visible, State locked, const String& text) noexcept { return VAULTAPI(CreateRadioButton)(posX, posY, sizeX, sizeY, visible, locked, text.c_str()); }
	ID CreateRadioButton(Value posX, Value posY, Value sizeX, Value sizeY, State visible, State locked, cRawString text) noexcept { return VAULTAPI(CreateRadioButton)(posX, posY, sizeX, sizeY, visible, locked, text); }
	State SetRadioButtonSelected(ID id, State selected) noexcept { return VAULTAPI(SetRadioButtonSelected)(id, selected); }
	State SetRadioButtonGroup(ID id, UCount group) noexcept { return VAULTAPI(SetRadioButtonGroup)(id, group); }
	ID CreateList(Value posX, Value posY, Value sizeX, Value sizeY, State visible, State locked, const String& text) noexcept { return VAULTAPI(CreateList)(posX, posY, sizeX, sizeY, visible, locked, text.c_str()); }
	ID CreateList(Value posX, Value posY, Value sizeX, Value sizeY, State visible, State locked, cRawString text) noexcept { return VAULTAPI(CreateList)(posX, posY, sizeX, sizeY, visible, locked, text); }
	State SetListMultiSelect(ID id, State multiselect) noexcept { return VAULTAPI(SetListMultiSelect)(id, multiselect); }
	ID AddListItem(ID id, const String& text) noexcept { return VAULTAPI(AddListItem)(id, text.c_str()); }
	ID AddListItem(ID id, cRawString text) noexcept { return VAULTAPI(AddListItem)(id, text); }
	State RemoveListItem(ID id) noexcept { return VAULTAPI(RemoveListItem)(id); }
	ID SetListItemContainer(ID id, ID container) noexcept { return VAULTAPI(SetListItemContainer)(id, container); }
	State SetListItemSelected(ID id, State selected) noexcept { return VAULTAPI(SetListItemSelected)(id, selected); }
	State SetListItemText(ID id, const String& text) noexcept { return VAULTAPI(SetListItemText)(id, text.c_str()); }
	State SetListItemText(ID id, cRawString text) noexcept { return VAULTAPI(SetListItemText)(id, text); }
}

#endif
