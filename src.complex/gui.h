
#include "core.h"
#include "Bmp.h"
#include "ogl.h"
#include "drawtext/drawtext.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <memory>
#include <utility> //pair

template<class T>
class ControlList // kind of like map but with selectable order
{	
	struct Tuple
	{
		std::string first;std::shared_ptr<T> second;

		Tuple(){};
		Tuple( const std::string f, const T& s){first=f;second=std::make_shared<T>(s);};
	};
	std::vector<Tuple> list;

	public:

	ControlList(){};

	ControlList(const ControlList &cl)
	{
		list.clear();
		loopi(0,cl.list.size())
		{
			T t(*cl.list[i].second);
			push(cl.list[i].first,t);
		}
	}
    ControlList& operator=(ControlList cl) {
		list.clear();
		loopi(0,cl.list.size())
		{
			T t(*cl.list[i].second);
			push(cl.list[i].first,t);
		}
        return *this;
    }

	void clear(){list.clear();};
	inline void push(std::string s,T &t)
	{
		list.push_back(Tuple(s,t));
	}
	void push_back(T& t)
	{
		push(str("%d",list.size()),t);
	}
	int add(T& t) {push_back(t);return list.size()-1;};
	int get_index(std::string s)
	{
		loopi(0,list.size()) if(s==list[i].first) return i;
		return -1;
	};
	inline T& operator [](std::string s)
	{
		loopi(0,list.size()) if(s==list[i].first) return *list[i].second;
		list.push_back(Tuple(s,T()));
		return *list[list.size()-1].second;
	}
	inline T& operator [](int i)
	{
		while(i>=list.size())push_back(T());
		return *list[i].second;
	}
	void erase(std::string s)
	{
		loopi(0,list.size()) if(s==list[i].first) { list.erase(list.begin()+i);};
	}
	void erase(int i)
	{
		list.erase(list.begin()+i);
	}
	int size() { return list.size(); }

	void to_front(std::string s)
	{
		int i=get_index(s);
		if(i<0) return;
		Tuple t=list[i];
		list.erase(list.begin()+i);
		list.insert(list.begin(),t);
	}
	void to_front(int i=0 )
	{
		Tuple t=list[i];
		list.erase(list.begin()+i);
		list.insert(list.begin(),t);
	}
};

Shader shader_scaled;//("../shader/SkinScaled");
Shader shader_inner;
Shader shader_repeated;

class Skin // class to draw a textured quad
{
public:

	enum SCALETYPE{ NODRAW=0,SCALE=1,REPEAT=2,SCALEINNER=3 };

	int tex_normal;
	int tex_hover;
	int tex_selected;
	int scale_type;
	int px[4],py[4]; // define scale geometry

	static int instances;

	void init(){tex_normal=tex_hover=tex_selected=0;loopi(0,4)px[i]=py[i]=0;scale_type=NODRAW;};

	Skin(){init();}
	Skin(std::string file_normal,std::string file_hover="",std::string file_selected="",
		int scaletype=SCALE, int x1=0,int x2=0,int y1=0,int y2=0)
	{
		init();

		std::string dim_file= 	(file_normal!="") ? file_normal : 
								(file_hover!="") ?	file_hover : file_selected;		
		
		scale_type=scaletype;
		
		if(dim_file=="") return;

		Bmp bmp(dim_file.c_str());
		px[0]=0;px[1]=x1;px[2]=x2;px[3]=bmp.width;
		py[0]=0;py[1]=y1;py[2]=y2;py[3]=bmp.height;
		if(file_normal  !="") tex_normal  =ogl_tex_bmp(file_normal.c_str());
		if(file_hover   !="") tex_hover   =ogl_tex_bmp(file_hover.c_str());
		if(file_selected!="") tex_selected=ogl_tex_bmp(file_selected.c_str());
	}

	inline void draw_scaled(float x,float y,float sx,float sy,float xmin,float xmax,float ymin,float ymax)
	{
		shader_scaled.begin();
		shader_scaled.setUniform1i("tex_skin",0);
		ogl_drawquad(x+xmin,y+ymin,x+xmax,y+ymax , xmin/sx,ymin/sy,xmax/sx,ymax/sy );
		shader_scaled.end();
	}

	inline void draw_repeated(float x,float y,float sx,float sy,float xmin,float xmax,float ymin,float ymax)
	{
		float px1=px[1],py1=py[1];
		if(sx<px[1]+px[3]-px[2]) px1=sx/2;
		if(sy<py[1]+py[3]-py[2]) py1=sy/2;

		//static Shader shader("../shader/SkinRepeated");
		shader_repeated.begin();
		shader_repeated.setUniform1i("tex_skin",0);
		shader_repeated.setUniform4f("px",px[0],px1,px[2],px[3]);
		shader_repeated.setUniform4f("py",py[0],py1,py[2],py[3]);
		shader_repeated.setUniform4f("pos",sx,sy,0,0);
		
		ogl_drawquad(x+xmin,y+ymin,x+xmax,y+ymax , xmin,ymin,xmax,ymax );
		shader_repeated.end();
	}
		
	inline void draw_scaledinner(float x,float y,float sx,float sy,float xmin,float xmax,float ymin,float ymax)
	{
		//static Shader shader("../shader/SkinScaledInner");
		shader_inner.begin();
		shader_inner.setUniform1i("tex_skin",0);
		shader_inner.setUniform4f("px",px[0],px[1],px[2],px[3]);
		shader_inner.setUniform4f("py",py[0],py[1],py[2],py[3]);
		shader_inner.setUniform4f("pos",sx,sy,0,0);
		//ogl_drawquad(x+xmin,y,x+xmax,y+sy , xmin,0,xmax,sy );
		ogl_drawquad(x+xmin,y+ymin,x+xmax,y+ymax , xmin,ymin,xmax,ymax );
		shader_inner.end();
	}

	inline void draw(float x,float y,float sx,float sy,float xmin=0,float xmax=-1,float ymin=0,float ymax=-1)
	{
		if(xmin==xmax || ymin==ymax) return;
		if(xmax<0) xmax= sx;if(ymax<0) ymax= sy;
		
		if (scale_type==SCALE) draw_scaled(x, y, sx, sy,xmin,xmax,ymin,ymax);
		if (scale_type==REPEAT) draw_repeated(x, y, sx, sy,xmin,xmax,ymin,ymax);
		if (scale_type==SCALEINNER) draw_scaledinner(x, y, sx, sy,xmin,xmax,ymin,ymax);
	}
};

class Gui
{
	public:

	Gui(){}

	class String;

	class MyVar
	{
		public:
		MyVar(){ init(); };
		~MyVar(){ exit(); };
		virtual void init(){printf("base init\n");};
		virtual void exit(){printf("base exit\n");};
	};
	struct Var
	{
		std::map<String, matrix44>		mat4;
		std::map<String, matrix44>		mat4d;
		std::map<String, vec4f>			vec4;
		std::map<String, vec4d>			vec4d;
		std::map<String, double>		number;
		std::map<String,String>			string;
		std::map<String,Skin>			skin;
		std::map<String,void*>			ptr;

		void load(char* name)
		{
			std::cout << "load config file " << name << std::endl;
			FILE *f1=fopenx(name,"r");
			char line[1000];
			int a1,a2,a3;vec4f v;float f;
			char* s1=0;char* s2=0;char* s3=0;

			while(fgets(line,1000,f1)!=NULL) 
			if(splitstr(line,&s1,&s2,&s3)==3)
			{
				bool cmdok=false;
				ifeq(s1,"string") {cmdok=true;string [s2]=s3;}
				ifeq(s1,"number") {cmdok=true;number [s2]=atof(s3);}
				ifeq(s1,"vec4f")  {cmdok=true;if(sscanf(s3,"%f %f %f %f",&v.x,&v.y,&v.z,&v.w)==4) vec4[s2]=v;}
				if(cmdok) printf("%s %s = %s\n",s1,s2,s3);
			}
			fclose(f1);
		}
	};
	static Var global; // global variables
	
	struct dtx_font *font; 
	static float screen_resolution_x,screen_resolution_y;
	int active_screen;

	enum Flags{ CONTEXT_MENU=1, CUSTOM_MOUSE=2 }; // passed to init function
	
	enum TYPE { NONE=0 , WINDOW=1 , BUTTON=2 , COMBO=3 , LABEL=4 , TEXTEDIT=5, CHECKBOX=6 , 
		SLIDER=7 , SCROLLBAR=8 , MENU=9 , TAB=10 , RADIO=11 };

	enum Align { CENTER=0,LEFT=1,RIGHT=2 };

	int flags;

	class Window; class Button;

	private:

	struct CallParams
	{ 
		CallParams( void (*callback_ptr)(Window *,Button* ,int ) , 
			Window* win=0, Button* ctrl=0,int id=0)
			{ func=callback_ptr;window=win;control=ctrl;index=id;}

		void (*func)(Window *window,Button* control,int index); 
		Window *window;Button* control;int index;
	};
	static std::vector<CallParams> call_list;

	class Active // active control
	{
		public:
		
		bool active;
		int type;
		Button *ptr; Window* w_ptr;
		int window_index;
		int control_index;

		void set(Button* p,int t,Window* win=0,int index=0)
		{
			if(ptr!=0)return;
			ptr=p;type=t;w_ptr=win;control_index=index;
			//if(mouse.button[0])printf("type %d index %d\n",type,index);
		}

		Active(){ type=NONE;active=0; }
	};
	static Active active_control;

	public:

	struct Mouse
	{
		int x,y,x2,y2,dx,dy,button[3],button2[3],button3[3],button_pressed[3],button_released[3],wheel,wheel_update;
		
		void init(){x=y=x2=y2=dx=dy=wheel=0;loopi(0,3)button[i]=button2[i]=button3[i]=button_pressed[i]=button_released[i]=0;};
		void update()
		{
			dx=x-x2;dy=y-y2;x2=x;y2=y;
			loopi(0,3) button_pressed [i]= (button [i]&&!button2[i]) ? 1 : 0;
			loopi(0,3) button_released[i]= (button2[i]&&!button [i]) ? 1 : 0;
			loopi(0,3) button3[i]=button2[i];
			loopi(0,3) button2[i]=button[i];
			wheel=wheel_update;wheel_update=0;
		}
	};
	struct Keyb
	{
		int key[512],key2[512],key_pressed[512],key_released[512];
		
