#ifndef ALIEN_H_
#define ALIEN_H_

struct _alien{
	char symbol; //the symbol that represents the alien
	int xpos; //x position of the alien
	int ypos; //y position of the alien
};

typedef struct _alien alien;

alien constructAlien(char symbol, int xpos, int ypos);

#endif /* ALIEN_H_ */
