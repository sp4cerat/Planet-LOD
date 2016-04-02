class Shader
{
public:
	Shader(){};
	Shader(std::string shadername)
	{
		printf("Shader %s\n",shadername.c_str());
		name=shadername;

		int type[5]=
		{
			GL_VERTEX_SHADER,
			GL_FRAGMENT_SHADER,
			GL_TESS_CONTROL_SHADER,
			GL_TESS_EVALUATION_SHADER,
			GL_GEOMETRY_SHADER
		};
		char extension[5][10]=
		{
			"/vs.txt",
			"/frag.txt",
			"/tcs.txt",
			"/tes.txt",
			"/geo.txt"
		};
		for (int i=0;i<5;i++) 
		{
			char filename[1000];
			memset(filename,0,1000);
			sprintf(filename,"%s%s",shadername.c_str(),extension[i]);
			attach(type[i],filename);
		};
		link();
	};
	void attach(int type,char* filename)
	{
		char* mem=read_file(filename);
		if(mem==0)return;
		GLuint handle = glCreateShader(type);
		glShaderSource(handle, 1, (const GLchar**)(&mem), 0);
		ogl_check_error();
		glCompileShader(handle);
		ogl_check_error();
		free(mem);

		GLint compileSuccess=0;
		GLchar compilerSpew[256];

		glGetShaderiv(handle, GL_COMPILE_STATUS, &compileSuccess);
		ogl_check_error();
		if(!compileSuccess)
		{
			glGetShaderInfoLog(handle, sizeof(compilerSpew), 0, compilerSpew);

			error_stop("Shader %s\n%s\ncompileSuccess=%d\n",filename,compilerSpew,compileSuccess);
		}
		handles.push_back(handle);
		//free(mem);
	}
	void link()
	{
		program_handle = glCreateProgram();
		for (int i=0;i<handles.size();i++)
		{
			glAttachShader(program_handle, handles[i]);
			ogl_check_error();
		}

		glLinkProgram(program_handle);
		ogl_check_error();

		GLint linkSuccess;
		GLchar compilerSpew[256];
		glGetProgramiv(program_handle, GL_LINK_STATUS, &linkSuccess);
		if(!linkSuccess)
		{
			glGetProgramInfoLog(program_handle, sizeof(compilerSpew), 0, compilerSpew);
			error_stop("Shader Linker:\n%s\nlinkSuccess=%d",compilerSpew,linkSuccess);
		}
		printf("%s linked successful\n",name.c_str());
		ogl_check_error();
	}
	void setUniformMatrix4fv(char* varname, GLsizei count, GLboolean transpose, GLfloat *value)
	{
		GLint loc = glGetUniformLocation(program_handle,varname);
		if (loc==-1) 
		{
			error_stop("Variable \"%s\" in shader \"%s\" not found\n",varname,name.c_str());
		};
		glUniformMatrix4fv(loc, count, transpose, value);
		ogl_check_error();
	}
	void setUniform1i(char* varname, GLint v0)
	{ 
		GLint loc = glGetUniformLocation(program_handle,varname);
		if (loc==-1) 
		{
			error_stop("Variable \"%s\" in shader \"%s\" not found\n",varname,name.c_str());
		};
		glUniform1i(loc, v0);
		ogl_check_error();
	}
	void setUniform1f(char* varname, GLfloat v0)
	{ 
		GLint loc = glGetUniformLocation(program_handle,varname);
		if (loc==-1) 
		{
			error_stop("Variable \"%s\" in shader \"%s\" not found\n",varname,name.c_str());
		};
		glUniform1f(loc, v0);
		ogl_check_error();
	}
	void setUniform2f(char* varname, GLfloat v0, GLfloat v1)
	{ 
		GLint loc = glGetUniformLocation(program_handle,varname);
		if (loc==-1) 
		{
			error_stop("Variable \"%s\" in shader \"%s\" not found\n",varname,name.c_str());
		};
		glUniform2f(loc, v0, v1);
		ogl_check_error();
	}
	void setUniform3f(char* varname, GLfloat v0, GLfloat v1, GLfloat v2)
	{ 
		GLint loc = glGetUniformLocation(program_handle,varname);
		if (loc==-1) 
		{
			error_stop("Variable \"%s\" in shader \"%s\" not found\n",varname,name.c_str());
		};
		glUniform3f(loc, v0, v1, v2);
		ogl_check_error();
	}
	void setUniform4f(char* varname, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
	{ 
		GLint loc = glGetUniformLocation(program_handle,varname);
		if (loc==-1) 
		{
			error_stop("Variable \"%s\" in shader \"%s\" not found\n",varname,name.c_str());
		};
		glUniform4f(loc, v0, v1, v2, v3);
		ogl_check_error();
	}
	void setUniformMatrix4fv(char* varname, int count ,float* val)
	{ 
		GLint loc = glGetUniformLocation(program_handle,varname);
		if (loc==-1) 
		{
			error_stop("Variable \"%s\" in shader \"%s\" not found\n",varname,name.c_str());
		};
		glUniformMatrix4fv(loc, count, GL_FALSE, val); 
		ogl_check_error();
	}
	
	void begin(void)
	{
		glUseProgram(program_handle);
		ogl_check_error();
	}
	void end(void)
	{
		glUseProgram(0);
		ogl_check_error();
	}
	GLint get_loc(char* varname)
	{
		return glGetUniformLocation(program_handle,varname);
	}

	GLuint program_handle;
private:
	std::vector<GLuint> handles;
	std::string name;

	char* read_file(char* fname)
	{
		if(fname==NULL)
		{
			printf ("GLSL read_file 0 pointer\n");
			return 0;
		}
		else
		{
			printf ("GLSL Loading %s\n",fname);
		}

		FILE * fp = fopen (fname, "rb");
		
		if (fp==0) 
		{
			printf ("File %s NOT FOUND\n",fname);
			return 0;
		}
		fseek(fp, 0L, SEEK_END);
		int fsize = ftell(fp);
		fseek(fp, 0L, SEEK_SET);
		char* mem=(char*)malloc(fsize+1);
		for(int i=0;i<fsize+1;i++)mem[i]=0;
		fread (mem, 1, fsize, fp);
		fclose (fp);
		return mem;
	}
};
