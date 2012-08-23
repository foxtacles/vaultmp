/* Vault-Tec Multiplayer */

// Constants

const Interval: {
	DEFAULT_PLAYER_RESPAWN	=	8000,
}

const Index: {
	FALLOUT3			=	0x01,
	NEWVEGAS			=	FALLOUT3 << FALLOUT3,
	FALLOUT_GAMES		=	FALLOUT3 | NEWVEGAS,
	ALL_GAMES			=	FALLOUT_GAMES,

	MAX_PLAYER_NAME		=	16,
	MAX_PASSWORD_SIZE	=	16,
	MAX_MESSAGE_LENGTH	=	64,
	MAX_CHAT_LENGTH		=	128,
}

const Type: {
	ID_REFERENCE	=	0x01,
	ID_OBJECT		=	ID_REFERENCE << ID_REFERENCE,
	ID_ITEM			=	ID_OBJECT << ID_REFERENCE,
	ID_CONTAINER	=	ID_ITEM << ID_REFERENCE,
	ID_ACTOR		=	ID_CONTAINER << ID_REFERENCE,
	ID_PLAYER		=	ID_ACTOR << ID_REFERENCE,

	ALL_OBJECTS		=	(ID_OBJECT | ID_ITEM | ID_CONTAINER | ID_ACTOR | ID_PLAYER),
	ALL_CONTAINERS	=	(ID_CONTAINER | ID_ACTOR | ID_PLAYER),
	ALL_ACTORS		=	(ID_ACTOR | ID_PLAYER),
}

const Limb: {
	None_			=	0x0000,
	Torso			=	0x0001,
	Head1			=	Torso << Torso,
	Head2			=	Head1 << Torso,
	LeftArm1		=	Head2 << Torso,
	LeftArm2		=	LeftArm1 << Torso,
	RightArm1		=	LeftArm2 << Torso,
	RightArm2		=	RightArm1 << Torso,
	LeftLeg1		= 	RightArm2 << Torso,
	LeftLeg2		=	LeftLeg1 << Torso,
	LeftLeg3		=	LeftLeg2 << Torso,
	RightLeg1		=	LeftLeg3 << Torso,
	RightLeg2		=	RightLeg1 << Torso,
	RightLeg3		=	RightLeg2 << Torso,
	Brain			=	RightLeg3 << Torso,
	Weapon			=	Brain << Torso,

	TORSO			=	(Torso),
	HEAD			=	(Head1 | Head2),
	LEFT_ARM		=	(LeftArm1 | LeftArm2),
	RIGHT_ARM		=	(RightArm1 | RightArm2),
	LEFT_LEG		=	(LeftLeg1 | LeftLeg2 | LeftLeg3),
	RIGHT_LEG		=	(RightLeg1 | RightLeg2 | RightLeg3),
	BRAIN			=	(Brain),
	WEAPON			=	(Weapon),

	ALL_LIMBS		=	(TORSO | HEAD | LEFT_ARM | RIGHT_ARM | LEFT_LEG | RIGHT_LEG | BRAIN | WEAPON),
}

const Death: {
	None			=	-1,
	Explosion		=	0,
	Gun				=	2,
	BluntWeapon		=	3,
	HandToHand		=	4,
	ObjectImpact	=	5,
	Poison			=	6,
	Radiation		=	7,
}

// Callbacks

forward OnSpawn(ID);
forward OnCellChange(ID, cell);
forward OnContainerItemChange(ID, base, count, Float:value);
forward OnActorValueChange(ID, index, Float:value);
forward OnActorBaseValueChange(ID, index, Float:value);
forward OnActorAlert(ID, Bool:alerted);
forward OnActorSneak(ID, Bool:sneaking);
forward OnActorDeath(ID, Limb:limbs, Death:cause);
forward OnActorEquipItem(ID, base, Float:value);
forward OnActorUnequipItem(ID, base, Float:value);
forward OnActorDropItem(ID, base, count, Float:value);
forward OnActorPickupItem(ID, base, count, Float:value);
forward OnPlayerDisconnect(ID, reason);
forward OnPlayerRequestGame(ID);
forward OnPlayerChat(ID, message{});
forward OnClientAuthenticate(const name{}, const pwd{});

// Functions

native timestamp();
native CreateTimer(const func{}, interval);
native CreateTimerEx(const func{}, interval, const def{}, {Fixed,Float,_}:...);
native KillTimer(timer);
native MakePublic(const func{}, const name{}, const def{});
native CallPublic(const name{}, {Fixed,Float,_}:...);

native SetServerName(const name{});
native SetServerMap(const map{});
native SetServerRule(const rule{}, const value{});
native GetGameCode();
native GetMaximumPlayers();
native GetCurrentPlayers();

native ValueToString(index, value{});
native AxisToString(index, value{});
native AnimToString(index, value{});
native BaseToString(base, value{});

native Bool:UIMessage(ID, const message{});
native Bool:ChatMessage(ID, const message{});
native SetRespawn(interval);
native SetSpawnCell(cell);
native Bool:IsValid(ID);
native Bool:IsObject(ID);
native Bool:IsItem(ID);
native Bool:IsContainer(ID);
native Bool:IsActor(ID);
native Bool:IsPlayer(ID);
native Bool:IsCell(cell);
native Bool:IsInterior(cell);
native Type:GetType(ID);
native GetConnection(ID);
native GetCount(Type:type);
native GetList(Type:type, id[]);

native GetReference(ID);
native GetBase(ID);
native GetName(ID, name{});
native GetPos(ID, &Float:X, &Float:Y, &Float:Z);
native GetAngle(ID, &Float:X, &Float:Y, &Float:Z);
native GetCell(ID);
native Bool:IsNearPoint(ID, Float:X, Float:Y, Float:Z, Float:R);
native GetItemCount(ID);
native Float:GetItemCondition(ID);
native Bool:GetItemEquipped(ID);
native Bool:GetItemSilent(ID);
native Bool:GetItemStick(ID);
native GetContainerItemCount(ID, base = 0);
native Float:GetActorValue(ID, index);
native Float:GetActorBaseValue(ID, index);
native GetActorMovingAnimation(ID);
native Bool:GetActorAlerted(ID);
native Bool:GetActorSneaking(ID);
native Bool:GetActorDead(ID);
native Bool:IsActorJumping(ID);
native GetPlayerRespawn(ID);
native GetPlayerSpawnCell(ID);

native Bool:SetPos(ID, Float:X, Float:Y, Float:Z);
native Bool:SetCell(ID, cell, Float:X = 0.00, Float:Y = 0.00, Float:Z = 0.00);
native Bool:AddItem(ID, base, count = 1, Float:value = 100.0, Bool:silent = Bool:true);
native RemoveItem(ID, base, count = 1, Bool:silent = Bool:true);
native RemoveAllItems(ID);
native SetActorValue(ID, index, Float:value);
native SetActorBaseValue(ID, index, Float:value);
native Bool:EquipItem(ID, base, Bool:silent = Bool:true, Bool:stick = Bool:true);
native Bool:UnequipItem(ID, base, Bool:silent = Bool:true, Bool:stick = Bool:true);
native KillActor(ID, Limb:limbs = Limb:None_, Death:cause = Death:None);
native SetPlayerRespawn(ID, interval);
native SetPlayerSpawnCell(ID, cell);
