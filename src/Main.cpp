////////////////////////////////////////////////////////////////////////////////
//
// Planet-LOD - A simple implementation for rendering planets with LOD
//
// (C) by Sven Forstmann in 2016
//
// Please credit this code if you use it anywhere
//
// License : MIT
// http://opensource.org/licenses/MIT
////////////////////////////////////////////////////////////////////////////////
// Mathlib included from 
// http://sourceforge.net/projects/nebuladevice/
////////////////////////////////////////////////////////////////////////////////
#include "gui.h"
////////////////////////////////////////////////////////////////////////////////

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
////////////////////////////////////////////////////////////////////////////////
void init_gui()
{
	gui.init( Gui::Flags::CONTEXT_MENU | Gui::Flags::CUSTOM_MOUSE ,
				"../data/gui_global.txt" , 
				"../data/gui_skin.txt");

	// Add simple Renderer
	{
		// render callback function
		auto render_func=[](Gui::Window *window,Gui::Button* control,int index)
		{	
			if((!control) || (!window) || window->get_toggled() ) return;

			Gui::Window &w=*((Gui::Window*) window);
			Gui::Button &b=w.button["canvas"];

			// resize button to window client area
			b.x=w.pad_left;
			b.y=w.pad_up;
			b.sx=w.sx-w.pad_left-w.pad_right;
			b.sy=w.sy-w.pad_up-w.pad_down;

			bool draw = 1;// (b.hover || b.pressed) && (gui.mouse.button[0] || gui.mouse.button[1] || gui.mouse.wheel != 0);
			if(control!=&b)draw=1;
		
			void *fb=b.var.ptr["fbo"]; if(!fb) return;
			FBO &fbo=*(FBO*)fb;

			if(b.sx!=fbo.width || b.sy!=fbo.height ) // resize FBO ?
			{
				fbo.clear();
				fbo.init(b.sx,b.sy);
				b.skin.tex_normal=b.skin.tex_hover=b.skin.tex_selected=fbo.color_tex;
				draw=1;
			}
			if (!draw) return;

			quaternion q(b.var.vec4["rotation"]);
			if(gui.mouse.button[0] & b.pressed)				// rotate by left mouse
			{
				quaternion qx,qy;
				qx.set_rotate_y( (float)gui.mouse.dx/100);
				qy.set_rotate_x(-(float)gui.mouse.dy/100);
				q=qy*qx*q;
				b.var.vec4["rotation"]=vec4f(q.x,q.y,q.z,q.w);
			}
			double z=b.var.number["zoom"];
			if(b.hover)										// zoom by wheel
			{
				z=clamp( z-gui.mouse.wheel*2 , 2,120 );
				b.var.number["zoom"]=z;
				gui.mouse.wheel=0;
			}
			vec4f pos=b.var.vec4["position"];
			if(gui.mouse.button[1] && (b.pressed||b.hover))	// move by middle button
			{
				pos=pos+vec4f((float)gui.mouse.dx*z/40000.0f,(float)gui.mouse.dy*z/40000.0f,0,0);
				b.var.vec4["position"]=pos;
			}

			// render to fbo
			fbo.enable();
			glClearColor(1,1,1,1);
			glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
			glMatrixMode(GL_PROJECTION);glPushMatrix();glLoadIdentity();
			gluPerspective(z, (GLfloat)b.sx/(GLfloat)b.sy, 0.01 , 10.0);
			glMatrixMode(GL_MODELVIEW);glPushMatrix();glLoadIdentity();
			glTranslatef(pos.x,pos.y,-pos.z);		// apply movement
			glMultMatrixf(&matrix44(q).m[0][0]);	// apply quaternion rotation
			glColor4f(0,0,0,1);
			glEnable(GL_DEPTH_TEST);
						
			// center of detail
			q.invert();
			vec3f center = matrix44(q).z_component().norm();

			////////////////////
			//
			//  Draw our World
			//
			World::draw(center);

			////////////////////
			//
			// Draw Bounding Box
			//
			glutWireCube(2);
			
			glDisable(GL_DEPTH_TEST);
			glPopMatrix(); glMatrixMode(GL_PROJECTION); glPopMatrix();
			fbo.disable();
		};

		Gui::Window &w = gui.screen[0];
		w.button["canvas"]=Gui::Button("",20,250);
		w.button["canvas"].skin=Skin("");
		w.button["canvas"].var.vec4["position"]=vec4f(0,0,2.5,0); // vec4
		w.button["canvas"].var.vec4["rotation"]=vec4f(1,0,0,0); // quaternion
		w.button["canvas"].var.number["zoom"]=80; // fov
		w.button["canvas"].var.ptr["fbo"]=0;
		w.button["canvas"].callback_all=render_func;
		w.button["canvas"].callback_init=[](Gui::Window *w,Gui::Button* b,int i) // call before drawing the first time
			{
				if(w)if(b){b->var.ptr["fbo"]=new FBO(100,100);} // 100,100 is just for initialization; will be resized
			};
		w.button["canvas"].callback_exit=[](Gui::Window *w,Gui::Button* b,int i) // called from the button's destructor
			{
				if(w)if(b)if(b->var.ptr["fbo"]){delete(((FBO*)b->var.ptr["fbo"]));}
			};
		w.label["detail"] = Gui::Label("Detail : 0.01",20, 20, 100, 20);
		w.slider["detail"] = Gui::Slider(0.01, 1, 0.01, 120, 20, 100, 20);
		w.slider["detail"].callback_pressed = [](Gui::Window *w, Gui::Button* b, int i) // call before drawing the first time
		{
			if (w)if (b) w->label["detail"].text = Gui::String("Detail : ") + ((Gui::Slider*)b)->val;
		};
		w.label["lod.ratio"] = Gui::Label("Ratio : 0.5", 20, 50, 100, 20);
		w.slider["lod.ratio"] = Gui::Slider(0.01, 2, 0.5, 120, 50, 100, 20);
		w.slider["lod.ratio"].callback_pressed = [](Gui::Window *w, Gui::Button* b, int i) // call before drawing the first time
		{
			if (w)if (b) w->label["lod.ratio"].text = Gui::String("Ratio : ") + ((Gui::Slider*)b)->val;
		};
	}
}
////////////////////////////////////////////////////////////////////////////////
void draw_gui()
{
	gui.draw();
}
////////////////////////////////////////////////////////////////////////////////
void exit_gui()
{
	gui.exit();
}
////////////////////////////////////////////////////////////////////////////////
// Glut Callbacks
void KeyDown1Static(int key, int x, int y)           { gui.keyb.key[ key&255 ] =true;  }
void KeyDown2Static(unsigned char key, int x, int y) { gui.keyb.key[ key&255 ] =true;  }
void KeyUp1Static(int key, int x, int y)             { gui.keyb.key[ key&255 ] =false; }
void KeyUp2Static(unsigned char key, int x, int y)   { gui.keyb.key[ key&255 ] =false; }
void MouseMotionStatic (int x,int y){	gui.mouse.x = x;gui.mouse.y = y;}
void MouseButtonStatic(int button_index, int state, int x, int y)
{
	if(button_index<=2) gui.mouse.button[button_index] =  ( state == GLUT_DOWN ) ? true : false;
	if(button_index>=3) gui.mouse.wheel_update+= button_index == 3 ? -1 : 1;
	MouseMotionStatic (x,y);	
}
void OnWindowClose( void ) { exit_gui(); }
////////////////////////////////////////////////////////////////////////////////

