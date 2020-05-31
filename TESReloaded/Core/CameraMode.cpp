#include "CameraMode.h"

#if defined(NEWVEGAS)
#define kSetDialogCamera 0x00953060
#define kUpdateCameraCollisions 0x0094A0C0
#define PlayerNode Player->renderData->niNode
#define kAction_AttackBow kAction_Attack_Latency
#define kAction_AttackBowArrowAttached kAction_Attack_Latency
static const UInt32 kUpdateCameraHook = 0x0094BDDA;
static const UInt32 kUpdateCameraReturn = 0x0094BDDF;
#elif defined(OBLIVION)
#define kSetDialogCamera 0x0066C6F0
#define kUpdateCameraCollisions 0x0065F080
#define PlayerNode Player->niNode
static const UInt32 kUpdateCameraHook = 0x0066BE6E;
static const UInt32 kUpdateCameraReturn = 0x0066BE7C;
#endif
static HighProcess* DialogActorProcess = NULL;
static NiPoint3 ReticleOffset = { 0.0f, 0.0f, 0.0f };

#if defined(OBLIVION) || defined(NEWVEGAS)
class CameraMode {
public:
	void TrackSetDialogCamera(Actor* Act, float Arg2, UInt8 Arg3);
	void TrackUpdateCameraCollisions(NiPoint3* CameraLocalPos, NiPoint3* PlayerWorldPos, UInt8 CameraChasing);
};

void (__thiscall CameraMode::* SetDialogCamera)(Actor*, float, UInt8);
void (__thiscall CameraMode::* TrackSetDialogCamera)(Actor*, float, UInt8);
void CameraMode::TrackSetDialogCamera(Actor* Act, float Arg2, UInt8 Arg3) {

	DialogActorProcess = (HighProcess*)Act->process;

}

void (__thiscall CameraMode::* UpdateCameraCollisions)(NiPoint3*, NiPoint3*, UInt8);
void (__thiscall CameraMode::* TrackUpdateCameraCollisions)(NiPoint3*, NiPoint3*, UInt8);
void CameraMode::TrackUpdateCameraCollisions(NiPoint3* CameraPosition, NiPoint3* PlayerPosition, UInt8 CameraChasing) {
	
	HighProcess* Proc = (HighProcess*)Player->process;
	UInt8 Crosshair = TheSettingManager->SettingsMain.CameraMode.Crosshair;
	UInt8 DisableFading = Player->DisableFading;
	UInt8 SitSleepState = Player->GetSitSleepState();
	
	if (SitSleepState >= 8 && SitSleepState <= 10) Player->DisableFading = 1;
	if (Player->isThirdPerson) {
		CameraChasing = !TheSettingManager->SettingsMain.CameraMode.ChasingThird;
	}
	else {
		Player->DisableFading = 1;
		CameraChasing = !TheSettingManager->SettingsMain.CameraMode.ChasingFirst;
	}
	(this->*UpdateCameraCollisions)(CameraPosition, PlayerPosition, CameraChasing);
	Player->DisableFading = DisableFading;
	if (Crosshair > 0) {
		if (Crosshair == 1) {
			MenuManager->SetCrosshair(0);
		}
		else {
			SInt16 CurrentAction = Proc->currentAction;
			if (CurrentAction == HighProcess::kAction_AttackBow || CurrentAction == HighProcess::kAction_AttackBowArrowAttached)
				MenuManager->SetCrosshair(1);
			else
				MenuManager->SetCrosshair(0);
		}
	}

}

