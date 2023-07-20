
/*
ボンドリストを作る.
バッファが足りないときは-1を返す
*/
template <class TBVECTOR>
int LCL::CreateBondList(int start_index, int count, const vec3d* r, double CUTOFF_SQR, TBVECTOR* bvector, BVEC_REGION* bvecStartEnd, int buf_sz_bond){
	// local variable ------------------------------;

#pragma omp parallel
{

	LCL_INFO info;
	vec3d periodic_vec[27];
	this->GetCellInfo(&info, periodic_vec);
	
	const int cell_wx = info.cell_x;
	const int cell_wy = info.cell_y;
	const int cell_wz = info.cell_z;
	const int* icell = info.icell;
	const int* first_in_cell = info.first_in_cell;
	const int* inext = info.inext;
	
	const int cell_wxwy = cell_wx * cell_wy;
	const int cell_wxwywz = cell_wxwy * cell_wz;



#ifdef _OPENMP
	// shift array pointer for each thread //
	const int th = omp_get_thread_num();
	const int thread_count = omp_get_num_threads();
	int bond_index = th * (buf_sz_bond/thread_count) + 1;

#else
	const int th = 0;
	const int thread_count = 1;
	int bond_index = 1;
#endif


	//int max_num = 0;
	
	//openmp parallelization for atoms//

	const int end_index = start_index + count;
	
	#pragma omp for
	for(int i = start_index; i < end_index; i++){
		const vec3d r_i = r[i];
		const int ic = icell[i];
		//const int ki = m_knd[i];
		const int start_point = bond_index;


		const int icell_z = ic / cell_wxwy;
		int nz[3] = {0, 0, 0};
		int tz[3] = {-cell_wxwy, 0, cell_wxwy};
		
		if(icell_z == 0){
			nz[0] = 18;	//2 * (3 * 3)
			tz[0] += cell_wxwywz;
		}
		if (icell_z == cell_wz - 1){
			nz[2] = 9;	//1 * (3 * 3)
			tz[2] -= cell_wxwywz;
		}

		const int icell_y = (ic / cell_wx) % cell_wy;
		int ny[3] = {0, 0, 0};
		int ty[3] = {-cell_wx, 0, cell_wx};
		if(icell_y == 0){
			ny[0] = 6;	//2 * 3
			ty[0] += cell_wxwy;
		}
		if (icell_y == cell_wy - 1){
			ny[2] = 3;	//1 * 3
			ty[2] -= cell_wxwy;
		}

		const int icell_x = ic % cell_wx;
		int nx[3] = {0, 0, 0};
		int tx[3] = {-1, 0, 1};
		if(icell_x == 0){
			nx[0] = 2;
			tx[0] = (cell_wx - 1);
		}
		if (icell_x == cell_wx - 1){
			nx[2] = 1;
			tx[2] = (1 - cell_wx);
		}

		// loop for adjoin cell and same cell//
		//int num_t = 0;
		for(int iz = 0; iz < 3; iz++){
			for(int iy = 0; iy < 3; iy++){
				for(int ix= 0; ix < 3; ix++){

					const int nxyz = nx[ix] + ny[iy] + nz[iz];
					const int target_cell = ic + tx[ix] + ty[iy] + tz[iz];
					
					//if(target_cell == ic) continue;
					
					//if(nxyz){//異なるセル//(単にperiodic境界を跨がない判定になってしまっている)
					//if(target_cell != ic){//異なるセル//(unit cellの際にすべてが同一セルになってしまう)
					if(((ix & (iy & iz)) & 0x1) == 0){//(unit cellでも動く)
					
					
						const vec3d rb_i = r_i - *(periodic_vec + nxyz);
						for (int j = first_in_cell[target_cell]; j != -1; j = inext[j]){
							const vec3d dd = rb_i - r[j];

							const double r2 = dd * dd;

							// judge the relative distance //
							if (r2 <= CUTOFF_SQR) {
		
								// add to bonding array //
								TBVECTOR& bv_i = bvector[bond_index];

								bv_i.pairIndex = j;
								bv_i.rij = dd;
								bv_i.dr = sqrt(r2);
		
								// increment of index //
								bond_index ++;
							}
						}
						
					
					}else{//同じセル//
						const vec3d rb_i = r_i;
						for (int j = first_in_cell[target_cell]; j != -1; j = inext[j]){
							if(i == j){
								//丁度ここまでで13/27セルと同じセル中の前半部分が終了//
								bvecStartEnd[i].twobodyendpoint = bond_index;	//end point for two-body force of i-th particle's bond//
							}else{
								const vec3d dd = rb_i - r[j];
								const double r2 = dd * dd;

								// judge the relative distance //
								if (r2 <= CUTOFF_SQR) {
		
									// add to bonding array //
									TBVECTOR& bv_i = bvector[bond_index];

									bv_i.pairIndex = j;
									bv_i.rij = dd;
									bv_i.dr = sqrt(r2);
		
									// increment of index //
									bond_index ++;
								}
							}
						}
					}
		

				}
			}
		}//iz//


		bvecStartEnd[i].startpoint = start_point;	//start point of i-th particle's bond array//
		bvecStartEnd[i].endpoint = bond_index;	//end point of i-th particle's bond//

	}//end of for i//

#ifdef _DEBUG
	bond_index = bond_index;
#endif

}//end of omp parallel//

/*
	//check of bond buffer size//
	{
		const int limit_sz = (th+1) * (buf_sz_bond/thread_count) + 1;
		if(bond_index >= limit_sz){
			return -1;
		}
	}
*/

	return 0;

};



