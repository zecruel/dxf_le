#include <stddef.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#define MAX_TRIANG 500
#define TOLERANCE 1e-6
#define MAX_SCAN_LINES 1000
#define MAX_P_NODES 1000

#define GLSL(src) "#version 150 core\n" #src

#if(0)
/* mantain one unique element in a sorted array */
int unique (int n, float * a) {
	int dst = 0, i;
	for (i = 1; i < n; ++i) {
	if (fabs (a[dst] - a[i]) > TOLERANCE)
		a[++dst] = a[i];
	}
	return dst + 1;
}

/* comparation function for qsort */
int cmpfunc (const void * a, const void * b) {
	float dif = *(float*)a - *(float*)b;
	if (fabs(dif) < TOLERANCE) return 0;
	else if (dif > 0.0) return 1;
	else return -1;
}
#endif

/* mantain one unique element in a sorted array */
int unique (int n, int * a) {
	int dst = 0, i;
	for (i = 1; i < n; ++i) {
	if (a[dst] != a[i])
		a[++dst] = a[i];
	}
	return dst + 1;
}

/* comparation function for qsort */
int cmpfunc (const void * a, const void * b) {
	return (*(int*)a - *(int*)b);
}

struct Vertex {
    GLfloat pos[3];
    GLfloat uv[2];
    GLubyte col[4];
};

struct ogl {
	struct Vertex *verts;
	GLuint *elems;
	int vert_count;
	int elem_count;
	
	int win_w, win_h, flip_y;
	GLubyte fg[4], bg[4]; /*foreground and background colors */
};

struct edge {
	int x0, y0, x1, y1;
};

struct p_node {
	int value;
	struct edge * edge;
};

/* comparation function for qsort */
int cmp_node (const void * a, const void * b) {
	struct p_node *aa, *bb;
	aa = (struct p_node *) a; bb = (struct p_node *) b;
	return (aa->value - bb->value);
}

int line_gl (struct ogl *gl_ctx, int p0[2], int p1[2], int thick){
	if (!gl_ctx) return 0;
	if (!gl_ctx->verts) return 0;
	if (!gl_ctx->elems) return 0;
	
	/* get polar parameters of line */
	float dx = p1[0] - p0[0];
	float dy = p1[1] - p0[1];
	float modulus = sqrt(pow(dx, 2) + pow(dy, 2));
	float cosine = 1.0;
	float sine = 0.0;
	
	if (modulus > TOLERANCE){
		cosine = dx/modulus;
		sine = dy/modulus;
	}
	
	int j = 0;
	
	if (thick <= 0) thick = 1;
	
	float tx = (float) thick / gl_ctx->win_w;
	float ty = (float) thick / gl_ctx->win_h;
		
	j = gl_ctx->vert_count;
	
	float x0 = ((float) p0[0] / gl_ctx->win_w) * 2.0 - 1.0;
	float y0 = ((float) p0[1] / gl_ctx->win_h) * 2.0 - 1.0;
	float x1 = ((float) p1[0] / gl_ctx->win_w) * 2.0 - 1.0;
	float y1 = ((float) p1[1] / gl_ctx->win_h) * 2.0 - 1.0;
	
	float flip_y = (gl_ctx->flip_y) ? -1.0 : 1.0;
	
	gl_ctx->verts[j].pos[0] = x0 - sine * tx;
	gl_ctx->verts[j].pos[1] = flip_y * (y0 + cosine * ty);
	gl_ctx->verts[j].pos[2] = 0.0;
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0]= 0.0;
	gl_ctx->verts[j].uv[1]= (float)(gl_ctx->flip_y);
	gl_ctx->vert_count ++;
	
	j = gl_ctx->vert_count;
	
	gl_ctx->verts[j].pos[0] = x0 + sine * tx;
	gl_ctx->verts[j].pos[1] = flip_y * (y0 - cosine * ty);
	gl_ctx->verts[j].pos[2] = 0.0;
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0]= 0.0;
	gl_ctx->verts[j].uv[1]= (float)(1 - gl_ctx->flip_y);
	gl_ctx->vert_count ++;
	
	j = gl_ctx->vert_count;
	
	gl_ctx->verts[j].pos[0] = x1 - sine * tx;
	gl_ctx->verts[j].pos[1] = flip_y * (y1 + cosine * ty);
	gl_ctx->verts[j].pos[2] = 0.0;
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0]= 1.0;
	gl_ctx->verts[j].uv[1]= (float)(gl_ctx->flip_y);
	gl_ctx->vert_count ++;
	
	j = gl_ctx->vert_count;
	
	gl_ctx->verts[j].pos[0] = x1 + sine * tx;
	gl_ctx->verts[j].pos[1] = flip_y * (y1 - cosine * ty);
	gl_ctx->verts[j].pos[2] = 0.0;
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0]= 1.0;
	gl_ctx->verts[j].uv[1]= (float)(1 - gl_ctx->flip_y);
	gl_ctx->vert_count ++;
	
	j = gl_ctx->elem_count * 3;
	
	gl_ctx->elems[j] = gl_ctx->vert_count - 4;
	gl_ctx->elems[j+1] = gl_ctx->vert_count - 3;
	gl_ctx->elems[j+2] = gl_ctx->vert_count - 2;
	gl_ctx->elems[j+3] = gl_ctx->vert_count - 3;
	gl_ctx->elems[j+4] = gl_ctx->vert_count - 2;
	gl_ctx->elems[j+5] = gl_ctx->vert_count - 1;
	
	gl_ctx->elem_count+= 2;
	
	return 1;
}