static NiPoint3 From = { 0.0f, 0.0f, 0.0f };
static NiPoint3 FromOffset = { 0.0f, 0.0f, 0.0f };
void UpdateCamera(NiAVObject* CameraNode, NiPoint3* LocalPosition) {
	
	bool CameraMode = TheSettingManager->SettingsMain.CameraMode.Enabled;
	NiMatrix33* CameraRotation = &CameraNode->m_localTransform.rot;
	NiPoint3* CameraPosition = &CameraNode->m_localTransform.pos;
	UInt8 SitSleepState = Player->GetSitSleepState();
	HighProcess* Proc = (HighProcess*)Player->process;

	if (SitSleepState < 8 || SitSleepState > 10) From.x = 0.0f;
	if (SitSleepState >= 8 && SitSleepState <= 10) {
		NiPoint3 v;
		NiMatrix33 mw, ml;
		float x, y, z, r;
		NiPoint3 Rot = { 0.0f, 0.0f, 0.0f };
		if (From.x == 0.0f) {
			From.x = CameraPosition->x;
			From.y = CameraPosition->y;
			From.z = CameraPosition->z - 20;
		}
		else {
			FromOffset = { 0.0f, 0.0f, 0.0f };
			if (TheKeyboardManager->OnControlPressed(2))
				FromOffset.x -= 5.0f;
			else if (TheKeyboardManager->OnControlPressed(3))
				FromOffset.x += 5.0f;
			if (TheKeyboardManager->OnControlPressed(0))
				FromOffset.y += 5.0f;
			else if (TheKeyboardManager->OnControlPressed(1))
				FromOffset.y -= 5.0f;
			if (TheKeyboardManager->OnControlPressed(4))
				FromOffset.z += 5.0f;
			else if (TheKeyboardManager->OnControlPressed(6))
				FromOffset.z -= 5.0f;
			if (FromOffset.x != 0.0f || FromOffset.y != 0.0f || FromOffset.z != 0.0f) {
				NiPoint3 r = CameraNode->m_worldTransform.rot * FromOffset;
				From.x += r.x;
				From.y += r.y;
				From.z += r.z;
			}
		}
		CameraPosition->x = From.x;
		CameraPosition->y = From.y;
		CameraPosition->z = From.z;
		NiPoint3* HeadPosition = &Proc->animData->nHead->m_worldTransform.pos;
		mw.GenerateRotationMatrixZXY(&Rot, 1);
		x = CameraPosition->x - HeadPosition->x;
		y = CameraPosition->y - HeadPosition->y;
		z = CameraPosition->z - HeadPosition->z;
		r = sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
		Rot.x = RadiansToDegrees(atan2(y, x)) + 90;
		Rot.y = RadiansToDegrees(acos(z / r)) - 90;
		Rot.z = 0;
		ml.GenerateRotationMatrixZXY(&Rot, 1);
		CameraNode->m_worldTransform.rot = mw;
		CameraNode->m_localTransform.rot = ml;
	}
	else if (CameraMode && (MenuManager->IsActive(Menu::MenuType::kMenuType_Dialog) || MenuManager->IsActive(Menu::MenuType::kMenuType_Persuasion))) {
		NiPoint3 v;
		NiMatrix33 mw, ml;
		float x, y, z, r;
		NiPoint3 Rot = { 0.0f, 0.0f, 0.0f };
		NiPoint3* HeadPosition = &Proc->animData->nHead->m_worldTransform.pos;
		v = PlayerNode->m_worldTransform.rot * TheSettingManager->SettingsMain.CameraMode.DialogOffset;
		CameraPosition->x = HeadPosition->x + v.x;
		CameraPosition->y = HeadPosition->y + v.y;
		CameraPosition->z = HeadPosition->z + v.z;
		HeadPosition = &DialogActorProcess->animData->nHead->m_worldTransform.pos;
		mw.GenerateRotationMatrixZXY(&Rot, 1);
		x = CameraPosition->x - HeadPosition->x;
		y = CameraPosition->y - HeadPosition->y;
		z = CameraPosition->z - HeadPosition->z;
		r = sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
		Rot.x = RadiansToDegrees(atan2(y, x)) + 90;
		Rot.y = RadiansToDegrees(acos(z / r)) - 90;
		Rot.z = 0;
		ml.GenerateRotationMatrixZXY(&Rot, 1);
		CameraNode->m_worldTransform.rot = mw;
		CameraNode->m_localTransform.rot = ml;
	}
	else if (CameraMode && !Player->IsVanityView()) {
		if (!Player->isThirdPerson) {
			if (!Proc->KnockedState && SitSleepState != 3 && SitSleepState != 5 && SitSleepState != 8 && SitSleepState != 9 && SitSleepState != 10) {
				NiPoint3* HeadPosition = &Proc->animData->nHead->m_worldTransform.pos;
				NiPoint3 r = PlayerNode->m_worldTransform.rot * TheSettingManager->SettingsMain.CameraMode.Offset;
				float x = RadiansToDegrees(atan2(CameraRotation->data[2][1], CameraRotation->data[2][2]));
				CameraPosition->x = HeadPosition->x + r.x;
				CameraPosition->y = HeadPosition->y + r.y;
				if (x <= -60.0f) CameraPosition->z = HeadPosition->z + r.z;
				ReticleOffset.x = r.x;
				ReticleOffset.y = r.y;
				ReticleOffset.z = r.z;
				#if defined(NEWVEGAS)
				memcpy(&Player->ReticleOffset, &ReticleOffset, 12);
				//Player->ReticleOffset.x = ReticleOffset.x;
				//Player->ReticleOffset.y = ReticleOffset.y;
				//Player->ReticleOffset.z = ReticleOffset.z;
				#endif
			}
		}
	}
	LocalPosition->x = CameraPosition->x;
	LocalPosition->y = CameraPosition->y;
	LocalPosition->z = CameraPosition->z;

}