		void init(){loopi(0,512)key[i]=key2[i]=key_pressed[i]=key_released[i]=0;}
		void update()
		{
			loopi(0,512) key_pressed [i]= (key [i]&&!key2[i]) ? 1 : 0;
			loopi(0,512) key_released[i]= (key2[i]&&!key [i]) ? 1 : 0;
			loopi(0,512) key2[i]=key[i];
		}
	};
	static Mouse mouse;
	static Keyb keyb;

	class Rect;
	static Rect clip_rect;

	class Window;

	class String // std::string with utf8 extension
	{
		void my_replace(const std::string& oldStr, const std::string& newStr){
		  size_t pos = 0;
		  while((pos = s.find(oldStr, pos)) != std::string::npos){
			 s.replace(pos, oldStr.length(), newStr);
			 pos += newStr.length();
		  }
		}

		public:

		std::string s;
		void set_text(wchar_t *s)
		{
			int wlen= lstrlenW( s ); 
			int len = WideCharToMultiByte( CP_UTF8,0,s,wlen,0,0,NULL, NULL );
			std::vector<char> result; result.resize(len+10,0);
			WideCharToMultiByte( CP_UTF8,0,s,wlen,&result[0], len,NULL, NULL );
			this->s=&result[0];
		}

		String(std::string s=""){ this->s=std::string(s); }
		String(wchar_t *c){ set_text(c); }
		String(const char *c){ this->s=std::string(c); }
		//String(int i,char* format=0){ if(format)s=str(format,i);else s=str("%d",i); }
		String(double f,char* format=0){ if(format)s=str(format,f);else {s= (f==floor(f)) ? str("%d",int(f)) : str("%lg",f);} }

		inline String& operator=(char* c){ s=c;return *this;}
		inline char& operator [](int i){return s[i];}
		inline const char* c_str(){return s.c_str();}
		inline void operator +=(String& st) { s.append(st.c_str());};
		inline String operator+ (const char* st){ s.append(st);return *this;};
		inline bool operator >(const String st) const { return st.s>s;};
		inline bool operator <(const String st) const { return st.s<s;};
		inline bool operator==(const String& b){ if(b.s.size()!=s.size())return 0; return (s.compare(b.s) != 0) ? 0 : 1; }
		inline bool operator!=(const String& b){ if(b.s.size()==s.size())return 0; return (s.compare(b.s) != 0) ? 1 : 0; }
		inline String operator+(const String& b) {String st=s;st.s.append(b.s);return st;}

		int find(char c,int pos=0){ std::size_t sz=s.find(c,pos); return ( sz == std::string::npos) ? -1 : sz ;};
		//int find(char *c,int pos=0){ std::size_t sz=s.find(c,pos,strlen(c)); return ( sz == std::string::npos) ? -1 : sz ;};
		int find(String c,int pos=-1){ std::size_t sz=pos < 0 ? s.find(c.s.c_str()):s.find(c.s.c_str(),pos); return ( sz == std::string::npos) ? -1 : sz ;};

		int find_first_of(char c,int pos=0){ std::size_t sz=s.find_first_of(c,pos); return ( sz == std::string::npos) ? -1 : sz ;};
		//int find_first_of(char *c,int pos=0){ std::size_t sz=s.find_first_of(c,pos,strlen(c)); return ( sz == std::string::npos) ? -1 : sz ;};
		int find_first_of(String c,int pos=-1){ std::size_t sz=pos < 0 ? s.find_first_of(c.s.c_str()):s.find_first_of(c.s.c_str(),pos); return ( sz == std::string::npos) ? -1 : sz ;};
		
		int find_last_of(char c,int pos=0){ std::size_t sz=s.find_last_of(c,pos); return ( sz == std::string::npos) ? -1 : sz ;};
		//int find_last_of(char *c,int pos=0){ std::size_t sz=s.find_last_of(c,pos,strlen(c)); return ( sz == std::string::npos) ? -1 : sz ;};
		int find_last_of(String c,int pos=-1){ std::size_t sz=pos < 0 ? s.find_last_of(c.s.c_str()):s.find_last_of(c.s.c_str(),pos); return ( sz == std::string::npos) ? -1 : sz ;};
		
		inline int length(){return s.length();};
		inline int size(){return s.size();};
		inline void resize(size_t t){ s.resize(t);};
		inline void erase(size_t t){s.erase(s.begin()+t);};
		
		void push_back(char c){s.push_back(c);}
		void replace(String c,String d){ my_replace(c.s,d.s);};
		void prln(){printf(s.c_str());printf("\n");};
		void pr(){printf(s.c_str());};
	};
	

	class Rect 
	{
		public : 

		float x,y,sx,sy; 

		Rect(float xx=0,float yy=0,float sxx=0,float syy=0){x=xx;y=yy;sx=sxx;sy=syy;}; 

		void extend_rect(Rect &r) 
		{
			r.sx=max(r.sx,x+sx); 
			r.sy=max(r.sy,y+sy);
		}
		bool is_clipped(int ox,int oy)//, Rect& clip_rect)
		{
			if(x+ox+sx < clip_rect.x) return true; 
			if(x+ox    > clip_rect.x+clip_rect.sx) return true; 
			if(y+oy+sy < clip_rect.y) return true; 
			if(y+oy    > clip_rect.y+clip_rect.sy) return true; 
			return false;
		}
		void set_rect(float xx,float yy,float sxx,float syy){x=xx;y=yy;sx=sxx;sy=syy;}

		void resize(float x,float y){ sx=x;sy=y; }
		void move(float px,float py,float sizex=-1,float sizey=-1){  x=px;y=py; if(sizex>=0) resize(sizex,sizey); }
	
		void crop(const Rect &r)
		{
			float xx=max(x,r.x);
			float yy=max(y,r.y);
			float sxx=max(min(x+sx,r.x+r.sx)-xx,0);
			float syy=max(min(y+sy,r.y+r.sy)-yy,0);
			set_rect(xx,yy,sxx,syy);
		}
	};

	class Label : public Rect
	{
		public:

		Align align;

		bool pressed,hover;

		float pad_left,pad_right,pad_up,pad_down;
		float minsx,minsy;String text; TYPE type; 
		vec4f textcolor,textcolor_hover,textcolor_pressed;
		int textsize;

		Window* parent; int parent_index;

		Var var; // window variables

		void init()
		{
			align=Align::LEFT;
			type=LABEL;
			x=sx=y=sy=minsx,minsy=0;
			pressed=hover=0;
			pad_left	=global.number["button_pad_left"];
			pad_up		=global.number["button_pad_up"];
			pad_right	=global.number["button_pad_right"];
			pad_down	=global.number["button_pad_down"];
			textcolor=global.vec4["font_color"];
			textcolor_hover=global.vec4["font_color_hover"];
			textcolor_pressed=global.vec4["font_color_pressed"];
			textsize=global.number["font_size"];
			parent_index=0;
			parent=0;
		};

		Label(){init();};
		Label(String buttontext,int x=0,int y=0,
			int width=global.number["button_size_x"],  // default
			int height=global.number["button_size_y"],Align alignment=LEFT) // default
		{
			init();this->x=x; this->y=y; minsx=sx=width; minsy=sy=height; text=buttontext; align=alignment;
		}
		inline void draw(float ox=0,float oy=0,bool cliptest=1)
		{
			if(text.size()==0) return;
			if(cliptest && Rect::is_clipped(ox,oy)) return;

			float align_x=0; // LEFT
			if(align==CENTER)	align_x=(sx-dtx_string_width(text.c_str()))/2-pad_left ;
			if(align==RIGHT)	align_x=sx-dtx_string_width(text.c_str())-pad_right-pad_left ;

			glPushMatrix();
			glTranslatef(int(ox+x+pad_left+align_x), 
					     int(oy+y+pad_up) , 0);

			vec4f color=	pressed ? textcolor_pressed : 
							hover   ? textcolor_hover : textcolor;

			glColor4f(color.x,color.y,color.z,color.w); 
			
			struct dtx_box b;
			b.x=0;b.y=0;b.width=sx-pad_left-pad_right;b.height=sy-pad_up-pad_down;

			dtx_string(text.c_str(),&b);	
			
			glPopMatrix();
		};
		inline bool mouseinside(float ox=0,float oy=0)
		{
			if( mouse.x>=ox+x ) if( mouse.x<=ox+x+sx ) 
			if( mouse.y>=oy+y ) if( mouse.y<=oy+y+sy ) return 1;			
			if( mouse.x2>=ox+x ) if( mouse.x2<=ox+x+sx ) 
			if( mouse.y2>=oy+y ) if( mouse.y2<=oy+y+sy ) return 1;
			return 0;
		}
	};

	class Button : public Label
	{  
		public:

		Skin skin;		// r/w
		int flags;		// r/w
		bool active;	// dont overwrite!

		bool call_init;	// call init callback ?

		enum Flags { NORMAL=0, DEACTIVATED=1 };

		void init(){ type=BUTTON;skin=global.skin["button"];active=0; 		
			callback_hover=callback_pressed=callback_all=callback_exit=callback_init=0;parent=0;
			flags=NORMAL;call_init=1;
		};
		
		Button():Label(){ init() ; align=CENTER; };
		~Button() { if(callback_exit) callback_exit(parent,this,parent_index); };

		Button(String buttontext,int x=0,int y=0,
			int width=global.number["button_size_x"],  // default
			int height=global.number["button_size_y"],
			Align alignment=CENTER,int flags=NORMAL,
			void (*callback_pressed)(Window *,Button* ,int )=0,
			Skin skin=global.skin["button"]) // default
			:Label(buttontext,x,y,width,height,alignment)
		{
			init(); 
			this->skin=skin;
			this->callback_pressed=callback_pressed;
			this->align=alignment;
			this->flags=flags;
		}

