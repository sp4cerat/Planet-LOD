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
matrix44d modelviewprojection;
////////////////////////////////////////////////////////////////////////////////
struct World
{
	static int tris_rendered;

	static void draw_triangle(	vec3d p1, vec3d p2, vec3d p3 )
	{
		glBegin(GL_TRIANGLES);
		glVertex3f(p1.x, p1.y, p1.z);
		glVertex3f(p2.x, p2.y, p2.z);
		glVertex3f(p3.x, p3.y, p3.z);
		glEnd();
		tris_rendered++;
	}

	static void draw_recursive(
		vec3d p1, vec3d p2, vec3d p3,
		vec3d center, double size = 1)
	{
		double ratio_size = size * gui.screen[0].slider["lod.ratio"].val; // default : 1
		double minsize = gui.screen[0].slider["detail"].val;    // default : 0.01
		vec3d edge_center[3] = { (p1 + p2) / 2, (p2 + p3) / 2, (p3 + p1) / 2 };
		bool edge_test[3]; double angle[3];

		loopi(0, 3)
		{ 
			vec3d d = center + edge_center[i];
			edge_test[i] = d.len() > ratio_size; 
			double dot = edge_center[i].dot(d.norm());
			angle[i] = acos(clamp(dot, -1, 1));
		}

		// culling
		if (max(angle[0], max(angle[1], angle[2])) < M_PI / 2 - 0.2) return;

		// draw
		if ((edge_test[0] && edge_test[1] && edge_test[2]) || size < minsize)
		{
			draw_triangle(p1, p2, p3);
			return;
		}

		// Recurse
		vec3d p[6] = { p1, p2, p3, edge_center[0], edge_center[1], edge_center[2] };
		int idx[12] = { 0, 3, 5,    5, 3, 4,    3, 1, 4,    5, 4, 2 };
		bool valid[4] = { 1, 1, 1, 1 };

		if (edge_test[0]){ p[3] = p1; valid[0] = 0; } // skip triangle 0 ?
		if (edge_test[1]){ p[4] = p2; valid[2] = 0; } // skip triangle 2 ?
		if (edge_test[2]){ p[5] = p3; valid[3] = 0; } // skip triangle 3 ?

		loopi(0, 4) if (valid[i])
		{
			int i1 = idx[3 * i + 0], i2 = idx[3 * i + 1], i3 = idx[3 * i + 2];
			draw_recursive(	p[i1].norm(), p[i2].norm(),	p[i3].norm(),center,size/2 );
		}		
	}
	static void draw(vec3d center)
	{
		// create icosahedron
		double t = (1.0 + sqrt(5.0)) / 2.0;

		std::vector<vec3d> p({ 
			{ -1, t, 0 }, { 1, t, 0 }, { -1, -t, 0 }, { 1, -t, 0 },
			{ 0, -1, t }, { 0, 1, t }, { 0, -1, -t }, { 0, 1, -t },
			{ t, 0, -1 }, { t, 0, 1 }, { -t, 0, -1 }, { -t, 0, 1 },
		});
		std::vector<int> idx({ 
			0, 11, 5, 0, 5, 1, 0, 1, 7, 0, 7, 10, 0, 10, 11,
			1, 5, 9, 5, 11, 4, 11, 10, 2, 10, 7, 6, 10, 7, 6, 7, 1, 8,
			3, 9, 4, 3, 4, 2, 3, 2, 6, 3, 6, 8, 3, 8, 9,
			4, 9, 5, 2, 4, 11, 6, 2, 10, 8, 6, 7, 9, 8, 1
		});

		tris_rendered = 0;

		loopi(0, idx.size() / 3)
		{
			vec3d p3[3] = { p[idx[i * 3 + 0]].norm() ,	// triangle point 1
							p[idx[i * 3 + 1]].norm() ,	// triangle point 2
							p[idx[i * 3 + 2]].norm() };	// triangle point 3
			draw_recursive(p3[0], p3[1], p3[2],	center);
		}
		gui.screen[0].label["stats"].text = Gui::String("Triangles: ") + tris_rendered;
	}
};
int World::tris_rendered; 

