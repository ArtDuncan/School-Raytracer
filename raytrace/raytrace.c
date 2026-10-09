

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <math.h>
#include <stdio.h>
#include <stdbool.h>

float camera[3] = {0, 0, 0};
float light[3] = {3, 5, -15};

typedef struct
{
    float xPos;
    float yPos;
    float zPos;
    float xDir;
    float yDir;
    float zDir;
} Ray;

typedef struct
{
    float color[3];
    int reflective;
} Material;

typedef struct
{
    float time;
    float xPos;
    float yPos;
    float zPos;
    Material material;
    Ray norm;
} rayHit;

typedef struct
{
    float xPos;
    float yPos;
    float zPos;
    float radius;
    Material material;
} Sphere;

typedef struct
{
    float a[3];
    float b[3];
    float c[3];
    Material material;
} Triangle;

Ray normVector(Ray ray)
{
    float factor = sqrt(ray.xDir * ray.xDir + ray.yDir * ray.yDir + ray.zDir * ray.zDir);
    Ray newRay;
    newRay.xDir = ray.xDir/factor;
    newRay.yDir = ray.yDir/factor;
    newRay.zDir = ray.zDir/factor;
    newRay.xPos = ray.xPos;
    newRay.yPos = ray.yPos;
    newRay.zPos = ray.zPos;
    return newRay;
}

Ray getRay(int x, int y)
{
    Ray newRay;
    newRay.xPos = 0;
    newRay.yPos = 0;
    newRay.zPos = 0;
    newRay.xDir = (((float)x + .5)/512) * 2 - 1;
    newRay.yDir = (-1 * ((float)y + .5)/512) * 2 + 1;
    newRay.zDir = -2;
    newRay = normVector(newRay);
    return newRay;
}

Sphere makeSphere(float x, float y, float z, float radius, Material mat)
{
    Sphere newSphere;
    newSphere.xPos = x;
    newSphere.yPos = y;
    newSphere.zPos = z;
    newSphere.radius = radius;
    newSphere.material = mat;
    return newSphere;
}

Triangle makeTriangle(float a0, float a1, float a2, float b0, float b1, float b2, float c0, float c1, float c2, Material mat)
{
    Triangle newTriangle;
    newTriangle.a[0] = a0;
    newTriangle.a[1] = a1;
    newTriangle.a[2] = a2;
    newTriangle.b[0] = b0;
    newTriangle.b[1] = b1;
    newTriangle.b[2] = b2;
    newTriangle.c[0] = c0;
    newTriangle.c[1] = c1;
    newTriangle.c[2] = c2;
    newTriangle.material = mat;
    return newTriangle;
}

rayHit raySphereI(Sphere sphere, Ray ray)
{
    Ray startCenter;
    rayHit hit;
    hit.material.color[0] = sphere.material.color[0];
    hit.material.color[1] = sphere.material.color[1];
    hit.material.color[2] = sphere.material.color[2];
    hit.material.reflective = sphere.material.reflective;
    startCenter.xPos = ray.xPos - sphere.xPos;
    startCenter.yPos = ray.yPos - sphere.yPos;
    startCenter.zPos = ray.zPos - sphere.zPos;
    float discriminant = 
    (ray.xDir * (startCenter.xPos) + ray.yDir * (startCenter.yPos) + ray.zDir * (startCenter.zPos)) *
    (ray.xDir * (startCenter.xPos) + ray.yDir * (startCenter.yPos) + ray.zDir * (startCenter.zPos)) -
    (ray.xDir * ray.xDir + ray.yDir * ray.yDir + ray.zDir * ray.zDir) *
    (((startCenter.xPos) * (startCenter.xPos) + (startCenter.yPos) * (startCenter.yPos) + (startCenter.zPos) * (startCenter.zPos)) -
    sphere.radius * sphere.radius);
    if(discriminant < 0)
    {
        hit.time = -1;
        return hit;
    }
    float time1 = (-1 * (ray.xDir * startCenter.xPos + ray.yDir * startCenter.yPos + ray.zDir * startCenter.zPos) +
    discriminant) / (ray.xDir * ray.xDir + ray.yDir * ray.yDir + ray.zDir * ray.zDir);
    float time2 = (-1 * (ray.xDir * startCenter.xPos + ray.yDir * startCenter.yPos + ray.zDir * startCenter.zPos) -
    discriminant) / (ray.xDir * ray.xDir + ray.yDir * ray.yDir + ray.zDir * ray.zDir);
    if(time1 < 0)
    {
        hit.time = time2;
        hit.xPos = time2 * ray.xDir;
        hit.yPos = time2 * ray.yDir;
        hit.zPos = time2 * ray.zDir;
        return hit;
    }
    if(time2 < 0)
    {
        hit.time = time1;
        hit.xPos = time1 * ray.xDir;
        hit.yPos = time1 * ray.yDir;
        hit.zPos = time1 * ray.zDir;
        return hit;
    }
    if(time1 < time2)
    {
        hit.time = time1;
        hit.xPos = time1 * ray.xDir;
        hit.yPos = time1 * ray.yDir;
        hit.zPos = time1 * ray.zDir;
        return hit;
    }
    hit.time = time2;
    hit.xPos = time2 * ray.xDir;
    hit.yPos = time2 * ray.yDir;
    hit.zPos = time2 * ray.zDir;
    return hit;
}