		void (*callback_hover)(Window *window,Button* control,int index);
		void (*callback_pressed)(Window *window,Button* control,int index);
		void (*callback_all)(Window *window,Button* control,int index);
		void (*callback_init)(Window *window,Button* control,int index);
		void (*callback_exit)(Window *window,Button* control,int index);

		void handle_callbacks(Window* w=0,int index=0)
		{
			if(flags==DEACTIVATED) return;
			parent=w;parent_index=index;
			if(call_init){ if(callback_init) call_list.push_back(CallParams(callback_init,parent,this,index));call_init=0;}
			if(active) { active_control.active=1;active_control.set(this,type,parent,index); }
			if(callback_all) call_list.push_back(CallParams(callback_all,parent,this,index));
		}
		
		bool find_active(int ox=0,int oy=0,Window* parent=0,int index=0)
		{
			if(flags==DEACTIVATED) return false;;

			if(Rect::is_clipped(ox,oy)) return false;
			if(mouseinside(ox,oy))
			{
				active_control.set(this,type,parent,index);
				return true;
			}
			return false;
		}

		void set_active(bool b)
		{
			if(flags==DEACTIVATED) return;
			active_control.active=b;
			active=b;
		}

		void draw(float ox=0,float oy=0, bool manage=true, float xmin=0, float xmax=-1, float ymin=0, float ymax=-1)
		{
			if(manage)if(Rect::is_clipped(ox,oy)) return;

			if(manage && active && flags!=DEACTIVATED)
			{
				pressed= mouseinside(ox,oy) ? 1:0;
				if(!mouse.button[0]) { pressed=false; set_active(0);}			
			}
			
			int			tex=skin.tex_normal;
			if(hover)	tex=skin.tex_hover;
			if(pressed)	tex=skin.tex_selected;

			ogl_bind(0,tex);  
			if(tex>0) skin.draw(x+ox,y+oy,sx,sy,xmin,xmax,ymin,ymax);
			ogl_bind(0,0);

			Label::draw(ox,oy,manage);		
			
			hover=0;//if(manage)pressed&=active;
		};

	};

	class Combo: public Button
	{
		public:

		Button select,background; bool selection_changed;

		void (*callback_selected)(Window *window,Button* control,int index);

		std::vector<Button> item; int selected; 

		void init(){ align=select.align=background.align=LEFT;selected=0;type=COMBO;selection_changed=0;callback_selected=0; };

		Combo():Button(){ init();}
		Combo (int pos_x,int pos_y,
			int width=global.number["button_size_x"],  // default
			int height=global.number["button_size_y"]) // default)
			:Button("",pos_x,pos_y,width,height,LEFT)
		{
			init();
			skin=global.skin["combo"];
			select.skin=global.skin["combo_select"];
			background.skin=global.skin["combo_background"];
		}
		void set_selected(int a)
		{
			selected = a;
			selection_changed = 1;
			text = item[a].text;
		}
		void add_item(String s)
		{
			Button b(s,x,y,sx,sy,LEFT);
			b.skin=select.skin;
			item.push_back( b );
			text=item[0].text;
		};

		void draw(float ox=0,float oy=0)
		{
			if(Rect::is_clipped(ox,oy)) return;

			if(active) hover=0;

			pressed=active;

			Button::draw(ox,oy,0);

			if(active) // open?
			{
				glDisable(GL_SCISSOR_TEST);

				background.set_rect(x,y+sy,sx,sy*item.size());
				background.draw(ox,oy,0);

				loopi(0,item.size())
				{
					item[i].set_rect(x,y+(1+i)*sy,sx,sy);
					bool inside=item[i].mouseinside(ox,oy);
					item[i].hover=inside;
					item[i].pressed= (i==selected) ? 1 : 0;
					item[i].draw(ox,oy,0);

					if(mouse.button_released[0] && inside)
					{
						text=item[i].text;
						selected=i;
						item[i].pressed=0;
						selection_changed=1;
					}
				}
				if(mouse.button_released[0]) 
					if(!mouseinside(ox,oy)) 
						set_active(0);

				glEnable(GL_SCISSOR_TEST);
			}
		}
		void handle_callbacks(Window* w=0,int index=0)
		{
			if(selection_changed)
			{
				if(callback_selected)
					call_list.push_back(CallParams(callback_selected,parent,this,index));

				selection_changed=0;
			}
			Button::handle_callbacks(w,index);
		}
		
	};

	class Radio: public Button
	{
		public:

		Button select;

		std::vector<Button> item; int selected; 

		void init(){ selected=0;type=RADIO; skin=Skin(); };

		Radio():Button(){ init();}
		Radio (int pos_x,int pos_y,
			int width=global.number["button_size_x"],  // default
			int height=global.number["button_size_y"]) // default)
			:Button("",pos_x,pos_y,width,height,LEFT)
		{
			init();
			Button b("",pos_x,pos_y,width,height,LEFT);
			b.skin=global.skin["checkbox"];
			b.pressed=1;
			add_item(b);
		}	
		void add_item(Button b)
		{
			b.pressed=b.hover=b.active=0;
			if(item.size()==0)b.pressed=1;
			item.push_back(b);
			Rect r= (Rect) item[0] ;
			loopi(1,item.size())r.extend_rect( (Rect) item[i] );
			x=r.x;y=r.y;sx=r.sx;sy=r.sy;
		};
		void add_item(float x,float y)
		{
			Button b=item[0];
			b.pressed=b.hover=b.active=0;
			b.x=x;b.y=y;
			add_item(b);
		}
		void select_index(int i)
		{
			loopj(0,item.size())item[j].pressed= i==j ? 1 : 0;
			selected=i;
		};
		void draw(float ox=0,float oy=0,Window* parent=0,int index=0)
		{
			loopi(0,item.size())
			{
				if(item[i].active)
				{
					item[i].pressed=item[i].mouseinside(ox,oy);
					if(mouse.button_released[0])
					{
						if(item[i].pressed) 
						{
							select_index(i);
							if(callback_pressed) call_list.push_back(CallParams(callback_pressed,parent,this,index));
						}
						item[i].set_active(0);
					}
				};
				item[i].draw(ox,oy,0);
			}
		}
		bool find_active(int ox=0,int oy=0,Window* parent=0,int index=0)
		{
			if(Rect::is_clipped(ox,oy)) return false;
			loopi(0,item.size())if(item[i].find_active(ox,oy,parent,index)) return true;
			return false;
		}	
		void handle_callbacks(Window* parent=0,int index=0)
		{
			this->parent=parent;
			loopi(0,item.size()) item[i].handle_callbacks(parent);
		}
	};

	class TextEdit: public Button
	{
		public:

		// Number specific
		Button up,down;Label unit;double number_step,number_min,number_max; 

		enum Flags{ NONE=0, MULTI_LINE=1, PASSWORD=2, NUMBER=4 };
		int flags;

		int max_length; bool password,dirty;

		void (*callback_text_entered)(Window *window,Button* control,int index);

		void init(){type=TEXTEDIT;flags=NONE;max_length=20;dirty=password=false;skin=global.skin["textedit"];callback_text_entered=0;
			number_step=10;number_min=0;number_max=9999; };

		TextEdit():Button(){ init(); }
		TextEdit (int maxlength,String linetext,int pos_x,int pos_y,
			int width=global.number["button_size_x"],  // default
			int height=global.number["button_size_y"],int flags=NONE,Align alignment=LEFT,
			String number_unit="cm") // default)
			:Button(linetext,pos_x,pos_y,width,height,alignment)
		{
			init();
			max_length=maxlength;
			this->flags=flags;
			if(flags&NUMBER)
			{
				up  =Button("",sx-sy,0,sy,sy/2);
				down=Button("",sx-sy,sy/2,sy,sy/2);
				up.skin=global.skin["textedit_up"];
				down.skin=global.skin["textedit_down"];
				pad_right+=sy;
				unit = Label(number_unit,0,0,sx-sy,sy,Align::RIGHT);
				unit.text=number_unit;
				pad_right+=sy;
			}
		}	

		void set_number_params(String unit_text,double number_step=10,double number_min=0,double number_max=99999)
		{
			unit.text=unit_text;
			this->number_step=number_step;
			this->number_min=number_min;
			this->number_max=number_max;
		}
		void verify_number()
		{
			if(!(flags&NUMBER)) return;
			double num=atof(text.c_str());
			num=clamp(num,number_min,number_max);
			text=String(num);
		}

		void draw(float ox=0,float oy=0)
		{
			if(Rect::is_clipped(ox,oy)) return;

			if(active) hover=0;

			pressed=active;

			bool cursor=(timeGetTime()>>9)&pressed;
			if(cursor) text.push_back('_');

			bool hover=this->hover;

			Button::draw(ox,oy,0,0);

			if(cursor) text.resize(text.size()-1);

			if(active) 
			{
				if(text.size()<max_length)
				loopi(32,255)
					if(keyb.key_pressed[i])
						text.push_back(i);

				if(keyb.key_pressed[8]) // delete
					if(text.size()>0)
						text.erase(text.length()-1);
				
				if(keyb.key_pressed[13]) // enter
					if(flags&MULTI_LINE)
						text.push_back(10); // if multi-line, go to next line
					else
					{
						verify_number();
						dirty=1;
						set_active(0); // is single line, deactivate
					}
				
				if(mouse.button_pressed[0]) 
					if(!mouseinside(ox,oy)) 
					{
						verify_number();
						set_active(0);
						dirty=1;
					}
			}
			if(flags&NUMBER)
			{
				up.draw(ox+x,oy+y);
				down.draw(ox+x,oy+y);
				unit.draw(ox+x,oy+y);
				
				if(mouse.button_pressed[0])
				{
					double num=atof(text.c_str());
					if(up.pressed) num+=number_step;
					if(down.pressed) num-=number_step;
					num=clamp(num,number_min,number_max);
					text=String(num);
				}
				if(hover && mouse.wheel)
				{
					double num=atof(text.c_str());
					num-=double(mouse.wheel)*number_step;
					num=clamp(num,number_min,number_max);
					text=String(num);
					mouse.wheel=0;
				}
			}
		}
		void handle_callbacks(Window* parent=0,int index=0)
		{
			this->parent=parent;
			if(active) { active_control.active=1;active_control.set(this,type,parent,index); }
			if(call_init){call_init=0;if(callback_init)call_list.push_back(CallParams(callback_init,parent,this,index));}
			if(callback_all) call_list.push_back(CallParams(callback_all,parent,this,index));
			if(callback_text_entered && dirty) call_list.push_back(CallParams(callback_text_entered,parent,this,index));
			dirty=0;

			if(flags&NUMBER)
			{
				up.		handle_callbacks(parent,index);
				down.	handle_callbacks(parent,index);
			}
		}
		bool find_active(int ox=0,int oy=0,Window* parent=0,int index=0)
		{
			if(flags&NUMBER)
			{
				if ( up.find_active(ox+x,oy+y,parent,index )) return true;
				if ( down.find_active(ox+x,oy+y,parent,index )) return true;
			}
			if ( Button::find_active(ox,oy,parent,index )) return true;
			return false;
		}	
	};