/*
ボンドリストを作る.
バッファが足りないときは-1を返す
*/
template <class TBVECTOR>
int LCL::CreateBondList_in_parallel(int start_index, int count, const vec3d* r, double CUTOFF_SQR, TBVECTOR* bvector, BVEC_REGION* bvecStartEnd, int buf_sz_bond, int* bond_begin, int* bond_end){
    // local variable ------------------------------;


    LCL_INFO info;
    vec3d periodic_vec[27];
    this->GetCellInfo(&info, periodic_vec);

    const int cell_wx = info.cell_x;
    const int cell_wy = info.cell_y;
    const int cell_wz = info.cell_z;
    const int* icell = info.icell;
    const int* first_in_cell = info.first_in_cell;
    const int* inext = info.inext;

    const int cell_wxwy = cell_wx * cell_wy;
    const int cell_wxwywz = cell_wxwy * cell_wz;



#ifdef _OPENMP
    // shift array pointer for each thread //
    const int th = omp_get_thread_num();
    const int thread_count = omp_get_num_threads();
    int bond_index = th * (buf_sz_bond / thread_count) + 1;
#else
    const int th = 0;
    const int thread_count = 1;
    int bond_index = 1;
#endif


    //int max_num = 0;

    //openmp parallelization for atoms//

    const int end_index = start_index + count;

#pragma omp for
    for (int i = start_index; i < end_index; i++){
        const vec3d r_i = r[i];
        const int ic = icell[i];
        //const int ki = m_knd[i];
        const int start_point = bond_index;


        const int icell_z = ic / cell_wxwy;
        int nz[3] = { 0, 0, 0 };
        int tz[3] = { -cell_wxwy, 0, cell_wxwy };

        if (icell_z == 0){
            nz[0] = 18;	//2 * (3 * 3)
            tz[0] += cell_wxwywz;
        }
        if (icell_z == cell_wz - 1){
            nz[2] = 9;	//1 * (3 * 3)
            tz[2] -= cell_wxwywz;
        }

        const int icell_y = (ic / cell_wx) % cell_wy;
        int ny[3] = { 0, 0, 0 };
        int ty[3] = { -cell_wx, 0, cell_wx };
        if (icell_y == 0){
            ny[0] = 6;	//2 * 3
            ty[0] += cell_wxwy;
        }
        if (icell_y == cell_wy - 1){
            ny[2] = 3;	//1 * 3
            ty[2] -= cell_wxwy;
        }

        const int icell_x = ic % cell_wx;
        int nx[3] = { 0, 0, 0 };
        int tx[3] = { -1, 0, 1 };
        if (icell_x == 0){
            nx[0] = 2;
            tx[0] = (cell_wx - 1);
        }
        if (icell_x == cell_wx - 1){
            nx[2] = 1;
            tx[2] = (1 - cell_wx);
        }

        // loop for adjoin cell and same cell//
        //int num_t = 0;
        for (int iz = 0; iz < 3; iz++){
            for (int iy = 0; iy < 3; iy++){
                for (int ix = 0; ix < 3; ix++){

                    const int nxyz = nx[ix] + ny[iy] + nz[iz];
                    const int target_cell = ic + tx[ix] + ty[iy] + tz[iz];

                    //if(target_cell == ic) continue;

                    //if(nxyz){//異なるセル//(単にperiodic境界を跨がない判定になってしまっている)
                    //if(target_cell != ic){//異なるセル//(unit cellの際にすべてが同一セルになってしまう)
                    if (((ix & (iy & iz)) & 0x1) == 0){//(unit cellでも動く)


                        const vec3d rb_i = r_i - *(periodic_vec + nxyz);
                        for (int j = first_in_cell[target_cell]; j != -1; j = inext[j]){
                            const vec3d dd = rb_i - r[j];

                            const double r2 = dd * dd;

                            // judge the relative distance //
                            if (r2 <= CUTOFF_SQR) {

                                // add to bonding array //
                                TBVECTOR& bv_i = bvector[bond_index];

                                bv_i.pairIndex = j;
                                bv_i.rij = dd;
                                bv_i.dr = sqrt(r2);

                                // increment of index //
                                bond_index++;
                            }
                        }


                    } else{//同じセル//
                        const vec3d rb_i = r_i;
                        for (int j = first_in_cell[target_cell]; j != -1; j = inext[j]){
                            if (i == j){
                                //丁度ここまでで13/27セルと同じセル中の前半部分が終了//
                                bvecStartEnd[i].twobodyendpoint = bond_index;	//end point for two-body force of i-th particle's bond//
                            } else{
                                const vec3d dd = rb_i - r[j];
                                const double r2 = dd * dd;

                                // judge the relative distance //
                                if (r2 <= CUTOFF_SQR) {

                                    // add to bonding array //
                                    TBVECTOR& bv_i = bvector[bond_index];

                                    bv_i.pairIndex = j;
                                    bv_i.rij = dd;
                                    bv_i.dr = sqrt(r2);

                                    // increment of index //
                                    bond_index++;
                                }
                            }
                        }
                    }


                }
            }
        }//iz//


        bvecStartEnd[i].startpoint = start_point;	//start point of i-th particle's bond array//
        bvecStartEnd[i].endpoint = bond_index;	//end point of i-th particle's bond//

    }//end of for i//



/*
//check of bond buffer size//
{
const int limit_sz = (th+1) * (buf_sz_bond/thread_count) + 1;
if(bond_index >= limit_sz){
return -1;
}
}
*/

    return 0;

};