int trap_gl (struct ogl *gl_ctx, int tl[2], int bl[2], int tr[2], int br[2]){
	if (!gl_ctx) return 0;
	if (!gl_ctx->verts) return 0;
	if (!gl_ctx->elems) return 0;
		
	int j = gl_ctx->vert_count;
	
	float flip_y = (gl_ctx->flip_y) ? -1.0 : 1.0;
	
	gl_ctx->verts[j].pos[0] = ((float) bl[0] / gl_ctx->win_w) * 2.0 - 1.0;
	gl_ctx->verts[j].pos[1] = flip_y * (((float) bl[1] / gl_ctx->win_h) * 2.0 - 1.0);
	gl_ctx->verts[j].pos[2] = 0.0;
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0]= 0.0;
	gl_ctx->verts[j].uv[1]= (float)(gl_ctx->flip_y);
	gl_ctx->vert_count ++;
	
	j = gl_ctx->vert_count;
	
	gl_ctx->verts[j].pos[0] = ((float) tl[0] / gl_ctx->win_w) * 2.0 - 1.0;
	gl_ctx->verts[j].pos[1] = flip_y * (((float) tl[1] / gl_ctx->win_h) * 2.0 - 1.0);
	gl_ctx->verts[j].pos[2] = 0.0;
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0]= 0.0;
	gl_ctx->verts[j].uv[1]= (float)(1 - gl_ctx->flip_y);
	gl_ctx->vert_count ++;
	
	j = gl_ctx->vert_count;
	
	gl_ctx->verts[j].pos[0] = ((float) br[0] / gl_ctx->win_w) * 2.0 - 1.0;
	gl_ctx->verts[j].pos[1] = flip_y * (((float) br[1] / gl_ctx->win_h) * 2.0 - 1.0);
	gl_ctx->verts[j].pos[2] = 0.0;
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0]= 1.0;
	gl_ctx->verts[j].uv[1]= (float)(gl_ctx->flip_y);
	gl_ctx->vert_count ++;
	
	j = gl_ctx->vert_count;
	
	gl_ctx->verts[j].pos[0] = ((float) tr[0] / gl_ctx->win_w) * 2.0 - 1.0;
	gl_ctx->verts[j].pos[1] = flip_y * (((float) tr[1] / gl_ctx->win_h) * 2.0 - 1.0);
	gl_ctx->verts[j].pos[2] = 0.0;
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0]= 1.0;
	gl_ctx->verts[j].uv[1]= (float)(1 - gl_ctx->flip_y);
	gl_ctx->vert_count ++;
	
	j = gl_ctx->elem_count * 3;
	
	gl_ctx->elems[j] = gl_ctx->vert_count - 4;
	gl_ctx->elems[j+1] = gl_ctx->vert_count - 3;
	gl_ctx->elems[j+2] = gl_ctx->vert_count - 2;
	gl_ctx->elems[j+3] = gl_ctx->vert_count - 3;
	gl_ctx->elems[j+4] = gl_ctx->vert_count - 2;
	gl_ctx->elems[j+5] = gl_ctx->vert_count - 1;
	
	gl_ctx->elem_count+= 2;
	
	return 1;
}

