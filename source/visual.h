
#ifndef __visual_h__
#define __visual_h__

//#define VISUAL_TYPE_ATOM			1
//#define VISUAL_TYPE_BOND			2
#define VISUAL_TYPE_TRAJ			3
#define VISUAL_TYPE_ATOMCOLOR		4
//#define VISUAL_TYPE_ATOMPOLY		5
//#define VISUAL_TYPE_BONDPOLY		6


#define VISUAL_ATOM_NONE			(0x0)
#define VISUAL_ATOM_SPHERE			(0x1)
#define VISUAL_ATOM_POINT			(0x2)

#define VISUAL_BOND_NONE			(0x0)
#define VISUAL_BOND_PIPE			(0x1)
#define VISUAL_BOND_ENERGYJ			(0x2)

#define VISUAL_TRAJ_NONE			(0x0)
#define VISUAL_TRAJ_ON				(0x1)


#define	VISUAL_ATOMCOLOR_Z			(0x0)
#define	VISUAL_ATOMCOLOR_BONDNUM	(0x1)
#define VISUAL_ATOMCOLOR_PRESSURE   (0x2)
#define VISUAL_ATOMCOLOR_HEIGHT     (0x3)


struct VISUAL_SETTING{
	int atom = 0;
	int atom_poly = 0;
	int atom_color = 0;
	int bond = 0;	
	int bond_poly = 0;	
    int history_frame = 0;
	double atom_radius = 0.0;
	double trajectory_width = 0.0;
};


#endif	//__visual_h__