////////////////////////////////////////////////////////////////////////////////
// render callback function
void render_callback(Gui::Window *window, Gui::Button* control, int index)
{
	if ((!control) || (!window) || window->get_toggled()) return;

	Gui::Window &w = *((Gui::Window*) window);
	Gui::Button &b = w.button["canvas"];

	// resize button to window client area
	b.x = w.pad_left;
	b.y = w.pad_up;
	b.sx = w.sx - w.pad_left - w.pad_right;
	b.sy = w.sy - w.pad_up - w.pad_down;

	void *fb = b.var.ptr["fbo"]; if (!fb) return;
	FBO &fbo = *(FBO*)fb;

	if (b.sx != fbo.width || b.sy != fbo.height) // resize FBO ?
	{
		fbo.clear();
		fbo.init(b.sx, b.sy);
		b.skin.tex_normal = b.skin.tex_hover = b.skin.tex_selected = fbo.color_tex;
	}

	quaternion q(b.var.vec4d["rotation"]);
	if (gui.mouse.button[0] & b.pressed)				// rotate by left mouse
	{
		quaternion qx, qy;
		qx.set_rotate_y((double)gui.mouse.dx / 100);
		qy.set_rotate_x(-(double)gui.mouse.dy / 100);
		q = qy*qx*q;
		b.var.vec4d["rotation"] = vec4d(q.x, q.y, q.z, q.w);
	}

	// center of detail

	double z = b.var.number["zoom"];
	vec4d pos = b.var.vec4d["position"];

	vec3d center = vec3d(-pos.x, -pos.y, -pos.z).norm();

	static uint t0 = timeGetTime(), t1 = 0; static double dt;
	t1 = t0; t0 = timeGetTime(); dt = dt*0.9+0.1*(t0 - t1);

	quaternion qi = q; qi.invert();
	vec3d dir_z = matrix44d(qi).z_component().norm();
	vec3d dir_x = matrix44d(qi).x_component().norm();
	vec3d dir_y = matrix44d(qi).y_component().norm();
	double speed = b.var.number["speed"];

	if (b.hover)										// zoom by wheel
	{
		speed = speed * (1 - 0.1* double(gui.mouse.wheel));
		b.var.number["speed"] = speed;		
		gui.mouse.wheel = 0;
	}

	double planetradius = 6371; // radius in km

	speed = speed * (dt / 100);
	if (gui.keyb.key[' ']) speed *= 10;

	static double km = 0;
	double kmh = speed * planetradius / (dt / (1000 * 3600));
	w.label["Speed"].text = str("Speed : %lg km/h ( %lg km total )", kmh,km);
	
	if (gui.keyb.key['w']){ pos = pos + vec4d(dir_z.x, dir_z.y, dir_z.z, 0)*speed; km += speed * planetradius; }
	if (gui.keyb.key['s']){ pos = pos - vec4d(dir_z.x, dir_z.y, dir_z.z, 0)*speed; km += speed * planetradius; }
	if (gui.keyb.key['a']){ pos = pos + vec4d(dir_x.x, dir_x.y, dir_x.z, 0)*speed; km += speed * planetradius; }
	if (gui.keyb.key['d']){ pos = pos - vec4d(dir_x.x, dir_x.y, dir_x.z, 0)*speed; km += speed * planetradius; }
	if (gui.keyb.key['q']){ pos = pos - vec4d(dir_y.x, dir_y.y, dir_y.z, 0)*speed; km += speed * planetradius; }
	if (gui.keyb.key['e']){ pos = pos + vec4d(dir_y.x, dir_y.y, dir_y.z, 0)*speed; km += speed * planetradius; }

	if (gui.mouse.button[1] && (b.pressed || b.hover))	// move by middle button
	{
		pos = pos + vec4d(dir_x.x, dir_x.y, dir_x.z, 0)*speed*gui.mouse.dx*5.0;
		pos = pos + vec4d(dir_y.x, dir_y.y, dir_y.z, 0)*speed*gui.mouse.dy*5.0;
	}
	b.var.vec4d["position"] = pos;

	w.label["Position"].text = str("Position xyz: %lg km %lg km %lg km", pos.x* planetradius, pos.y* planetradius, pos.z* planetradius);

	// render to fbo
	fbo.enable();
	glClearColor(1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
	gluPerspective(z, (GLfloat)b.sx / (GLfloat)b.sy, 0.0000002, 64 );
	glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();

	glMultMatrixf(&matrix44(q).m[0][0]);	// apply quaternion rotation
	glTranslatef(pos.x, pos.y, pos.z);		// apply movement

	glColor4f(0, 0, 0, 1);
	glEnable(GL_DEPTH_TEST);


	////////////////////
	//
	//  Draw our World
	//
	if (!gui.screen[0].checkbox["solid"].checked)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	if (pos.len() < 1)pos.norm();

	World::draw(vec3d(pos.x, pos.y, pos.z));

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	////////////////////
	//
	// Draw Bounding Box
	//
	glutWireCube(2);

	glDisable(GL_DEPTH_TEST);
	glPopMatrix(); glMatrixMode(GL_PROJECTION); glPopMatrix();
	fbo.disable();

	// FPS
	{
		static uint t = timeGetTime(), f = 0;
		int tt = timeGetTime(); f++;
		if (tt - t >= 1000)
		{
			gui.screen[0].label["fps"].text = Gui::String("Framerate: ") +  f + " fps";
			f = 0; t = tt;
		}
	};
};
////////////////////////////////////////////////////////////////////////////////
void init_gui()
{
	gui.init( Gui::Flags::CONTEXT_MENU | Gui::Flags::CUSTOM_MOUSE ,
				"../data/gui_global.txt" , 
				"../data/gui_skin.txt");

	Gui::Window &w = gui.screen[0];
	w.button["canvas"]=Gui::Button("",20,250);
	w.button["canvas"].skin=Skin("");
	w.button["canvas"].var.number["speed"] = 0.01;
	w.button["canvas"].var.vec4d["position"] = vec4d(0, 0, 2.5, 0); // vec4
	w.button["canvas"].var.vec4d["rotation"]=vec4d(1,0,0,0); // quaternion
	w.button["canvas"].var.number["zoom"]=80; // fov
	w.button["canvas"].var.ptr["fbo"]=0;
	w.button["canvas"].callback_all=render_callback;
	w.button["canvas"].callback_init=[](Gui::Window *w,Gui::Button* b,int i) // call before drawing the first time
		{
			if(w)if(b){b->var.ptr["fbo"]=new FBO(100,100);} // 100,100 is just for initialization; will be resized
		};
	w.button["canvas"].callback_exit=[](Gui::Window *w,Gui::Button* b,int i) // called from the button's destructor
		{
			if(w)if(b)if(b->var.ptr["fbo"]){delete(((FBO*)b->var.ptr["fbo"]));}
		};
	int y = 20;
	w.label["detail"] = Gui::Label("Detail : 0.000001",20, y, 100, 20);
	w.slider["detail"] = Gui::Slider(0.000001, 1, 0.000001, 120, y, 100, 20);
	w.slider["detail"].callback_pressed = [](Gui::Window *w, Gui::Button* b, int i) // call before drawing the first time
	{
		if (w)if (b) w->label["detail"].text = Gui::String("Detail : ") + ((Gui::Slider*)b)->val;
	};
	y += 30;
	w.label["lod.ratio"] = Gui::Label("Ratio : 3", 20, y, 100, 20);
	w.slider["lod.ratio"] = Gui::Slider(0.01, 8, 3, 120, y, 100, 20);
	w.slider["lod.ratio"].callback_pressed = [](Gui::Window *w, Gui::Button* b, int i) // call before drawing the first time
	{
		if (w)if (b) w->label["lod.ratio"].text = Gui::String("Ratio : ") + ((Gui::Slider*)b)->val;
	};
	y += 30;
	w.label["Solid"] = Gui::Label("Solid", 20, y, 100, 20);
	w.checkbox["solid"] = Gui::CheckBox("", 0, 120, y, 16, 16);
	y += 30;	
	w.label["Position"] = Gui::Label("Position: ", 20, y, 400, 20);
	y += 30;
	w.label["Speed"] = Gui::Label("Speed: 0.01 ", 20, y, 400, 20);
	y += 30;
	w.label["stats"] = Gui::Label("default", 20, y, 200, 20);
	y += 30;
	w.label["fps"] = Gui::Label("10", 20, y, 200, 20);
	y += 30;
	w.label["help"] = Gui::Label("Help:\nw,s,a,d,q,e : move around\nspace : speed up\nmouse : rotate", 20, y, 200, 200);
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