	class ScrollBar: public Button
	{
		public:
		enum Flags { HORIZONTAL=1, VERTICAL=2 };
		
		Flags flags;
		float val;		
		Button bar,button_start,button_center,button_end;
		
		void init(Flags fflags=VERTICAL)
		{
			type=SCROLLBAR;
			flags=fflags;
			bar.skin			= (flags&VERTICAL) ? global.skin["vscrollbar"]					: global.skin["hscrollbar"];
			button_start.skin	= (flags&VERTICAL) ? global.skin["vscrollbar_button_start"]		: global.skin["hscrollbar_button_start"];
			button_center.skin	= (flags&VERTICAL) ? global.skin["vscrollbar_button_center"]	: global.skin["hscrollbar_button_center"];
			button_end.skin		= (flags&VERTICAL) ? global.skin["vscrollbar_button_end"]		: global.skin["hscrollbar_button_end"];
			val=0.5;
			button_start .sy=button_start .sx=20;
			button_center.sy=button_center.sx=20;
			button_end   .sy=button_end   .sx=20;
		};

		ScrollBar():Button(){ init();};
		ScrollBar (float val_default,
			float pos_x=0,float pos_y=0,
			float width=global.number["button_size_x"],  // default
			float height=global.number["button_size_y"],
			Flags flags=VERTICAL) // default)
			:Button("",pos_x,pos_y,width,height)
		{
			init(flags);
			val=val_default; 
		}	

		void draw(float ox=0,float oy=0)
		{
			if(Rect::is_clipped(ox,oy)) return;

			if(flags&VERTICAL)
			{
				button_start.set_rect(x,y,sx,button_start.sy);
				button_start.draw(ox,oy,1);

				bar.set_rect(x,y+button_start.sy,sx,sy-button_start.sy-button_end.sy);
				bar.draw(ox,oy,1);

				button_end.set_rect(x,y+sy-button_end.sy,sx,button_end.sy);
				button_end.draw(ox,oy,1);

				button_center.set_rect(x,bar.y+ val*(bar.sy-button_center.sy),sx,button_center.sy);
				
				if(mouse.button_pressed[0])
				{
					if(button_start.active)val-=0.01;
					if(button_end.active)val+=0.01;
					if(bar.active)val+=mouse.y<button_center.y?-.1:.1;
				}
				if(button_center.active)
				{
					static float y0=0;
					if(mouse.button_pressed[0]) { y0=mouse.y-button_center.y; }
					else { button_center.y=mouse.y-y0; }

					button_center.y=clamp(button_center.y,bar.y,bar.y+bar.sy-button_center.sy);
					val=(button_center.y-bar.y)/(bar.sy-button_center.sy);					
				}
				val=clamp(val,0,1);
				button_center.draw(ox,oy,1);
			}
			if(flags&HORIZONTAL)
			{
				button_start.set_rect(x,y,button_start.sx,sy);
				button_start.draw(ox,oy,1);

				bar.set_rect(x+button_start.sx,y,sx-button_start.sx-button_end.sx,sy);
				bar.draw(ox,oy,1);

				button_end.set_rect(x+sx-button_end.sx,y,button_end.sx,sy);
				button_end.draw(ox,oy,1);

				button_center.set_rect(bar.x+ val*(bar.sx-button_center.sx),y,button_center.sx,sy);
				
				if(mouse.button_pressed[0])
				{
					if(button_start.active)val-=0.1;
					if(button_end.active)val+=0.1;
					if(bar.active)val+=mouse.x<button_center.x?-.1:.1;
				}
				if(button_center.active)
				{
					static float x0=0;
					if(mouse.button_pressed[0]) { x0=mouse.x-button_center.x; }
					else { button_center.x=mouse.x-x0; }

					button_center.x=clamp(button_center.x,bar.x,bar.x+bar.sx-button_center.sx);
					val=(button_center.x-bar.x)/(bar.sx-button_center.sx);					
				}
				val=clamp(val,0,1);
				button_center.draw(ox,oy,1);
			}
		}
		bool find_active(int ox=0,int oy=0,Window* parent=0,int index=0)
		{
			if(Rect::is_clipped(ox,oy)) return false;
			if(button_start .find_active(ox,oy,parent,index )) return true;
			if(button_end   .find_active(ox,oy,parent,index )) return true;
			if(button_center.find_active(ox,oy,parent,index )) return true;
			if(bar			.find_active(ox,oy,parent,index )) return true;
			return false;
		}	
		void handle_callbacks(Window* parent=0,int index=0)
		{
			this->parent=parent;
			button_start.handle_callbacks(parent);
			button_end.handle_callbacks(parent);
			button_center.handle_callbacks(parent);
			bar.handle_callbacks(parent);
			if(callback_all) call_list.push_back(CallParams(callback_all,parent,this,index));
		}
	};

	class Slider: public Button
	{
		public:

		enum Flags { HORIZONTAL=1, VERTICAL=2, PROGRESSBAR=4 };
		
		int flags;
		float val,val_min,val_max;		
		Button left,right,button;

		void init()
		{
			type=SLIDER;
			left.skin=global.skin["slider_horiz_left"];
			right.skin=global.skin["slider_horiz_right"];
			button.skin=global.skin["slider_horiz_button"];
			val=val_min=0;val_max=1;
		};

		Slider():Button(){ init();};
		Slider (float val_min,float val_max,float val_default,
			float pos_x=0,float pos_y=0,
			float width=global.number["button_size_x"],  // default
			float height=global.number["button_size_y"],
			int flags=HORIZONTAL) // default)
			:Button("",pos_x,pos_y,width,height)
		{
			init();
			val=val_default; 
			skin=Skin();
			this->val_min=val_min;
			this->val_max=val_max;
			this->flags=flags;
			if(flags&VERTICAL)
			{
				left.skin=global.skin["slider_vert_left"];
				right.skin=global.skin["slider_vert_right"];
				button.skin=global.skin["slider_vert_button"];
			}
			if(flags&PROGRESSBAR)
			{ 
				if(flags&HORIZONTAL)
				{
					left.skin=global.skin["progress_horizontal_left"];
					right.skin=global.skin["progress_horizontal_right"];
				}
				if(flags&VERTICAL)
				{
					left.skin=global.skin["progress_vertical_left"];
					right.skin=global.skin["progress_vertical_right"];
				}
				button.flags=DEACTIVATED;right.flags=DEACTIVATED;left.flags=DEACTIVATED;button.skin=Skin(); 
			}
		}	
		void draw(float ox=0,float oy=0)
		{
			if(Rect::is_clipped(ox,oy)) return ;

			if(active) hover=0;

			left.hover=right.hover=button.hover=hover & !(flags&PROGRESSBAR);
			left.pressed=right.pressed=button.pressed=active;
			left.set_rect(x,y,sx,sy);
			right.set_rect(x,y,sx,sy);

			if(flags&HORIZONTAL)
			{
				float slider_pixel_pos=(val-val_min)*sx/(val_max-val_min);
				button.set_rect(x-sy/2+slider_pixel_pos,y,sy,sy); // square shaped button ; todo: add ratio
				left.draw(ox,oy,0,0,slider_pixel_pos);
				right.draw(ox,oy,0,slider_pixel_pos,sx);
			}
			else
			{
				float slider_pixel_pos=sy-(val-val_min)*sy/(val_max-val_min);
				button.set_rect(x,y-sx/2+slider_pixel_pos,sx,sx); // square shaped button ; todo: add ratio
				right.draw(ox,oy,0,0,sx,0,slider_pixel_pos);
				left.draw(ox,oy,0,0,sx,slider_pixel_pos,sy);
			}
			button.draw(ox,oy,0);

			Button::draw(ox,oy,0);
	
			if(active)  
			{
				if(flags & PROGRESSBAR)	set_active(0);
				else
				{
					if(flags&HORIZONTAL)
						val=clamp((mouse.x-x-ox)*(val_max-val_min)/sx+val_min,val_min,val_max);
					else
						val=clamp((sy-(mouse.y-y-oy))*(val_max-val_min)/sy+val_min,val_min,val_max);

					if(!mouse.button[0]) set_active(0);
				}
			}
			if(!mouseinside(ox,oy)) hover=0;
			pressed=active;
		}
	};

	class CheckBox: public Button
	{
		public:

		bool checked;

		void init(){type=CHECKBOX;checked=0;}

