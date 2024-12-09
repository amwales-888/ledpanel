
#define znew (z=36969*(z&65535)+(z>>16))
#define wnew (w=18000*(w&65535)+(w>>16))
#define MWC ((znew<<16)+wnew )

static unsigned long z=362436069; 
static unsigned long w=521288629;

int random(unsigned long count) {

	return MWC % count;
}



//1+MWC%10; would provide the proverbial "take a number from 1 to 10", (but with not quite, but virtually, equal probabilities)