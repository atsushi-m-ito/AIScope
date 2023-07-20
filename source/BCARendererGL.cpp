
#define _USE_MATH_DEFINES
#include <math.h>
#include <string.h>
#include "BCARendererGL.h"



BCARendererGL::BCARendererGL(){

}

BCARendererGL::~BCARendererGL(){

}


void BCARendererGL::Draw(const Trajectory* first, int frameno, double m_particleRadius, float* m_traj_color, int periodic_flag, double* boxaxisorg){

	for( const Trajectory* t = first; t != NULL; t = t->next){
		RenderOne(t->m_buffer, t->m_bufsz, frameno + 1, m_particleRadius, m_traj_color, periodic_flag, boxaxisorg);
	}
    glFlush();
}



//指定されたフレームまでの軌道を描画
void BCARendererGL::RenderOne(TRAJECTORY_POS *buffer, int count, int frame, double particleRadius, float* traj_color, int periodic_flag, double* boxaxisorg){
	const float start_clr[] = {0.0, 1.0, 0.0,1.0};
	
	const float end_clr[] = {1.0, 0.0, 0.0,1.0};
	
	if(buffer->tm > frame){ return;}	//何も書かない

	//始点の表示

	double x = buffer->x;
	double y = buffer->y;
	double z = buffer->z;

	//周期境界を何度跨いだか//
	int current_periodic_x = (int)floor((x - boxaxisorg[9]) / boxaxisorg[0]);
	int current_periodic_y = (int)floor((y - boxaxisorg[10]) / boxaxisorg[4]);
	int current_periodic_z = (int)floor((z - boxaxisorg[11]) / boxaxisorg[8]);
	if (periodic_flag){
		x -= boxaxisorg[0] * (double)current_periodic_x;
		y -= boxaxisorg[4] * (double)current_periodic_y;
//		z -= boxaxisorg[8] * (double)current_periodic_z;
	}
	
	
	glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,start_clr);
	AI_setSphere(particleRadius, 16, 8, x, y, z);
	//glPopMatrix();


		//表示対象が一点だけなので軌道を書かない
	if(count <= 1){ return;}
	


	
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, traj_color);

	glBegin(GL_LINE_STRIP); 
		
	
	TRAJECTORY_POS* buf;
	for (buf = buffer; buf != buffer + count; buf++){
		if(buf->tm > frame){break;}


		if (periodic_flag){
			//デフォルトで折りたたんであるデータと折りたたんでないデータの両方に対応した処理//

			double prev_x = x;
			double prev_y = y;
			double prev_z = z;


			x = buf->x;
			y = buf->y;
			z = buf->z;


			int periodic_x = (int)floor((x - boxaxisorg[9]) / boxaxisorg[0]);
			int periodic_y = (int)floor((y - boxaxisorg[10]) / boxaxisorg[4]);
			int periodic_z = (int)floor((z - boxaxisorg[11]) / boxaxisorg[8]);
			
			x -= boxaxisorg[0] * (double)periodic_x;
			y -= boxaxisorg[4] * (double)periodic_y;
		//	z -= boxaxisorg[8] * (double)periodic_z;
			
			if ((fabs(x - prev_x) * 2.0 > boxaxisorg[0]) || (fabs(y - prev_y) * 2.0 > boxaxisorg[4]) ){

				//if ((current_periodic_x != periodic_x) || (current_periodic_y != periodic_y)){// && (current_periodic_z == periodic_z)){
				glEnd();

				glBegin(GL_LINE_STRIP);
			}

			current_periodic_x = periodic_x;
			current_periodic_y = periodic_y;
			current_periodic_z = periodic_z;

		}
		else{

			x = buf->x;
			y = buf->y;
			z = buf->z;

		}
		glVertex3f(x, y, z);

	}

	glEnd();


	//終点の表示
	glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,end_clr);
	AI_setSphere(particleRadius, 16, 8, x, y, z);
	//glPopMatrix();

}


void BCARendererGL::AI_setSphere(GLdouble radius, GLint slices, GLint stacks, float x, float y, float z){
	
	int i, k;
	double daXY = 2.0*M_PI/double(slices);
	double daZX = M_PI/double(stacks);
	double angleZX;
	double angleXY;
	double *cosZX, *sinZX;
	double cosXY, sinXY;
	double ex, ey;
	double *bfrex, *bfrey;
	

	
	cosZX = new double[stacks - 1];
	sinZX = new double[stacks - 1];
	bfrex = new double[stacks - 1];
	bfrey = new double[stacks - 1];

	angleZX = daZX;
	for (i = 0; i < stacks - 1; i++){
		cosZX[i] = cos(angleZX);
		bfrex[i] = sinZX[i] = sin(angleZX);
		bfrey[i] = 0.0;
		angleZX += daZX;
		
	}


	angleXY = daXY;

	for (k = 0; k < slices; k++) {
		cosXY = cos(angleXY);
		sinXY = sin(angleXY);


		glBegin(GL_TRIANGLE_STRIP); 
			
		glNormal3f(0.0f,0.0f,1.0f);
		glVertex3f(x, y, (float)radius + z);
		
		
		for (i = 0; i < stacks - 1; i++) {
			ex = sinZX[i] * cosXY;
			ey = sinZX[i] * sinXY;

			glNormal3f((float)bfrex[i], (float)bfrey[i], (float)cosZX[i]);
			glVertex3f((float)(radius * bfrex[i]) + x, (float)(radius * bfrey[i]) + y, (float)(radius * cosZX[i]) + z);
			
			glNormal3f((float)ex, (float)ey , (float)cosZX[i]);
			glVertex3f((float)(radius * ex) + x, (float)(radius * ey) + y, (float)(radius * cosZX[i]) + z);
			bfrex[i] = ex;
			bfrey[i] = ey;
		}

		glNormal3f(0.0f,0.0f,-1.0f);
		glVertex3f(x, y, -(float)radius + z);
		
		glEnd();
		
		angleXY += daXY;

	}
	
	delete [] cosZX;
	delete [] sinZX;
	delete [] bfrex;
	delete [] bfrey;

}