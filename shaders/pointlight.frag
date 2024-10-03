#version 430

out vec4 colorOut;

struct Materials {
	vec4 diffuse;
	vec4 ambient;
	vec4 specular;
	vec4 emissive;
	float shininess;
	int texCount;
};

uniform Materials mat;

uniform vec4 headlight_dir;			// Boat headlight diretion
uniform vec4 headlight_dir2;		// Boat headlight 2 diretion
uniform float headlight_angle;		// Boat headlight angle
uniform float headlight_exp;		// Boat headlight quality 

uniform float buoy_const_att;
uniform float buoy_linear_att;
uniform float buoy_quad_att;

uniform bool isSunActive;
uniform bool isBuoyLightsActive;
uniform bool isHeadlightsActive;

in Data {
	vec3 normal;
	vec3 eye;
	vec3 sunLightDir;
	vec3 buoyLightDir[6];
	vec3 headlightDir;
	vec3 headlightDir2;
} DataIn;

vec3 l;				// normalized light direction
vec3 h;				// half vector
float intensity;
float distance;		
float intSpec;		// specular component
float attenuation;
float spotEffect;
vec3 sd;

void main() {

	vec4 totalDiffuse = vec4(0.0);
	vec4 totalSpecular = vec4(0.0);

	vec3 n = normalize(DataIn.normal);
	vec3 e = normalize(DataIn.eye);


	// Light 1 - Sun light (Directional)
	if( isSunActive == true ){
		l = normalize(DataIn.sunLightDir);  
		intensity = max(dot(n, l), 0.0);	

		if (intensity > 0.0) {
			h = normalize(l + e);					
			intSpec = max(dot(h, n), 0.0);	
			totalSpecular += mat.specular * pow(intSpec, mat.shininess); 
		}
		totalDiffuse += intensity * mat.diffuse;  
	}
	


	// Buoy lights (Point)
	if( isBuoyLightsActive == true ){
		for(int i = 0; i < DataIn.buoyLightDir.length; i++){
			l = normalize(DataIn.buoyLightDir[i]);				 
			distance = length(DataIn.buoyLightDir[i]);			
			attenuation = 1.0 / (buoy_const_att + buoy_linear_att * distance + buoy_quad_att * distance * distance);
			intensity = max(dot(n, l), 0.0) * attenuation;

			if (intensity > 0.0) {
				h = normalize(l + e);
				intSpec = max(dot(h, n), 0.0);
				totalSpecular += mat.specular * pow(intSpec, mat.shininess) * attenuation;
			}
			totalDiffuse += intensity * mat.diffuse;
		}
	}
	


	// Boat headlight (Spotlight)
	if( isHeadlightsActive == true ){
		l = normalize(DataIn.headlightDir);
		sd = normalize(vec3(-headlight_dir));
		spotEffect = dot(l, sd);	// Angle between spotlight direction and light direction

		if (spotEffect > headlight_angle) {
			attenuation = pow(spotEffect, headlight_exp); 
			intensity = max(dot(n, l), 0.0) * attenuation;

			if (intensity > 0.0) {
				h = normalize(l + e);
				intSpec = max(dot(h, n), 0.0);
				totalSpecular += mat.specular * pow(intSpec, mat.shininess) * attenuation;
			}
			totalDiffuse += intensity * mat.diffuse;
		}

		// Boat headlight 2 (Spotlight)
		l = normalize(DataIn.headlightDir2);
		sd = normalize(vec3(-headlight_dir2));
		spotEffect = dot(l, sd);  

		if (spotEffect > headlight_angle) {
			attenuation = pow(spotEffect, headlight_exp);  
			intensity = max(dot(n, l), 0.0) * attenuation;

			if (intensity > 0.0) {
				h = normalize(l + e);
				intSpec = max(dot(h, n), 0.0);
				totalSpecular += mat.specular * pow(intSpec, mat.shininess) * attenuation;
			}
			totalDiffuse += intensity * mat.diffuse;
		}
	}

	colorOut = max(totalDiffuse + totalSpecular, mat.ambient);
}