		CheckBox():Button(){ Button:init();init();}
		CheckBox (String txt,bool checked,int pos_x,int pos_y,
			int width=global.number["button_size_x"],  // default
			int height=global.number["button_size_y"],
			Align align=Align::CENTER,int flags=0) // default)
			:Button("",pos_x,pos_y,width,height,align,flags)
		{
			init();
			text=txt;
			skin=global.skin["checkbox"];
			this->checked=checked;
		}	
		void draw(float ox=0,float oy=0)
		{
			if(Rect::is_clipped(ox,oy)) return ;

			if(active) hover=0;

			pressed=checked;

			Button::draw(ox,oy,0);

			if(active)  if(mouse.button_released[0])
			{
				set_active(0);
			}
		}
		bool find_active(int ox=0,int oy=0,Window* parent=0,int index=0)
		{
			if(flags==DEACTIVATED) return false;;

			if(Rect::is_clipped(ox,oy)) return false;
			if(mouseinside(ox,oy))
			{
				if(mouse.button_pressed[0]) checked^=1;
				active_control.set(this,type,parent,index);
				return true;
			}
			return false;
		}
		void handle_callbacks(Window* parent=0,int index=0)
		{
			this->parent=parent;
			this->parent_index=index;	
			if(active)if(mouse.button_released[0])if(callback_pressed) call_list.push_back(CallParams(callback_pressed,parent,this,index));
			if(callback_all) call_list.push_back(CallParams(callback_all,parent,this,index));
		}
	};
	class Menu; class Tab;
	class Window: public Button
	{
		public:

		enum Flags {	CLOSED=0 , VISIBLE=1, RESIZEABLE=2 , TITLEBAR=4 , 
						CLOSEBUTTON=8 , LOCKED=16 , HSCROLLBAR=32 , VSCROLLBAR=64 , TOGGLEBUTTON=128 , 
						MOVE_TO_FRONT = 256 };

		int flags; 
		int title_height;

		Button title,closebutton,resizebutton,togglebutton;
		ScrollBar hscrollbar,vscrollbar;
		bool vscrollbar_visible,hscrollbar_visible;
		float scrollx,scrolly,closebutton_padx,closebutton_pady,togglebutton_padx,togglebutton_pady;
		
		private:
		Rect clientrect,clip; // inner rect
		bool toggled;
		
		public:
		ControlList<Window>		window;
		ControlList<Button>		button;
		ControlList<CheckBox>	checkbox;
		ControlList<Combo>		combo;
		ControlList<Label>		label;
		ControlList<TextEdit>	textedit;	
		ControlList<Slider>		slider;	
		ControlList<Menu>		menu;	
		ControlList<Tab>		tab;	
		ControlList<Radio>		radio;	

		void init(){
			flags=CLOSED;type=WINDOW;
			title_height=global.number["window_title_height"];
			title.pad_up=global.number["window_title_pady"];
			pad_left	=global.number["window_frame_pad_left"];
			pad_right	=global.number["window_frame_pad_right"];
			pad_up		=global.number["window_frame_pad_up"];
			pad_down	=global.number["window_frame_pad_down"];
			skin=		global.skin["window"];
			title.skin=	global.skin["window_title"];	
			closebutton.skin=	global.skin["window_close"];	closebutton.text="";
			resizebutton.skin=global.skin["window_resize"];	resizebutton.text="";
			togglebutton.skin=global.skin["window_toggle"];	togglebutton.text="";
			vscrollbar=ScrollBar(0,0,0,20,20,ScrollBar::VERTICAL);
			hscrollbar=ScrollBar(0,0,0,20,20,ScrollBar::HORIZONTAL);
			vscrollbar.sx=global.number["window_vscrollbar_width"];
			hscrollbar.sy=global.number["window_hscrollbar_height"];
			resizebutton.sx=global.number["window_resizebutton_width"],
			resizebutton.sy=global.number["window_resizebutton_height"];
			togglebutton_padx=global.number["window_togglebutton_padx"];
			togglebutton_pady=global.number["window_togglebutton_pady"];			
			togglebutton.sx=global.number["window_togglebutton_width"];
			togglebutton.sy=global.number["window_togglebutton_height"];
			closebutton_padx=global.number["window_closebutton_padx"];
			closebutton_pady=global.number["window_closebutton_pady"];
			closebutton.sx=global.number["window_closebutton_width"];
			closebutton.sy=global.number["window_closebutton_height"];
			scrollx=scrolly=0;toggled=0;vscrollbar_visible=0;hscrollbar_visible=0;
		};

		Window () : Button() { init(); };

		Window (String windowtitle,int pos_x=0,int pos_y=0,int width=200,int height=200, int wflags=RESIZEABLE|TITLEBAR|CLOSEBUTTON|HSCROLLBAR|VSCROLLBAR|TOGGLEBUTTON|MOVE_TO_FRONT)
			:Button("",pos_x,pos_y,width,height)
		{
			init();	flags=wflags; title.text=windowtitle;
		}

		void close(){ flags=CLOSED; }
		void resize(float x,float y){ sx=x;sy=y;minsx=x;minsy=y; }
		void minsize(float x,float y){ minsx=x;minsy=y; }
	
		bool get_toggled(){ return toggled;}
		void set_toggled(bool t)
		{
			togglebutton.set_active(0);

			if(t==toggled) return;

			toggled=t;

			if( toggled )
			{
				var.number["tmp_window_height"]=sy;
				sy=title_height;
			}
			else sy=var.number["tmp_window_height"];
		}
		void draw(float ox=0,float oy=0,Rect* parent_rect=0)
		{
			//if(Rect::is_clipped(ox,oy)) return ;
			if(flags==CLOSED) return;

			// move window?
			if(!(flags&LOCKED))
			if(title.active)
			{
				x+=mouse.dx;y+=mouse.dy;float borderdist=20;
				y=clamp(y,0,screen_resolution_y-title_height);
				x=clamp(x,0,screen_resolution_x-borderdist);
				if(parent_rect)
				{
					x=min(x,parent_rect->x+parent_rect->sx-borderdist);
					y=min(y,parent_rect->y+parent_rect->sy-borderdist);
				}
				if(!mouse.button[0]) title.set_active(0);
			}
			//resize?
			if(resizebutton.active)
			{
				static float x0=0,y0=0;
				if(mouse.button_pressed[0])	{ x0=mouse.x-sx;y0=mouse.y-sy; }
				else						{ sx=mouse.x-x0;sy=mouse.y-y0; }

				sx=max(sx,minsx);sy=max(sy,minsy);
			}
			if(togglebutton.active&&mouse.button_released[0]) 
			{
				set_toggled(toggled^1);
			}		
			bool hover=this->hover; // this->hover cleared by Button::draw

			Button::draw(ox,oy,false);

			// set frame, clipped area and client rect
			// frame = inner frame of the window
			// clip = visible region
			// client rect = stuff inside the window

			Rect frame (	pad_left+x+ox ,
							pad_up  +y+oy ,
							sx-pad_right-pad_left,
							sy-pad_down-pad_up);

			clip=frame; // clip to visible area
			if(parent_rect) clip.crop(*parent_rect);

			if(flags&TITLEBAR) // handle title
			{
				title.set_rect(0,0,sx,title_height);
				title.draw(x+ox,y+oy);
			}
			if(flags&CLOSEBUTTON)
			{
				closebutton.set_rect(	 sx-closebutton.sx-closebutton_padx,closebutton_pady,closebutton.sx,closebutton.sy ); 
				closebutton.draw(x+ox,y+oy);
				if(closebutton.active){close();return;}
			}
			if(flags&TOGGLEBUTTON)
			{
				togglebutton.set_rect( sx-togglebutton.sx-togglebutton_padx,togglebutton_pady,togglebutton.sx,togglebutton.sy ); 
				togglebutton.pressed=toggled;
				togglebutton.draw(x+ox,y+oy,0);

				if ( toggled ) return; // skip drawing
			}
			if(flags&VSCROLLBAR)
			{
				vscrollbar_visible = (clientrect.sy>frame.sy+pad_up);
				vscrollbar.set_rect(frame.sx+frame.x,frame.y,vscrollbar.sx,frame.sy);
 				if(vscrollbar_visible) vscrollbar.draw(0,0);
			}			
			if(flags&HSCROLLBAR)
			{	
				hscrollbar_visible = (clientrect.sx>frame.sx+pad_left);
				hscrollbar.set_rect(frame.x,frame.y+frame.sy,frame.sx,hscrollbar.sy);
 				if(hscrollbar_visible)hscrollbar.draw(0,0);			
			}
			if(flags&RESIZEABLE)//resize button
			{	
				resizebutton.set_rect(sx-resizebutton.sx,sy-resizebutton.sy,resizebutton.sx,resizebutton.sy);
				resizebutton.draw(x+ox,y+oy);
			}
			scrollx=clamp(clientrect.sx-frame.sx-pad_left,0,clientrect.sx)*hscrollbar.val; 
			scrolly=clamp(clientrect.sy-frame.sy-pad_up,0,clientrect.sy)*vscrollbar.val;
			if(!hscrollbar_visible)scrollx=0;
			if(!vscrollbar_visible)scrolly=0;
			float ox_scroll=x+ox-scrollx, oy_scroll=y+oy-scrolly;
			
			clip_rect=clip;
			
			glEnable(GL_SCISSOR_TEST);
			glScissor(clip.x,screen_resolution_y-clip.y-clip.sy,clip.sx,clip.sy);
			ogl_check_error();
			
			// draw content	
			loopi(0,button.size())		button[button.size()-1-i].draw(ox_scroll,oy_scroll);
			loopi(0,radio.size())		radio[radio.size()-1-i].draw(ox_scroll,oy_scroll,this,i);
			loopi(0,checkbox.size())	checkbox[checkbox.size()-1-i].draw(ox_scroll,oy_scroll);
			loopi(0,slider.size())		slider[slider.size()-1-i].draw(ox_scroll,oy_scroll);
			loopi(0,textedit.size())	textedit[textedit.size()-1-i].draw(ox_scroll,oy_scroll);
			loopi(0,label.size())		label [label .size()-1-i].draw(ox_scroll,oy_scroll);			
			loopi(0,combo .size())		combo[combo .size()-1-i].draw(ox_scroll,oy_scroll);
			loopi(0,tab .size())		tab[tab .size()-1-i].draw(ox_scroll,oy_scroll);
			loopi(0,window.size())		window[window.size()-1-i].draw(ox_scroll,oy_scroll,&frame);
			
			glScissor(ox+x,screen_resolution_y-(oy+y+sy),sx,sy);
			clip_rect.set_rect(ox+x,oy+y,sx,sy);
			ogl_check_error();

			loopi(0,menu .size())		menu[menu .size()-1-i].draw(x+ox,y+oy);

			if(parent_rect)
			{
				Rect &r=*(parent_rect);clip_rect=r;
				glScissor(r.x,screen_resolution_y-r.y-r.sy,r.sx,r.sy);
			}
			else
			{
				glScissor(0,0,screen_resolution_x,screen_resolution_y);
				clip_rect.set_rect(0,0,screen_resolution_x,screen_resolution_y);
			}
			ogl_check_error();
			
			// capture mouse wheel if not done by subwindow.
			if(flags&VSCROLLBAR) if(hover)
			{
				vscrollbar.val=clamp(vscrollbar.val+float(mouse.wheel)*0.1,0,1);
				mouse.wheel=0;
			}
			
			// check window stuff
			
			loopi(0,window.size())
			{
				if(window[i].flags==Window::CLOSED ){ window.erase(i); break;}
				if(window[i].flags&Window::MOVE_TO_FRONT)
				{
					window[i].flags&=0x7fffffff - Window::MOVE_TO_FRONT;
					window.to_front(i);
					break;
				}
			}
		}
		void window_rect(Rect &r) 
		{
			loopi(0,button.size())		button[i]	.extend_rect(r);
			loopi(0,radio.size())		radio[i]	.extend_rect(r);
			loopi(0,checkbox.size())	checkbox[i]	.extend_rect(r);
			loopi(0,slider.size())		slider[i]	.extend_rect(r);
			loopi(0,textedit.size())	textedit[i]	.extend_rect(r);
			loopi(0,label.size())		label [i]	.extend_rect(r);
			loopi(0,window.size())		window[i]	.extend_rect(r);
			loopi(0,combo .size())		combo[i]	.extend_rect(r);
			loopi(0,tab .size())		tab[i]		.extend_rect(r);
			loopi(0,menu .size())		menu[i]		.extend_rect(r);
		}
		void handle_callbacks(Window* parent=0,int index=0)
		{
			this->parent=parent;
			if(flags==CLOSED) return;

			// get client area
			clientrect.set_rect(x,y,0,0);
			window_rect(clientrect);

			// callbacks
			loopi(0,menu .size())		menu[i].handle_callbacks(this,i);
			loopi(0,window.size())		
			{
				window[i].handle_callbacks(this,i);
			}
			loopi(0,tab .size())		tab[i].handle_callbacks(this,i);
			loopi(0,combo .size())		combo[i].handle_callbacks(this,i);
			loopi(0,button.size())		button[i].handle_callbacks(this,i);
			loopi(0,radio.size())		radio[i].handle_callbacks(this,i);
			loopi(0,checkbox.size())	checkbox[i].handle_callbacks(this,i);
			loopi(0,slider.size())		slider[i].handle_callbacks(this,i);
			loopi(0,textedit.size())	textedit[i].handle_callbacks(this,i);

			if(flags&CLOSEBUTTON)closebutton.handle_callbacks(this,index);
			if(flags&TITLEBAR)	 title.handle_callbacks(this,index);
			if(flags&RESIZEABLE) resizebutton.handle_callbacks(this,index);
			if(flags&HSCROLLBAR) if(hscrollbar_visible) hscrollbar.handle_callbacks(this,index);
			if(flags&VSCROLLBAR) if(vscrollbar_visible) vscrollbar.handle_callbacks(this,index);
			if(flags&TOGGLEBUTTON)togglebutton.handle_callbacks(this,index);

			if(call_init){call_init=0;if(callback_init)call_list.push_back(CallParams(callback_init,parent,this,index));}
			if(callback_all) call_list.push_back(CallParams(callback_all,parent,this,index));

			if(active) { active_control.active=1;active_control.set(this,type,parent,index); }
		}

