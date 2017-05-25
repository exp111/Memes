#include "autoknife.h"

bool Settings::AutoKnife::enabled = false;
bool Settings::AutoKnife::Filters::enemies = true;
bool Settings::AutoKnife::Filters::allies = false;
bool Settings::AutoKnife::onKey = true;

void AutoKnife::CreateMove(CUserCmd *cmd)
{
	if (!engine->IsInGame())
		return;

	if (!Settings::AutoKnife::enabled)
		return;

	if (!inputSystem->IsButtonDown(Settings::Triggerbot::key) && Settings::AutoKnife::onKey)
		return;

	C_BasePlayer* localplayer = (C_BasePlayer*) entityList->GetClientEntity(engine->GetLocalPlayer());
	if (!localplayer || !localplayer->GetAlive())
		return;

	C_BaseCombatWeapon* activeWeapon = (C_BaseCombatWeapon*) entityList->GetClientEntityFromHandle(localplayer->GetActiveWeapon());
	if (!activeWeapon)
		return;

	ItemDefinitionIndex itemDefinitionIndex = *activeWeapon->GetItemDefinitionIndex();
	if (!Util::Items::IsKnife(itemDefinitionIndex) && itemDefinitionIndex != ItemDefinitionIndex::WEAPON_TASER)
		return;

	if (itemDefinitionIndex == ItemDefinitionIndex::WEAPON_TASER && activeWeapon->GetAmmo() == 0)
		return;

	Vector traceStart, traceEnd;
	trace_t tr;

	QAngle viewAngles;
	engine->GetViewAngles(viewAngles);
	QAngle viewAngles_rcs = viewAngles + *localplayer->GetAimPunchAngle() * 2.0f;

	Math::AngleVectors(viewAngles_rcs, traceEnd);

	traceStart = localplayer->GetEyePosition();
	traceEnd = traceStart + (traceEnd * 8192.0f);
	
	Ray_t ray;
	ray.Init(traceStart, traceEnd);
	CTraceFilter traceFilter;
	traceFilter.pSkip = localplayer;
	trace->TraceRay(ray, 0x46004003, &traceFilter, &tr);

	C_BasePlayer* player = (C_BasePlayer*) tr.m_pEntityHit;
	if (!player)
		return;

	if (player->GetClientClass()->m_ClassID != EClassIds::CCSPlayer)
		return;

	if (player == localplayer
		|| player->GetDormant()
		|| !player->GetAlive()
		|| player->GetImmune())
		return;

	if (player->GetTeam() != localplayer->GetTeam() && !Settings::AutoKnife::Filters::enemies)
		return;

	if (player->GetTeam() == localplayer->GetTeam() && !Settings::AutoKnife::Filters::allies)
		return;

	float Distance = localplayer->GetVecOrigin().DistTo(player->GetVecOrigin());
	if (activeWeapon->GetNextPrimaryAttack() < globalVars->curtime)
	{
		if (itemDefinitionIndex == ItemDefinitionIndex::WEAPON_TASER)
		{
			if (Distance <= 184.f)
			{
				cmd->buttons |= IN_ATTACK;
			}
		}
		else
		{
			if (Distance <= 67.f && player->GetHealth() > 35)
			{
				cmd->buttons |= IN_ATTACK2;
			}
			else if (Distance <= 78.f)
			{
				cmd->buttons |= IN_ATTACK;
			}
		}
	}
}
