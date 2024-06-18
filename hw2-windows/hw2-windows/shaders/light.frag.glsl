# version 330 core
// Do not use any version older than 330!

/* This is the fragment shader for reading in a scene description, including 
   lighting.  Uniform lights are specified from the main program, and used in 
   the shader.  As well as the material parameters of the object.  */

vec4 ComputeLight(const in vec3 dir, const in vec4 lightcolor, const in vec3 normal, const in vec3 halfvec, const in vec4 _diffuse, const in vec4 _specular, const in float _shininess);

// Inputs to the fragment shader are the outputs of the same name of the vertex shader.
// Note that the default output, gl_Position, is inaccessible!
in vec3 mynormal; // from vertex shader outputs
in vec4 myvertex; 

// You will certainly need this matrix for your lighting calculations
uniform mat4 modelview;

// This first defined output of type vec4 will be the fragment color
out vec4 fragColor;

uniform vec3 color;

const int numLights = 10; 
uniform bool enablelighting; // are we lighting at all (global).
uniform vec4 lightposn[numLights]; // positions of lights 
uniform vec4 lightcolor[numLights]; // colors of lights
uniform int numused;               // number of lights used

// Now, set the material parameters.
// I use ambient, diffuse, specular, shininess. 
// But, the ambient is just additive and doesn't multiply the lights.  

uniform vec4 ambient; 
uniform vec4 diffuse; 
uniform vec4 specular; 
uniform vec4 emission; 
uniform float shininess; 

void main (void) 
{       
    if (enablelighting) {       
        vec4 finalcolor; 

        // YOUR CODE FOR HW 2 HERE
        // A key part is implementation of the fragment shader

        vec4 vertex = modelview * myvertex;
        
        // construct the eye viewing direction vector
        const vec3 eyepos = vec3(0.0,0.0,0.0);
        vec3 mypos = vertex.xyz / vertex.w;
        vec3 eyedir = normalize(eyepos - mypos);

        // apply inverse transpose of the transformations done on modelview to the normals
        mat3 it_mv = mat3(transpose(inverse(modelview)));
        vec3 normal = normalize(it_mv * mynormal);

        // compute lights and sum to get final fragColor
        finalcolor = vec4(0.0,0.0,0.0,1.0);
        for(int i = 0; i < numused; i++) {
            vec4 lightpos = lightposn[i]; // already applied modelview in display.cpp

            // when lightpos.w is 0 = directional light; otherwise, point
            vec3 lightpos_dehom = (lightpos.w != 0.0) ? (lightpos.xyz / lightpos.w) : lightpos.xyz;
            vec3 lightdir = (lightpos.w != 0.0) ? normalize(lightpos_dehom - mypos) : normalize(lightpos_dehom);
            vec3 half = normalize(lightdir + eyedir);
            vec4 vcolor = ComputeLight(
                lightdir, lightcolor[i], normal, half, diffuse, specular, shininess);

            
            finalcolor += vcolor;
        }

        finalcolor += (ambient + emission);

        // Color all pixels black for now, remove this in your implementation!
        // finalcolor = vec4(0.0f, 1.0f, 0.0f, 1.0f); 

        fragColor = finalcolor; 
    } else {
        fragColor = vec4(color, 1.0f); 
    }
}

vec4 ComputeLight(const in vec3 dir,
                  const in vec4 lightcolor,
                  const in vec3 normal,
                  const in vec3 halfvec, 
                  const in vec4 _diffuse, 
                  const in vec4 _specular,
                  const in float _shininess) {
    // first compute diffuse lighting
    float n_dot_L = dot(normal, dir);
    vec4 lambert = _diffuse * lightcolor * max(n_dot_L, 0.0);

    // add specular highlight
    float n_dot_H = dot(normal, halfvec);
    vec4 phong = _specular * lightcolor * pow(max(n_dot_H, 0.0), _shininess);
    
    vec4 ret = lambert + phong;
    return ret;
}