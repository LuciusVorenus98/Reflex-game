#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include "image.h"

#define pi M_PI

static int window_width, window_height;
static int timer_active;
static int height[9];
static int active[9];
static int life;
static int score;
static int high_score;
static int level[7];
static int pointer;
static float theta;
static float delta_theta;
static int timer_rotate1_active;
static int timer_rotate2_active;
static int first_rotate_active;
static int mistake;
static int levelup;
static float camerap;
static GLuint names[5];

static void on_reshape(int width, int height){
	window_width = width;
	window_height = height;
}

//change highscore.txt file
static void highscore(){
	FILE * fp;
	fp = fopen ("highscore.txt", "r+");
	int hs;
	fscanf(fp, "%d", &hs);
	fclose(fp);
	if(high_score > hs){
		fp = fopen("highscore.txt", "w+");
		fclose(fp);
		fp = fopen("highscore.txt", "r+");
		fprintf(fp, "%d", high_score);
		fclose(fp);
	}
}

//auxiliary function for initialization of textures
static void tex_init(int i, Image * image){
    char filename[10] = "tex";
    char ind[2];
    sprintf(ind, "%d", i);
    strcat(filename, ind);
    strcat(filename, ".bmp");
    
    image_read(image, filename);
    
    glBindTexture(GL_TEXTURE_2D, names[i]);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                 image->width, image->height, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, image->pixels);
}

//initialization of textures
static void initialize(void){
    Image * image;

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_TEXTURE_2D);

    glTexEnvf(GL_TEXTURE_ENV,
              GL_TEXTURE_ENV_MODE,
              GL_REPLACE);

    image = image_init(0, 0);
	glGenTextures(5, names);
	
	for(int i = 1; i < 5; i++){
		tex_init(i, image);
	}
                 
    glBindTexture(GL_TEXTURE_2D, 0);
	
    image_done(image);
}

//change texture on levelup
static void on_timer_levelup(int value){
	if(value != 0)
		return;
	levelup = 0;
}

//change texutre on mistake
static void on_timer_mistake(int value){
	if(value != 0)
		return;
	mistake = 0;
}

//random rotation of camera
static void on_timer_rotate(int value){
	if(value != 0)
		return;
	if(timer_rotate1_active)
		theta += delta_theta;
	else if(timer_rotate2_active)
		theta -= delta_theta;
	glutPostRedisplay();
	if(timer_rotate1_active == 1 || timer_rotate2_active == 1)
		glutTimerFunc(50, on_timer_rotate, 0);
}

//animation at the start
static void on_timer_camera(int value){
	if(value != 0)
		return;
	camerap += 0.0005;
	glutPostRedisplay();
	if(camerap < 1)
		glutTimerFunc(50, on_timer_camera, 0);
	else
		camerap = 1;
}

static void calculate_score(){
	if(score > high_score)
		high_score = score;
	score -= 10;
	if(pointer != 0)
		pointer--;
	mistake = 1;
}

static void check_height(int i){
	if(height[i] == 6){
		height[i] = 0;
		active[i] = 0;
		life--;
		if(score > high_score)
			high_score = score;
		score -= 10;
		if(pointer != 0)
			pointer--;
		mistake = 1;
		printf("Broj zivota: %d\n", life);
	}
}

static void check_end(){
	if(life <= 0){
		int i;
		for(i=0; i<5; i++)
			printf("\n");
		printf("Igra je zavrsena!\n");
		printf("High-score: %d\n", high_score);
		printf("Nivo: %d\n", pointer+1);
		if(score > high_score)
			high_score = score;
		highscore();
		exit(0);
	}
}

static void pressed_button(int i){
	height[i] = 0;
	if(active[i] == 0){
		life--;
		printf("Broj zivota: %d\n", life);
		check_end();
		calculate_score();
	}
	else{
		score++;
		printf("SCORE: %d\n",score);
	}
	active[i] = 0;
	glutPostRedisplay();
}