void disp(void);
void reshape(int x, int y);
void keyb(unsigned char key, int x, int y);

int main(int argc, char **argv)
{
	
	glutInit(&argc, argv);
	glutInitWindowSize(1024, 768);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutCreateWindow("PLANET LOD - BY SVEN FORSTMANN in 2016");

	glutIdleFunc(disp);
	glutDisplayFunc(disp);
	glutReshapeFunc(reshape);

	//glutSpecialFunc(&KeyDown1Static);
	//glutSpecialUpFunc(&KeyUp1Static);
	glutKeyboardFunc(KeyDown2Static);
	glutKeyboardUpFunc(KeyUp2Static);
	glutMotionFunc(MouseMotionStatic);
	glutPassiveMotionFunc(MouseMotionStatic);
	glutMouseFunc (MouseButtonStatic);
	glutCloseFunc (OnWindowClose);

	glewInit();
	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	init_gui();

	glutMainLoop();

	return 0;
}


void disp(void)
{
	glClearColor(1,1,1,0);
	glClear(GL_COLOR_BUFFER_BIT);
	
	draw_gui();
	
	glutSwapBuffers();
}

void reshape(int x, int y)
{
	glViewport(0, 0, x, y);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, x, y, 0, -1, 1);
	gui.screen_resolution_x=x;
	gui.screen_resolution_y=y;
}






