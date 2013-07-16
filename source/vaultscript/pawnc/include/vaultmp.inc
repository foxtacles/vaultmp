/* Vault-Tec Multiplayer */

// Constants

const Index: {
	MAX_PLAYER_NAME = 16,
	MAX_PASSWORD_SIZE = 16,
	MAX_MESSAGE_LENGTH = 64,
	MAX_CHAT_LENGTH = 128,
}

const Type: {
	ID_REFERENCE = 0x01,
	ID_OBJECT = ID_REFERENCE << ID_REFERENCE,
	ID_ITEM = ID_OBJECT << ID_REFERENCE,
	ID_CONTAINER = ID_ITEM << ID_REFERENCE,
	ID_ACTOR = ID_CONTAINER << ID_REFERENCE,
	ID_PLAYER = ID_ACTOR << ID_REFERENCE,
	ID_WINDOW = ID_PLAYER << ID_REFERENCE,
	ID_BUTTON = ID_WINDOW << ID_REFERENCE,
	ID_TEXT = ID_BUTTON << ID_REFERENCE,
	ID_EDIT = ID_TEXT << ID_REFERENCE,

	ALL_OBJECTS = (ID_OBJECT | ID_ITEM | ID_CONTAINER | ID_ACTOR | ID_PLAYER),
	ALL_CONTAINERS = (ID_CONTAINER | ID_ACTOR | ID_PLAYER),
	ALL_ACTORS = (ID_ACTOR | ID_PLAYER),
	ALL_WINDOWS = (ID_WINDOW | ID_BUTTON | ID_TEXT | ID_EDIT),
}

const ActorValue: {
	Aggression = 0x00,
	Confidence = 0x01,
	Energy = 0x02,
	Responsibility = 0x03,
	Mood = 0x04,
	Strength = 0x05,
	Perception = 0x06,
	Endurance = 0x07,
	Charisma = 0x08,
	Intelligence = 0x09,
	Agility = 0x0A,
	Luck = 0x0B,
	ActionPoints = 0x0C,
	CarryWeight = 0x0D,
	CritChance = 0x0E,
	HealRate = 0x0F,
	Health = 0x10,
	MeleeDamage = 0x11,
	DamageResistance = 0x12,
	PoisonResistance = 0x13,
	RadResistance = 0x14,
	SpeedMultiplier = 0x15,
	Fatigue = 0x16,
	Karma = 0x17,
	XP = 0x18,
	Head = 0x19,
	Torso = 0x1A,
	LeftArm = 0x1B,
	RightArm = 0x1C,
	LeftLeg = 0x1D,
	RightLeg = 0x1E,
	Brain = 0x1F,
	Barter = 0x20,
	BigGuns = 0x21,
	EnergyWeapons = 0x22,
	Explosives = 0x23,
	Lockpick = 0x24,
	Medicine = 0x25,
	MeleeWeapons = 0x26,
	Repair = 0x27,
	Science = 0x28,
	SmallGuns = 0x29,
	Sneak = 0x2A,
	Speech = 0x2B,
	Throwing = 0x2C,
	Unarmed = 0x2D,
	InventoryWeight = 0x2E,
	Paralysis = 0x2F,
	Invisibility = 0x30,
	Chameleon = 0x31,
	NightEye = 0x32,
	DetectLifeRange = 0x33,
	FireResistance = 0x34,
	WaterBreathing = 0x35,
	RadLevel = 0x36,
	BloodyMess = 0x37,
	UnarmedDamage = 0x38,
	Assistance = 0x39,

	EnergyResistance = 0x3C,
	EMPResistance = 0x3D,
	Var1Medical = 0x3E,
	Variable02 = 0x3F,
	Variable03 = 0x40,
	Variable04 = 0x41,
	Variable05 = 0x42,
	Variable06 = 0x43,
	Variable07 = 0x44,
	Variable08 = 0x45,
	Variable09 = 0x46,
	Variable10 = 0x47,
	IgnoreCrippledLimbs = 0x48,
};