		bool find_active(int ox=0,int oy=0,Window* parent=0,int index=0)
		{
			if(flags==CLOSED) return false;
			float ox_scroll=x+ox-scrollx, oy_scroll=y+oy-scrolly;

			Rect tmp_cliprect=clip_rect;

			loopi(0,menu.size())if(menu[i].find_active(ox+x,oy+y,this,index))	goto window_ret_true;

			if(!Button::mouseinside(ox,oy)) return false;

			// window standard controls
			if(flags&CLOSEBUTTON)if(closebutton.find_active(x+ox,y+oy,this,index))	goto window_ret_true;
			if(flags&TOGGLEBUTTON)if(togglebutton.find_active(x+ox,y+oy,this,index)) goto window_ret_true;
			if(flags&TITLEBAR)	 if(title.find_active(x+ox,y+oy,this,index))	goto window_ret_true;
			if(flags&RESIZEABLE) if(resizebutton.find_active(x+ox,y+oy,this,index))	goto window_ret_true;
			if(flags&HSCROLLBAR) if(hscrollbar_visible)  if(hscrollbar.find_active(0,0,this,index))	goto window_ret_true;
			if(flags&VSCROLLBAR) if(vscrollbar_visible)  if(vscrollbar.find_active(0,0,this,index))	goto window_ret_true;

			Rect frame (pad_left+x+ox,pad_up+y+oy,sx-pad_right-pad_left,sy-pad_down-pad_up);
			if(mouse.x>=frame.x)if(mouse.y>=frame.y)
			if(mouse.x<=frame.sx+frame.x)if(mouse.y<=frame.sy+frame.y)
			{
				clip_rect=clip;
				loopi(0,window.size())		if(window[i].find_active(ox_scroll,oy_scroll,this,index ))	goto window_ret_true;
				loopi(0,tab.size())			if(tab[i].find_active(ox_scroll,oy_scroll,this,index))		goto window_ret_true;
				loopi(0,combo.size())		if(combo[i].find_active(ox_scroll,oy_scroll,this,index))		goto window_ret_true;
				loopi(0,slider.size())		if(slider[i].find_active(ox_scroll,oy_scroll,this,index))		goto window_ret_true;
				loopi(0,checkbox.size())	if(checkbox[i].find_active(ox_scroll,oy_scroll,this,index))	goto window_ret_true;
				loopi(0,textedit.size())	if(textedit[i].find_active(ox_scroll,oy_scroll,this,index))	goto window_ret_true;	
				loopi(0,radio.size())		if(radio[i].find_active(ox_scroll,oy_scroll,this,index))		goto window_ret_true;
				loopi(0,button.size())		if(button[i].find_active(ox_scroll,oy_scroll,this,index))		goto window_ret_true;
				clip_rect.set_rect(ox+x,oy+y,sx,sy);
			}
			if(mouseinside(ox,oy))
			{
				active_control.set(this,type,parent,index);
				goto window_ret_true;
			}
			return false;

			window_ret_true:

			clip_rect=tmp_cliprect;
			hover=true;
			return true;
		}
	};

	class Menu: public Button
	{
		public:

		enum MenuType { NORMAL_MENU = 0 , SUB_MENU=1 };

		Window window;	float tmp_hover;

		void init(){ type=MENU;tmp_hover=0; window.sx=window.sy=0;};

		Menu():Button(){ init();}
		Menu (String text,int pos_x,int pos_y,
			float width=global.number["button_size_x"],  // default
			float height=global.number["button_size_y"], // default)
			float menuwidth=global.number["menu_size_x"],
			int flags=NORMAL_MENU)
			:Button(text,pos_x,pos_y,width,height,LEFT)
		{
			init();
			this->flags=flags;
			float ox= flags == NORMAL_MENU ? 0 : sx;
			float oy= flags == NORMAL_MENU ? sy : 0;
			window=Window("",pos_x+ox,pos_y+oy,menuwidth,0,Window::LOCKED);
			window.skin=skin; // use the button skin
			window.pad_down=window.pad_up=0;
			window.pad_left=window.pad_right=0;
			skin.tex_normal=0; 
		}	
		void resize_window()
		{
			Rect r(0,0,0,0);
			window.extend_rect(r);
			window.sx=r.sx-x;window.sy=r.sy-y;
		}
		void add_item(String s , void (*callback_item)(Window *,Button* ,int )=0)
		{
			Button b(s,0,window.sy,window.sx,sy,LEFT);
			b.skin=global.skin["menu"];
			b.callback_pressed=callback_item;
			window.button.push_back(b);
			resize_window();
		};
		void add_menu(String s)
		{
			Menu m(s,0,window.sy,window.sx,sy,sx,SUB_MENU);
			m.skin=global.skin["submenu"];
			window.menu.push_back(m);
			resize_window();
		};
		void add_item(Button b,void (*callback_item)(Window *,Button* ,int )=0)
		{
			if(callback_item)b.callback_pressed=callback_item;
			window.button.push_back(b);
			resize_window();
		};
		bool mouse_over(float ox,float oy)
		{
			if(mouseinside(ox,oy))
			{
				if(flags==SUB_MENU) tmp_hover=1;
				else tmp_hover = pressed ? 1 : 0; 
				return 1;
			}
			if(tmp_hover>0.25)
			{
				if(window.mouseinside(ox,oy))
				{
					tmp_hover=1;
					return 1;
				}
				loopi(0,window.menu.size())
					if(window.menu[i].mouse_over(window.x+ox,window.y+oy))
						{tmp_hover=(flags==SUB_MENU) ? 1 : (pressed ? 1 : 0);return 1;}
			}	
			return 0;
		}

		void open_at(float pos_x,float pos_y) // for context menu
		{
			if(window.sx==0) return;
			sx=0;sy=0;x=pos_x;y=pos_y;text="";active=1;hover=1;pressed=1;tmp_hover=1;
		};

		bool is_closed(){ return tmp_hover==0 ;};

