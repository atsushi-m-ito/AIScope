
#ifdef USE_MPI
#include "mpi.h"
#endif


#include "AFSIv10.h"
#include "NBonds.h"

#include "IPotential.h"
//#include "glips.h"
#include <stdlib.h>

IPotential* CreatePotential(TYPE_POTENTIAL type_pot, int maxcount){
	switch (type_pot){
/*
	case TYPE_POT_BOP:
		return new BOP(maxcount);
	case TYPE_POT_LJ:
		return new LJ( maxcount, 1.0, 1.0, 2.5);
	case TYPE_POT_REBO:
	case TYPE_POT_REBOv3:
		return new REBOv3(maxcount);
	case TYPE_POT_REO:
	case TYPE_POT_REOv2:
		return new REOv2(maxcount);
	case TYPE_POT_TERSOFF:
		return new TERSOFF(maxcount);
	case TYPE_POT_AFS:
		return new AFS(maxcount);
*/
	case TYPE_POT_AFSI:
	case TYPE_POT_AFSIv7:
		return new AFSIv7(maxcount);
/*
	case TYPE_POT_THOMASFERMI:
		return new ThomasFermi(maxcount);
*/
	case TYPE_POT_NBONDS:
		return new NBonds(maxcount);

//	case TYPE_POT_AFSHeAr:
//		return new AFSHeAr(KIND_W, maxcount);
	}

	return NULL;
}

