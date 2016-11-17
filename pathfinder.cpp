#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glut.h>
#endif

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <SOIL.h>

#include "pathfinder.h"

bool loadTexture(char *path, GLuint *texture){
    *texture = SOIL_load_OGL_texture(path,
            SOIL_LOAD_AUTO,
            SOIL_CREATE_NEW_ID,
            SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_MULTIPLY_ALPHA
            );
    if(*texture == 0){
        return false;
    } else {
        return true;
    }
}


void PathFinder::set_dest( Direction new_dest ){
	this->init_dest = Dest;
	this->Dest = new_dest;
	ismoving =true;
}

PathFinder::PathFinder(int x_position, int y_position, int maze_width, int maze_height)
{
	old_x = current_x = 20.0 + 10.0 * x_position;
	old_y = current_y = 20.0 + 10.0 * y_position;
	recursion_stack = new int[maze_width * maze_height * 4];	// size of worst case
	stack_top = -1;

	/* initialzing status factor */
	ismoving = false;
	walk_status = 0;
	eye_status = 0;
	rolling_status = 0;
	goal_ceremony_status = 0;
	degree_7 = sin(7 * atan(-1) / 180);	// sin( 7 * PI / 180)
        textureId = 20;
        if (!loadTexture("textures/lava.tga", &textureId)) {
            std::cout << "not found\n";
        }

	lists();
	init_dest = Dest = RIGHT;
}

void PathFinder::lists(){

	glNewList( Body, GL_COMPILE );
            
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, textureId);

            glBegin( GL_POLYGON );
                glEdgeFlag( GL_TRUE );
                    for (float angle = 0; angle < 2 * M_PI; angle = angle + 0.01) {
                        glTexCoord2f(0.5 * cos(angle) + 0.5, 0.5 * sin(angle) + 0.5);
                        glVertex2f(20 * cos(angle) + 20, 20 * sin(angle) + 20);
                    }
	    glEnd();
            glDisable(GL_TEXTURE_2D);
	glEndList();

}

void PathFinder::Move()
{
	double movingfactor = 28.5 * fabs(sin(degree_7 * walk_status) - sin(degree_7 * (walk_status - 1)));
	// length of leg * abs_value( sin(7_deg * walk_status) - sin(7_deg * (walk_status-1)))
	// movement of one frame of the animation

	if( rolling_status == ROLL_FACT ){
		switch(Dest) {
		case UP:
			current_y += movingfactor;//( walk_status >= 0 )? walk_status/3.0 : -walk_status/3.0;
			break;
		case DOWN:
			current_y -= movingfactor;//( walk_status >= 0 )? walk_status/3.0 : -walk_status/3.0;
			break;
		case LEFT:
			current_x -= movingfactor;//( walk_status >= 0 )? walk_status/3.0 : -walk_status/3.0;
			break;
		case RIGHT:
			current_x += movingfactor;//( walk_status >= 0 )? walk_status/3.0 : -walk_status/3.0;
			break;
		}
	}

	if(abs(old_x - current_x) >= 10.0) {
		current_x = old_x + ((Dest == RIGHT) ? 10 : -10);
		old_x = current_x;
		ismoving = false;
	}
	else if(abs(old_y - current_y) >= 10.0) {
		current_y = old_y + ((Dest == UP) ? 10 : -10);
		old_y = current_y;
		ismoving = false;
	}

	if(rolling_status == ROLL_FACT) {	// if it is rotating then do not move the leg, arm and eye
		walk_status++;
		if( walk_status > 5 )
			walk_status = -4;
		eye_status++;
		if( eye_status >= 30 )
			eye_status = 0;
	}
}

void PathFinder::Draw()
{
	glTranslatef( 30, 50, 0 );

	// draw body
	double rotateAngle = 0;
	switch (init_dest) {
	case LEFT:
		rotateAngle = 180.0;
		break;
	case UP:
		rotateAngle = 90.0;
		break;
	case DOWN:
		rotateAngle = -90.0;
		break;
	}
	if( rolling_status < ROLL_FACT ){
		switch (init_dest) {
		case LEFT:
			if( Dest == RIGHT )
				rotateAngle += 180.0/ROLL_FACT*rolling_status;
			else if( Dest == UP )
				rotateAngle += -90.0/ROLL_FACT*rolling_status;
			else if( Dest == DOWN )
				rotateAngle += 90.0/ROLL_FACT*rolling_status;
			break;
		case RIGHT:
			if( Dest == LEFT )
				rotateAngle += 180.0/ROLL_FACT*rolling_status;
			else if( Dest == UP )
				rotateAngle += 90.0/ROLL_FACT*rolling_status;
			else if( Dest == DOWN )
				rotateAngle += -90.0/ROLL_FACT*rolling_status;
			break;
		case UP:
			if( Dest == DOWN )
				rotateAngle += 180.0/ROLL_FACT*rolling_status;
			else if( Dest == LEFT )
				rotateAngle += 90.0/ROLL_FACT*rolling_status;
			else if( Dest == RIGHT )
				rotateAngle += -90.0/ROLL_FACT*rolling_status;
			break;
		case DOWN:
			if( Dest == LEFT )
				rotateAngle += -90.0/ROLL_FACT*rolling_status;
			else if( Dest == UP )
				rotateAngle += 180.0/ROLL_FACT*rolling_status;
			else if( Dest == RIGHT )
				rotateAngle += 90.0/ROLL_FACT*rolling_status;
			break;
		}
	}
	glTranslatef(20,15,0);
	glTranslatef(-20, -15, 0);
	glColor3f( bodyColorR, bodyColorG, bodyColorB );
	glCallList( Body );

	//glPushMatrix();

}

void PathFinder::UpdateStatus() {
	if (ismoving) Move();
	if (rolling_status < ROLL_FACT) {
		rolling_status++;
		if (init_dest == Dest) rolling_status = ROLL_FACT;
		if (rolling_status == ROLL_FACT) init_dest = Dest;
	}
	if(get_goal == true) goal_ceremony_status++;
}