// To find orientation of ordered triplet (p, q, r).
// The function returns following values
// 0 --> p, q and r are colinear
// 1 --> Clockwise
// 2 --> Counterclockwise
int orientation(int p[2], int q[2], int r[2])
{
    // See https://www.geeksforgeeks.org/orientation-3-ordered-points/
    // for details of below formula.
    int val = (q[1] - p[1]) * (r[0] - q[0]) -
              (q[0] - p[0]) * (r[1] - q[1]);
  
    if (val == 0) return 0;  // colinear
  
    return (val > 0)? 1: 2; // clock or counterclock wise
}

int polygon_gl (struct ogl *gl_ctx, int n, struct edge edges[]){
	if (!gl_ctx) return 0;
	if (!gl_ctx->verts) return 0;
	if (!gl_ctx->elems) return 0;
	
	if (2 * n > MAX_SCAN_LINES) return 0;
	
	int scan_lines[MAX_SCAN_LINES];
	struct p_node nodes1[MAX_P_NODES], nodes2[MAX_P_NODES];
	
	int i = 0, j = 0, k = 0, scan_count = 0, n1 = 0, n2 = 0;
	int trap_x[MAX_P_NODES][4];
	int trap_count = 0;
	
	for (i = 0; i < n; i++){
		scan_lines[scan_count] = edges[i].y0;
		scan_count++;
		scan_lines[scan_count] = edges[i].y1;
		scan_count++;
	}
	qsort(scan_lines, scan_count, sizeof(int), cmpfunc);
	
	scan_count = unique (scan_count, scan_lines);
	
	for (i = 1; i < scan_count; i++){
		
		n1 = 0;
		n2 = 0;
		trap_count = 0;
		
		for(j = 0; j < n; j++){
			if (((edges[j].y0 <= scan_lines[i - 1] && edges[j].y1 >= scan_lines[i - 1]) || 
			(edges[j].y1 <= scan_lines[i - 1] && edges[j].y0 >= scan_lines[i - 1])) &&
			((edges[j].y0 <= scan_lines[i] && edges[j].y1 >= scan_lines[i]) || 
			(edges[j].y1 <= scan_lines[i ] && edges[j].y0 >= scan_lines[i]))){
				if (edges[j].y0 == scan_lines[i - 1]) {
					nodes1[n1].value = edges[j].x0;
					nodes1[n1].edge = &edges[j];
					n1++;
				}
				else if (edges[j].y1 == scan_lines[i-1]) {
					nodes1[n1].value = edges[j].x1;
					nodes1[n1].edge = &edges[j];
					n1++;
				}
				else {
					nodes1[n1].value = (int) round(edges[j].x1 + (double) (scan_lines[i - 1] - edges[j].y1)/(edges[j].y0 - edges[j].y1)*(edges[j].x0 - edges[j].x1));
					nodes1[n1].edge = &edges[j];
					n1++;
				}
				
				if (edges[j].y0 == scan_lines[i]) {
					nodes2[n2].value = edges[j].x0;
					nodes2[n2].edge = &edges[j];
					n2++;
				}
				else if (edges[j].y1 == scan_lines[i]) {
					nodes2[n2].value = edges[j].x1;
					nodes2[n2].edge = &edges[j];
					n2++;
				}
				else {
					nodes2[n2].value = (int) round(edges[j].x1 + (double) (scan_lines[i] - edges[j].y1)/(edges[j].y0 - edges[j].y1)*(edges[j].x0 - edges[j].x1));
					nodes2[n2].edge = &edges[j];
					n2++;
				}
			}
		}
		
		qsort(nodes1, n1, sizeof(struct p_node), cmp_node);
		qsort(nodes2, n2, sizeof(struct p_node), cmp_node);
		
		int bl = 0, br = n2 - 1;
		int tl = 0, tr = n1 - 1;
		int complete = 0;
		int last_tl = nodes1[n1 - 1].value, last_tr = nodes1[0].value;
		int last_bl = nodes2[n2 - 1].value, last_br = nodes2[0].value;
		int valid_diag = 0;
		
		complete = 0;
		trap_count = 0;
		while (!complete){
			int p1[2], p2[2], q1[2], q2[2], o1, o2, o3, o4;
			valid_diag = 1;
			for (j = 0; j  < n2 - 1; j++){
				p1[0] = nodes2[j].edge->x0;
				p1[1] = nodes2[j].edge->y0;
				q1[0] = nodes2[j].edge->x1;
				q1[1] = nodes2[j].edge->y1;
				
				p2[0] = nodes1[tl].value;
				p2[1] = scan_lines[i-1];
				q2[0] = nodes2[br].value;
				q2[1] = scan_lines[i];
				
				o1 = orientation(p1, q1, p2);
				o2 = orientation(p1, q1, q2);
				o3 = orientation(p2, q2, p1);
				o4 = orientation(p2, q2, q1);

				// General case
				if (o1 > 0 && o2 > 0 && o1 != o2 && o3 != o4){
					if (j  >= n2 - 2) valid_diag = 0;
					j = 0;
					br--;
				}
			}
			
			if (valid_diag && (last_tl != nodes1[tl].value || last_br != nodes2[br].value)){
				//printf("d(%d,%d) ", nodes1[tl].value, nodes2[br].value);
				
				trap_x[trap_count][0] = nodes1[tl].value;
				trap_x[trap_count][3] = nodes2[br].value;
				trap_count++;
			}
			
			last_tl = nodes1[tl].value;
			last_br = nodes2[br].value;
			
			if(nodes2[br].value == nodes2[n2 - 1].value) complete = 1;
			
			br = n2 - 1;
			tl++;
		}
		
		complete = 0;
		trap_count = 0;
		while (!complete){
			int p1[2], p2[2], q1[2], q2[2], o1, o2, o3, o4;
			valid_diag = 1;
			for (j = 0; j  < n1 - 1; j++){
				p1[0] = nodes1[j].edge->x0;
				p1[1] = nodes1[j].edge->y0;
				q1[0] = nodes1[j].edge->x1;
				q1[1] = nodes1[j].edge->y1;
				
				p2[0] = nodes2[bl].value;
				p2[1] = scan_lines[i];
				q2[0] = nodes1[tr].value;
				q2[1] = scan_lines[i-1];
				
				o1 = orientation(p1, q1, p2);
				o2 = orientation(p1, q1, q2);
				o3 = orientation(p2, q2, p1);
				o4 = orientation(p2, q2, q1);

				// General case
				if (o1 > 0 && o2 > 0 && o1 != o2 && o3 != o4){
					if (j  >= n1 - 2) valid_diag = 0;
					j = 0;
					tr--;
				}
			}
			
			if (valid_diag && (last_tr != nodes1[tr].value || last_bl != nodes2[bl].value)){
				//printf("u(%d,%d) ", nodes2[bl].value, nodes1[tr].value);
				
				trap_x[trap_count][1] = nodes2[bl].value;
				trap_x[trap_count][2] = nodes1[tr].value;
				trap_count++;
			}
			
			last_tr = nodes1[tr].value;
			last_bl = nodes2[bl].value;
			
			if(nodes1[tr].value == nodes1[n1 - 1].value) complete = 1;
			
			tr = n1 - 1;
			bl++;
		}
		
		for (j = 0; j < trap_count; j++){
			//int trap_gl (struct ogl *gl_ctx, int tl[2], int bl[2], int tr[2], int br[2]){
			trap_gl (gl_ctx, (int[]){trap_x[j][0], scan_lines[i-1]},
				(int[]){trap_x[j][1], scan_lines[i]},
				(int[]){trap_x[j][2], scan_lines[i-1]},
				(int[]){trap_x[j][3], scan_lines[i]});
		}
		
		//printf("- y = %d\n", scan_lines[i]);
	}
	
	return 1;
}

