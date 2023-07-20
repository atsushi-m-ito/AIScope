

//texture
Texture3D tex3d : register(t0);
Texture1D tex_color_map : register(t1);
SamplerState samp : register(s0);
SamplerState samp_cm : register(s1);

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

	float dummy1;

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
	*/ 
	//float cross_sections[8 * 9];
	float4 cross_sections[2 * 9];  
};


struct PS_INPUT
{
	float4 pos : SV_POSITION;
//	float3 normal : NORMAL;
	float3 ray : RAY;
	float3 tex : TEXCOORD;         // テクスチャ座標
	float3 org_pos : POSITION;
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
	return min(max(log(value / alpha_min)/ alpha_log_max_per_min, 0.0f), 1.0f);;

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
	}
	else if (uvw.x > 1.0f) {
		uvw -= ray * ((uvw.x - 1.0f) / ray.x);
		within_box = false;
	}
	if (uvw.y < 0.0f) {
		uvw -= ray * ((uvw.y-0.0f) / ray.y);
		within_box = false;
	}
	else if (uvw.y > 1.0f) {
		uvw -= ray * ((uvw.y - 1.0f) / ray.y);
		within_box = false;
	}
	if (uvw.z < 0.0f) {
		uvw -= ray * (uvw.z / ray.z);
		within_box = false;
	}
	else if (uvw.z > 1.0f) {
		uvw -= ray * ((uvw.z - 1.0f) / ray.z);
		within_box = false;
	}
	
	UVW_WithinBox res;
	res.uvw = uvw;
	res.within_box = within_box;
	return res;
}


// adopt maximum 
float4 main_max(PS_INPUT input) : SV_Target
{	
	float3 uvw = input.tex;

	float3 ray = input.ray;
	ray = normalize(ray);
	//ray = mul(ray,inv_abc);
	ray = delta_ray * float3(inv_abc_m11 * ray.x + inv_abc_m12 * ray.y + inv_abc_m13 * ray.z,
		inv_abc_m21 * ray.x + inv_abc_m22 * ray.y + inv_abc_m23 * ray.z,
		inv_abc_m31 * ray.x + inv_abc_m32 * ray.y + inv_abc_m33 * ray.z);
	
	
	//ray = float3(inv_abc_a * ray.x, inv_abc_b * ray.y, inv_abc_c * ray.z);
	//ray = float3(ray.x*0.2f, ray.y * 0.2f, ray.z * 0.2f);
	
		
	float color = 0.0f;// tex3d.Sample(samp, input.tex.xyz).r;
	bool within_box = true;
	for (int i = 0; i <= num_marching; ++i) {
		if (within_box) {
			
			//boxから出た時は境界ピッタリに引き戻す//
			//副産物として初手が自動で境界になる(ノイズ除去効果大)//
			UVW_WithinBox res = IsInsideAndAdjustBoundary(uvw, ray);
			uvw = res.uvw;
			const float c = tex3d.Sample(samp, uvw.xyz).r;
			uvw += ray;
			within_box = (i==0) ? true : res.within_box; //初手は必ずbox内部にする//

			//color is maximum//
			color = max(color, c);

		}
	}
	
	float v = Normalize(color);
	float a = NormalizeAlpha(color);
	return float4(GetColorMap(v),a);


}