static __declspec(naked) void UpdateCameraHook() {

	__asm {
#if defined(NEWVEGAS)
		push	0
		push	0
		push	ecx
		pushad
		lea		edx, [ebp - 0x60]
		push	edx
		push	eax
		call	UpdateCamera
		pop		eax
		pop		edx
		popad
#elif defined (OBLIVION)
		pushad
		lea		ecx, [esp + 0x58]
		push	ecx
		push	eax
		call	UpdateCamera
		pop		eax
		pop		ecx
		popad
#endif
		jmp		kUpdateCameraReturn
	}

}
#if defined(OBLIVION)
void SetReticleOffset(NiPoint3* CameraPos) {
	
	CameraPos->x += ReticleOffset.x;
	CameraPos->y += ReticleOffset.y;
	CameraPos->z += ReticleOffset.z;

}

static const UInt32 kSetReticleOffsetHook = 0x00580796;
static const UInt32 kSetReticleOffsetReturn = 0x0058079C;
static __declspec(naked) void SetReticleOffsetHook() {

	__asm {
		lea		eax, [esp + 0x20]
		push	eax
		call	SetReticleOffset
		pop		eax
		mov     ecx, Player
		jmp		kSetReticleOffsetReturn
	}

}
#endif

void CreateCameraModeHook() {

	*((int *)&SetDialogCamera)				= kSetDialogCamera;
	TrackSetDialogCamera					= &CameraMode::TrackSetDialogCamera;
	*((int *)&UpdateCameraCollisions)		= kUpdateCameraCollisions;
	TrackUpdateCameraCollisions				= &CameraMode::TrackUpdateCameraCollisions;

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)SetDialogCamera,			*((PVOID *)&TrackSetDialogCamera));
	DetourAttach(&(PVOID&)UpdateCameraCollisions,	*((PVOID *)&TrackUpdateCameraCollisions));
	DetourTransactionCommit();
	WriteRelJump(kUpdateCameraHook,	(UInt32)UpdateCameraHook);
#if defined(OBLIVION)
	WriteRelJump(kSetReticleOffsetHook, (UInt32)SetReticleOffsetHook);
	WriteRelJump(0x0066B769, 0x0066B795); // Does not lower the player Z axis value (fixes the bug of the camera on feet after resurrection)
	WriteRelJump(0x006738B1, 0x00673935); // Cancels the fPlayerDeathReloadTime
#elif defined(NEWVEGAS)
	WriteRelJump(0x00761DE7, 0x00761DF4); // Avoids to toggle the body
	WriteRelJump(0x0093FC04, 0x0093FC3B); // Avoids to toggle the body
	WriteRelJump(0x00950244, 0x0095027E); // Avoids to toggle the body
#endif
}

void CreateCameraModeSleepingHook() {

	*((int *)&UpdateCameraCollisions)	= kUpdateCameraCollisions;
	TrackUpdateCameraCollisions			= &CameraMode::TrackUpdateCameraCollisions;

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)UpdateCameraCollisions,	*((PVOID *)&TrackUpdateCameraCollisions));
	DetourTransactionCommit();

	WriteRelJump(kUpdateCameraHook, (UInt32)UpdateCameraHook);

}

#elif defined (SKYRIM)
class CameraMode {
public:
	int TrackSetCameraState(TESCameraState* CameraState);
	void TrackManageButtonEvent(ButtonEvent* Event, int Arg2);
	void TrackSetCameraPosition();
};

