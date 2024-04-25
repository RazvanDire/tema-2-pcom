/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef DEBUG_HPP_
#define DEBUG_HPP_		1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

/* error printing macro */
#define ERR(call_description)				\
	do {						\
		fprintf(stderr, "(%s, %d): ",		\
			__FILE__, __LINE__);		\
		perror(call_description);		\
	} while (0)

/* print error (call ERR) and exit */
#define DIE(assertion, call_description)		\
	do {						\
		if (assertion) {			\
			ERR(call_description);		\
			exit(EXIT_FAILURE);		\
		}					\
	} while (0)

#ifdef __cplusplus
}
#endif

#endif

