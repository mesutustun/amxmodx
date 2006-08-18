/*
 * DoDX 
 * Copyright (c) 2004 Lukasz Wlasinski
 *
 *
 *    This program is free software; you can redistribute it and/or modify it
 *    under the terms of the GNU General Public License as published by the
 *    Free Software Foundation; either version 2 of the License, or (at
 *    your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful, but
 *    WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software Foundation,
 *    Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *    In addition, as a special exception, the author gives permission to
 *    link the code of this program with the Half-Life Game Engine ("HL
 *    Engine") and Modified Game Libraries ("MODs") developed by Valve,
 *    L.L.C ("Valve").  You must obey the GNU General Public License in all
 *    respects for all of the code used other than the HL Engine and MODs
 *    from Valve.  If you modify this file, you may extend this exception
 *    to your version of the file, but you are not obligated to do so.  If
 *    you do not wish to do so, delete this exception statement from your
 *    version.
 *
 */
#include "amxxmodule.h"
#include "dodx.h"

void Client_ResetHUD_End(void* mValue)
{
	mPlayer->clearStats = gpGlobals->time + 0.25f;
}

void Client_RoundState(void* mValue)
{
	if ( mPlayer ) return;
	int result = *(int*)mValue;
	if ( result == 1 )
	{
		for (int i=1;i<=gpGlobals->maxClients;i++)
		{
			CPlayer *pPlayer = GET_PLAYER_POINTER_I(i);
			if (pPlayer->ingame) 
			{
				pPlayer->clearRound = gpGlobals->time + 0.25f;
			}
		}
	}
}

void Client_TeamScore(void* mValue)
{
	static int index;

	switch(mState++)
	{
	case 0:
		index = *(int*)mValue;
		break;
	case 1:
		switch (index)
		{
		case 1:
			AlliesScore = *(int*)mValue;
			break;
		case 2:
			AxisScore = *(int*)mValue;
			break;
		}
		break;
	}
}

void Client_ObjScore(void* mValue)
{
	static CPlayer *pPlayer;
	static int score;

	switch(mState++)
	{
	case 0:
		pPlayer = GET_PLAYER_POINTER_I(*(int*)mValue);
		break;
	case 1:
		score = *(int*)mValue;
		if ( (pPlayer->lastScore = score - (int)(pPlayer->savedScore)) && isModuleActive() )
		{
			pPlayer->updateScore(pPlayer->current,pPlayer->lastScore);
			pPlayer->sendScore = (int)(gpGlobals->time + 0.25f);
		}
		pPlayer->savedScore = score;
		break;
	}
}


void Client_CurWeapon(void* mValue)
{
  static int iState;
  static int iId;

  switch (mState++)
  {
  case 0: 
    iState = *(int*)mValue;
    break;
  case 1:
    if (!iState) break; 
    iId = *(int*)mValue;
	break;
  case 2:
    if ( !iState || !isModuleActive() )
		break;
	int iClip = *(int*)mValue;
	mPlayer->current = iId;

   	if ( weaponData[iId].needcheck )
	{
		iId = get_weaponid(mPlayer);
		mPlayer->current = iId;
	}

	if (iClip > -1) {
		if ( mPlayer->current == 17 )
		{
			if ( iClip+2 == mPlayer->weapons[iId].clip)
				mPlayer->saveShot(iId);
		}
		else 
		{
			if ( iClip+1 == mPlayer->weapons[iId].clip)
				mPlayer->saveShot(iId);
		}
	}
    mPlayer->weapons[iId].clip = iClip;
  }
}

/*
Nie ma damage event ...
*/
void Client_Health_End(void* mValue){

    if ( !isModuleActive() )
		return;

	edict_t *enemy = mPlayer->pEdict->v.dmg_inflictor;
	int damage = (int)mPlayer->pEdict->v.dmg_take;

	if ( !mPlayer || !damage || !enemy )
		return;
	
	int weapon = 0;
	int aim = 0;
		
	mPlayer->pEdict->v.dmg_take = 0.0; 
	
	CPlayer* pAttacker = NULL;

	if ( enemy->v.flags & (FL_CLIENT | FL_FAKECLIENT) )
	{
		pAttacker = GET_PLAYER_POINTER(enemy);
		weapon = pAttacker->current;

		if ( weaponData[weapon].needcheck )
			weapon = get_weaponid(pAttacker);

		aim = pAttacker->aiming;

		if ( weaponData[weapon].melee )
			pAttacker->saveShot(weapon);
	}
	else 
		g_grenades.find( enemy , &pAttacker , weapon );

	int TA = 0;
	
	if ( !pAttacker )
	{
		pAttacker = mPlayer;
	}

	if ( pAttacker->index != mPlayer->index )
	{ 
		pAttacker->saveHit( mPlayer , weapon , damage, aim );

		if ( mPlayer->pEdict->v.team == pAttacker->pEdict->v.team )
			TA = 1;
	}

	MF_ExecuteForward( iFDamage, pAttacker->index, mPlayer->index, damage, weapon, aim, TA );

	if ( !mPlayer->IsAlive() )
	{
		pAttacker->saveKill(mPlayer,weapon,( aim == 1 ) ? 1:0 ,TA);
		MF_ExecuteForward( iFDeath, pAttacker->index, mPlayer->index, weapon, aim, TA );
	}
}

