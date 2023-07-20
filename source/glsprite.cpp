
#include "glsprite.h"
#include "debugprint.h"

#pragma warning(disable : 4996)

glsprite::glsprite(){
	m_spr_fst = NULL;
	m_txt_fst = NULL;
}

glsprite::~glsprite(){
	
	ClearSprite();

	ReleaseTextureAll();
}

//==============================================================================
//テクスチャのリスト管理
//==============================================================================
TEXTURE_LIST* glsprite::FindTextureList(const TCHAR* filename){
	//ファイルネームを元に、読み込み済みのテクスチャーを取得
	//見つからなかった場合はNULLを返す
	//見つかって読み込み中止するときは、t->referをインクリメントする

	TEXTURE_LIST* t = m_txt_fst;
	while (t){
		if(_tcscmp(t->filename, filename) == 0){
			return t;
		}
		t = t->next;
	}

	//見つからなかった場合はNULLを返す
	return NULL;
}


TEXTURE_LIST* glsprite::AddTextureList(GLuint texture, int w, int h, const TCHAR* filename){
	//リストに追加

	TEXTURE_LIST* t;

	//Listに追加
	if(m_txt_fst==NULL){
		t = m_txt_fst = new TEXTURE_LIST;
	}else{
		t = m_txt_fst;
		while(t->next){
			t = t->next;
		}
		t->next = new TEXTURE_LIST;
		t = t->next;
	}
	t->next = NULL;

	t->texture = texture;
	t->w = w;
	t->h = h;
	if(filename){
		t->filename = _tcsdup(filename);
	}else{
		t->filename = NULL;
	}
	t->refer = 1;
	return t;


}


void glsprite::ReferTexture(GLuint textureId){
	//テクスチャの削除
	//referをデクリメントして、0になったらVRAMから削除

	TEXTURE_LIST* t = m_txt_fst;
	while (t){
		if(t->texture == textureId){
			t->refer ++;
			break;

		}
		t = t->next;
	}

	return;

}

void glsprite::ReleaseTexture(GLuint textureId){
	//テクスチャの削除
	//referをデクリメントして、0になったらVRAMから削除

	
	TEXTURE_LIST* t = m_txt_fst;
	TEXTURE_LIST* old_t = NULL;
	while (t){
		if(t->texture == textureId){
			t->refer --;
			
			if( t->refer <= 0){	//テクスチャ削除				
				glDeleteTextures(1, &(t->texture));

				//リストから削除
				delete [] t->filename;
				if(old_t){
					old_t->next = t->next; 
				}else{//t = m_fst_txtのはず
					m_txt_fst = t->next;
				}
				delete t;
			}
			
			break;

		}
		old_t = t;
		t = t->next;
	}

	return;

}


int glsprite::ReleaseTextureAll(){
	//全てのテクスチャを開放
	//開放した数を返す

	TEXTURE_LIST* t;
	int cnt = 0;

	while (m_txt_fst){
		t = m_txt_fst;
		m_txt_fst = m_txt_fst->next;

		glDeleteTextures(1, &(t->texture));
		delete t;

		cnt++;

	}
	//m_txt_fst = NULL;		//なってるはず

	return cnt;
}

//==============================================================================
//テクスチャの Load 関数========================================================
//==============================================================================
GLuint glsprite::LoadTextureBMP(const TCHAR* fname){
	return tagLoadTextureAlphaBMP24(fname,0,0,0,255);
}


GLuint glsprite::LoadTextureAlphaBMP(const TCHAR* fname, BYTE red, BYTE green, BYTE blue){
	return tagLoadTextureAlphaBMP24(fname,red,green,blue,0);
}

GLuint glsprite::tagLoadTextureAlphaBMP24(const TCHAR* fname, BYTE red, BYTE green, BYTE blue, BYTE alpha){
	FILE *fp;
	BITMAPINFOHEADER bmih;
	BYTE* texturebuf;
	BYTE* newBuf;
	GLuint textureIdx;
	int resW,resH;

	//すでに読み込んでいないかチェック
	TEXTURE_LIST *t = FindTextureList(fname);
	if( t ){
		t->refer ++;
		return t->texture;
	}


	//画像ロード
	fp = _tfopen(fname,_T("rb"));
	if(fp == NULL){
		return (GLuint)-1;
	}

	readDIB(fp, &bmih, &texturebuf);
	fclose(fp);
	
	

	//画像ロード
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1 , &textureIdx);
	glBindTexture(GL_TEXTURE_2D , textureIdx);

	//転送を4バイトごとに,windibと仕様が合うし、32bitでは高速
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	
	if(alpha==255){	//alphaが255なら透過しない
		
		//データフォーマット赤青入れ替えと幅を2の累乗化
		newBuf = convDIBtoGLTEX(&bmih, texturebuf, 3, &resW, &resH);
		if(newBuf){
			delete [] texturebuf;
			texturebuf = newBuf;
		}
		glTexImage2D(GL_TEXTURE_2D, 0, 3, resW, resH,
			0 , GL_RGB , GL_UNSIGNED_BYTE , texturebuf	);
		delete [] texturebuf;
		

	}else{	//alphaが255以下なら指定色(red,green,blue)に指定alphaをセット
		
		newBuf = convDIBtoGLTEX(&bmih, texturebuf, 4, &resW, &resH);
		if(newBuf){
			delete [] texturebuf;
			texturebuf = newBuf;
		}
		setGLTEXAlpha(newBuf, resW*resH*4, red, green, blue, alpha);
	
		glTexImage2D(GL_TEXTURE_2D, 0, 4, resW, resH,
			0 , GL_RGBA , GL_UNSIGNED_BYTE , newBuf	);		
		delete [] newBuf;

	}

	//次のパラメータを各テクスチャごとに設定しないと表示されない場合がある			
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	//マテリアルカラーを無視する
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glDisable(GL_TEXTURE_2D);

	//Listに追加
	AddTextureList(textureIdx, bmih.biWidth, bmih.biHeight, fname);

	return textureIdx;
	
}

