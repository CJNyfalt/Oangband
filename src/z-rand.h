/* File: z-rand.h */

/*
 * Copyright (c) 1997 Ben Harrison, and others
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */

#ifndef INCLUDED_Z_RAND_H
#define INCLUDED_Z_RAND_H

#include "h-basic.h"



/**** Available constants ****/


/*
 * The "degree" of the "complex" Random Number Generator.
 * This value is hard-coded at 63 for a wide variety of reasons.
 */
#define RAND_DEG 63




/**** Available macros ****/


/**
 * Generates a random signed long integer X where "0 <= X < M" holds.
 *
 * The integer X falls along a uniform distribution.
 */
#define randint0(M) ((s32b) Rand_div(M))


/**
 * Generates a random signed long integer X where "1 <= X <= M" holds.
 *
 * The integer X falls along a uniform distribution.
 */
#define randint1(M) ((s32b) Rand_div(M) + 1)

/**
 * Generate a random signed long integer X where "A - D <= X <= A + D" holds.
 * Note that "rand_spread(A, D)" == "rand_range(A - D, A + D)"
 *
 * The integer X falls along a uniform distribution.
 */
#define rand_spread(A, D) ((A) + (randint0(1 + (D) + (D))) - (D))

/**
 * Return TRUE one time in `x`.
 */
#define one_in_(x) (!randint0(x))


/**** Available Variables ****/

/**
 * Whether we are currently using the "quick" method or not.
 */
extern bool Rand_quick;

/**
 * The state used by the "quick" RNG.
 */
extern u32b Rand_value;

/**
 * The state used by the "complex" RNG.
 */
extern u16b Rand_place;
extern u32b Rand_state[RAND_DEG];


/**** Available Functions ****/

/**
 * Initialise the RNG state with the given seed.
 */
void Rand_state_init(u32b seed);

/**
 * Generates a random unsigned long integer X where "0 <= X < M" holds.
 *
 * The integer X falls along a uniform distribution.
 */
u32b Rand_div(u32b m);

/**
 * Generate a signed random integer within `stand` standard deviations of
 * `mean`, following a normal distribution.
 */
s16b Rand_normal(int mean, int stand);


/**
 * Emulate a number `num` of dice rolls of dice with `sides` sides.
 */
int damroll(int num, int sides);

/**
 * Generates a random signed long integer X where "A <= X <= B"
 * Note that "rand_range(0, N-1)" == "randint0(N)".
 *
 * The integer X falls along a uniform distribution.
 */
int rand_range(int A, int B);


#endif /* INCLUDED_Z_RAND_H */