int main(int argc, char *argv[])
{
	struct ogl gl_ctx;
	gl_ctx.vert_count = 0;
	gl_ctx.elem_count = 0;
	gl_ctx.verts = NULL;
	gl_ctx.elems = NULL;
	gl_ctx.win_w = 800;
	gl_ctx.win_h = 600;
	gl_ctx.flip_y = 0;
	gl_ctx.fg[0] = 255; gl_ctx.fg[1] = 255; gl_ctx.fg[2] = 255; gl_ctx.fg[3] = 255;
	gl_ctx.bg[0] = 100; gl_ctx.bg[1] = 100; gl_ctx.bg[2] = 100; gl_ctx.bg[3] = 255;
	
	struct edge test_edges[] = {
		{500, 100, 300, 300},
		{300, 300, 300, 600},
		{300, 600, 200, 600},
		{200, 600, 100, 700},
		{100, 700, 500, 900},
		{500, 900, 800, 700},
		{800, 700, 900, 700},
		{900, 700, 900, 600},
		{900, 600, 700, 200},
		{700, 200, 500, 100},
		{600, 300, 500, 400},
		{500, 400, 650, 500},
		{650, 500, 600, 300}
	};
	
	/* buffer setup */
        GLsizei vs = sizeof(struct Vertex);
        size_t vp = offsetof(struct Vertex, pos);
        size_t vt = offsetof(struct Vertex, uv);
        size_t vc = offsetof(struct Vertex, col);
	
	SDL_Init(SDL_INIT_VIDEO);
	
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	
	SDL_Window* window = SDL_CreateWindow("OpenGL", 100, 100, gl_ctx.win_w, gl_ctx.win_h, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
	SDL_GLContext context = SDL_GL_CreateContext(window);
	
	/* Init GLEW */
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	/* Create Vertex Array Object */
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	/* Create a Vertex Buffer Object and copy the vertex data to it */
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, MAX_TRIANG*3*vs, NULL, GL_STREAM_DRAW); //GL_STATIC_DRAW);
	
	/* Create a Element Buffer Object */
	GLuint ebo;
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_TRIANG*3*sizeof(GLuint), NULL,  GL_STREAM_DRAW);//GL_STATIC_DRAW); //GL_STREAM_DRAW
	
	/* Create and compile the vertex shader */
	const char* vertexSource = GLSL(
		in vec3 position;
		in vec2 uv;
		in vec4 color;
		out vec4 vertexColor;
		out vec2 texcoord;
	
		void main() {
			gl_Position = vec4(position, 1.0);
			vertexColor = color;
			texcoord = uv;
		}
	); /* =========== vertex shader */

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);

	/* Create and compile the fragment shader */
	const char* fragmentSource = GLSL(
		in vec4 vertexColor;
		in vec2 texcoord;
		
		out vec4 outColor;
		
		uniform sampler2D tex;

		void main() {
			outColor = texture(tex, texcoord) * vertexColor;
		}
	); /* ========== fragment shader */

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);

	/* Link the vertex and fragment shader into a shader program */
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glBindFragDataLocation(shaderProgram, 0, "outColor");
	glLinkProgram(shaderProgram);
	glUseProgram(shaderProgram);
		
	/*texture */
	GLuint textures[2];
	glGenTextures(2, textures);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[0]);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	/* blank texture (default) */
	GLubyte blank[] = {255,255,255,255};
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, blank);
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textures[1]);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	// Black/white checkerboard
	GLubyte pixels[] = {
		0,0,0,255,    255,255,255,255,
		255,255,255,255,    0,0,0,255
	};
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	
	glUniform1i(glGetUniformLocation(shaderProgram, "tex"), 1);

	/* Specify the layout of the vertex data */
	GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
	GLint uvAttrib = glGetAttribLocation(shaderProgram, "uv");
	GLint colorAttrib = glGetAttribLocation(shaderProgram, "color");
	
	glEnableVertexAttribArray(posAttrib);
	glEnableVertexAttribArray(uvAttrib);
	glEnableVertexAttribArray(colorAttrib);
	
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, vs, (void*)vp);
        glVertexAttribPointer(uvAttrib, 2, GL_FLOAT, GL_FALSE, vs, (void*)vt);
        glVertexAttribPointer(colorAttrib, 4, GL_UNSIGNED_BYTE, GL_TRUE, vs, (void*)vc);
	
	glEnable(GL_BLEND); 
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	/* load vertices/elements directly into vertex/element buffer */
        gl_ctx.verts = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
        gl_ctx.elems = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
	
	//polygon_gl (&gl_ctx, 13, test_edges);
	
	int i = 0, j;
	SDL_Event windowEvent;
	while (1) {
		
		if (SDL_PollEvent(&windowEvent)) {
			if (windowEvent.type == SDL_QUIT) break;
			if (windowEvent.type == SDL_MOUSEBUTTONDOWN) {
				if (windowEvent.button.button == SDL_BUTTON_LEFT){
					i = !i;
				}
				else if(windowEvent.button.button == SDL_BUTTON_RIGHT){
					gl_ctx.flip_y = !gl_ctx.flip_y;
				}
				
			}
		}
		
		//i = !i;
		glUniform1i(glGetUniformLocation(shaderProgram, "tex"), i);
		
		SDL_GetWindowSize(window, &gl_ctx.win_w, &gl_ctx.win_h);
		glViewport(0, 0, gl_ctx.win_w, gl_ctx.win_h);
		
		gl_ctx.fg[0] = 255; gl_ctx.fg[1] = 0; gl_ctx.fg[2] = 0; gl_ctx.fg[3] = 150;
		line_gl(&gl_ctx, (int[]){5, 5}, (int[]){120, 100}, 2);
		gl_ctx.fg[0] = 255; gl_ctx.fg[1] = 0; gl_ctx.fg[2] = 255; gl_ctx.fg[3] = 150;
		line_gl(&gl_ctx, (int[]){50, 50}, (int[]){800, 600}, 0);
		gl_ctx.fg[0] = 255; gl_ctx.fg[1] = 255; gl_ctx.fg[2] = 0; gl_ctx.fg[3] = 150;
		line_gl(&gl_ctx, (int[]){5, 5}, (int[]){5, 100}, 3);
		gl_ctx.fg[0] = 0; gl_ctx.fg[1] = 0; gl_ctx.fg[2] = 255; gl_ctx.fg[3] = 150;
		line_gl(&gl_ctx, (int[]){800, 0}, (int[]){120, 200}, 20);
		
		gl_ctx.fg[0] = 0; gl_ctx.fg[1] = 255; gl_ctx.fg[2] = 0; gl_ctx.fg[3] = 255;
		for (j = 0; j < 13; j++){
			int p0[2], p1[2];
			p0[0] = test_edges[j].x0;
			p0[1] = test_edges[j].y0;
			p1[0] = test_edges[j].x1;
			p1[1] = test_edges[j].y1;
			
			line_gl(&gl_ctx, p0, p1, 2);
		}
		
		gl_ctx.fg[0] = 0; gl_ctx.fg[1] = 255; gl_ctx.fg[2] = 0; gl_ctx.fg[3] = 100;
		polygon_gl (&gl_ctx, 13, test_edges);
		
		
		
		/* Clear the screen to black */
		glClearColor((GLfloat) gl_ctx.bg[0] /255, (GLfloat) gl_ctx.bg[1] /255, (GLfloat) gl_ctx.bg[2] /255, (GLfloat) gl_ctx.bg[3] /255);
		glClear(GL_COLOR_BUFFER_BIT);

		/* Draw a triangle from the 3 vertices */
		//glDrawArrays(GL_TRIANGLES, 0, 3);
		glDrawElements(GL_TRIANGLES, gl_ctx.elem_count*3, GL_UNSIGNED_INT, 0);
		
		gl_ctx.vert_count = 0;
		gl_ctx.elem_count = 0;

		/* Swap buffers */
		SDL_GL_SwapWindow(window);
	}
	
	glUnmapBuffer(GL_ARRAY_BUFFER);
        glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	
	/* Delete allocated resources */
	glDeleteProgram(shaderProgram);
	glDeleteShader(fragmentShader);
	glDeleteShader(vertexShader);
	glDeleteBuffers(1, &ebo);
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
	
	SDL_GL_DeleteContext(context);
	SDL_Quit();
	return 0;
}