/*
GLuint glsprite::LoadTexturePNG(const TCHAR* fname){
	BYTE* texturebuf;
	GLuint textureIdx;
	
	int resW,resH;

	//すでに読み込んでいないかチェック
	TEXTURE_LIST *t = FindTextureList(fname);
	if( t ){
		t->refer ++;
		return t->texture;
	}

	//画像ロード
	texturebuf = LoadPNGtoGLRGBA(fname,&resW, &resH);
	if (texturebuf == NULL) return 0;

	glEnable(GL_TEXTURE_2D);
	glGenTextures(1 , &textureIdx);
	if(textureIdx == 0){ return 0;}
	glBindTexture(GL_TEXTURE_2D , textureIdx);

	//転送を4バイトごとに,windibと仕様が合うし、32bitでは高速
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	
	glTexImage2D(GL_TEXTURE_2D, 0, 4, resW, resH,
		0 , GL_RGBA , GL_UNSIGNED_BYTE , texturebuf);
	
	delete [] texturebuf;


	//次のパラメータを各テクスチャごとに設定しないと表示されない場合がある			
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	//マテリアルカラーを無視する
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glDisable(GL_TEXTURE_2D);

	
	AddTextureList(textureIdx, resW, resH, fname);

	return textureIdx;
	
}
*/

//==============================================================================
//sprite用関数========================================================
//==============================================================================

void glsprite::ClearSprite(){
	SPRITE_LIST *s;
	while (m_spr_fst){
		s = m_spr_fst;
		m_spr_fst = m_spr_fst->next;
		delete s;
	}
	//m_spr_fst = NULL;		//なってるはず
}



void glsprite::SetSprite(int destx, int desty, int destw, int desth, int srcx, int srcy, int srcw, int srch, GLuint textureId){
	SPRITE_LIST *s;

	//テクスチャを探す
	TEXTURE_LIST *t = m_txt_fst;
	while(t){
		if(t->texture == textureId){	//みつかった
			break;
		}
		t = t->next;
	}
	if(t==NULL){ return;}	//みつからない
			

	//Listに追加
	if(m_spr_fst==NULL){
		s = m_spr_fst = new SPRITE_LIST;
	}else{
		s = m_spr_fst;
		while(s->next){
			s = s->next;
		}
		s->next = new SPRITE_LIST;
		s = s->next;
	}
	
	s->destx = destx;
	s->desty = desty;
	s->destw = destw;
	s->desth = desth;
	s->srcx1 = (float)srcx/(float)(t->w);
	s->srcy1 = (float)srcy/(float)(t->h);
	s->srcx2 = (float)(srcx+srcw)/(float)(t->w);
	s->srcy2 = (float)(srcy+srch)/(float)(t->h);
	s->texture = textureId;
	s->next = NULL;

}


void glsprite::DrawSprite(){
	SPRITE_LIST *s;

	glColor3f(1,1,1);
	glEnable(GL_TEXTURE_2D);

	s = m_spr_fst;
	while (s){
		
		glBindTexture(GL_TEXTURE_2D , s->texture);

		glBegin(GL_QUADS);
			glTexCoord2f(s->srcx1, s->srcy1);
			glVertex2i(s->destx, s->desty);
			glTexCoord2f(s->srcx1, s->srcy2);
			glVertex2i(s->destx, s->desty + s->desth);
			glTexCoord2f(s->srcx2, s->srcy2);
			glVertex2i(s->destx + s->destw, s->desty + s->desth);
			glTexCoord2f(s->srcx2, s->srcy1);
			glVertex2i(s->destx + s->destw, s->desty);
		glEnd();
		s = s->next;
	}
	
	glDisable(GL_TEXTURE_2D);
}


void glsprite::DrawSpriteAlpha(){
	
		//glEnable(GL_ALPHA_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		DrawSprite();
		
		glDisable(GL_BLEND);
		//glDisable(GL_ALPHA_TEST);
		
}

int glsprite::GetTextureSize(GLuint textureId, int* pwidth, int* pheight){
	
	//textureを探す
	TEXTURE_LIST *t = m_txt_fst;
	while(t){
		if(t->texture == textureId){	//みつかった
			if(pwidth){ *pwidth = t->w;}
			if(pheight){ *pheight = t->h;}
			return textureId;
		}
		t = t->next;
	}

	return 0;
}