void Client_AmmoX(void* mValue)
{
  static int iAmmo;

  switch (mState++)
  {
  case 0:
    iAmmo = *(int*)mValue;
    break;
  case 1:
	if (!mPlayer ) 
		break;
    for(int i = 1; i < MAX_WEAPONS ; ++i)
	{
      if (iAmmo == weaponData[i].ammoSlot)
        mPlayer->weapons[i].ammo = *(int*)mValue;
	}
  }
}

void Client_AmmoShort(void* mValue)
{
  static int iAmmo;

  switch (mState++)
  {
  case 0:
    iAmmo = *(int*)mValue;
    break;
  case 1:
	if (!mPlayer ) break;
    for(int i = 1; i < MAX_WEAPONS ; ++i) 
	{
      if (iAmmo == weaponData[i].ammoSlot)
        mPlayer->weapons[i].ammo = *(int*)mValue;
	}
  }
}

/*
Working on being able to modify and switch weapons as they are sent to the client

void WeaponList(void* value)
{
	if(!mPlayer)
		return;

	if(!mPlayer->ingame || ignoreBots(mPlayer->pEdict))
		return;

	switch(mPlayer->position)
	{
		case 0: MF_Log("pszName = %s", value); break;			// string	weapon name
		case 1: MF_Log("pszAmmo1 = %d", (int)value); break;		// byte		Ammo Type
		case 2: MF_Log("iMaxAmmo1 = %d", (int)value); break;	// byte     Max Ammo 1
		case 3: MF_Log("pszAmmo2 = %d", (int)value); break;		// byte		Ammo2 Type
		case 4: MF_Log("iMaxAmmo2 = %d", (int)value); break;	// byte     Max Ammo 2
		case 5: MF_Log("iSlot = %d", (int)value); break;		// byte		bucket
		case 6: MF_Log("iPosition = %d", (int)value); break;	// byte		bucket pos
		case 7: MF_Log("iId = %d", (int)value); break;			// byte		id (bit index into pev->weapons)
		case 8: MF_Log("iFlags = %d", (int)value); break;		// byte		Flags
	};

	mPlayer->position++;
}

void WeaponList_End(void* mValue)
{
	if(!mPlayer)
		return;

	MF_Log("Done with %d", mPlayer->position);
	mPlayer->position = 0;
}

struct weapon_info_s
{
	char *pszName;			// string	weapon name
	int pszAmmo1;			// byte		Ammo Type
	int iMaxAmmo1;			// byte     Max Ammo 1
	int pszAmmo2;			// byte		Ammo2 Type
	int iMaxAmmo2;			// byte     Max Ammo 2
	int iSlot;				// byte		bucket
	int iPosition;			// byte		bucket pos
	int iId;				// byte		id (bit index into pev->weapons)
	int iFlags;				// byte		Flags
}weapon_info_t;

MESSAGE_BEGIN( MSG_ONE, gmsgWeaponList, NULL, pev );  
	WRITE_STRING(pszName);			// string	weapon name
	WRITE_BYTE(GetAmmoIndex(II.pszAmmo1));	// byte		Ammo Type
	WRITE_BYTE(II.iMaxAmmo1);				// byte     Max Ammo 1
	WRITE_BYTE(GetAmmoIndex(II.pszAmmo2));	// byte		Ammo2 Type
	WRITE_BYTE(II.iMaxAmmo2);				// byte     Max Ammo 2
	WRITE_BYTE(II.iSlot);					// byte		bucket
	WRITE_BYTE(II.iPosition);				// byte		bucket pos
	WRITE_BYTE(II.iId);						// byte		id (bit index into pev->weapons)
	WRITE_BYTE(II.iFlags);					// byte		Flags
MESSAGE_END();
*/