static void on_timer(int value){
	if(value != 0)
		return;
		
	time_t t;
	srand((unsigned) time(&t));
	
	//generate pseudo random number for changing camera rotation direction
	int switcher = rand();
	switcher = switcher % 2;
	
	check_end();
	
	//level up
	if(score > (pointer+1)*10 && pointer != 6){
		pointer++;
		levelup = 1;
		glutPostRedisplay();
	}
	
	//turn on camera rotation after first level	
	if(pointer >= 1 && first_rotate_active == 0){
		timer_rotate1_active = 1;
		first_rotate_active = 1;
		glutTimerFunc(50, on_timer_rotate, 0);
	}

	//change texture on level up	
	if(levelup){
		glutPostRedisplay();
		glutTimerFunc(500, on_timer_levelup, 0);
	}
	
	//change texutre on mistake
	if(mistake){
		glutPostRedisplay();
		glutTimerFunc(500, on_timer_mistake, 0);
	}
	
	//change camera rotation direction	
	if(timer_rotate1_active && switcher == 0){
		timer_rotate1_active = 0;
		timer_rotate2_active = 1;
	}
	else if(timer_rotate2_active && switcher == 0){
		timer_rotate1_active = 1;
		timer_rotate2_active = 0;
	}
	
	int num = rand();
	num = num % 9;
	
	while( (num == 0 && active[0] == 1) || (num == 1 && active[1] == 1) ||
		(num == 2 && active[2] == 1) || (num == 3 && active[2] == 1) ||
		(num == 4 && active[4] == 1) || (num == 5 && active[3] == 1) ||
		(num == 6 && active[6] == 1) || (num == 7 && active[4] == 1) ||
		(num == 8 && active[8] == 1) ){
			num = rand();
			num = num % 9;
	}
	
	//increasing height of active pole
	for(int i = 0; i < 9; i++){
		if(active[i])
			height[i]++;
	}
	
	//increasing new random pole
	for(int i = 0; i < 9; i++){
		if(num == i && active[i] == 0){
			height[i]++;
			active[i] = 1;
			break;
		}
	}
	
	//check max height of a pole
	for(int i = 0; i < 9; i++)
		check_height(i);
	
	check_end();
	
	glutPostRedisplay();
	
	if(timer_active)
		glutTimerFunc(level[pointer], on_timer, 0);
}

static void on_keyboard(unsigned char key, int x, int y){
	switch(key){
		case 27:
			highscore();
			exit(0);
			break;
		case 'g':
		case 'G':
			if(!timer_active){
				timer_active = 1;
				glutTimerFunc(level[pointer], on_timer, 0);
			}
			glutPostRedisplay();
			break;
		case 's':
		case 'S':
			timer_active = 0;
			glutPostRedisplay();
			break;
		case '1':
			pressed_button(0);
			break;
		case '2':
			pressed_button(1);
			break;
		case '3':
			pressed_button(2);
			break;
		case '4':
			pressed_button(3);
			break;
		case '5':
			pressed_button(4);
			break;
		case '6':
			pressed_button(5);
			break;
		case '7':
			pressed_button(6);
			break;
		case '8':
			pressed_button(7);
			break;
		case '9':
			pressed_button(8);
			break;
	}
}

