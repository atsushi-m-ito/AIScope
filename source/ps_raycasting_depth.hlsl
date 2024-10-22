

//texture
Texture3D tex3d : register(t0);
Texture1D tex_color_map : register(t1);
Texture2D<float> depth2d : register(t2);
SamplerState samp : register(s0);
SamplerState samp_cm : register(s1);
SamplerState samp_depth : register(s2);

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------

cbuffer CBuffer : register(b0)
{
	int num_marching;
	float range_min;
	float range_log_max_per_min;
	float alpha_min;
	float alpha_log_max_per_min;
	//float3x3 inv_abc;
	/*
	float inv_abc_a;
	float inv_abc_b;
	float inv_abc_c;
	*/

	float delta_ray;

	float inv_abc_m11;
	float inv_abc_m21;
	float inv_abc_m31;
	float inv_abc_m12;
	float inv_abc_m22;
	float inv_abc_m32;
	float inv_abc_m13;
	float inv_abc_m23;
	float inv_abc_m33;

	float screen_w;
	float screen_h;
	float comera_x;
	float comera_y;
    float comera_z;
    float P_33;
    float P_43;
};


cbuffer CBuffer1 : register(b1)
{
	/*仕様としてConstantBufferの配列は1要素が16byteになってしまう.
	* すなわち、
	* float cb[8];
	* とすると、8*4byteではなく、8*16byte必要になる。
	* さらにおそらくHLSLコード中からの1要素のアクセスcb[i]では、i*16byte目のデータを読むことになる。
	* よって、すなおに
	* float4 cb[2];
	* とするか、1D textureとして生成する
	* 同じ理由で
	* float 3 also needs 16 byte packing, then float 4 should be used for 3D vector.
	*
	* 最初の6つ(2*6)はボックスの境界面
	* 続く3つ(2*3)はユーザー定義断面
	*/
	//float cross_sections[8 * 9];
	float4 cross_sections[2 * 9];
};


struct PS_INPUT
{
	float4 pos : SV_POSITION;
	//float3 ray : RAY;	
	float3 tex : TEXCOORD;         // テクスチャ座標
	float3 org_pos : POSITION0;
	//float4 proj_pos : POSITION1;
	//float4 ray_z : Z_VIEW_RAY;
	//float4 test : RAY2;	
};

/*
//スペクトル色を返す//
float3 SpectrumColor(float value) {
	const int SIZE = 9;
	float tr[SIZE + 1] = {  38.f / 255.f, 76.f / 255.f,  0,  0,  0,  0, 60.f / 255.f,120.f / 255.f,180.f / 255.f,255.f / 255.f };
	float tg[SIZE + 1] = {   0,  0,  0,140.f / 255.f,180.f / 255.f,250.f / 255.f,180.f / 255.f,120.f / 255.f, 60.f / 255.f,  0 };
	float tb[SIZE + 1] = { 106.f / 255.f,211.f / 255.f,230.f / 255.f,120.f / 255.f, 60.f / 255.f,  0,  0,  0,  0,  0 };


	float3 rgb;
	if (value < 0.0) {
		rgb = float3(0.0,0.0,0.0);
	}

	float pos = value * (float)SIZE;
	//float tt = frac(pos);

	int ipos = (int)floor(pos);
	if (ipos >= SIZE) {
		rgb = float3(tr[SIZE], tg[SIZE], tb[SIZE]);
	}
	else if (ipos < 0) {
		rgb = float3(tr[0], tg[0], tb[0]);
	}
	else {
		float tt = pos - (float)ipos;
		rgb = float3(tr[ipos] + (tt * (tr[ipos + 1] - tr[ipos])),
					tg[ipos] + (tt * (tg[ipos + 1] - tg[ipos])),
					tb[ipos] + (tt * (tb[ipos + 1] - tb[ipos])));
	}
	return rgb;
}
*/