// adopt the following formula
// dA/dx = (1-A(x)) a(x)  ...(1)
// dC/dx = (1-A(x)) c(x)  ...(2)
// where c(x) and a(x) are densities of color and alpha at position x, respectively.
// Rendering color and alpha are C and A.
// From eq.(1), 
// d(1-A)/dx = - (1-A(x)) a(x)
// then,
// 1-A(x) = {1-A(0)}exp{ -\int^x_0 a(x') dx'}
// Here, A(0) is 0.
// Consequently,
// A(x) = 1 - exp{ -\int^x_0 a(x') dx'}
float4 main_front_to_back(PS_INPUT input) : SV_Target
{
	float3 uvw = input.tex;

	float3 ray = input.ray;
	ray =  normalize(ray);
	//ray = mul(ray,inv_abc);
	ray = delta_ray * float3(inv_abc_m11 * ray.x + inv_abc_m12 * ray.y + inv_abc_m13 * ray.z,
		inv_abc_m21 * ray.x + inv_abc_m22 * ray.y + inv_abc_m23 * ray.z,
		inv_abc_m31 * ray.x + inv_abc_m32 * ray.y + inv_abc_m33 * ray.z);

	//ray = float3(inv_abc_a * ray.x, inv_abc_b * ray.y, inv_abc_c * ray.z);
	//ray = float3(ray.x*0.2f, ray.y * 0.2f, ray.z * 0.2f);
	float total_color = 0.0f;// tex3d.Sample(samp, input.tex.xyz).r;
	float total_alpha = 0.0f;
	float dx = 0.5;
	bool within_box = true;
	for (int i = 0; i <= num_marching; ++i) {
		if (within_box) {

			//boxから出た時は境界ピッタリに引き戻す//
			//副産物として初手が自動で境界になる(ノイズ除去効果大)//
			UVW_WithinBox res = IsInsideAndAdjustBoundary(uvw, ray);
			uvw = res.uvw;
			const float c = tex3d.Sample(samp, uvw.xyz).r;
			uvw += ray;
			within_box = (i == 0) ? true : res.within_box; //初手は必ずbox内部にする//

			//estimate color//
			const float v = Normalize(c);
			const float a = NormalizeAlpha(c);

			total_color += (1.0f - total_alpha) * v * dx;
			total_alpha += (1.0f - total_alpha) * a * dx;
			total_color = min(total_color, 1.0f);
			total_alpha = min(total_alpha, 1.0f);

		}
		//if (next_break) break;
	//	++i;
	}

	return float4(GetColorMap(total_color), total_alpha);


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
float4 main_front_to_back2(PS_INPUT input) : SV_Target
{
	float3 uvw = input.tex;

	float3 ray = input.ray;
	ray = normalize(ray);
	//ray = mul(ray,inv_abc);
	ray = float3(inv_abc_m11 * ray.x + inv_abc_m12 * ray.y + inv_abc_m13 * ray.z,
		inv_abc_m21 * ray.x + inv_abc_m22 * ray.y + inv_abc_m23 * ray.z,
		inv_abc_m31 * ray.x + inv_abc_m32 * ray.y + inv_abc_m33 * ray.z);
	ray *= delta_ray;

	//ray = float3(inv_abc_a * ray.x, inv_abc_b * ray.y, inv_abc_c * ray.z);
	//ray = float3(ray.x*0.2f, ray.y * 0.2f, ray.z * 0.2f);
	float total_color = 0.0f;// tex3d.Sample(samp, input.tex.xyz).r;
	float total_alpha = 0.0f;
	float dx = 0.25f;  //here, dx=025 is magic number //
	bool within_box = true;
	for (int i = 0; i <= num_marching; ++i) {
		if (within_box) {

			//boxから出た時は境界ピッタリに引き戻す//
			//副産物として初手が自動で境界になる(ノイズ除去効果大)//
			UVW_WithinBox res = IsInsideAndAdjustBoundary(uvw, ray);
			uvw = res.uvw;

			if (total_alpha < 1.0f) {

				const float c = tex3d.Sample(samp, uvw.xyz).r;

				//estimate color//
				//const float v = Normalize(c);
				const float a = NormalizeAlpha(c);

				//total_color = max(total_color, c);
				total_color = total_color * total_alpha + c * (1.0f- total_alpha);
				total_alpha += (1.0f - total_alpha) * a * dx;
				//total_color = min(total_color, 1.0f);
				//total_alpha = min(total_alpha, 1.0f);
			}

			uvw += ray;
			within_box = (i == 0) ? true : res.within_box; //初手は必ずbox内部にする//

		}
		//if (next_break) break;
	//	++i;
	}

	const float v = Normalize(total_color);
	return float4(GetColorMap(v), total_alpha);

}

float inner(float3 a, float3 b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
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
float4 main_front_to_back3(PS_INPUT input) : SV_Target
{
	float3 uvw = input.tex;

	float3 ray = input.ray;
	ray = normalize(ray);

	//Estimate cross point between ray and boundary/cross-sections//
	//t_start and t_end is the range of ray-marching//
	const float3 org_ray = ray;
	const float3 org_pos = input.org_pos;
	float t_start = 0.0f;
	float t_end = 1.0e10f;
	{
		for (int k = 0; k < 2 * 9; k += 2) {
			const float3 n = normalize(cross_sections[k + 1]).xyz;
			const float r_n = inner(org_ray, n);
			const float t = inner(float3(cross_sections[k].x - org_pos.x, cross_sections[k].y - org_pos.y, cross_sections[k].z - org_pos.z), n) / r_n;
			if (r_n < 0.0) {
				t_start = max(t, t_start);
			}
			else if (r_n > 0.0) {
				if (t < 0.0) {
					t_start = t_end+1.0f;
				}else{
					t_end = min(t, t_end);
				}
			}
		}
	}
	ray = float3(inv_abc_m11 * ray.x + inv_abc_m12 * ray.y + inv_abc_m13 * ray.z,
		inv_abc_m21 * ray.x + inv_abc_m22 * ray.y + inv_abc_m23 * ray.z,
		inv_abc_m31 * ray.x + inv_abc_m32 * ray.y + inv_abc_m33 * ray.z);

	uvw += ray * t_start;  //手前のbox境界 or cross-sectionを始点にする//
	ray *= delta_ray;
	
	//ray = float3(inv_abc_a * ray.x, inv_abc_b * ray.y, inv_abc_c * ray.z);
	//ray = float3(ray.x*0.2f, ray.y * 0.2f, ray.z * 0.2f);
	float total_color = 0.0f;// tex3d.Sample(samp, input.tex.xyz).r;
	float total_alpha = 0.0f;
	float dx = 0.25f;  //here, dx=025 is magic number, and both this for coloe and alpha should be same value. //
	//float integral_alpha = 0.0;  //here, int a(x) dx //
	float t = t_start;

	//corresponding to step i = 0//
	if (t <= t_end) {
		const float c = tex3d.Sample(samp, uvw.xyz).r;
		total_color = c;
		total_alpha = NormalizeAlpha(c);

		uvw += ray;
		t += delta_ray;
	}

	for (int i = 1; i <= num_marching; ++i) {
		if (t <= t_end) 
		{

			if (total_alpha < 1.0f) {

				const float c = tex3d.Sample(samp, uvw.xyz).r;

				//estimate color//
				//const float v = Normalize(c);
				const float a = NormalizeAlpha(c);


				const float ratio = min(t_end - t, delta_ray) / delta_ray;//for supress noize//

				//total_color = max(total_color, c);
				//total_color = total_color * total_alpha + c * (1.0f - total_alpha);
				total_color += (1.0f - total_alpha) * (c - total_color)* dx * ratio;
				total_alpha += (1.0f - total_alpha) * a * dx* ratio;
			//	integral_alpha += a * dx * ratio;

			}

			uvw += ray;

			t += delta_ray;
		}
	}

	//integral_alpha = 1.0f - exp(-integral_alpha);
	//total_alpha equals to integral_alpha

	const float v = Normalize(total_color);
	return float4(GetColorMap(v), total_alpha);

	
	//return float4(total_alpha, total_alpha, total_alpha, 1.0f);
	//return float4(integral_alpha, integral_alpha, integral_alpha, 1.0f);

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
float4 main_front_to_back4(PS_INPUT input) : SV_Target
{
	float3 uvw = input.tex;

	float3 ray = input.ray;
	ray = normalize(ray);

	//Estimate cross point between ray and boundary/cross-sections//
	//t_start and t_end is the range of ray-marching//
	const float3 org_ray = ray;
	const float3 org_pos = input.org_pos;


	float t_start = 0.0;
	float t_end = 1.0e10f;
	{//for box boundary
		for (int k = 0; k < 2 * 6; k += 2) {
			const float3 n = normalize(cross_sections[k + 1]).xyz;
			const float r_n = inner(org_ray, n);
			const float t = inner(float3(cross_sections[k].x - org_pos.x, cross_sections[k].y - org_pos.y, cross_sections[k].z - org_pos.z), n) / r_n;
			if (r_n < 0.0) {
				t_start = max(t, t_start);
			}
			else if (r_n > 0.0) {
				if (t < 0.0) {
					t_start = t_end;
				}
				else {
					t_end = min(t, t_end);
				}
			}
		}
	}


	const float t_user_start_blank = 0.0f;
	const float t_user_end_limit = 1.0e10f;
	float t_user_start = t_start;
	float t_user_end = t_end;
	float and_or_flag[3] = { 0.0f, 1.0f, 1.0f };
	float red = 0.0f;
	float blue = 0.0f;
	{//for user cross section boundary
		for (int k = 2 * 6; k < 2 * 9; k += 2) {
			const float3 n = normalize(cross_sections[k + 1]).xyz;
			const float r_n = inner(org_ray, n);
			const float t = inner(float3(cross_sections[k].x - org_pos.x, cross_sections[k].y - org_pos.y, cross_sections[k].z - org_pos.z), n) / r_n;
			if ((and_or_flag[k / 2 - 6] == 0.0f) || (k/2-6==0)){//for and//
				
				if (r_n < 0.0) {
					if (t < t_end) {
						t_user_start = max(t, t_user_start);   //for and//
					}
					else {

					}
				}
				else if (r_n > 0.0) {
					if (t < t_start) {
						t_user_start = t_end;
						t_user_end = t_start;// max(ut, t_user_start);   //for and//
					}
					else {
						t_user_end = min(t, t_user_end);   //for and//
					}
				}
				
			}
			else {//for or//
				
				if (r_n < 0.0) {
					if (t <= t_end) {
						t_user_start = min(max(t, t_start), t_user_start); //for or//
						t_user_end = t_end;
					}
				}
				else if (r_n > 0.0) {
					if (t < t_start) {
						//t_user_end = min(t, t_user_end);//for or//
						//t_user_start = t_start;
					}
					else {
						t_user_end = min(max(t, t_user_end), t_end);// min(max(t, t_user_end), t_end); //for or//
						t_user_start = t_start;
					}
				}
				
			}
		}
	}
	red = (t_user_start - t_start) / (t_end - t_start);
	blue = (t_user_end - t_start) / (t_end - t_start);
	t_start = t_user_start;
	t_end = t_user_end;


	ray = float3(inv_abc_m11 * ray.x + inv_abc_m12 * ray.y + inv_abc_m13 * ray.z,
		inv_abc_m21 * ray.x + inv_abc_m22 * ray.y + inv_abc_m23 * ray.z,
		inv_abc_m31 * ray.x + inv_abc_m32 * ray.y + inv_abc_m33 * ray.z);

	uvw += ray * t_start;  //手前のbox境界 or cross-sectionを始点にする//
	ray *= delta_ray;

	//ray = float3(inv_abc_a * ray.x, inv_abc_b * ray.y, inv_abc_c * ray.z);
	//ray = float3(ray.x*0.2f, ray.y * 0.2f, ray.z * 0.2f);
	float total_color = 0.0f;// tex3d.Sample(samp, input.tex.xyz).r;
	float total_alpha = 0.0f;
	float dx = 0.25f;  //here, dx=025 is magic number, and both this for coloe and alpha should be same value. //
	//float integral_alpha = 0.0;  //here, int a(x) dx //
	float t = t_start;

	//corresponding to step i = 0//
	if (t < t_end) {
		const float c = tex3d.Sample(samp, uvw.xyz).r;
		total_color = c;
		total_alpha = NormalizeAlpha(c);

		uvw += ray;
		t += delta_ray;
	}

	for (int i = 1; i <= num_marching; ++i) {
		if (t <= t_end)
		{

			if (total_alpha < 1.0f) {

				const float c = tex3d.Sample(samp, uvw.xyz).r;

				//estimate color//
				//const float v = Normalize(c);
				const float a = NormalizeAlpha(c);


				const float ratio = min(t_end - t, delta_ray) / delta_ray;//for supress noize//

				//total_color = max(total_color, c);
				//total_color = total_color * total_alpha + c * (1.0f - total_alpha);
				total_color += (1.0f - total_alpha) * (c - total_color) * dx * ratio;
				total_alpha += (1.0f - total_alpha) * a * dx * ratio;
				//	integral_alpha += a * dx * ratio;

			}

			uvw += ray;

			t += delta_ray;
		}
	}

	//integral_alpha = 1.0f - exp(-integral_alpha);
	//total_alpha equals to integral_alpha

	const float v = Normalize(total_color);
	//return float4(GetColorMap(v), total_alpha);
	return float4(red, GetColorMap(v).g,blue, total_alpha);

	//return float4(total_alpha, total_alpha, total_alpha, 1.0f);
	//return float4(integral_alpha, integral_alpha, integral_alpha, 1.0f);

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
	}
	else { //only 1st range is effective//
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
		
		
	}
	else if (range_start >= range_end) { //no range is effective//
		range_start = my_start;
		range_end = my_end;
	}
	else { //only 1st range is effective//
		
		if (range_end < my_start) {//add 2nd range//
			range_start2 = my_start;
			range_end2 = my_end;
		}
		else if (my_end < range_start) {//1st range is shifted to 2nd range, and add to 1st range//
			range_start2 = range_start;
			range_end2 = range_end;
			range_start = my_start;
			range_end = my_end;
		}
		else {
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
float4 main_front_to_back5(PS_INPUT input) : SV_Target
{
	const float3 uvw0 = input.tex;

	float3 ray = input.ray;
	ray = normalize(ray);

	//Estimate cross point between ray and boundary/cross-sections//
	//t_start and t_end is the range of ray-marching//
	const float3 org_ray = ray;
	const float3 org_pos = input.org_pos;


	float t_start = 0.0;
	float t_end = 1.0e10f;
	{//for box boundary
		for (int k = 0; k < 2 * 6; k += 2) {
			const float3 n = normalize(cross_sections[k + 1]).xyz;
			const float r_n = inner(org_ray, n);
			const float t = inner(float3(cross_sections[k].x - org_pos.x, cross_sections[k].y - org_pos.y, cross_sections[k].z - org_pos.z), n) / r_n;
			if (r_n < 0.0) {
				t_start = max(t, t_start);
			}
			else if (r_n > 0.0) {
				if (t < 0.0) {
					t_start = t_end;
				}
				else {
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
	//float and_or_flag[3] = { 0.0f,0.0f, 1.0f };
	float red = 0.0f;
	float blue = 0.0f;
	int num_user_effective = 0;
	{//for user cross section boundary
		for (int k = 2 * 6; k < 2 * 9; k += 2) {
			const float3 n = normalize(cross_sections[k + 1]).xyz;
			const float r_n = inner(org_ray, n);
			const float t = inner(float3(cross_sections[k].x - org_pos.x, cross_sections[k].y - org_pos.y, cross_sections[k].z - org_pos.z), n) / r_n;

			//if ((and_or_flag[k / 2 - 6] == 0.0f) || (k / 2 - 6 == 0)) {//for and//
			if ((cross_sections[k + 1].w == 0.0f) || (num_user_effective == 0)) {//for and//			
				if (r_n < 0.0) {
					and_range( min(t,t_box_end), t_box_end, t_start, t_end, t_start2, t_end2);  //for and//
					++num_user_effective;
				}
				else if (r_n > 0.0) {
					if (t < t_box_start) {
						and_range(t_box_end, t_box_end, t_start, t_end, t_start2, t_end2);  //for and//
					}
					else {
						and_range(t_box_start, t, t_start, t_end, t_start2, t_end2);  //for and//																	
					}
					++num_user_effective;
				}

			}
			else {//for or//
				if (r_n < 0.0) {
					if (t <= t_box_end) {
						or_range(max(t, t_box_start), t_box_end, t_start, t_end, t_start2, t_end2);//for or//
					}
				}
				else if (r_n > 0.0) {

					if (t < t_box_start) {
						//t_user_end = min(t, t_user_end);//for or//
						//t_user_start = t_start;
						//or_range(t_box_start, t_box_end, t_start, t_end, t_start2, t_end2);//for or//						
					}
					else {
						or_range(t_box_start, min(t,t_box_end), t_start, t_end, t_start2, t_end2);//for or//						
					}
					
				}
				
			}
		}
	}
	red = (t_start - t_box_start) / (t_box_end - t_box_start);
	blue = (t_box_end - t_start2) / (t_box_end - t_box_start);
	and_range( t_box_start, t_box_end, t_start, t_end, t_start2, t_end2);  //for and//


	ray = float3(inv_abc_m11 * ray.x + inv_abc_m12 * ray.y + inv_abc_m13 * ray.z,
		inv_abc_m21 * ray.x + inv_abc_m22 * ray.y + inv_abc_m23 * ray.z,
		inv_abc_m31 * ray.x + inv_abc_m32 * ray.y + inv_abc_m33 * ray.z);


	//ray = float3(inv_abc_a * ray.x, inv_abc_b * ray.y, inv_abc_c * ray.z);
	//ray = float3(ray.x*0.2f, ray.y * 0.2f, ray.z * 0.2f);
	float total_color = 0.0f;// tex3d.Sample(samp, input.tex.xyz).r;
	float total_alpha = 0.0f;
	float dx = 0.25f;  //here, dx=025 is magic number, and both this for coloe and alpha should be same value. //
	//float integral_alpha = 0.0;  //here, int a(x) dx //
	float t = t_start;
	//uvw += ray * ((t - t_start) / delta_ray);

	int sector = 0;
	//corresponding to step i = 0//
	if (t < t_end) {
		const float3 uvw = uvw0 + (ray * t);  //手前のbox境界 or cross-sectionを始点にする//
		const float c = tex3d.Sample(samp, uvw.xyz).r;
		total_color = c;
		total_alpha = NormalizeAlpha(c);

		t += delta_ray;
		sector = 1;
	}
	float t_sector = t_end;
	
	for (int i = 1; i <= num_marching; ++i) {
		if (sector > 0)
		{

			if (total_alpha < 1.0f) {
				const float3 uvw = uvw0 + (ray * t);  //手前のbox境界 or cross-sectionを始点にする//
				const float c = tex3d.Sample(samp, uvw.xyz).r;

				//estimate color//
				//const float v = Normalize(c);
				const float a = NormalizeAlpha(c);


				const float ratio = min(max(0.0f, t_sector - (t - delta_ray)), delta_ray) / delta_ray;//for supress noize//

				//total_color = max(total_color, c);
				//total_color = total_color * total_alpha + c * (1.0f - total_alpha);
				total_color += (1.0f - total_alpha) * (c - total_color) * dx * ratio;
				total_alpha += (1.0f - total_alpha) * a * dx * ratio;
				//	integral_alpha += a * dx * ratio;

				if (t > t_end2) {
					sector = 0;
				}
				else if (t > t_end) {
					if (t_end2 == t_start2) { //only 1s range//
						sector = 0;
					}else if(sector==1){
						sector = 2;
						t = t_start2;
						t_sector = t_end2;

						const float3 uvw = uvw0 + (ray * t);  //手前のbox境界 or cross-sectionを始点にする//
						const float c = tex3d.Sample(samp, uvw.xyz).r;
						const float a = NormalizeAlpha(c);
						//total_color += (1.0f - total_alpha) * (c - total_color) ;
						//total_alpha += (1.0f - total_alpha) * a ;
						
					}
				}

			}
			t += delta_ray;
			
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
	//return main_max(input);
	//return main_front_to_back(input);
	//return main_front_to_back2(input);
	//return main_front_to_back3(input);
	return main_front_to_back5(input);
}

