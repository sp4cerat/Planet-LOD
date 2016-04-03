### Planet-LOD - Spherical Level-of-Detail

#### License : MIT
#### http://opensource.org/licenses/MIT

**Summary** 

This is a simple example of how to render a planet with spherical LOD in less than 100 lines of c++ code. The algorithm renders the planet as cube with 6 sides. Each side is basically a quad which is being subdivided based on the center of detail.

What the code is : 

* A simple demo to show how a planet renderer with quadtree works
* As short as possible so you can experiment with the algorithm
* Easy to understand

What the code is not:

* A ready to use planet rendering library with shaders frustum culling etc
* A way to render a planet with best performance
* A demonstration of modern high performance gl rendering

**Screenshot** 

![Screenshot1](https://github.com/sp4cerat/Planet-LOD/blob/master/screenshot/Animation.gif?raw=true)


	struct World
	{
		enum { search_res = 4 }; // search 4x4 points in a tile if a recusion is required
		enum { tiles_res = 2 };  // tile res : resolution of the tiles 
		enum { radius = 1 };     // planet radius
	
		static void draw_quad(vec3f pos, float size) 
		{
			// comment out the following line for solid rendering
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glBegin(GL_QUADS);
			loopi(0, 4)
			{
				vec3f p = pos + vec3f((i & 1) ^ (i >> 1), (i >> 1), 0)*size;
				p = p.norm() * radius;
				glVertex3f(p.x, p.y, p.z);
			}
			glEnd();
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);		
		};
		static void draw_recursive(vec3f p, float size, vec3f center)
		{
			float tile_size = float(size) / float(tiles_res);
			float ratio		= gui.screen[0].slider["lod.ratio"].val;
			float minsize	= gui.screen[0].slider["detail"].val;
	
			loopi(0, tiles_res) loopj(0, tiles_res)
			{
				float a = float(i) *size / float(tiles_res) + p.x;
				float b = float(j) *size / float(tiles_res) + p.y;
	
				float dist = 999999;
				loopk(0, search_res + 1)	
				loopl(0, search_res + 1)
				{
					float dotp = vec3f(
						a + (float(k) / search_res)*tile_size, 
						b + (float(l) / search_res)*tile_size, p.z).norm().dot(center);
					float d = acos(dotp) / M_PI; // anlge
					dist = min(dist, d);
				}
				if (dist > 0.5) continue;//culling
	
				// recurse ?
				if (dist < ratio*size && size > minsize)
					draw_recursive(vec3f(a, b, p.z), tile_size, center);	// yes
				else
					draw_quad(vec3f(a, b, p.z), tile_size);					// no
			}
		};
		static void draw(vec3f center)
		{
			// center is the center of detail on the sphere
			// ||center|| = 1
			vec3f patch_start(-1, -1, -1);
			float patch_size = 2; // plane xy size
	
			// Draw 6 Planes
	
			// Front
			World::draw_recursive(patch_start, 2, center);
			// Back
			glRotatef(180, 0, 1, 0);
			World::draw_recursive(patch_start, patch_size, vec3f(-center.x, center.y, -center.z));
			glRotatef(-180, 0, 1, 0);
			// Right
			glRotatef(90, 0, 1, 0);
			World::draw_recursive(patch_start, patch_size, vec3f(-center.z, center.y, center.x));
			glRotatef(-90, 0, 1, 0);
			// Left
			glRotatef(-90, 0, 1, 0);
			World::draw_recursive(patch_start, patch_size, vec3f(center.z, center.y, -center.x));
			glRotatef(90, 0, 1, 0);
			// Up
			glRotatef(90, 1, 0, 0);
			World::draw_recursive(patch_start, patch_size, vec3f(center.x, center.z, -center.y));
			glRotatef(-90, 1, 0, 0);
			// Down
			glRotatef(-90, 1, 0, 0);
			World::draw_recursive(patch_start, patch_size, vec3f(center.x, -center.z, center.y));
			glRotatef(90, 1, 0, 0);
		}
	};
