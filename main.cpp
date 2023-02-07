/*************************************
 * Tri Tracer By David Owairu
 * created 24/01/2022
 *
 * References
 * [1]https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm - 24/01/2022
 * [2]http://www.cplusplus.com/forum/general/130110/ - 25/01/2022
 * [3]https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/moller-trumbore-ray-triangle-intersection - 24/01/2022
 * [4]https://stackoverflow.com/questions/36974817/splitting-input-string-based-on-whitespace-and-commas - 25/01/2022
 * [5]https://ogldev.org/www/tutorial19/tutorial19.html - 26/01/2022
 * [6]https://en.cppreference.com/w/cpp/language/operators - 26/01/2022
 * [7]http://ggt.sourceforge.net/gmtlReferenceGuide-0.6.1-html/namespacegmtl.html#af3cfac0b6f428e50cc92d2d2b6affae9 - 24/01/2022
 * [8]https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/geometry-of-a-triangle -25/01/2022
 * [9]https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/ray-triangle-intersection-geometric-solution - 24/01/2022
 * [10]https://www.scratchapixel.com/lessons/3d-basic-rendering/introduction-to-shading/shading-lights - 25/01/2022
 * [11]https://www.scratchapixel.com/lessons/3d-basic-rendering/introduction-to-shading/diffuse-lambertian-shading - 25/01/2022
 * [12]https://www.scratchapixel.com/lessons/3d-basic-rendering/introduction-to-shading/ligth-and-shadows - 26/01/2022
 * [13] Advanced for Computer graphics Slides[Ian Stephenson][2022]
*************************************/
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <gmtl/gmtl.h>
#undef main
#define PI 3.14159265
#define W 1600
#define H 900
using namespace gmtl;
using namespace std;

class Color {
public:
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 1.0f;

    Color() {};//default constructor
    Color(float _r, float _g, float _b)//constructor
    {
        r = _r;
        g = _g;
        b = _b;
    }

    //Operator Overloads
    Color operator+ (Color & other)//add two colors
    {
        Color c;
        c.r = r + other.r;
        c.g = g + other.g;
        c.b = b + other.b;
        return c;
    }

    Color operator+ (float other)//add color and float
    {
        Color c;
        c.r = r + other;
        c.g = g + other;
        c.b = b + other;
        return c;
    }

    const Color operator* (const float other) const//multiply color and float
    {
        Color c;
        c.r = r * other;
        c.g = g * other;
        c.b = b * other;
        return c;
    }

    const Color operator* (const Point3f& other) const//multiply color and point
    {
        Color c;
        c.r = r * other.mData[0];
        c.g = g * other.mData[1];
        c.b = b * other.mData[2];
        return c;
    }

    const Color operator* (const Vec3f& other) const//multiply color and vector
    {
        Color c;
        c.r = r * other.mData[0];
        c.g = g * other.mData[1];
        c.b = b * other.mData[2];
        return c;
    }

    const Color operator* (const Color& other) const//multiply two colors
    {
        Color c;
        c.r = r * other.r;
        c.g = g * other.g;
        c.b = b * other.b;
        return c;
    }
    const Color operator/ (const float other) const//divide color by float
    {
        Color c;
        c.r = r / other;
        c.g = g / other;
        c.b = b / other;
        return c;
    }
};

class Polygon
{
public:
    Point3f coord0;
    Point3f coord1;
    Point3f coord2;
    Vec3f normal;
    Color col;
    Color specColour;

    Polygon() {};//default constructor
    Polygon(Point3f _coord0, Point3f _coord1, Point3f _coord2, Color _col)//constructor
    {
        coord0 = _coord0;
        coord1 = _coord1;
        coord2 = _coord2;
        col = _col;
    }
};

struct hit_t {
    bool hit = false;
    bool inShadow = false;
    float dist = 9999999;
    Color col;
    Point3f hitpoint;
};

struct light
{
    Point3f pos;
    Color col;
    float intensity;
};