		void draw(float ox=0,float oy=0,Window *parent=0,int index=0)
		{
			if(Rect::is_clipped(ox,oy)) return;
			
			bool mouseover=mouse_over( ox, oy);
			this->hover=(mouseover&hover) || tmp_hover>0.25;
			

			if(active){ set_active(0);pressed=mouseinside(ox,oy);}
			else if(pressed && !mouseover) pressed=0;
			
			tmp_hover*=0.5;

			Button::draw(ox,oy,0);

			if(tmp_hover>0.25) // open?
			{
				Rect tmp=clip_rect;clip_rect=Rect(0,0,screen_resolution_x,screen_resolution_y);
				glDisable(GL_SCISSOR_TEST);
				if(flags==SUB_MENU){ window.x=x+sx;window.y=y;} else {window.x=x;window.y=y+sy;};
				window.hover=0;
				window.draw(ox,oy);
				glEnable(GL_SCISSOR_TEST);
				clip_rect=tmp;

				if(mouse.button_released[0])
				if(window.find_active(ox,oy))
				if(active_control.type!=WINDOW)
				if(active_control.type!=NONE)
				if(active_control.type!=MENU)
				{
					tmp_hover=0;pressed=0;
					if(callback_pressed) call_list.push_back(CallParams(callback_pressed,parent,this,index));
				}
			}
		}
		void handle_callbacks(Window* parent=0,int index=0)
		{
			this->parent=parent;

			Button::handle_callbacks(parent, index);
			if(tmp_hover>0.25)window.handle_callbacks(parent);

			if(active) { active_control.active=1;active_control.set(this,type,parent,index); }
		}

		bool find_active(int ox=0,int oy=0,Window* parent=0,int index=0)
		{
			if(Button::find_active(ox,oy,parent,index))	return true;

			if(tmp_hover>0.25)
				if(window.find_active(ox,oy,parent,index))return true;

			return false;
		}
	};
	
	class Tab: public Button
	{
		public:

		ControlList<Button> button;	
		ControlList<Window> window;	

		enum Flags { MOVABLE=1, LOCKED=0 };
		
		int selected,flags; float tabwidth,tabheight;

		void init(){ type=MENU;selected=0;selected=0;pad_up=pad_down=pad_left=pad_right=5;};

		Tab():Button(){ init();}
		Tab (int pos_x,int pos_y,
			float width=200,  // default
			float height=200, // default)
			float tabwidth=global.number["tab_size_x"],
			float tabheight=global.number["tab_size_y"],
			int flags=LOCKED)
			:Button("",pos_x,pos_y,width,height,LEFT)
		{
			init();
			this->tabwidth=tabwidth;this->tabheight=tabheight;this->flags=flags;	
			skin=global.skin["tab"];
		}	
		void remove_tab(int i)
		{
			if(i<window.size())
			{
				window.erase(i);
				button.erase(i);
			}
		}
		void add_tab(String title,Window* win=0)
		{
			if(button.size()==0)
			{
				button[0]=Button(title,pad_left,pad_up,tabwidth,tabheight,LEFT);;
				button[0].pressed=1;
			}
			else
			{
				Button &last=button[button.size()-1];
				float right=last.sx+last.x;
				Button b(title,right,last.y,tabwidth,tabheight,LEFT);
				b.skin=button[0].skin;
				button.push_back(b);
			}
			
			Window w=win ? *win : Window("");

			w.flags=Window::LOCKED;
			w.skin=global.skin["tab_window"]; // use the button skin
			w.pad_down=w.pad_up=2;
			w.pad_left=w.pad_right=2;
			w.set_rect(pad_left,pad_up+tabheight,sx-pad_left-pad_right,sy-tabheight-pad_up-pad_down);
			window.push_back(w);
		};
		void draw(float ox=0,float oy=0)
		{
			if(Rect::is_clipped(ox,oy)) return;
			
			Button::draw(ox,oy,0);
			Rect clip=clip_rect;
			window[selected].draw(ox+x,oy+y,&clip);
			clip_rect=clip;

			loopi(0,button.size())
			{
				Button &b=button[i];
				if(b.active)
				{
					loopj(0,button.size()) if(i!=j){button[j].set_active(0);button[j].pressed=0;}
					if(mouse.button_released[0])
					{
						b.set_active(0);
					}
					b.pressed=1;
					selected=i;
				}
				b.draw(ox+x,oy+y,0);
			}
			if(active)
			{
				if(flags!=LOCKED){x+=mouse.dx;y+=mouse.dy;}
				hover=1;window[selected].hover=1;
				if(!mouse.button[0]) set_active(0);
			}
			else
			{
				hover=0;
			}
		}
		void handle_callbacks(Window* parent=0,int index=0)
		{
			this->parent=parent;

			Button::handle_callbacks(parent, index);
			loopi(0,button.size()) button[i].handle_callbacks(parent,i);
			window[selected].handle_callbacks(parent,selected);

			if(active) { active_control.active=1;active_control.set(this,type,parent,index); }
		}
		bool find_active(int ox=0,int oy=0,Window* parent=0,int index=0)
		{
			if(Rect::is_clipped(ox,oy)) return false;

			loopi(0,button.size()) if(button[i].find_active(ox+x,oy+y,parent,index))	return true;

			if(window[selected].find_active(ox+x,oy+y,parent,index)) 
			{
				if(flags&MOVABLE)
				if(mouse.button[0] && active_control.type==WINDOW)
				{
					window[selected].set_active(0);
					active_control.set(this,type);
				}
				return true;
			}
			if(mouseinside(ox,oy))
			{
				active_control.set(this,type,parent,index);
				return true;
			}
			return false;
		}
	};

	ControlList<Window>		dialog;
	ControlList<Window>		screen;

	Window new_screen()
	{
		Window w("default",0,0,screen_resolution_x,screen_resolution_y,Window::LOCKED);
		w.skin=Skin();
		w.pad_down=w.pad_up=w.pad_left=w.pad_right=0;
		return w;
	}
	void set_screen_active(int i) {active_screen=i;};
	void set_screen_active(String s) {active_screen=screen.get_index(s.s);};

	void global_skin_load(String filename)
	{
		char* s1;char* s2;char* s3;

		FILE *f1=fopenx(filename.c_str(),"r");
		char line[1000];
		int a1,a2,a3;vec4f v;float f;

		Skin skin; 
		int c=0;String name="",normal="",hover="",pressed="",scale="";

		while(fgets(line,1000,f1)!=NULL) 
		if((c=splitstr(line,&s1,&s2,&s3))>=2)
		{
			bool cmdok=false;
			ifeq(s1,"skin")
			{
				if(c==3) // 3 strings
				{
					ifeq(s2,"begin") {name=String(s3);normal=hover=pressed=scale="";};
					ifeq(s2,"normal") normal=String(s3);
					ifeq(s2,"hover") hover=String(s3);
					ifeq(s2,"pressed") pressed=String(s3);
					ifeq(s2,"scale") scale=String(s3);
				}
				if(c==2)ifeq(s2,"end")
				{
					Skin skin; 
					if(scale=="") skin=Skin(normal.s,hover.s,pressed.s);
					else 
					{
						ifeq(s3,"scale") skin=Skin(normal.s,hover.s,pressed.s,Skin::SCALE);
						else
						{
							char type[100];float x1=0,x2=0,y1=0,y2=0;
							sscanf(s3,"%s %f %f %f %f",type,&x1,&x2,&y1,&y2);
							ifeq(type,"inner") skin=Skin(normal.s,hover.s,pressed.s,Skin::SCALEINNER,x1,x2,y1,y2);
							ifeq(type,"repeated") skin=Skin(normal.s,hover.s,pressed.s,Skin::REPEAT,x1,x2,y1,y2);
						}
					}
					global.skin[name]=skin;
				}
			};
		}
		fclose(f1);
	}

	void exit() 
	{ 	
		dialog.clear();
		screen.clear();  
	};

	bool init(int flags=0,String cfg_vars="../data/gui_global.txt",String cfg_skin="../data/gui_skin.txt")
	{
		shader_scaled=Shader("../shader/SkinScaled");
		shader_inner=Shader("../shader/SkinScaledInner");
		shader_repeated=Shader("../shader/SkinRepeated");

		mouse.init(); keyb.init();

		global.load((char*)cfg_vars.c_str());
		global_skin_load(cfg_skin);

		this->flags=flags;

		if(!(font = dtx_open_font(global.string["font_name"].c_str(), 0))) error_stop("font file not found");//meiryob.ttc
		dtx_prepare_range(font, global.number["font_size"], 0, 256);			/* ASCII */
		dtx_prepare_range(font, global.number["font_size"], 0x370, 0x400);		/* greek */
		dtx_prepare_range(font, global.number["font_size"], 0x400, 0x500);		/* cyrilic */
		dtx_prepare_range(font, global.number["font_size"], 0x3000 , 0x9fc0);	/* kanji */

		active_screen=0;
		screen[0]=new_screen();

		int vp[4];
		glGetIntegerv(GL_VIEWPORT, vp);
		screen_resolution_x=vp[2];
		screen_resolution_y=vp[3];

		return true;
	}

	private:

	void active_set_hover(bool h)
	{
		if(!active_control.ptr) return;
		active_control.ptr->hover=h;
	}
	void active_set_active(bool a)
	{
		if(!active_control.ptr) return;
		active_control.ptr->set_active(a);
	}
	void find_active_control()
	{
		clip_rect.set_rect(0,0,screen_resolution_x,screen_resolution_y);
		active_control.active=0;
		active_control.ptr=0;
		active_control.type=NONE;
		active_control.window_index=-1;

		Window * parent=&screen[active_screen];

		loopi(0, screen[active_screen].menu.size())		if( screen[active_screen].menu[i].find_active(0,0,parent,i)) return ;
		loopi(0, screen[active_screen].window.size()) 
		{
			if (  screen[active_screen].window[i].find_active(0,0,parent,i))
			{
				active_control.window_index=i;
				return ;
			}
		}
		clip_rect.set_rect(0,0,screen_resolution_x,screen_resolution_y);
		loopi(0, screen[active_screen].tab.size())		if( screen[active_screen].tab[i]		.find_active(0,0,parent,i)) return ;
		loopi(0, screen[active_screen].radio.size())	if( screen[active_screen].radio[i]		.find_active(0,0,parent,i)) return ;
		loopi(0, screen[active_screen].checkbox.size())	if( screen[active_screen].checkbox[i]	.find_active(0,0,parent,i)) return ;
		loopi(0, screen[active_screen].textedit.size())	if( screen[active_screen].textedit[i]	.find_active(0,0,parent,i)) return ;
		loopi(0, screen[active_screen].combo.size())	if( screen[active_screen].combo[i]		.find_active(0,0,parent,i)) return ;
		loopi(0, screen[active_screen].slider.size())	if( screen[active_screen].slider[i]		.find_active(0,0,parent,i)) return ;
		loopi(0, screen[active_screen].button.size())	if( screen[active_screen].button[i]		.find_active(0,0,parent,i)) return ;
		return ;
	}