const Limb: {
	None_ =	0x0000,
	Torso_ = 0x0001,
	Head1 =	Torso_ << Torso_,
	Head2 =	Head1 << Torso_,
	LeftArm1 = Head2 << Torso_,
	LeftArm2 = LeftArm1 << Torso_,
	RightArm1 = LeftArm2 << Torso_,
	RightArm2 = RightArm1 << Torso_,
	LeftLeg1 =  RightArm2 << Torso_,
	LeftLeg2 = LeftLeg1 << Torso_,
	LeftLeg3 = LeftLeg2 << Torso_,
	RightLeg1 = LeftLeg3 << Torso_,
	RightLeg2 = RightLeg1 << Torso_,
	RightLeg3 = RightLeg2 << Torso_,
	Brain_ = RightLeg3 << Torso_,
	Weapon = Brain_ << Torso_,

	TORSO =	(Torso_),
	HEAD =	(Head1 | Head2),
	LEFT_ARM = (LeftArm1 | LeftArm2),
	RIGHT_ARM = (RightArm1 | RightArm2),
	LEFT_LEG = (LeftLeg1 | LeftLeg2 | LeftLeg3),
	RIGHT_LEG = (RightLeg1 | RightLeg2 | RightLeg3),
	BRAIN = (Brain_),
	WEAPON = (Weapon),

	ALL_LIMBS = (TORSO | HEAD | LEFT_ARM | RIGHT_ARM | LEFT_LEG | RIGHT_LEG | BRAIN | WEAPON),
}

const Death: {
	None = -1,
	Explosion = 0,
	Gun = 2,
	BluntWeapon = 3,
	HandToHand = 4,
	ObjectImpact = 5,
	Poison = 6,
	Radiation = 7,
}

const Lock: {
	Unlocked = 0xFFFFFFFF,
	Broken = 0xFFFFFFFE,
	VeryEasy = 0,
	Easy = 25,
	Average = 50,
	Hard = 75,
	VeryHard = 100,
	Impossible = 255,
}

const Emoticon: {
	Happy = 0,
	Sad = 1,
	Neutral = 2,
	Pain = 3,
}

const Interval: {
	DEFAULT_PLAYER_RESPAWN = 8000,
}

// Callbacks

forward OnSpawn(ID);
forward OnCellChange(ID, cell);
forward OnLockChange(ID, player, Lock:lock);
forward OnContainerItemChange(ID, item, count, Float:value);
forward OnActorValueChange(ID, ActorValue:index, Float:value);
forward OnActorBaseValueChange(ID, ActorValue:index, Float:value);
forward OnActorAlert(ID, Bool:alerted);
forward OnActorSneak(ID, Bool:sneaking);
forward OnActorDeath(ID, killer, Limb:limbs, Death:cause);
forward OnActorEquipItem(ID, item, Float:value);
forward OnActorUnequipItem(ID, item, Float:value);
forward OnActorDropItem(ID, item, count, Float:value);
forward OnActorPickupItem(ID, item, count, Float:value, owner);
forward OnActorPunch(ID, Bool:power);
forward OnActorFireWeapon(ID, weapon);
forward OnPlayerDisconnect(ID, reason);
forward OnPlayerRequestGame(ID);
forward OnPlayerChat(ID, message{});
forward OnWindowMode(ID, Bool:enabled);
forward OnWindowClick(ID, player);
forward OnWindowTextChange(ID, player, const text{});
forward OnClientAuthenticate(const name{}, const pwd{});
forward OnGameYearChange(year);
forward OnGameMonthChange(month);
forward OnGameDayChange(day);
forward OnGameHourChange(hour);
forward OnServerInit();
forward OnServerExit();

// Functions

native timestamp();
native CreateTimer(const func{}, interval);
native CreateTimerEx(const func{}, interval, const def{}, {Fixed,Float,_}:...);
native KillTimer(timer = 0);
native MakePublic(const func{}, const name{}, const def{});
native CallPublic(const name{}, {Fixed,Float,_}:...);

