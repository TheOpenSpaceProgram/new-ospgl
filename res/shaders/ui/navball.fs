#version 330
out vec4 FragColor;


in vec3 vColor;
in vec2 vTex;
in vec3 vNormal;
in vec3 vPos;
in vec3 vPosOriginal;

uniform sampler2D tex;


uniform float iconSize;

// Spherical coordinates of prograde.
// Fourth value indicates rotation alongside normal axis
uniform vec4 progradePos;
uniform sampler2D progradeTex;

uniform vec4 retrogradePos;
uniform sampler2D retrogradeTex;

uniform vec4 normalPos;
uniform sampler2D normalTex;

uniform vec4 antinormalPos;
uniform sampler2D antinormalTex;

uniform vec4 radialInPos;
uniform sampler2D radialInTex;

uniform vec4 radialOutPos;
uniform sampler2D radialOutTex;

vec3 findPlaneIntersect(vec3 planeNormal, vec3 planePoint, vec3 abc)
{
	// x = at, y = bt, z = ct
	// Nx(at - Px) + Ny(bt - Py) + Nz(ct - Pz) = 0
	// (Nx * a) * t - (Nx * Px) + (Ny * b) * t - (Ny * Py) + (Nz * c) * t - (Nz * Pz)
	// At - I + Bt - J + Ct - K = 0
	// (A + B + C)t = I + J + K
	// t = (I + J + K) / (A + B + C)
	// Being A = (Nx * a), B = (Ny * b), C = (Nz * c)
	// I = (Nx * Px), J = (Ny * Py), K = (Nz * Pz)

	float A = planeNormal.x * abc.x;
	float B = planeNormal.y * abc.y;
	float C = planeNormal.z * abc.z;

	float I = planeNormal.x * planePoint.x;
	float J = planeNormal.y * planePoint.y;
	float K = planeNormal.z * planePoint.z;

	float t = (I + J + K) / (A + B + C);

	return vec3(t * abc.x, t * abc.y, t * abc.z);
}

vec2 findPlaneUV(vec3 coords, vec3 normal)
{
	vec3 u = normalize(vec3(normal.y, -normal.x, 0));
	vec3 v = cross(normal, u);

	return vec2(dot(u, coords), dot(v, coords));
}

vec4 doIndicator(vec3 vpos, vec3 ipos, sampler2D tex)
{
	vec3 vdiff = vpos.xyz - ipos.xyz;
	float dist = length(vdiff);
	if(dist < iconSize)
	{
		vec2 uv = findPlaneUV(vpos.xyz, -ipos.xyz) * (0.5 / iconSize) + vec2(0.5, 0.5);
		return texture(tex, uv);
	}
	else
	{
		return vec4(0, 0, 0, 0);
	}
}

void main()
{
	vec3 norm = normalize(vNormal);
	vec3 lightDir = normalize(vec3(1.0, -1.0, 0));
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * vec3(1, 1, 1);
	diffuse = max(vec3(0.2, 0.2, 0.2), diffuse);

	// Mapping (https://en.wikipedia.org/wiki/UV_mapping#Finding_UV_on_a_sphere)
	float pi = 3.14159265359;
	vec2 mapped = vec2(-atan(-vPosOriginal.x, -vPosOriginal.z)/(2*pi) + 0.5, (asin(-vPosOriginal.y) / pi + 0.5));
	vec3 col = vec3(texture(tex, vec2(0.25 - mapped.x, 1.0 - mapped.y)));

	vec4 prograde = doIndicator(vPosOriginal.xyz, progradePos.xyz, progradeTex);
	col = (prograde.xyz * prograde.w) + (col * (1.0 - prograde.w));
	vec4 retrograde = doIndicator(vPosOriginal.xyz, retrogradePos.xyz, retrogradeTex);
	col = (retrograde.xyz * retrograde.w) + (col * (1.0 - retrograde.w));
	vec4 normal = doIndicator(vPosOriginal.xyz, normalPos.xyz, normalTex);
	col = (normal.xyz * normal.w) + (col * (1.0 - normal.w));
	vec4 antinormal = doIndicator(vPosOriginal.xyz, antinormalPos.xyz, antinormalTex);
	col = (antinormal.xyz * antinormal.w) + (col * (1.0 - antinormal.w));
	vec4 radialin = doIndicator(vPosOriginal.xyz, radialInPos.xyz, radialInTex);
	col = (radialin.xyz * radialin.w) + (col * (1.0 - radialin.w));
	vec4 radialout = doIndicator(vPosOriginal.xyz, radialOutPos.xyz, radialOutTex);
	col = (radialout.xyz * radialout.w) + (col * (1.0 - radialout.w));


	FragColor = vec4(col * diffuse, 1.0);
}