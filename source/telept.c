#include "Doom.h"

/**********************************

	Kill all monsters around the given spot

**********************************/

static void P_Telefrag(mobj_t *thing,Fixed x,Fixed y)
{
	Fixed delta;		/* Differance in size */
	Fixed size;			/* Size of the critter */
	mobj_t *m;			/* Pointer to object */

	for (m=mobjhead.next;m!=&mobjhead;m=m->next) {
		if (m->flags & MF_SHOOTABLE) {		/* Can I kill it? */
			size = m->radius + thing->radius + (4<<FRACBITS);
			delta = m->x - x;		/* Differance to object */
			if (delta >= -size && delta < size) {	/* In x range? */
				delta = m->y - y;		/* Y differance */
				if (delta >= -size && delta < size) {	/* In y range? */
					DamageMObj(m,thing,thing,10000);	/* Blast it */
					m->flags &= ~(MF_SOLID|MF_SHOOTABLE);	/* Gone! */
				}
			}
		}
	}
}

/**********************************

	Teleport a player or monster

**********************************/

Boolean EV_Teleport(line_t *line,mobj_t *thing)
{
	Word i;		/* Temp */
	Word tag;	/* ID tag */
	Boolean	flag;	/* Result temp */
	sector_t *sector;	/* Pointer to temp sector */
	sector_t *sec;		/* Loop sector pointer */
	mobj_t *m;		/* Running object pointer */
	mobj_t *fog;	/* Fog object */
	angle_t an;		/* Angle */
	vector_t DivLine;
	Fixed oldx,oldy,oldz;	/* X,Y and Z before teleportation */

	if (thing->flags & MF_MISSILE) {
		return FALSE;			/* Don't teleport missiles */
	}

	MakeVector(line,&DivLine);
	if (!PointOnVectorSide(thing->x,thing->y,&DivLine)) {	/* Which side... */
		return FALSE;		/* so you can get out of teleporter */
	}

	tag = line->tag;		/* Cache the teleport item tag */
	i = 0;
	sec = sectors;			/* Get the sector pointer */
	do {
		if (sec->tag == tag) {
			for (m=mobjhead.next ; m != &mobjhead ; m=m->next) {
				if (m->InfoPtr != &mobjinfo[MT_TELEPORTMAN]) {
					continue;		/* not a teleportman */
				}
				sector = m->subsector->sector;	/* Get the sector attachment */
				if (sector!=sec) {
					continue;		/* wrong sector */
				}
				oldx = thing->x;		/* Previous x,y,z */
				oldy = thing->y;
				oldz = thing->z;
				thing->flags |= MF_TELEPORT;	/* Mark as a teleport */
				P_Telefrag(thing,m->x,m->y);	/* Frag everything at dest */
				flag = P_TryMove(thing,m->x,m->y);	/* Put it there */
				thing->flags &= ~MF_TELEPORT;	/* Clear the flag */
				if (!flag) {
					return FALSE;	/* (Can't teleport) move is blocked */
				}
				thing->z = thing->floorz;		/* Set the proper z */

/* Spawn teleport fog at source and destination */

				fog = SpawnMObj(oldx,oldy,oldz,&mobjinfo[MT_TFOG]);	/* Previous */
				S_StartSound(&fog->x,sfx_telept);
				an = m->angle >> ANGLETOFINESHIFT;
				fog = SpawnMObj(m->x+20*finecosine[an], m->y+20*finesine[an]
					,thing->z,&mobjinfo[MT_TFOG]);		/* In front of player */
				S_StartSound(&fog->x,sfx_telept);
				if (thing->player) {		/* Add a timing delay */
					thing->reactiontime = TICKSPERSEC+(TICKSPERSEC/5);	/* don't move for a bit */
				}
				thing->angle = m->angle;		/* Set the angle */
				thing->momx = thing->momy = thing->momz = 0;	/* No sliding */
				return TRUE;	/* I did it */
			}
		}
		++sec;		/* Next sector */
	} while (++i<numsectors);
	return FALSE;	/* Didn't teleport */
}