native SetServerName(const name{});
native SetServerMap(const map{});
native SetServerRule(const rule{}, const value{});
native GetMaximumPlayers();
native GetCurrentPlayers();

native ValueToString(index, value{});
native AxisToString(index, value{});
native AnimToString(index, value{});
native BaseToString(base, value{});

native Bool:Kick(ID);
native Bool:UIMessage(ID, const message{}, Emoticon:emoticon = Emoticon:Happy);
native Bool:ChatMessage(ID, const message{});
native SetRespawnTime(interval);
native SetSpawnCell(cell);
native SetConsoleEnabled(Bool:enabled);
native SetGameWeather(weather);
native SetGameTime(time);
native SetGameYear(year);
native SetGameMonth(month);
native SetGameDay(day);
native SetGameHour(hour);
native SetTimeScale(Float:scale);
native Bool:IsValid(ID);
native Bool:IsObject(ID);
native Bool:IsItem(ID);
native Bool:IsContainer(ID);
native Bool:IsActor(ID);
native Bool:IsPlayer(ID);
native Bool:IsCell(cell);
native Bool:IsInterior(cell);
native Bool:IsItemList(ID);
native Bool:IsWindow(ID);
native Bool:IsButton(ID);
native Bool:IsText(ID);
native Bool:IsEdit(ID);
native Bool:IsChatbox(ID);
native Type:GetType(ID);
native GetConnection(ID);
native GetCount(Type:type);
native GetList(Type:type, id[]);
native GetRespawnTime();
native GetSpawnCell();
native Bool:GetConsoleEnabled();
native GetGameWeather();
native GetGameTime();
native GetGameYear();
native GetGameMonth();
native GetGameDay();
native GetGameHour();
native Float:GetTimeScale();

native GetID(ref);
native GetReference(ID);
native GetBase(ID);
native GetPos(ID, &Float:X, &Float:Y, &Float:Z);
native GetAngle(ID, &Float:X, &Float:Y, &Float:Z);
native GetCell(ID);
native Lock:GetLock(ID);
native GetOwner(ID);
native GetBaseName(ID, name{});
native Bool:IsNearPoint(ID, Float:X, Float:Y, Float:Z, Float:R);
native GetItemContainer(ID);
native GetItemCount(ID);
native Float:GetItemCondition(ID);
native Bool:GetItemEquipped(ID);
native Bool:GetItemSilent(ID);
native Bool:GetItemStick(ID);
native GetContainerItemCount(ID, item = 0);
native GetContainerItemList(ID, id[]);
native Float:GetActorValue(ID, ActorValue:index);
native Float:GetActorBaseValue(ID, ActorValue:index);
native GetActorIdleAnimation(ID);
native GetActorMovingAnimation(ID);
native GetActorWeaponAnimation(ID);
native Bool:GetActorAlerted(ID);
native Bool:GetActorSneaking(ID);
native Bool:GetActorDead(ID);
native GetActorBaseRace(ID);
native Bool:GetActorBaseSex(ID);
native Bool:IsActorJumping(ID);
native GetPlayerRespawnTime(ID);
native GetPlayerSpawnCell(ID);
native Bool:GetPlayerConsoleEnabled(ID);
native GetPlayerWindowCount(ID);
native GetPlayerWindowList(ID, id[]);
native GetPlayerChatboxWindow(ID);