/*
ボンドリストを作る.
バッファが足りないときは-1を返す
*/
template <typename TBVECTOR>
int GetBondList(LCL* pLCL, int i, const vec3d* r, double CUTOFF_SQR, TBVECTOR* bvector, int* half_point, const int bvector_size){
	// local variable ------------------------------;


	int num_bonds = 0;
	int twobody_endpoint = 0;

	LCL_INFO info;
	//vec3d periodic_vec[27];
	//pLCL->GetCellInfo(&info, periodic_vec);
	vec3d* periodic_vec;
	pLCL->GetCellInfo(&info, periodic_vec);
	
	const int cell_wx = info.cell_x;
	const int cell_wy = info.cell_y;
	const int cell_wz = info.cell_z;
	const int* icell = info.icell;
	const int* first_in_cell = info.first_in_cell;
	const int* inext = info.inext;
	
	const int cell_wxwy = cell_wx * cell_wy;
	const int cell_wxwywz = cell_wxwy * cell_wz;



	{
		const vec3d r_i = r[i];
		const int ic = icell[i];
		//const int ki = m_knd[i];

	//	worker.Begin(i);
	//	const int start_point = bond_index;


		const int icell_z = ic / cell_wxwy;
		int nz[3] = {0, 0, 0};
		int tz[3] = {-cell_wxwy, 0, cell_wxwy};
		
		if(icell_z == 0){
			nz[0] = 18;	//2 * (3 * 3)
			tz[0] += cell_wxwywz;
		}
		if (icell_z == cell_wz - 1){
			nz[2] = 9;	//1 * (3 * 3)
			tz[2] -= cell_wxwywz;
		}

		const int icell_y = (ic / cell_wx) % cell_wy;
		int ny[3] = {0, 0, 0};
		int ty[3] = {-cell_wx, 0, cell_wx};
		if(icell_y == 0){
			ny[0] = 6;	//2 * 3
			ty[0] += cell_wxwy;
		}
		if (icell_y == cell_wy - 1){
			ny[2] = 3;	//1 * 3
			ty[2] -= cell_wxwy;
		}

		const int icell_x = ic % cell_wx;
		int nx[3] = {0, 0, 0};
		int tx[3] = {-1, 0, 1};
		if(icell_x == 0){
			nx[0] = 2;
			tx[0] = (cell_wx - 1);
		}
		if (icell_x == cell_wx - 1){
			nx[2] = 1;
			tx[2] = (1 - cell_wx);
		}

		// loop for adjoin cell and same cell//
		//int num_t = 0;
		for(int iz = 0; iz < 3; iz++){
			for(int iy = 0; iy < 3; iy++){
				for(int ix= 0; ix < 3; ix++){

					const int nxyz = nx[ix] + ny[iy] + nz[iz];
					const int target_cell = ic + tx[ix] + ty[iy] + tz[iz];
					
					//if(target_cell == ic) continue;
					
					//if(nxyz){//異なるセル//(単にperiodic境界を跨がない判定になってしまっている)
					//if(target_cell != ic){//異なるセル//(unit cellの際にすべてが同一セルになってしまう)
					if(((ix & (iy & iz)) & 0x1) == 0){//(unit cellでも動く)
					
					
						const vec3d rb_i = r_i - *(periodic_vec + nxyz);
                        for (int j = first_in_cell[target_cell]; j != -1; j = inext[j]){
							const vec3d dd = rb_i - r[j];

							const double r2 = dd * dd;

							// judge the relative distance //
							if (r2 <= CUTOFF_SQR) {
								if(bvector_size <= num_bonds){
									return -1;
								}
		
								//worker.Work(i, j, dd, r2);
								
								// add to bonding array //
								TBVECTOR& bv_i = bvector[num_bonds];

								bv_i.pairIndex = j;
								bv_i.rij = dd;
								bv_i.dr = sqrt(r2);
		
								// increment of index //
								num_bonds ++;
								
							}
						}
						
					
					}else{//同じセル//
						const vec3d rb_i = r_i;
                        for (int j = first_in_cell[target_cell]; j != -1; j = inext[j]){

							if(i == j){
								//丁度ここまでで13/27セルと同じセル中の前半部分が終了//
								twobody_endpoint = num_bonds;	//end point for two-body force of i-th particle's bond//
								//worker.Intermidiate();

							}else{

								const vec3d dd = rb_i - r[j];
								const double r2 = dd * dd;

								// judge the relative distance //
								if (r2 <= CUTOFF_SQR) {
									if(bvector_size <= num_bonds){
										return -1;
									}	
									//worker.Work(i, j, dd, r2);
									
									// add to bonding array //
									TBVECTOR& bv_i = bvector[num_bonds];

									bv_i.pairIndex = j;
									bv_i.rij = dd;
									bv_i.dr = sqrt(r2);
		
									// increment of index //
									num_bonds ++;
									
								}
							}
						}
					}
		

				}
			}
		}//iz//

//		worker.End();
//		bvecStartEnd[i].startpoint = start_point;	//start point of i-th particle's bond array//
//		bvecStartEnd[i].endpoint = bond_index;	//end point of i-th particle's bond//

		*half_point = twobody_endpoint;


	}//end of for i//

	




	return num_bonds;


};