void SetPixel(int x, int y, SDL_Renderer* renderer, Color color) {
    color.r = min(max(color.r, 0.0f), 1.0f);
    color.g = min(max(color.g, 0.0f), 1.0f);
    color.b = min(max(color.b, 0.0f), 1.0f);
    SDL_SetRenderDrawColor(renderer, color.r * 255, color.g * 255, color.b * 255, 255);
    SDL_RenderDrawPoint(renderer, x, y);
}

/*based on code by "Rishit Sanmukhani" on stackoverflow.com - reference[4] , originates here:
    Technique for splitting strings using while loops,vectors and getline*/
vector<Polygon> LoadVerts(){
    string filename = "objectHeavy.txt";
    vector<Polygon> Trivec;
    ifstream input;
    input.open(filename);

    string line;
    int i = 0;

    while (getline(input, line)) {
        istringstream ss(line);
        string space;
        Polygon Triangle;
        int tricount = 0;

        while (getline(ss, space, ' ')) {
            istringstream ss2(space);
            string commas;
            Point3f vert;
            int count = 0;

            while (getline(ss2, commas, ',') && count < 3) {
                float coord = stof(commas);
                vert.mData[count] = coord;
                count++;
            }
            if (tricount == 0)
            {
                Triangle.coord0 = vert;
            }
            if (tricount == 1)
            {
                Triangle.coord1 = vert;
            }
            if (tricount == 2)
            {
                Triangle.coord2 = vert;
            }
            tricount++;
        }

        Triangle.col = Color(1.0f, 1.0f, 1.0f);
        Triangle.specColour = Color(1.0f, 1.0f, 1.0f);
        Vec3f vertex0 = Triangle.coord0;
        Vec3f vertex1 = Triangle.coord1;
        Vec3f vertex2 = Triangle.coord2;
        Vec3f edge1, edge2;
        edge1 = vertex1 - vertex0;
        edge2 = vertex2 - vertex0;
        cross(Triangle.normal, edge1, edge2);
        normalize(Triangle.normal);
        Trivec.push_back(Triangle);
    }

    input.close();
    return Trivec;
}
/*code based on "Rishit Sanmukhani" ends here.*/

Color diffuseLighting(Color incidentLi, Vec3f LiVec, Polygon plane) {
    //made use of references [11] and [13] 
    Color finalColor;
    float reflectivity = dot(plane.normal, LiVec);
    Color reflectedLight = incidentLi * reflectivity;
    finalColor = (plane.col /PI) * (reflectedLight);
    return finalColor;
}

Color SpecularHighlight(Color incidentLi, Vec3f view, Vec3f LiVec, Polygon plane) {
    //made use of references [5] and [13] 
    Color finalColor;
    float roughness = 0.025f;
    float ndotl_x2 = dot(LiVec,plane.normal) * 2;
    Vec3f specRefVec;
    specRefVec = LiVec - plane.normal * ndotl_x2;
    float specFac = dot(specRefVec, view);
    if (specFac > 0.0f) {
        finalColor = plane.specColour * pow(specFac, (1 / roughness));
    }
    return finalColor;
}

/*based on code reference[1] on sratchapixel.com, originates here :
    caluculates ray - triangle intersection*/
hit_t RayIntersectsTriangle(const Rayf& Ray, const Polygon& plane)
{
    hit_t myHit;
    const float EPSILON = 0.0000001f;
    Vec3f vertex0 = plane.coord0;
    Vec3f vertex1 = plane.coord1;
    Vec3f vertex2 = plane.coord2;
    Vec3f edge1, edge2, h, s, q;
    float a, f, u, v;
    edge1 = vertex1 - vertex0;
    edge2 = vertex2 - vertex0;
    cross(h, Ray.mDir, edge2);
    a = dot(edge1, h);
    if (a > -EPSILON)
        return myHit;    // This ray is parallel to this triangle.
    f = 1.0f / a;
    s = Ray.mOrigin - vertex0;
    u = f * dot(s, h);
    if (u < 0.0f || u > 1.0f)
        return myHit;
    q = cross(q, s, edge1);
    v = f * dot(Ray.mDir, q);
    if (v < 0.0f || u + v > 1.0f)
        return myHit;
   
    myHit.dist = f * dot(edge2, q);//t
    if (myHit.dist > EPSILON) // ray intersection
    {
        myHit.hitpoint = Ray.mOrigin + Ray.mDir * myHit.dist;
        myHit.hit = true;
        return myHit;
    }
    else 
        return myHit;
}
/*code based on reference[1] ends here.*/