rayHit rayTriI(Triangle tri, Ray ray)
{
    rayHit hit;
    hit.time = -1;
    hit.material.color[0] = tri.material.color[0];
    hit.material.color[1] = tri.material.color[1];
    hit.material.color[2] = tri.material.color[2];
    hit.material.reflective = tri.material.reflective;
    float A = tri.a[0] - tri.b[0];
    float B = tri.a[1] - tri.b[1];
    float C = tri.a[2] - tri.b[2];
    float D = tri.a[0] - tri.c[0];
    float E = tri.a[1] - tri.c[1];
    float F = tri.a[2] - tri.c[2];
    float G = ray.xDir;
    float H = ray.yDir;
    float I = ray.zDir;
    float J = tri.a[0] - ray.xPos;
    float K = tri.a[1] - ray.yPos;
    float L = tri.a[2] - ray.zPos;
    float M = A * (E * I - H * F) + B * (G * F - D * I) + C * (D * H - E * G);
    float time = (-1 * (F * (A * K - J * B) + E * (J * C - A * L) + D * (B * L - K * C))) / M;
    if(time < 0)
    {
        return hit;
    }
    float gamma = (I * (A * K - J * B) + H * (J * C - A * L) + G * (B * L - K * C)) / M;
    if(gamma < 0 || gamma > 1)
    {
        return hit;
    }
    float beta = (J * (E * I - H * F) + K * (G * F - D * I) + L * (D * H - E * G)) / M;
    if(beta < 0 || beta > 1 - gamma)
    {
        return hit;
    }
    hit.time = time;
    hit.material = tri.material;
    hit.xPos = time * ray.xDir;
    hit.yPos = time * ray.yDir;
    hit.zPos = time * ray.zDir;
    return hit;
}

Ray sphereNormal(Sphere sphere, float x, float y, float z)
{
    Ray newRay;
    newRay.xPos = sphere.xPos;
    newRay.yPos = sphere.yPos;
    newRay.zPos = sphere.zPos;
    newRay.xDir = x - sphere.xPos;
    newRay.yDir = y - sphere.yPos;
    newRay.zDir = z - sphere.zPos;
    newRay = normVector(newRay);
    return newRay;
}

Ray triNormal(Triangle tri, float x, float y, float z)
{
    Ray newRay;
    float A[3] = {tri.b[0] - tri.a[0], tri.b[1] - tri.a[1], tri.b[2] - tri.a[2]};
    float B[3] = {tri.c[0] - tri.a[0], tri.c[1] - tri.a[1], tri.c[2] - tri.a[2]};
    newRay.xPos = x;
    newRay.yPos = y;
    newRay.zPos = z;
    newRay.xDir = A[1] * B[2] - A[2] * B[1];
    newRay.yDir = A[2] * B[0] - A[0] * B[2];
    newRay.zDir = A[0] * B[1] - A[1] * B[0];
    newRay = normVector(newRay);
    return newRay;
}