/*
ボンドリストを作る.
バッファが足りないときは-1を返す
*/
template <class WORKER>
int LoopBondList(LCL* pLCL, int start_index, int count, const vec3d* r, double CUTOFF_SQR, WORKER worker){
	// local variable ------------------------------;

//#pragma omp parallel
{

	LCL_INFO info;
	vec3d periodic_vec[27];
	pLCL->GetCellInfo(&info, periodic_vec);
	
	const int cell_wx = info.cell_x;
	const int cell_wy = info.cell_y;
	const int cell_wz = info.cell_z;
	const int* icell = info.icell;
	const int* first_in_cell = info.first_in_cell;
	const int* inext = info.inext;
	
	const int cell_wxwy = cell_wx * cell_wy;
	const int cell_wxwywz = cell_wxwy * cell_wz;


	//int max_num = 0;
	
	//openmp parallelization for atoms//

	const int end_index = start_index + count;
	
	#pragma omp for
	for(int i = start_index; i < end_index; i++){
		const vec3d r_i = r[i];
		const int ic = icell[i];
		//const int ki = m_knd[i];

		worker.Begin(i);
	//	const int start_point = bond_index;


		const int icell_z = ic / cell_wxwy;
		int nz[3] = {0, 0, 0};
		int tz[3] = {-cell_wxwy, 0, cell_wxwy};
		
		if(icell_z == 0){
			nz[0] = 18;	//2 * (3 * 3)
			tz[0] += cell_wxwywz;
		}
		if (icell_z == cell_wz - 1){
			nz[2] = 9;	//1 * (3 * 3)
			tz[2] -= cell_wxwywz;
		}

		const int icell_y = (ic / cell_wx) % cell_wy;
		int ny[3] = {0, 0, 0};
		int ty[3] = {-cell_wx, 0, cell_wx};
		if(icell_y == 0){
			ny[0] = 6;	//2 * 3
			ty[0] += cell_wxwy;
		}
		if (icell_y == cell_wy - 1){
			ny[2] = 3;	//1 * 3
			ty[2] -= cell_wxwy;
		}

		const int icell_x = ic % cell_wx;
		int nx[3] = {0, 0, 0};
		int tx[3] = {-1, 0, 1};
		if(icell_x == 0){
			nx[0] = 2;
			tx[0] = (cell_wx - 1);
		}
		if (icell_x == cell_wx - 1){
			nx[2] = 1;
			tx[2] = (1 - cell_wx);
		}

		// loop for adjoin cell and same cell//
		//int num_t = 0;
		for(int iz = 0; iz < 3; iz++){
			for(int iy = 0; iy < 3; iy++){
				for(int ix= 0; ix < 3; ix++){

					const int nxyz = nx[ix] + ny[iy] + nz[iz];
					const int target_cell = ic + tx[ix] + ty[iy] + tz[iz];
					
					//if(target_cell == ic) continue;
					
					//if(nxyz){//異なるセル//(単にperiodic境界を跨がない判定になってしまっている)
					//if(target_cell != ic){//異なるセル//(unit cellの際にすべてが同一セルになってしまう)
					if(((ix & (iy & iz)) & 0x1) == 0){//(unit cellでも動く)
					
					
						const vec3d rb_i = r_i - *(periodic_vec + nxyz);
						for (int j = first_in_cell[target_cell]; j != -1; j = inext[j]){
							const vec3d dd = rb_i - r[j];

							const double r2 = dd * dd;

							// judge the relative distance //
							if (r2 <= CUTOFF_SQR) {
		
								worker.Work(i, j, dd, r2);
								/*
								// add to bonding array //
								TBVECTOR& bv_i = bvector[bond_index];

								bv_i.pairIndex = j;
								bv_i.rij = dd;
								bv_i.dr = sqrt(r2);
		
								// increment of index //
								bond_index ++;
								*/
							}
						}
						
					
					}else{//同じセル//
						const vec3d rb_i = r_i;
						for (int j = first_in_cell[target_cell]; j != -1; j = inext[j]){
							if(i == j){
								//丁度ここまでで13/27セルと同じセル中の前半部分が終了//
								//bvecStartEnd[i].twobodyendpoint = bond_index;	//end point for two-body force of i-th particle's bond//
								worker.Intermidiate();

							}else{

								const vec3d dd = rb_i - r[j];
								const double r2 = dd * dd;

								// judge the relative distance //
								if (r2 <= CUTOFF_SQR) {
									
									worker.Work(i, j, dd, r2);
									/*
									// add to bonding array //
									TBVECTOR& bv_i = bvector[bond_index];

									bv_i.pairIndex = j;
									bv_i.rij = dd;
									bv_i.dr = sqrt(r2);
		
									// increment of index //
									bond_index ++;
									*/
								}
							}
						}
					}
		

				}
			}
		}//iz//

		worker.End(i);
//		bvecStartEnd[i].startpoint = start_point;	//start point of i-th particle's bond array//
//		bvecStartEnd[i].endpoint = bond_index;	//end point of i-th particle's bond//

	}//end of for i//



}//end of omp parallel//



	return 0;

};