float3 GetColorMap(float value) {
	return tex_color_map.Sample(samp_cm, value).rgb;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------

float Normalize(float value) {
	return min(max(log(value / range_min) / range_log_max_per_min, 0.0), 1.0f);

}
float NormalizeAlpha(float value) {
	return min(max(log(value / alpha_min) / alpha_log_max_per_min, 0.0f), 1.0f);;

}

struct UVW_WithinBox {
	float3 uvw;
	bool within_box;
};

UVW_WithinBox IsInsideAndAdjustBoundary(float3 uvw, float3 ray) {

	bool within_box = true;
	//set the last position to just boundary in order to suppress noise//
	if (uvw.x < 0.0f) {
		uvw -= ray * (uvw.x / ray.x);
		within_box = false;
	} else if (uvw.x > 1.0f) {
		uvw -= ray * ((uvw.x - 1.0f) / ray.x);
		within_box = false;
	}
	if (uvw.y < 0.0f) {
		uvw -= ray * ((uvw.y - 0.0f) / ray.y);
		within_box = false;
	} else if (uvw.y > 1.0f) {
		uvw -= ray * ((uvw.y - 1.0f) / ray.y);
		within_box = false;
	}
	if (uvw.z < 0.0f) {
		uvw -= ray * (uvw.z / ray.z);
		within_box = false;
	} else if (uvw.z > 1.0f) {
		uvw -= ray * ((uvw.z - 1.0f) / ray.z);
		within_box = false;
	}

	UVW_WithinBox res;
	res.uvw = uvw;
	res.within_box = within_box;
	return res;
}

float inner(float3 a, float3 b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}


void and_range(in float my_start, in float my_end, inout float range_start, inout float range_end, inout float range_start2, inout float range_end2) {
	if (my_start >= my_end) {
		range_start = my_start;
		range_end = my_end;
		range_start2 = my_end;
		range_end2 = my_end;
		return;
	}

	if (range_start2 < range_end2) { //2nd range is effective//
		range_start = max(my_start, range_start);
		range_end = min(my_end, range_end);
		range_start2 = max(my_start, range_start2);
		range_end2 = min(my_end, range_end2);

		if (range_end2 < range_start2) {
			range_end2 = range_start2;
		}
		if (range_end < range_start) {
			range_start = range_start2;
			range_end = range_end2;
			range_start2 = range_end2;//set to non-effective//
		}
	} else { //only 1st range is effective//
		range_start = max(my_start, range_start);
		range_end = min(my_end, range_end);
	}

}


void or_range(in float my_start, in float my_end, inout float range_start, inout float range_end, inout float range_start2, inout float range_end2) {
	if (my_start >= my_end) {
		range_start = my_start;
		range_end = my_end;
		range_start2 = my_end;
		range_end2 = my_end;
		return;
	}


	if (range_start2 < range_end2) { //2nd range is effective//

		if ((range_start <= my_end) && (my_start <= range_end)) {
			range_start = min(my_start, range_start);
			range_end = max(my_end, range_end);
		}
		if ((range_start2 <= my_end) && (my_start <= range_end2)) {
			range_start2 = min(my_start, range_start2);
			range_end2 = max(my_end, range_end2);
		}

		if (range_start2 <= range_end) {
			range_end = range_end2;
			range_start2 = range_end2;
		}//shurinc to 1st range//


	} else if (range_start >= range_end) { //no range is effective//
		range_start = my_start;
		range_end = my_end;
	} else { //only 1st range is effective//

		if (range_end < my_start) {//add 2nd range//
			range_start2 = my_start;
			range_end2 = my_end;
		} else if (my_end < range_start) {//1st range is shifted to 2nd range, and add to 1st range//
			range_start2 = range_start;
			range_end2 = range_end;
			range_start = my_start;
			range_end = my_end;
		} else {
			range_start = min(my_start, range_start);
			range_end = max(my_end, range_end);
		}
	}

}


// adopt the following formula
// dA/dx = (1-A(x)) a(x)  ...(1)
// C is max{c(x) : A(x) < 1}  ...(2)
// where c(x) and a(x) are densities of color and alpha at position x, respectively.
// Rendering color and alpha are C and A.
// From eq.(1), 
// d(1-A)/dx = - (1-A(x)) a(x)
// then,
// 1-A(x) = {1-A(0)}exp{ -\int^x_0 a(x') dx'}
// Here, A(0) is 0.
// Consequently,
// A(x) = 1 - exp{ -\int^x_0 a(x') dx'}
float4 main_front_to_back5_depth(PS_INPUT input) : SV_Target
{
	const float3 uvw0 = input.tex;

	
	//bug, accuracy is not enough when ray is recieved from vertex shader//
	//float3 ray = input.ray;
	
	//accuracy is enough when ray is calculated in pixel chader by using camera position in world coordinate//
	float3 ray = (input.org_pos - float3(comera_x, comera_y, comera_z)); 	
	const float init_length_ray = sqrt(mul(ray,ray));
	//ray = normalize(ray);
    const float dt = delta_ray / init_length_ray;


	
	
	//float2 depth_uv = float2(input.proj_pos.x / input.proj_pos.w * 0.5f + 0.5f, -input.proj_pos.y / input.proj_pos.w * 0.5f + 0.5f);
	//float2 depth_uv = float2(input.pos.x * 0.5f + 0.5f, input.pos.y * 0.5f + 0.5f);
    float2 depth_uv = float2(input.pos.x / screen_w, input.pos.y / screen_h);
	float depth = depth2d.Sample(samp_depth, depth_uv);

	//float z_f = z-far limit in veiw coordinate;
	//float z_n = z-near limit in veiw coordinate;
	//float z_d = z_f * z_n / (z_f - (z_f - z_n) * depth);

	//P_33 == z_f / (z_f - z_n)
	//P_43 == -z_n*z_f / (z_f - z_n)
	float z_d = -P_43 / (P_33 - depth);
	//here, input.pos.w == z_v, and z_v is z_coordinate of ray in view coordinate before normalized//
	//init_length_ray is length of 
	float t_depth = (z_d / input.pos.w );


	//Estimate cross point between ray and boundary/cross-sections//
	//t_start and t_end is the range of ray-marching//

	const float3 org_ray = ray; 

	const float3 org_pos = input.org_pos;

	
	


	float t_start = 0.0;
	float t_end = t_depth;// 1.0e10f;
	{//for box boundary
		for (int k = 0; k < 2 * 6; k += 2) {
			const float3 n = normalize(cross_sections[k + 1]).xyz;
			const float r_n = inner(org_ray, n);
            const float t = inner(float3(cross_sections[k].x - comera_x, cross_sections[k].y - comera_y, cross_sections[k].z - comera_z), n) / r_n;
			if (r_n < 0.0) {
				t_start = max(t, t_start);
			} else /*if (r_n > 0.0)*/ {
				if (t < 0.0) {
					t_start = t_end;
				} else {
					t_end = min(t, t_end);
				}
			}
		}
	}

	const float t_box_start = t_start;
	const float t_box_end = t_end;
	t_start = t_box_start;
	t_end = t_box_end;
	float t_start2 = t_box_end;
	float t_end2 = t_box_end;
	int num_user_effective = 0;
	{//for user cross section boundary
		for (int k = 2 * 6; k < 2 * 9; k += 2) {
			const float3 n = normalize(cross_sections[k + 1]).xyz;
			const float r_n = inner(org_ray, n);
            const float t = inner(float3(cross_sections[k].x - comera_x, cross_sections[k].y - comera_y, cross_sections[k].z - comera_z), n) / r_n;

			//if ((and_or_flag[k / 2 - 6] == 0.0f) || (k / 2 - 6 == 0)) {//for and//
			//wにand_orのflagがセットされている. 0ならand//
            if ((cross_sections[k + 1].w == 0.0f) || (num_user_effective == 0)) { //for and//			
                if (r_n < 0.0)
                {
                    and_range(min(t, t_box_end), t_box_end, t_start, t_end, t_start2, t_end2); //for and//
                    ++num_user_effective;
                }
                else if (r_n > 0.0)
                {
                    if (t < t_box_start)
                    {
                        and_range(t_box_end, t_box_end, t_start, t_end, t_start2, t_end2); //for and//
                    }
                    else
                    {
                        and_range(t_box_start, t, t_start, t_end, t_start2, t_end2); //for and//																	
                    }
                    ++num_user_effective;
                }

            }
			else { //for or//
				if (r_n < 0.0) {
					if (t <= t_box_end) {
						or_range(max(t, t_box_start), t_box_end, t_start, t_end, t_start2, t_end2);//for or//
					}
				} else if (r_n > 0.0) {

					if (t < t_box_start) {
						//t_user_end = min(t, t_user_end);//for or//
						//t_user_start = t_start;
						//or_range(t_box_start, t_box_end, t_start, t_end, t_start2, t_end2);//for or//						
					} else {
						or_range(t_box_start, min(t,t_box_end), t_start, t_end, t_start2, t_end2);//for or//						
					}

				}

			}
		}
	}
	
	and_range(t_box_start, t_box_end, t_start, t_end, t_start2, t_end2);  //for and//

	
	ray = float3(inv_abc_m11 * ray.x + inv_abc_m12 * ray.y + inv_abc_m13 * ray.z,
		inv_abc_m21 * ray.x + inv_abc_m22 * ray.y + inv_abc_m23 * ray.z,
		inv_abc_m31 * ray.x + inv_abc_m32 * ray.y + inv_abc_m33 * ray.z);
		
	
	float total_color = 0.0f;
	float total_alpha = 0.0f;
	float dx = 0.25f;  //here, dx=025 is magic number, and both this for coloe and alpha should be same value. //
	float t = t_start;
	
	
	int sector = 0;
	//corresponding to step i = 0//
	if (t < t_end) {
        const float3 uvw = uvw0 + (ray * (t - 1.0f)); //uvw0はt=1地点のuvw値//
		const float c = tex3d.Sample(samp, uvw.xyz).r;
		total_color = c;
		total_alpha = NormalizeAlpha(c);

        t += dt;
		sector = 1;
	}
	float t_sector = t_end;

	for (int i = 1; i <= num_marching; ++i) {
		if (sector > 0)
		{

			if (total_alpha < 1.0f) {
                const float3 uvw = uvw0 + (ray * (t - 1.0f)); //uvw0はt=1地点のuvw値//
				const float c = tex3d.Sample(samp, uvw.xyz).r;

				//estimate color//
				//const float v = Normalize(c);
				const float a = NormalizeAlpha(c);

				//終点付近のノイズが入るのを防ぐ//
				//t + dt > t_sector(t_end)になった時に、dtを小さくしてt_sector=t+dt'ピッタリになるように重みを下げる//
                const float ratio = min(max(0.0f, t_sector - (t - dt)), dt) / dt; //for supress noize//
  
				
				//total_color = max(total_color, c);
				//total_color = total_color * total_alpha + c * (1.0f - total_alpha);
				total_color += (1.0f - total_alpha) * (c - total_color) * dx * ratio;
				total_alpha += (1.0f - total_alpha) * a * dx * ratio;
				//	integral_alpha += a * dx * ratio;

				if (t > t_end2) {
					sector = 0;
				} else if (t > t_end) {
					if (t_end2 == t_start2) { //only 1s range//
						sector = 0;
					} else if (sector == 1) {
						sector = 2;
						t = t_start2;
						t_sector = t_end2;

                        const float3 uvw = uvw0 + (ray * (t - 1.0f)); //uvw0はt=1地点のuvw値//
						const float c = tex3d.Sample(samp, uvw.xyz).r;
						const float a = NormalizeAlpha(c);
						//total_color += (1.0f - total_alpha) * (c - total_color) ;
						//total_alpha += (1.0f - total_alpha) * a ;

					}
				}

			}
            t += dt;

		}
	}

	//integral_alpha = 1.0f - exp(-integral_alpha);
	//total_alpha equals to integral_alpha

	const float v = Normalize(total_color);
	return float4(GetColorMap(v), total_alpha);
	//return float4(GetColorMap(v).r, (t_start2==t_end2)?0.0f:1.0f, GetColorMap(v).b, total_alpha);//total_alpha

	//return float4(total_alpha, total_alpha, total_alpha, 1.0f);
	//return float4(integral_alpha, integral_alpha, integral_alpha, 1.0f);

}


float4 main(PS_INPUT input) : SV_Target
{
	return main_front_to_back5_depth(input);
}