int (__thiscall CameraMode::* SetCameraState)(TESCameraState*);
int (__thiscall CameraMode::* TrackSetCameraState)(TESCameraState*);
int CameraMode::TrackSetCameraState(TESCameraState* CameraState) {
	
	PlayerCamera* Camera = (PlayerCamera*)this;
	bool IsWeaponOut = false;

	if (Camera->cameraNode->m_name && CameraState->camera->thirdPersonState2 != NULL) {
		if (CameraState->stateId == TESCameraState::CameraState::kCameraState_FirstPerson) {
			if (TheRenderManager->FirstPersonView && TogglePOV) {
				CameraState = Camera->thirdPersonState2;
				TheRenderManager->FirstPersonView = false;
			}
			else {
				CameraState = Camera->thirdPersonState2;
				TheRenderManager->FirstPersonView = true;
			}
		}
		else if (CameraState->stateId == TESCameraState::CameraState::kCameraState_ThirdPerson2) TheRenderManager->FirstPersonView = false;
		if (TheRenderManager->FirstPersonView && CameraState->stateId != TESCameraState::CameraState::kCameraState_ThirdPerson2) TheRenderManager->FirstPersonView = false;
		if (!TheRenderManager->FirstPersonView && CameraState->stateId == TESCameraState::CameraState::kCameraState_ThirdPerson2) {
			IsWeaponOut = Player->actorState.IsWeaponOut();
			Camera->AllowVanityMode = !IsWeaponOut;
			Camera->UpdateOverShoulder(IsWeaponOut);
		}
		TogglePOV = false;
	}
	return (this->*SetCameraState)(CameraState);

}

void (__thiscall CameraMode::* ManageButtonEvent)(ButtonEvent*, int);
void (__thiscall CameraMode::* TrackManageButtonEvent)(ButtonEvent*, int);
void CameraMode::TrackManageButtonEvent(ButtonEvent* Event, int Arg2) {
	
	ThirdPersonState* State = (ThirdPersonState*)(this - 0x10); //ecx is ThirdPersonState for PlayerInputHandler (class is "shifted" due to the multi inheritance)
	
	(this->*ManageButtonEvent)(Event, Arg2);
	if (State->stateId == TESCameraState::CameraState::kCameraState_ThirdPerson2) {
		if (PlayerControls::Get()->IsCamSwitchControlEnabled()) {
			if (State->TogglePOV) TogglePOV = true;
			if (TheRenderManager->FirstPersonView && *Event->GetControlID() == InputStringHolder::Get()->zoomOut) State->camera->SetCameraState(State->camera->thirdPersonState2);
		}
	}

}

void (__thiscall CameraMode::* SetCameraPosition)();
void (__thiscall CameraMode::* TrackSetCameraPosition)();
void CameraMode::TrackSetCameraPosition() {

	ThirdPersonState* State = (ThirdPersonState*)this;
	BSFixedString Head;

	if (TheRenderManager->FirstPersonView) {
		Head.Create("NPC Head [Head]");
		NiNode* ActorNode = Player->GetNiRootNode(0);
		NiPoint3* HeadPosition = &ActorNode->GetObjectByName(&Head)->m_worldTransform.pos;
		NiPoint3 v = ActorNode->m_worldTransform.rot * TheSettingManager->SettingsMain.CameraMode.Offset;
		State->CameraPosition.x = HeadPosition->x + v.x;
		State->CameraPosition.y = HeadPosition->y + v.y;
		State->CameraPosition.z = HeadPosition->z + v.z;
		State->OverShoulderPosX = State->OverShoulderPosY = State->OverShoulderPosZ = 0.0f;
		State->camera->AllowVanityMode = 0;
		Head.Dispose();
	}
	(this->*SetCameraPosition)();
	
}

void CreateCameraModeHook()
{

	*((int *)&SetCameraState)			= 0x006533D0;
	TrackSetCameraState					= &CameraMode::TrackSetCameraState;
	*((int *)&ManageButtonEvent)		= 0x00840BE0;
	TrackManageButtonEvent				= &CameraMode::TrackManageButtonEvent;
	*((int *)&SetCameraPosition)		= 0x0083F690;
	TrackSetCameraPosition				= &CameraMode::TrackSetCameraPosition;

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)SetCameraState,		*((PVOID *)&TrackSetCameraState));
	DetourAttach(&(PVOID&)ManageButtonEvent,	*((PVOID *)&TrackManageButtonEvent));
	DetourAttach(&(PVOID&)SetCameraPosition,	*((PVOID *)&TrackSetCameraPosition));
	DetourTransactionCommit();
	
	SafeWrite8(0x0083F69B, 0); // Stops PlayerCharacter fading

}
#endif