/*
ボンドリストを作る.
バッファが足りないときは-1を返す
*/
template <class TBVECTOR>
int LCL::CreateBondList_TEST0(int start_index, int count, const vec3d* r, double CUTOFF_SQR, TBVECTOR* bvector, BVEC_REGION* bvecStartEnd, int buf_sz_bond){
    // local variable ------------------------------;

#pragma omp parallel
{

    LCL_INFO info;
    vec3d periodic_vec[27];
    this->GetCellInfo(&info, periodic_vec);

    const int cell_wx = info.cell_x;
    const int cell_wy = info.cell_y;
    const int cell_wz = info.cell_z;
    const int* icell = info.icell;
    const int* first_in_cell = info.first_in_cell;
    const int* inext = info.inext;

    const int cell_wxwy = cell_wx * cell_wy;
    const int cell_wxwywz = cell_wxwy * cell_wz;



#ifdef _OPENMP
    // shift array pointer for each thread //
    const int th = omp_get_thread_num();
    const int thread_count = omp_get_num_threads();
    int bond_index = th * (buf_sz_bond / thread_count) + 1;
#else
    const int th = 0;
    const int thread_count = 1;
    int bond_index = 1;
#endif


    //int max_num = 0;

    //openmp parallelization for atoms//

    const int end_index = start_index + count;

#pragma omp for
    for (int i = start_index; i < end_index; i++){
        const vec3d r_i = r[i];
        const int ic = icell[i];
        //const int ki = m_knd[i];
        const int start_point = bond_index;


        const int icell_z = ic / cell_wxwy;
        int nz[3] = { 0, 0, 0 };
        int tz[3] = { -cell_wxwy, 0, cell_wxwy };

        if (icell_z == 0){
            nz[0] = 18;	//2 * (3 * 3)
            tz[0] += cell_wxwywz;
        }
        if (icell_z == cell_wz - 1){
            nz[2] = 9;	//1 * (3 * 3)
            tz[2] -= cell_wxwywz;
        }

        const int icell_y = (ic / cell_wx) % cell_wy;
        int ny[3] = { 0, 0, 0 };
        int ty[3] = { -cell_wx, 0, cell_wx };
        if (icell_y == 0){
            ny[0] = 6;	//2 * 3
            ty[0] += cell_wxwy;
        }
        if (icell_y == cell_wy - 1){
            ny[2] = 3;	//1 * 3
            ty[2] -= cell_wxwy;
        }

        const int icell_x = ic % cell_wx;
        int nx[3] = { 0, 0, 0 };
        int tx[3] = { -1, 0, 1 };
        if (icell_x == 0){
            nx[0] = 2;
            tx[0] = (cell_wx - 1);
        }
        if (icell_x == cell_wx - 1){
            nx[2] = 1;
            tx[2] = (1 - cell_wx);
        }

        // loop for adjoin cell and same cell//
        //int num_t = 0;
        for (int iz = 0; iz < 3; iz++){
            for (int iy = 0; iy < 3; iy++){
                for (int ix = 0; ix < 3; ix++){

                    const int nxyz = nx[ix] + ny[iy] + nz[iz];
                    const int target_cell = ic + tx[ix] + ty[iy] + tz[iz];

                    //if(target_cell == ic) continue;

                    //if(nxyz){//異なるセル//(単にperiodic境界を跨がない判定になってしまっている)
                    //if(target_cell != ic){//異なるセル//(unit cellの際にすべてが同一セルになってしまう)
                    if (((ix & (iy & iz)) & 0x1) == 0){//(unit cellでも動く)


                        const vec3d rb_i = r_i - *(periodic_vec + nxyz);
                        for (int j = first_in_cell[target_cell]; j != -1; j = inext[j]){
                            const vec3d dd = rb_i - r[j];

                            const double r2 = dd * dd;

                            // judge the relative distance //
                            if (r2 <= CUTOFF_SQR) {

                                // add to bonding array //
                                //TBVECTOR& bv_i = bvector[bond_index];
                                TBVECTOR bv_i;
                                
                                bv_i.pairIndex = j;
                                bv_i.rij.x = dd.x;
                                bv_i.rij.y = dd.y;
                                bv_i.rij.z = dd.z;
                                bv_i.dr = sqrt(r2);
                                
                                //bv_i.r[0] = dd.x;
                                //bv_i.r[1] = dd.y;
                                //bv_i.r[2] = dd.z;

                                // increment of index //
                                bond_index++;

                            }
                        }


                    } else{//同じセル//
                        
                        const vec3d rb_i = r_i;
                        for (int j = first_in_cell[target_cell]; j != -1; j = inext[j]){
                            if (i == j){
                                //丁度ここまでで13/27セルと同じセル中の前半部分が終了//
                                bvecStartEnd[i].twobodyendpoint = bond_index;	//end point for two-body force of i-th particle's bond//
                            } else{
                                const vec3d dd = rb_i - r[j];
                                const double r2 = dd * dd;

                                // judge the relative distance //
                                if (r2 <= CUTOFF_SQR) {

                                    // add to bonding array //
                                    TBVECTOR& bv_i = bvector[bond_index];

                                    bv_i.pairIndex = j;
                                    bv_i.rij = dd;
                                    bv_i.dr = sqrt(r2);

                                    // increment of index //
                                    bond_index++;
                                }
                            }
                        }
                    }


                }
            }
        }//iz//


        bvecStartEnd[i].startpoint = start_point;	//start point of i-th particle's bond array//
        bvecStartEnd[i].endpoint = bond_index;	//end point of i-th particle's bond//

    }//end of for i//

#ifdef _DEBUG
    bond_index = bond_index;
#endif

}//end of omp parallel//

/*
//check of bond buffer size//
{
const int limit_sz = (th+1) * (buf_sz_bond/thread_count) + 1;
if(bond_index >= limit_sz){
return -1;
}
}
*/

return 0;

};