Ray lightRay(float x, float y, float z, int reflects, float xoff[10], float yoff[10], float zoff[10])
{
    Ray newRay;
    for( int i = 0; i < reflects; i++)
    {
        x = x + xoff[i];
        y = y + yoff[i];
        z = z + zoff[i];
    }
    newRay.xDir = light[0] - x;
    newRay.yDir = light[1] - y;
    newRay.zDir = light[2] - z;
    newRay.xPos = x;
    newRay.yPos = y;
    newRay.zPos = z;
    return newRay;
}

Ray rayReflect(Ray ogRay, Ray normalRay, float x, float y, float z)
{
    Ray newRay;
    newRay.xPos = x;
    newRay.yPos = y;
    newRay.zPos = z;
    newRay.xDir = (ogRay.xDir - (2 * (ogRay.xDir * normalRay.xDir + ogRay.yDir * normalRay.yDir + ogRay.zDir * normalRay.zDir) * normalRay.xDir));
    newRay.yDir = (ogRay.yDir - (2 * (ogRay.xDir * normalRay.xDir + ogRay.yDir * normalRay.yDir + ogRay.zDir * normalRay.zDir) * normalRay.yDir));
    newRay.zDir = (ogRay.zDir - (2 * (ogRay.xDir * normalRay.xDir + ogRay.yDir * normalRay.yDir + ogRay.zDir * normalRay.zDir) * normalRay.zDir));
    newRay = normVector(newRay);
    return newRay;
}