	void handle_callbacks()
	{
		clip_rect.set_rect(0,0,screen_resolution_x,screen_resolution_y);
		active_control.active=0;
		active_control.ptr=0;
		active_control.type=NONE;
		active_control.window_index=-1;

		screen[active_screen].handle_callbacks(&screen[active_screen],active_screen);
		
		if(!active_control.active) return;
		if(!active_control.ptr)return;

		Button &b=*((Button*)active_control.ptr);
		Window &w=*((Window*)active_control.w_ptr); // might be 0 in case of background controls

		int control_index=active_control.control_index;
		if(b.hover || b.active || b.pressed) if(b.callback_hover  )
		{
			call_list.push_back(CallParams (b.callback_hover,&w,&b,control_index));
		}
		if(b.callback_pressed)
		{
			if(active_control.type==BUTTON && mouse.button_released[0]) 
				call_list.push_back(CallParams (b.callback_pressed,&w,&b,control_index));
			else
			if(active_control.type!=BUTTON) 
				call_list.push_back(CallParams (b.callback_pressed,&w,&b,control_index));
		}
	}

	void manager()
	{
		screen[active_screen].sx=screen_resolution_x;
		screen[active_screen].sy=screen_resolution_y;
		
		handle_callbacks();

		if(active_control.active) return;

		// hover flag
		find_active_control();
		active_set_hover(1);

		// callback if not active
		if(active_control.ptr)
		if(active_control.ptr->callback_hover)
		{
			Window &w=*((Window*)active_control.w_ptr); 
			Button &b=*((Button*)active_control.ptr);
			if(b.hover || b.active || b.pressed) 
				call_list.push_back(CallParams (b.callback_hover,&w,&b,active_control.control_index));
		}

		// handle activate control
		if(mouse.button_pressed[0])
		{
			/*
			if(active_control.ptr)
			printf("windows index %d type %d index %d Rect %.1f %.1f %.1f %.1f\n",
				active_control.window_index,
				active_control.type,
				active_control.control_index,
				active_control.ptr->x,
				active_control.ptr->y,
				active_control.ptr->sx,
				active_control.ptr->sy
				);*/

			if(active_control.type!=WINDOW)
				active_set_active(1);

			// current frame		
			loopi(0,screen[active_screen].window.size()) screen[active_screen].window[i].pressed=0;

			if(active_control.window_index>=0)
			{
				screen[active_screen].window[active_control.window_index].flags|=Window::MOVE_TO_FRONT;
				screen[active_screen].window[active_control.window_index].pressed=1;
			}
			active_control.ptr=0;
			if(active_control.type!=WINDOW)
				active_control.active=1;
		}
	}

	void custom_mouse()
	{
		// hide windows mouse cursor if required
		int screen_pos_x = glutGet((GLenum)GLUT_WINDOW_X);
		int screen_pos_y = glutGet((GLenum)GLUT_WINDOW_Y);
		static int cur_bef=-1;
		int cur_now=1;
		POINT p;
		if (GetCursorPos(&p))
		{
			int x=p.x-screen_pos_x;
			int y=p.y-screen_pos_y;

			if(x>=0)if(x<screen_resolution_x)
			if(y>=0)if(y<screen_resolution_y)
			{
				mouse.x=p.x-screen_pos_x;
				mouse.y=p.y-screen_pos_y;
			}
			cur_now=0;
		}
		if(cur_now!=cur_bef) 
			glutSetCursor(cur_now?GLUT_CURSOR_NONE:GLUT_CURSOR_NONE); 

		//	ShowCursor(cur_now);
		cur_bef=cur_now;

		// Mouse Arrow
		static Button b("",0,0,30,30,CENTER,0,0,global.skin["mouse_arrow"]);
		b.flags=Button::DEACTIVATED;
		b.draw(mouse.x,mouse.y,0);
	}

	public:

	void draw()
	{
		keyb.update();
		mouse.update();

		manager(); // handles active controls

		// Callbacks
		loopi(0,call_list.size()) 
		{
			CallParams cp=call_list[i];
			cp.func(cp.window,cp.control,cp.index); // hover,pressed,all callbacks
		}	
		call_list.clear();

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		dtx_use_font(font, global.number["font_size"]);
		clip_rect.set_rect(0,0,screen_resolution_x,screen_resolution_y);

		screen[active_screen].draw();

		// context menu
		if(flags & CONTEXT_MENU)
		{
			if(mouse.button_pressed[2]) screen[active_screen].menu["context_menu"].open_at(mouse.x,mouse.y);
			if(screen[active_screen].menu["context_menu"].is_closed())screen[active_screen].menu["context_menu"].x=-100;
		}

		// custom mouse pointer ?
		if(flags & CUSTOM_MOUSE) custom_mouse();

		// close sub-window?
		//loopi(0,screen[active_screen].window.size()) if( screen[active_screen].window[i].flags==Window::CLOSED ) window.erase(i);
	}

};	

float Gui::screen_resolution_x=800;
float Gui::screen_resolution_y=600;
Gui::Var Gui::global;
Gui::Mouse Gui::mouse;
Gui::Keyb Gui::keyb;
Gui::Active Gui::active_control;
Gui::Rect Gui::clip_rect;
std::vector<Gui::CallParams> Gui::call_list;
Gui gui;

Gui::String gui_file_get_chg_dir(Gui::String cur,Gui::String dir)
{
	if(dir=="..")
	{
		int last=cur.find_last_of("/\\");
		if(last!= -1) cur.resize(last);
		//if(last>2) cur.resize(last);
		
		return cur;
	}
	return cur+"\\"+dir;
};
Gui::String gui_file_get_size_str(long a)
{
	if(a>1024*1024) return str("%2.1lf MB",double(a)/(1024*1024));
	if(a>1024) return str("%2.1lf KB",double(a)/(1024));
	return str("%2.0lf B",double(a));
}
Gui::Window gui_file_list_window(Gui::String path=".",Gui::String extension="")
{
	std::vector<std::string> listdirs,listfiles;std::vector<long> listfilesize;

	file_get_dir((path+"\\").c_str(),listdirs,listfiles,listfilesize,(char*)extension.c_str());

	Gui::Window w=Gui::Window("",20,80,300,220,Gui::Window::VSCROLLBAR|Gui::Window::LOCKED); //subwindow
	w.var.string["extension"]=extension;
	w.skin=Skin();
	w.pad_up=0;
	w.pad_down=0;
	w.pad_left=0;
	w.pad_right=20;
	w.title_height=0;
 
	auto button_select_callback = [](Gui::Window *window,Gui::Button* control,int index)
	{
		Gui::Button &b=*((Gui::Button*) control);
		Gui::Window &w=*((Gui::Window*) window);

		if(!w.parent) return;

		if(w.label[index].text.s==std::string("<dir>"))
		{
			w.parent->label["dir"].text=gui_file_get_chg_dir(w.parent->label["dir"].text,b.text);
			w=gui_file_list_window(	
				w.parent->label["dir"].text,
				w.var.string["extension"]);
		}
		else
		{
			w.parent->textedit["Filename"].text=b.text;
			w.parent->textedit["Filename"].var.string["dir"]=w.parent->label["dir"].text;
		}
	}; 

	loopi(0,listdirs.size()+listfiles.size())
	{
		Gui::String name= i<listdirs.size() ? listdirs[i] : listfiles[i-listdirs.size()] ;
		Gui::String filesize= i<listdirs.size() ? "<dir>":  gui_file_get_size_str(listfilesize[i-listdirs.size()]) ;
		w.button[i] = Gui::Button(name,0,i*20,280);
		w.button[i].pad_right=90;
		w.button[i].skin=gui.global.skin["file_list_window"];
		w.button[i].align=Gui::LEFT;
		w.button[i].callback_pressed=button_select_callback;
		w.label[i]  = Gui::Label(filesize,220,i*20,280);
	}
	return w;
}

Gui::Window gui_file_dialog(Gui::String title="Load File",
			Gui::String opentext="Open",
			Gui::String canceltext="Cancel",
			Gui::String path="C:\\",
			Gui::String extension="",
			float pos_x=100,float pos_y=100,
			void (*callback_open)(Gui::Window *,Gui::Button* ,int )=0 )
{
	Gui::Window w=Gui::Window(title,pos_x,pos_y,340,410,Gui::Window::TITLEBAR|Gui::Window::CLOSEBUTTON|Gui::Window::MOVE_TO_FRONT);

	w.skin=gui.global.skin["file_dialog"];
	w.pad_right=0; w.pad_up=0;

	// Ok + Cancel Gui::Buttons + Callback
	w.button["ok"]=Gui::Button(opentext ,20,360,80);
	if(callback_open)w.button["ok"].callback_pressed=callback_open;

	w.button["cancel"]=Gui::Button(canceltext,240,360,80);	
	w.button["cancel"].callback_pressed=[](Gui::Window *wp,Gui::Button* ,int )
	{
		((Gui::Window*) wp)->flags=Gui::Window::CLOSED; 			
	}; 

	w.label["dir"]=Gui::Label(path,20,40,300,20);
	w.textedit["Filename"]=Gui::TextEdit(30,Gui::String("*")+extension,20,320,300);
	w.textedit["Filename"].var.string["dir"]=path;
	w.window["list"]=gui_file_list_window(path,extension);

	return w;
}