/*
ボンドリストを作る.
バッファが足りないときは-1を返す
*/
template <class TBVECTOR>
int LCL::CreateBondList_TEST1(int start_index, int count, const vec3d* r, double CUTOFF_SQR, TBVECTOR* bvector, BVEC_REGION* bvecStartEnd, int buf_sz_bond, const int max_num_bond){
    // local variable ------------------------------;

#pragma omp parallel
{

    LCL_INFO info;
    vec3d periodic_vec[27];
    this->GetCellInfo(&info, periodic_vec);

    const int cell_wx = info.cell_x;
    const int cell_wy = info.cell_y;
    const int cell_wz = info.cell_z;
    const int* icell = info.icell;
    const int* first_in_cell = info.first_in_cell;
    const int* inext = info.inext;

    const int cell_wxwy = cell_wx * cell_wy;
    const int cell_wxwywz = cell_wxwy * cell_wz;



#ifdef _OPENMP
    // shift array pointer for each thread //
    const int th = omp_get_thread_num();
    const int thread_count = omp_get_num_threads();
    int bond_index = th * (buf_sz_bond / thread_count) + 1;
#else
    const int th = 0;
    const int thread_count = 1;
    int bond_index = 1;
#endif


    //int max_num = 0;

    //openmp parallelization for atoms//

    const int end_index = start_index + count;
    int previous_cell = -1;

    int num_pair;
    unsigned char* pair_required = new unsigned char[max_num_bond];
    int* pair_index = new int[max_num_bond];
    vec3d* pair_position = new vec3d[max_num_bond];
    double* pair_bvec = new double[max_num_bond * 4];

    size_t total_num_bond = 0;
    size_t total_num_pair = 0;

#pragma omp for
    for (int i = start_index; i < end_index; i++){
        const vec3d r_i = r[i];
        const int ic = icell[i];


        if (previous_cell != ic){

            const int icell_z = ic / cell_wxwy;
            int nz[3] = { 0, 0, 0 };
            int tz[3] = { -cell_wxwy, 0, cell_wxwy };

            if (icell_z == 0){
                nz[0] = 18;	//2 * (3 * 3)
                tz[0] += cell_wxwywz;
            }
            if (icell_z == cell_wz - 1){
                nz[2] = 9;	//1 * (3 * 3)
                tz[2] -= cell_wxwywz;
            }

            const int icell_y = (ic / cell_wx) % cell_wy;
            int ny[3] = { 0, 0, 0 };
            int ty[3] = { -cell_wx, 0, cell_wx };
            if (icell_y == 0){
                ny[0] = 6;	//2 * 3
                ty[0] += cell_wxwy;
            }
            if (icell_y == cell_wy - 1){
                ny[2] = 3;	//1 * 3
                ty[2] -= cell_wxwy;
            }

            const int icell_x = ic % cell_wx;
            int nx[3] = { 0, 0, 0 };
            int tx[3] = { -1, 0, 1 };
            if (icell_x == 0){
                nx[0] = 2;
                tx[0] = (cell_wx - 1);
            }
            if (icell_x == cell_wx - 1){
                nx[2] = 1;
                tx[2] = (1 - cell_wx);
            }

            num_pair = 0;

            // loop for adjoin cell and same cell//
            //int num_t = 0;
            for (int iz = 0; iz < 3; iz++){
                for (int iy = 0; iy < 3; iy++){
                    for (int ix = 0; ix < 3; ix++){

                        const int nxyz = nx[ix] + ny[iy] + nz[iz];
                        const int target_cell = ic + tx[ix] + ty[iy] + tz[iz];


                        for (int j = first_in_cell[target_cell]; j != -1; j = inext[j]){
                            const vec3d r_j = r[j] + *(periodic_vec + nxyz);
                            pair_index[num_pair] = j;
                            pair_position[num_pair] = r_j;
                            num_pair++;
                        }


                    }
                }
            }//iz//
        }//(previous_cell != ic)//


        //const int ki = m_knd[i];
        const int start_point = bond_index;
        int twobodyendpoint = -1;

        for (int pair_id = 0; pair_id < num_pair; pair_id++){
            const int j = pair_index[pair_id];
            const vec3d& r_j = pair_position[pair_id];

            if (i == j){
                //丁度ここまでで13/27セルと同じセル中の前半部分が終了//
                twobodyendpoint = bond_index;	//end point for two-body force of i-th particle's bond//
            } else{

                const vec3d dd = r_i - r_j;
                const double r2 = dd * dd;

                // judge the relative distance //
                if (r2 <= CUTOFF_SQR) {
                    // add to bonding array //
                    TBVECTOR& bv_i = bvector[bond_index];

                    bv_i.pairIndex = pair_index[pair_id];
                    bv_i.rij.x = dd.x;
                    bv_i.rij.y = dd.y;
                    bv_i.rij.z = dd.z;
                    bv_i.dr = sqrt(r2);

                    // increment of index //
                    bond_index++;
                }
            }
        }



        //    total_num_pair += num_pair;
        bvecStartEnd[i].startpoint = start_point;	//start point of i-th particle's bond array//
        bvecStartEnd[i].endpoint = bond_index;	//end point of i-th particle's bond//
        bvecStartEnd[i].twobodyendpoint = twobodyendpoint;	//end point for two-body force of i-th particle's bond//
        //    total_num_bond += bond_index - start_point;
    }//end of for i//

    delete[] pair_index;
    delete[] pair_position;
    delete[] pair_required;
    delete[] pair_bvec;

    //printf("mean_num_bond : %d /  %d\n", total_num_bond / (count / thread_count), total_num_pair / (count / thread_count));
}//end of omp parallel//

/*
//check of bond buffer size//
{
const int limit_sz = (th+1) * (buf_sz_bond/thread_count) + 1;
if(bond_index >= limit_sz){
return -1;
}
}
*/

return 0;

};