hit_t TracePoly(const Rayf& Ray, const Polygon& poly, const vector<Polygon>& polys, const light& Li) {
    hit_t myHit;
    int numpolys = polys.size();
    myHit = RayIntersectsTriangle(Ray, poly);
    if (myHit.hit) {
        //lighting claculation
        Vec3f liVec = myHit.hitpoint -Li.pos;

        Color incidentLight;
        float sqr = dot(liVec, liVec);
        incidentLight = Li.col * (Li.intensity )/sqr;
        normalize(liVec);

        float distToLight = pow((Li.pos.mData[0] - myHit.hitpoint[0]), 2) + pow((Li.pos.mData[1] - myHit.hitpoint[1]), 2)
            + pow((Li.pos.mData[2] - myHit.hitpoint[2]), 2);
        distToLight = sqrt(distToLight);
        
        //shadow check
        Rayf shadowRay = Rayf(myHit.hitpoint, -liVec);
        hit_t shadowHit;
        for (int i = 0; i < numpolys; i++) {

            shadowHit = RayIntersectsTriangle(shadowRay, polys[i]);
            if (shadowHit.hit && shadowHit.dist <= distToLight - 0.001f)
            {
                shadowHit.inShadow = true;
                break;
            }
        }

        float ambientLi = 0.05f;
        
        //diffuse shading
        Color DLC = diffuseLighting(incidentLight, liVec, poly);
        myHit.col = DLC + ambientLi;

        
        if (shadowHit.inShadow) {
            myHit.col = myHit.col * ambientLi;
        }
        else {
            //specualar highlight
            Color SLC = SpecularHighlight(incidentLight, -Ray.mDir, liVec, poly);
            myHit.col = myHit.col + SLC;
        }
    }
    return myHit;
}

hit_t TraceMesh(const Rayf& Ray, vector<Polygon>& plane, const light& Li) {
    hit_t bestHit;
    int numpolys = plane.size();
    for (int poly = 0; poly < numpolys; poly++)
    {
        hit_t h = TracePoly(Ray, plane[poly],plane,Li);//trace particular tri/polygon
        if (h.hit)
        {
            if (h.dist < bestHit.dist)//check if current polygon ray is closer to the camera that previous one
            {
                bestHit = h;
            }
        }
    }
    return bestHit;
};

int main() {

    SDL_Surface* windowSurface = NULL;

    //SDL Initializations
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        std::cout << "error:" << SDL_GetError();
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("SDL_Temp", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, W, H, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        std::cout << "Unfortunatelt program could not create window";
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Surface* image = SDL_CreateRGBSurface(0, W, H, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    vector<Polygon> mesh = LoadVerts();
    light myLight = { Point3f(0,-50,5),Color(0.654f,0.2367f,0.344f),15000.0f };
    float focalLength = 700;

    for (int x = 0; x < W; x++)
    {
        for (int y = 0; y < H; y++)
        {
            Point3f cameraOrigin = Point3f(0.0f, 0.0f, 0.0f);
            Vec3f pixelVec = Vec3f((x - W / 2), (y - H / 2), focalLength);
            normalize(pixelVec);
            Rayf ray = Rayf(cameraOrigin, pixelVec);
            hit_t objh = TraceMesh(ray, mesh,myLight);
            if (objh.hit)
            {
                SetPixel(x, y, renderer, objh.col);
            }
        }
        if (x % 10 == 0) cout << 0;//progress bar
    }

    SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_ARGB8888, image->pixels, image->pitch);
    SDL_SaveBMP(image, "Final_Image.bmp");
    bool running = true;

    while (running)
    {
        SDL_Event event;
        (SDL_PollEvent(&event));
        if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
            break;
        SDL_RenderPresent(renderer);
    }

    SDL_FreeSurface(image);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}