int main()
{
    int imageWidth = 512;
    int imageHeight = 512;
    int numChannels = 3;
    unsigned char* imageData = malloc(sizeof(unsigned char) * imageHeight * imageWidth * numChannels);

    Ray currentRay;

    Material redMat;
    redMat.reflective = 0;
    redMat.color[0] = 1;
    redMat.color[1] = 0;
    redMat.color[2] = 0;
    Material blueMat;
    blueMat.reflective = 0;
    blueMat.color[0] = 0;
    blueMat.color[1] = 0;
    blueMat.color[2] = 1;
    Material whiteMat;
    whiteMat.reflective = 0;
    whiteMat.color[0] = 1;
    whiteMat.color[1] = 1;
    whiteMat.color[2] = 1;
    Material reflectMat;
    reflectMat.reflective = 1;
    reflectMat.color[0] = 0;
    reflectMat.color[1] = 0;
    reflectMat.color[2] = 0;

    int sphereNum = 3;
    Sphere* spheres = malloc(sizeof(Sphere) * sphereNum);
    spheres[0] = makeSphere(-3, -1, -14, 1, redMat);
    spheres[1] = makeSphere(0, 0, -16, 2, reflectMat);
    spheres[2] = makeSphere(3, -1, -14, 1, reflectMat);

    int triNum = 5;
    Triangle* tris = malloc(sizeof(Triangle) * triNum);
    tris[0] = makeTriangle(-8, -2, -20, 8, -2, -20, 8, 10, -20, blueMat);
    tris[1] = makeTriangle(-8, -2, -20, 8, 10, -20, -8, 10, -20, blueMat);
    tris[2] = makeTriangle(-8, -2, -20, 8, -2, -10, 8, -2, -20, whiteMat);
    tris[3] = makeTriangle(-8, -2, -20, -8, -2, -10, 8, -2, -10, whiteMat);
    tris[4] = makeTriangle(8, -2, -20, 8, -2, -10, 8, 10, -20, redMat);

    float reflectx[10];
    float reflecty[10];
    float reflectz[10];
    
    rayHit hits[sphereNum + triNum];
    Ray norm;
    Ray toLight;
    float diffuse;
    float smallestT;
    int smallestI;
    int reflects;

    for(int h = 0; h < imageHeight; h++)
    {
        for (int w = 0; w < imageWidth; w++)
        {
            reflects = 0;
            currentRay = getRay(w, h);
            while(reflects < 10)
            {
                smallestT = -1;
                smallestI = -1;
                for(int i = 0; i < sphereNum; i++)
                {
                    hits[i] = raySphereI(spheres[i], currentRay);
                    if((smallestT == -1 && hits[i].time > 0.05) || (hits[i].time < smallestT && hits[i].time > 0.01))
                    {
                        smallestI = i;
                        smallestT = hits[i].time;
                    }
                }
            
                for(int i = 0; i < triNum; i++)
                {
                    hits[i + sphereNum] = rayTriI(tris[i], currentRay);
                    if((smallestT == -1 && hits[i + sphereNum].time > 0.01) || (hits[i + sphereNum].time < smallestT && hits[i + sphereNum].time > 0.05))
                    {
                        smallestI = i + sphereNum;
                        smallestT = hits[i + sphereNum].time;
                    }
                }

                if(smallestT == -1)
                {
                    imageData[((h*imageWidth) + w)*3] = 0;
                    imageData[((h*imageWidth) + w)*3 +1] = 0;
                    imageData[((h*imageWidth) + w)*3 +2] = 0;
                    reflects = 11;
                }
                else
                {
                    if(smallestI < sphereNum)
                    {
                        norm = sphereNormal(spheres[smallestI], hits[smallestI].xPos, hits[smallestI].yPos, hits[smallestI].zPos);
                    }
                    else
                    {
                        norm = triNormal(tris[smallestI - sphereNum], hits[smallestI].xPos, hits[smallestI].yPos, hits[smallestI].zPos);
                    }
                    toLight = lightRay(hits[smallestI].xPos, hits[smallestI].yPos, hits[smallestI].zPos, reflects, reflectx, reflecty, reflectz);
                    toLight = normVector(toLight);

                    if(hits[smallestI].material.reflective == 1 && reflects < 10 && hits[smallestI].time > .01)
                    {
                        currentRay = rayReflect(currentRay, norm, hits[smallestI].xPos, hits[smallestI].yPos, hits[smallestI].zPos);
                        reflectx[reflects] = hits[smallestI].xPos;
                        reflecty[reflects] = hits[smallestI].yPos;
                        reflectz[reflects] = hits[smallestI].zPos;
                        reflects++;
                    }
                    else
                    {
                        diffuse = toLight.xDir * norm.xDir + toLight.yDir * norm.yDir + toLight.zDir * norm.zDir;
                        if(diffuse < .2)
                        {
                            diffuse = .2;
                        }
                        rayHit currentHit;
                        for(int i = 0; i < sphereNum; i++)
                        {
                            currentHit = raySphereI(spheres[i], toLight);
                            if(currentHit.time > 0.0001 && i != smallestI)
                            {
                                diffuse = .2;
                            }
                        }
                        for(int i = 0; i < triNum; i++)
                        {
                            currentHit = rayTriI(tris[i], toLight);
                            if(currentHit.time > 0.0001&& (i + sphereNum) != smallestI)
                            {
                                diffuse = .2;
                            }
                        }
                        imageData[((h*imageWidth) + w)*3] = diffuse * hits[smallestI].material.color[0] * 255;
                        imageData[((h*imageWidth) + w)*3 +1] = diffuse * hits[smallestI].material.color[1] * 255;
                        imageData[((h*imageWidth) + w)*3 +2] = diffuse * hits[smallestI].material.color[2] * 255;
                        reflects = 11;
                    }
                }
            }
            if(reflects == 10)
            {
                imageData[((h*imageWidth) + w)*3] = 0;
                imageData[((h*imageWidth) + w)*3 +1] = 0;
                imageData[((h*imageWidth) + w)*3 +2] = 0;
            }
        } 
    }

    stbi_write_png("reference.png", imageWidth, imageHeight, numChannels, imageData, imageWidth * numChannels);
    free(imageData);
    free(tris);
    free(spheres);
    return 0;
}