/*
ボンドリストを作る.
バッファが足りないときは-1を返す
*/
template <class TBVECTOR>
int LCL::CreateBondList_TEST2(int start_index, int count, const vec3d* r, double CUTOFF_SQR, TBVECTOR* bvector, BVEC_REGION* bvecStartEnd, int buf_sz_bond, const int max_num_bond){
    // local variable ------------------------------;

#pragma omp parallel
{

    LCL_INFO info;
    vec3d periodic_vec[27];
    this->GetCellInfo(&info, periodic_vec);

    const int cell_wx = info.cell_x;
    const int cell_wy = info.cell_y;
    const int cell_wz = info.cell_z;
    const int* icell = info.icell;
    const int* first_in_cell = info.first_in_cell;
    const int* inext = info.inext;

    const int cell_wxwy = cell_wx * cell_wy;
    const int cell_wxwywz = cell_wxwy * cell_wz;



#ifdef _OPENMP
    // shift array pointer for each thread //
    const int th = omp_get_thread_num();
    const int thread_count = omp_get_num_threads();
    int bond_index = th * (buf_sz_bond / thread_count) + 1;
#else
    const int th = 0;
    const int thread_count = 1;
    int bond_index = 1;
#endif


    //int max_num = 0;

    //openmp parallelization for atoms//

    const int end_index = start_index + count;
    int previous_cell = -1;
    
    int num_pair;
    unsigned char* pair_required = new unsigned char[max_num_bond];
    int* pair_index = new int[max_num_bond];
    vec3d* pair_position = new vec3d[max_num_bond];
    double* pair_bvec = new double[max_num_bond*4];
    
    size_t total_num_bond = 0;
    size_t total_num_pair = 0;

#pragma omp for
    for (int i = start_index; i < end_index; i++){
        const vec3d r_i = r[i];
        const int ic = icell[i];


        if (previous_cell != ic){

            const int icell_z = ic / cell_wxwy;
            int nz[3] = { 0, 0, 0 };
            int tz[3] = { -cell_wxwy, 0, cell_wxwy };

            if (icell_z == 0){
                nz[0] = 18;	//2 * (3 * 3)
                tz[0] += cell_wxwywz;
            }
            if (icell_z == cell_wz - 1){
                nz[2] = 9;	//1 * (3 * 3)
                tz[2] -= cell_wxwywz;
            }

            const int icell_y = (ic / cell_wx) % cell_wy;
            int ny[3] = { 0, 0, 0 };
            int ty[3] = { -cell_wx, 0, cell_wx };
            if (icell_y == 0){
                ny[0] = 6;	//2 * 3
                ty[0] += cell_wxwy;
            }
            if (icell_y == cell_wy - 1){
                ny[2] = 3;	//1 * 3
                ty[2] -= cell_wxwy;
            }

            const int icell_x = ic % cell_wx;
            int nx[3] = { 0, 0, 0 };
            int tx[3] = { -1, 0, 1 };
            if (icell_x == 0){
                nx[0] = 2;
                tx[0] = (cell_wx - 1);
            }
            if (icell_x == cell_wx - 1){
                nx[2] = 1;
                tx[2] = (1 - cell_wx);
            }

            num_pair = 0;

            // loop for adjoin cell and same cell//
            //int num_t = 0;
            for (int iz = 0; iz < 3; iz++){
                for (int iy = 0; iy < 3; iy++){
                    for (int ix = 0; ix < 3; ix++){

                        const int nxyz = nx[ix] + ny[iy] + nz[iz];
                        const int target_cell = ic + tx[ix] + ty[iy] + tz[iz];

                        
                        for (int j = first_in_cell[target_cell]; j != -1; j = inext[j]){
                            const vec3d r_j = r[j] + *(periodic_vec + nxyz);
                            pair_index[num_pair] = j;
                            pair_position[num_pair] = r_j;
                            num_pair++;
                        }


                    }
                }
            }//iz//
        }//(previous_cell != ic)//

        
        //const int ki = m_knd[i];
        const int start_point = bond_index;
        int twobodyendpoint = -1;
        
        for (int pair_id = 0; pair_id < num_pair; pair_id++){
            const int j = pair_index[pair_id];
            const vec3d& r_j = pair_position[pair_id];

            if (i == j){
                //丁度ここまでで13/27セルと同じセル中の前半部分が終了//
                twobodyendpoint = bond_index;	//end point for two-body force of i-th particle's bond//
            }

            const vec3d dd = r_i - r_j;
            const double r2 = dd * dd;

            // judge the relative distance //
            if (r2 <= CUTOFF_SQR) {
                pair_required[pair_id] = 1;
            } else{
                pair_required[pair_id] = 0;
            }

            pair_bvec[pair_id * 4] = dd.x;
            pair_bvec[pair_id * 4 + 1] = dd.y;
            pair_bvec[pair_id * 4 + 2] = dd.z;
            pair_bvec[pair_id * 4 + 3] = sqrt(r2);
        }
    
        for (int pair_id = 0; pair_id < num_pair; pair_id++){

            if (pair_required[pair_id]){

                // add to bonding array //
                TBVECTOR& bv_i = bvector[bond_index];

                bv_i.pairIndex = pair_index[pair_id];
                bv_i.rij.x = pair_bvec[pair_id * 4];
                bv_i.rij.y = pair_bvec[pair_id * 4 + 1];
                bv_i.rij.z = pair_bvec[pair_id * 4 + 2];
                bv_i.dr = pair_bvec[pair_id * 4 + 3];

                // increment of index //
                bond_index++;
            }
        }



    //    total_num_pair += num_pair;
        bvecStartEnd[i].startpoint = start_point;	//start point of i-th particle's bond array//
        bvecStartEnd[i].endpoint = bond_index;	//end point of i-th particle's bond//
        bvecStartEnd[i].twobodyendpoint = twobodyendpoint;	//end point for two-body force of i-th particle's bond//
    //    total_num_bond += bond_index - start_point;
    }//end of for i//

    delete[] pair_index;
    delete[] pair_position;
    delete[] pair_required;
    delete[] pair_bvec;

    //printf("mean_num_bond : %d /  %d\n", total_num_bond / (count / thread_count), total_num_pair / (count / thread_count));
}//end of omp parallel//

/*
//check of bond buffer size//
{
const int limit_sz = (th+1) * (buf_sz_bond/thread_count) + 1;
if(bond_index >= limit_sz){
return -1;
}
}
*/

return 0;

};

