class FBO {

	public:

	enum Type { COLOR=1 , DEPTH=2 }; // Bits

	int color_tex;
	int color_bpp;
	int depth_tex;
	int depth_bpp;
	//int depth_rb;

	Type type;

	int width;
	int height;

	int tmp_viewport[4];

	
	GLuint fbo; // frame buffer object ref
	GLuint dbo; // depth buffer object ref

	FBO (int texWidth,int texHeight, int format=GL_RGBA,int datatype=GL_UNSIGNED_BYTE)
	{
		color_tex = -1;
		depth_tex = -1;
		fbo = -1;
		dbo = -1;
		init (texWidth, texHeight,format,datatype);
	}
	~FBO()
	{
		clear ();
	}

	void clear ()
	{		
		if(color_tex!=-1)
		{
			// destroy objects
			
			glDeleteTextures(1, (GLuint*)&color_tex);
			glDeleteTextures(1, (GLuint*)&depth_tex);
			glDeleteRenderbuffersEXT(1, &dbo);
			glDeleteFramebuffersEXT(1, &fbo);
		}
	}

	void enable(int customwidth=-1,int customheight=-1)
	{
		//glBindTexture(GL_TEXTURE_2D, 0);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, dbo);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, 
			GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, color_tex, 0);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, 
			GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, depth_tex, 0);

		glGetIntegerv(GL_VIEWPORT, tmp_viewport);

		if(customwidth>0)
			glViewport(0, 0, customwidth, customheight);		// Reset The Current Viewport And Perspective Transformation
		else
			glViewport(0, 0, width, height);		// Reset The Current Viewport And Perspective Transformation

		//glMatrixMode(GL_PROJECTION);
		//glPushMatrix();
		//glLoadIdentity();
		//gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);
		//glMatrixMode(GL_MODELVIEW);

	}

	void disable()
	{
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
		glViewport(
			tmp_viewport[0],
			tmp_viewport[1],
			tmp_viewport[2],
			tmp_viewport[3]);
		//glMatrixMode(GL_PROJECTION);
		//glPopMatrix();
		//glMatrixMode(GL_MODELVIEW);
	}

	void init (int texWidth,int texHeight, int format=GL_RGBA,int datatype=GL_UNSIGNED_BYTE)//,Type type = Type(COLOR | DEPTH),int color_bpp=32,int depth_bpp=24)
	{
	//	clear ();
		this->width = texWidth;
		this->height = texHeight;

		glGenFramebuffersEXT(1, &fbo); 
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);  

		/*
		glGenRenderbuffersEXT(1, &rb_color);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, rb_color);
		glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_RGBA8, width, height);
		// Attach it as color attachment to the FBO
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, 
		GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER_EXT, rb_color);
		*/
		ogl_check_error();

		// init texture
		glGenTextures(1, (GLuint*)&color_tex);
		glBindTexture(GL_TEXTURE_2D, color_tex);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 
		//	texWidth, texHeight, 0, 
		//	GL_RGBA, GL_FLOAT, NULL);

		glTexImage2D(GL_TEXTURE_2D, 0, format, texWidth, texHeight, 0,
				format, datatype, NULL);

		//GL_TEXTURE_2D,GL_RGBA, bmp.width, bmp.height,
		//	/*GL_RGBA*/GL_BGRA_EXT, GL_UNSIGNED_BYTE, bmp.data );

		ogl_check_error();
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2DEXT(
			GL_FRAMEBUFFER_EXT, 
			GL_COLOR_ATTACHMENT0_EXT, 
			GL_TEXTURE_2D, color_tex, 0);
		ogl_check_error();

		glGenRenderbuffersEXT(1, &dbo);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, dbo);
		
			glGenTextures(1, (GLuint*)&depth_tex);
			glBindTexture(GL_TEXTURE_2D, depth_tex);
			//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, 
			//	texWidth, texHeight, 0, 
			//	GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
			ogl_check_error();
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			glTexParameteri (GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTexImage2D (GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, 
				texWidth, texHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, 
								GL_TEXTURE_2D, depth_tex, 0);
								/*

			glGenRenderbuffersEXT(1,  (GLuint*)&depth_rb);
			glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depth_rb);
			glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT32, width, height);
			// Attach it as color attachment to the FBO
			glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, 
				GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depth_rb);*/

			ogl_check_error();
			glBindTexture(GL_TEXTURE_2D, 0);// don't leave this texture bound or fbo (zero) will use it as src, want to use it just as dest GL_DEPTH_ATTACHMENT_EXT
			glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
			ogl_check_error();

		check_framebuffer_status();
		ogl_check_error();
	    
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);

		check_framebuffer_status();
		ogl_check_error();
	}

	private:


	void check_framebuffer_status()
	{
		GLenum status;
		status = (GLenum) glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
		switch(status) {
			case GL_FRAMEBUFFER_COMPLETE_EXT:
				return;
				break;
			case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
				printf("Unsupported framebuffer format\n");
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
				printf("Framebuffer incomplete, missing attachment\n");
				break;
//			case GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT:
//				printf("Framebuffer incomplete, duplicate attachment\n");
//				break;
			case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
				printf("Framebuffer incomplete, attached images must have same dimensions\n");
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
				printf("Framebuffer incomplete, attached images must have same format\n");
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
				printf("Framebuffer incomplete, missing draw buffer\n");
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
				printf("Framebuffer incomplete, missing read buffer\n");
				break;
			case 0:
				printf("Not ok but trying...\n");
				return;
				break;
			default:;
				printf("Framebuffer error code %d\n",status);
				break;
		};
		printf("Programm Stopped!\n");
		while(1)Sleep(100);;
	}
};
