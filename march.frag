#define MAX_STEPS 200
#define MAX_DIST 200.
#define SURF_DIST .01

uniform mat4 MVP;
uniform vec3 cameraPos;
uniform float fov;
uniform float time;
uniform int mode;

uniform vec2 iResolution;

float Tan(float rad) {
	return tan((3.1415927/180.)*fov);
}

float remap(float val, float low1, float high1, float low2, float high2) {
	return low2 + (val - low1) * (high2 - low2) / (high1-low1);
}

//Mandelbox and mandelbulb examples adapted from following source
//https://github.com/JPBotelho/Raymarched-Fractals/blob/master/Content/DistanceFunc.cginc#L61
float mandelbox(vec3 p) {
	vec4 z = vec4(p, 1.);
	vec4 c = z;

	for(int i = 0; i < 16; i++) {
		z.xyz = clamp(z.xyz, -1., 1.) * 2. - z.xyz;
		
		float zDot = dot(z.xyz, z.xyz);
		if (zDot < 0.5) z *= 2.;
		else if (zDot < 1.) z *= 1./zDot;

		z = 2.* z + c;
	}

	return length(z.xyz)/abs(z.w);
}

float mandelbulb(vec3 p)
{
	vec3 w = p;
    float m = dot(w, w);

	float dz = 1.0;
        
	for(int i = 0; i < 15; i++)
    {
        dz = 8. * pow(sqrt(m), 7.0)*dz + 1.0;
        float r = length(w);
        float b = 8. * acos(w.y / r);
        float a = 8. * atan(w.z, w.x);
        w = p + pow(r, 8.) * vec3(sin(b) * sin(a), cos(b), sin(b) * cos(a));

        m = dot(w, w);
		if(m > 256.0)
            break;
    }
    return 0.25*log(m)*sqrt(m)/dz;
}

float sdDinamMandelbulb(vec3 pos, float power)
{
    vec3 z = pos;
    float r = 0.;
    float dr = 1.;
    for(int i = 0; i < 5; i++) 
    {
        r = length(z);
        if(r > 100.) break;
        
        float theta = acos(z.z / r);
        float phi = atan(z.y, z.x);
        
        dr = power * pow(r, power-1.)*dr+1.;
        
        r = pow(r, power);
        theta *= power;
        phi *= power;
        
        z = r * vec3(sin(theta) * cos(phi), 
                sin(theta) * sin(phi), 
                cos(theta));

        z += pos;
    }
    return 0.5 * log(r) * r / dr;

}

float sphereSDF(vec3 p, float r) {
	p.xyz = mod((p.xyz), 1.0) - vec3(0.5);
	return length(p)-r;
}

float sceneSDF(vec3 p) {
	//[SDF]
	//return de_tetrahedron(vec4(p, 0.0), 1.);
	//if (mode == 0) 
		//return sphereSDF(p, 0.3);
	//else if (mode == 1)
		//return mandelbox(p);
	//else if (mode == 2)
		//return mandelbulb(p);
	//else
		//return sdDinamMandelbulb(p, remap(time, -1., 1., 4., 9.));
}

vec3 normal(vec3 p) {
	float d = sceneSDF(p);
	vec2 e = vec2(.01, 0);

	vec3 n = d - vec3(
		sceneSDF(p-e.xyy),
		sceneSDF(p-e.yxy),
		sceneSDF(p-e.yyx)
	);

	return normalize(n);
}

vec2 rayMarch(vec3 ro, vec3 rd) {
	float total_dist = 0.;
	int steps = 0;

	for (int i = 0; i < MAX_STEPS; i++) {
		steps += 1;
		vec3 pos = ro + rd*total_dist;
		float min_dist = sceneSDF(pos);
		total_dist += min_dist;
		if (total_dist > MAX_DIST || min_dist< SURF_DIST) break;
	}

	return vec2(total_dist, steps);
}

vec3 convertToWorld(vec3 rd) {
	return (MVP * vec4(rd, 0.)).xyz;
}

vec3 phongLightContrib (vec3 k_diffuse, vec3 k_specular, float shininess, vec3 point, vec3 cameraPos, vec3 lightPos, float lightIntensity) {
	vec3 N = normal(point);
	vec3 L = normalize(lightPos-point);
	vec3 R = normalize(reflect(-L, N));
	vec3 V = normalize(cameraPos-point);
	float LN = max(dot(L, N), 0.0);
	float RV = max(dot(R, V), 0.0);

	vec3 col = k_diffuse * LN + k_specular * pow(RV, shininess);
	return col * lightIntensity;
}

vec3 phongColor (vec3 k_ambient, vec3 k_diffuse, vec3 k_specular, float shininess, vec3 point, vec3 cameraPos) {
	vec3 col = k_ambient * 0.5;

	vec3 lightPos1 = vec3(20., 10., -40.);
	float lightIntensity1 = 0.3;
	vec3 contrib1 = phongLightContrib(k_diffuse, k_specular, shininess, point, cameraPos, lightPos1, lightIntensity1);
	col += contrib1;

	vec3 lightPos2 = vec3(50., 5., 20.);
	float lightIntensity2 = 0.5;
	vec3 contrib2 = phongLightContrib(k_diffuse, k_specular, shininess, point, cameraPos, lightPos2, lightIntensity2);
	col += contrib2;

	return col;
}


void main() {
	vec2 uv = (gl_FragCoord.xy - .5*iResolution.xy)/iResolution.y;

	float aspectRatio = iResolution.x/iResolution.y;
	float pixelX = (gl_FragCoord.x + .5)/iResolution.x;
	float pixelY = (gl_FragCoord.y + .5)/iResolution.y;
	float screenX = (2. * pixelX - 1.) * aspectRatio * Tan(fov/2.);
	float screenY = (2. * pixelY - 1.) * Tan(fov/2.);

	uv = vec2(screenX, screenY);


	vec3 dir = normalize(vec3(uv.x, uv.y, -1.));

	vec3 ro = cameraPos;
	vec3 rd = normalize(convertToWorld(dir));

	vec2 ray = rayMarch(ro, rd);
	float d = ray.x;
	float steps = float(ray.y);
	vec3 p = ro + rd * d;
	
	vec3 col = vec3(0);
	if (sceneSDF(p) < SURF_DIST) {
		vec3 k_ambient = (normal(p)+vec3(1))/2.;
		vec3 k_diffuse = k_ambient;
		vec3 k_specular = vec3(1., 1., 1.);
		//vec3 k_ambient = vec3(0.2125, 0.1275, 0.054);
		//vec3 k_diffuse = vec3(0.714, 0.4284, 0.18144);
		//vec3 k_specular = vec3(0.393548, 0.271906, 0.166721);
		float shininess = 25.6;
		col = phongColor(k_ambient, k_diffuse, k_specular, shininess, p, ro);
	} else {
		col = vec3(0.8235, 0.9451, 1.);
		float amount = steps/200.;
		col += vec3(-0.2, 0.5, -0.2)*amount;
	}

	gl_FragColor = vec4(col, 1.0);
}