static void on_display(void){
	//setting the light
	GLfloat light_position[] = { 10*sin(theta)+5, 40-20*camerap, 10*cos(theta)+5+5*sin(theta), 0 };
    GLfloat light_ambient[] = { 0.2, 0.2, 0.2, 1 };
    GLfloat light_diffuse[] = { 0.8, 0.8, 0.8, 1 };
    GLfloat light_specular[] = { 0.2, 0.2, 0.2, 1 };
    
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, window_width, window_height);
	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(
            60,
            window_width/(float)window_height,
            1, 150);
	
	glutTimerFunc(20, on_timer_camera, 0);
	
	//starting animation and camera animation
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(
		10*sin(theta)+5, 40-20*camerap, 10*cos(theta)+5+5*sin(theta),
		5, 0, 5,
		0, 1, 0
    );
    
    //drawing poles
    glColor3f(0, 0, 1);
    glPushMatrix();
		glTranslatef(1, 3, 1);
		glScalef(1, height[6]+3, 1);
		glutSolidCube(2);
    glPopMatrix();
    
    glPushMatrix();
		glTranslatef(5, 3, 1);
		glScalef(1, height[7]+3, 1);
		glutSolidCube(2);
	glPopMatrix();
    
    glPushMatrix();
		glTranslatef(9, 3, 1);
		glScalef(1, height[8]+3, 1);
		glutSolidCube(2);
	glPopMatrix();
	
    glColor3f(1, 1, 0);
    glPushMatrix();
		glTranslatef(1, 3, 5);
		glScalef(1, height[3]+3, 1);
		glutSolidCube(2);
    glPopMatrix();
    
    glPushMatrix();
		glTranslatef(5, 3, 5);
		glScalef(1, height[4]+3, 1);
		glutSolidCube(2);
    glPopMatrix();
    
    glPushMatrix();
		glTranslatef(9, 3, 5);
		glScalef(1, height[5]+3, 1);
		glutSolidCube(2);
    glPopMatrix();
	
	glColor3f(0, 1, 0);
	glPushMatrix();
		glTranslatef(1, 3, 9);
		glScalef(1, height[0]+3, 1);
		glutSolidCube(2);
    glPopMatrix();
	
	glPushMatrix();
		glTranslatef(5, 3, 9);
		glScalef(1, height[1]+3, 1);
		glutSolidCube(2);
    glPopMatrix();
    
    glPushMatrix();
		glTranslatef(9, 3, 9);
		glScalef(1, height[2]+3, 1);
		glutSolidCube(2);
    glPopMatrix();
    
    //crtanje postolja
    glColor3f(0.12, 0, 0);
    glPushMatrix();
    	glTranslatef(5,0,5);
    	glScalef(1,0.4,1);
    	glutSolidCube(16);
    glPopMatrix();
    
    //setting texture for a stand
    glBindTexture(GL_TEXTURE_2D, names[1]);
    glBegin(GL_QUADS);
        glNormal3f(0, 1, 0);

        glTexCoord2f(0.1, 0.1);
        glVertex3f(-3, 3.21, 13);

        glTexCoord2f(1, 0.1);
        glVertex3f(13, 3.21, 13);

        glTexCoord2f(1, 1);
        glVertex3f(13, 3.21, -3);

        glTexCoord2f(0.1, 1);
        glVertex3f(-3, 3.21, -3);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
	
	//setting texture for a background
	glBindTexture(GL_TEXTURE_2D, names[2]);
    
    glBegin(GL_QUADS);
        glNormal3f(0, 1, 0);

        glTexCoord2f(0, 0);
        glVertex3f(-200, 0, 200);

        glTexCoord2f(20, 0);
        glVertex3f(200, 0, 200);

        glTexCoord2f(20, 20);
        glVertex3f(200, 0, -200);

        glTexCoord2f(0, 20);
        glVertex3f(-200, 0, -200);
    glEnd();
    
    //setting texture for mistake
    if(mistake){
		glBindTexture(GL_TEXTURE_2D, names[3]);
		
		glBegin(GL_QUADS);
		    glNormal3f(0, 1, 0);

		    glTexCoord2f(0, 0);
		    glVertex3f(-200, 0.1, 200);

		    glTexCoord2f(20, 0);
		    glVertex3f(200, 0.1, 200);

		    glTexCoord2f(20, 20);
		    glVertex3f(200, 0.1, -200);

		    glTexCoord2f(0, 20);
		    glVertex3f(-200, 0.1, -200);
		glEnd();
	}
	
	//setting texture for level up
	if(levelup){
		glBindTexture(GL_TEXTURE_2D, names[4]);
		
		glBegin(GL_QUADS);
		    glNormal3f(0, 1, 0);

		    glTexCoord2f(0, 0);
		    glVertex3f(-200, 0.1, 200);

		    glTexCoord2f(20, 0);
		    glVertex3f(200, 0.1, 200);

		    glTexCoord2f(20, 20);
		    glVertex3f(200, 0.1, -200);

		    glTexCoord2f(0, 20);
		    glVertex3f(-200, 0.1, -200);
		glEnd();
	}
    glBindTexture(GL_TEXTURE_2D, 0);
    
	glutSwapBuffers();
}

int main(int argc, char** argv){
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(800,800);
	glutInitWindowPosition(100,100);
	glutCreateWindow(argv[0]);
	glutFullScreen();
	
	timer_active = 0;
	timer_rotate1_active = 0;
	timer_rotate2_active = 0;
	first_rotate_active = 0;
	camerap = 0;
	mistake = 0;
	levelup = 0;
	score = 0;
	high_score = 0;
	pointer = 0;
	theta = 0;
	delta_theta = pi / 90;
	level[0] = 1200;
	level[1] = 1000;
	level[2] = 800;
	level[3] = 600;
	level[4] = 500;
	level[5] = 400;
	level[6] = 350;
	
	glEnable(GL_COLOR_MATERIAL);
	
	glutDisplayFunc(on_display);
	glutReshapeFunc(on_reshape);
	glutKeyboardFunc(on_keyboard);
	glClearColor(0, 0, 0, 0);
	glEnable(GL_DEPTH_TEST);
	glLineWidth(3);
	life = 3;
	initialize();
	glutMainLoop();
	return 0;
}