native CreateObject(object, ID, cell = 0, Float:X = 0.00, Float:Y = 0.00, Float:Z = 0.00);
native Bool:DestroyObject(ID);
native Bool:SetPos(ID, Float:X, Float:Y, Float:Z);
native Bool:SetAngle(ID, Float:X, Float:Y, Float:Z);
native Bool:SetCell(ID, cell, Float:X = 0.00, Float:Y = 0.00, Float:Z = 0.00);
native Bool:SetLock(ID, Lock:lock);
native Bool:SetOwner(ID, owner);
native Bool:SetBaseName(ID, const name{});
native CreateItem(item, ID, cell = 0, Float:X = 0.00, Float:Y = 0.00, Float:Z = 0.00);
native Bool:SetItemCount(ID, count);
native Bool:SetItemCondition(ID, Float:condition);
native CreateContainer(container, ID, cell = 0, Float:X = 0.00, Float:Y = 0.00, Float:Z = 0.00);
native CreateItemList(source, base = 0);
native Bool:AddItem(ID, item, count = 1, Float:value = 100.0, Bool:silent = Bool:true);
native AddItemList(ID, source, base = 0);
native RemoveItem(ID, item, count = 1, Bool:silent = Bool:true);
native RemoveAllItems(ID);
native CreateActor(actor, ID, cell = 0, Float:X = 0.00, Float:Y = 0.00, Float:Z = 0.00);
native SetActorValue(ID, ActorValue:index, Float:value);
native SetActorBaseValue(ID, ActorValue:index, Float:value);
native Bool:EquipItem(ID, item, Bool:silent = Bool:true, Bool:stick = Bool:true);
native Bool:UnequipItem(ID, item, Bool:silent = Bool:true, Bool:stick = Bool:true);
native Bool:PlayIdle(ID, idle);
native Bool:SetActorMovingAnimation(ID, anim);
native Bool:SetActorWeaponAnimation(ID, anim);
native Bool:SetActorAlerted(ID, Bool:alerted);
native Bool:SetActorSneaking(ID, Bool:sneaking);
native Bool:FireWeapon(ID);
native KillActor(ID, Limb:limbs = Limb:None_, Death:cause = Death:None);
native Bool:SetActorBaseRace(ID, race);
native Bool:AgeActorBaseRace(ID, age);
native Bool:SetActorBaseSex(ID, Bool:female);
native SetPlayerRespawnTime(ID, interval);
native SetPlayerSpawnCell(ID, cell);
native SetPlayerConsoleEnabled(ID, Bool:enabled);
native Bool:AttachWindow(ID, window);
native Bool:DetachWindow(ID, window);
native ForceWindowMode(ID, Bool:enabled);

native GetParentWindow(ID);
native GetWindowRoot(ID);
native GetWindowChildCount(ID);
native GetWindowChildList(ID, id[]);
native GetWindowPos(ID, &Float:X, &Float:Y, &Float:offset_X, &Float:offset_Y);
native GetWindowSize(ID, &Float:X, &Float:Y, &Float:offset_X, &Float:offset_Y);
native Bool:GetWindowVisible(ID);
native Bool:GetWindowLocked(ID);
native GetWindowText(ID, text{});

native CreateWindow(Float:posX, Float:posY, Float:offset_posX, Float:offset_posY, Float:sizeX, Float:sizeY, Float:offset_sizeX, Float:offset_sizeY, Bool:visible, Bool:locked, const text{});
native Bool:DestroyWindow(ID);
native Bool:AddChildWindow(ID, child);
native Bool:RemoveChildWindow(ID, child);
native Bool:SetWindowPos(ID, Float:X, Float:Y, Float:offset_X, Float:offset_Y);
native Bool:SetWindowSize(ID, Float:X, Float:Y, Float:offset_X, Float:offset_Y);
native Bool:SetWindowVisible(ID, Bool:visible);
native Bool:SetWindowLocked(ID, Bool:locked);
native Bool:SetWindowText(ID, const text{});
native CreateButton(Float:posX, Float:posY, Float:offset_posX, Float:offset_posY, Float:sizeX, Float:sizeY, Float:offset_sizeX, Float:offset_sizeY, Bool:visible, Bool:locked, const text{});
native CreateText(Float:posX, Float:posY, Float:offset_posX, Float:offset_posY, Float:sizeX, Float:sizeY, Float:offset_sizeX, Float:offset_sizeY, Bool:visible, Bool:locked, const text{});
native CreateEdit(Float:posX, Float:posY, Float:offset_posX, Float:offset_posY, Float:sizeX, Float:sizeY, Float:offset_sizeX, Float:offset_sizeY, Bool:visible, Bool:locked, const text{});
